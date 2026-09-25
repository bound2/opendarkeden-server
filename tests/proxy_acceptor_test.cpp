#include <unistd.h>

#include <chrono>
#include <string>

#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <sys/socket.h>

#include "Exception.h"
#include "Properties.h"
#include "ProxyAcceptor.h"
#include "Socket.h"

namespace {
class Peer {
public:
    explicit Peer(unsigned short port) {
        fd = ::socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = htons(port);
        EXPECT_EQ(0, ::connect(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)));
    }
    ~Peer() {
        ::close(fd);
    }
    void send(const std::string& bytes) {
        ASSERT_EQ(bytes.size(), static_cast<size_t>(::send(fd, bytes.data(), bytes.size(), MSG_NOSIGNAL)));
    }
    // True once the acceptor has closed this connection; false after a
    // second of waiting, or if any byte arrives instead.
    bool sawEof() {
        for (int attempt = 0; attempt < 200; ++attempt) {
            char byte;
            const auto received = ::recv(fd, &byte, 1, MSG_DONTWAIT);
            if (received == 0)
                return true;
            if (received > 0 || (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR))
                return false;
            ::usleep(5000);
        }
        return false;
    }
    int fd;
};

std::string header(unsigned short port, const char* ip = "203.0.113.21") {
    return std::string("PROXY TCP4 ") + ip + " 127.0.0.1 32123 " + std::to_string(port) + "\r\n";
}
} // namespace

TEST(ProxyAcceptor, FragmentedHeaderPreservesPeerIdentityAndPacketBytes) {
    de::ProxyAcceptor acceptor(0);
    Peer peer(acceptor.port());
    const auto line = header(acceptor.port());
    peer.send(line.substr(0, 12));
    EXPECT_TRUE(acceptor.poll().empty());
    peer.send(line.substr(12) + std::string("\0\1\2\xff", 4));
    auto accepted = acceptor.poll();
    ASSERT_EQ(1u, accepted.size());
    EXPECT_EQ("203.0.113.21", accepted[0]->getHost());
    EXPECT_EQ(32123u, accepted[0]->getPort());
    EXPECT_EQ(inet_addr("203.0.113.21"), accepted[0]->getHostIP());
    // The managers assume the nonblocking mode they set on a public socket.
    EXPECT_TRUE(accepted[0]->isNonBlocking());
    char bytes[4]{};
    ASSERT_EQ(4u, accepted[0]->receive(bytes, sizeof(bytes)));
    EXPECT_EQ(std::string("\0\1\2\xff", 4), std::string(bytes, sizeof(bytes)));
    ASSERT_EQ(4u, accepted[0]->send(bytes, sizeof(bytes)));
    char response[4]{};
    ASSERT_EQ(4, ::recv(peer.fd, response, sizeof(response), 0));
    EXPECT_EQ(std::string(bytes, 4), std::string(response, 4));
}

TEST(ProxyAcceptor, IndependentPlayersKeepDifferentAddresses) {
    de::ProxyAcceptor acceptor(0);
    Peer first(acceptor.port()), second(acceptor.port());
    first.send(header(acceptor.port(), "198.51.100.1"));
    second.send(header(acceptor.port(), "198.51.100.2"));
    auto accepted = acceptor.poll();
    ASSERT_EQ(2u, accepted.size());
    EXPECT_NE(accepted[0]->getHost(), accepted[1]->getHost());
}

TEST(ProxyAcceptor, RejectsInvalidAndOverlongHeaders) {
    de::ProxyAcceptor acceptor(0);
    // Every line but the one under test is valid, so each is refused for its
    // own reason.
    const std::string port = std::to_string(acceptor.port());
    const std::string otherPort = std::to_string(acceptor.port() == 65535 ? 1 : acceptor.port() + 1);
    const std::string invalid[] = {
        "PROXY UNKNOWN\r\n",
        "PROXY TCP6 ::1 ::1 1 " + port + "\r\n",
        "PROXY TCP4 999.0.0.1 127.0.0.1 1 " + port + "\r\n",
        "PROXY TCP4 1.2.3 127.0.0.1 1 " + port + "\r\n",
        // inet_pton stops at the NUL, which must not hide the bytes after it.
        "PROXY TCP4 203.0.113.21" + std::string(1, '\0') + "9 127.0.0.1 1 " + port + "\r\n",
        "PROXY TCP4 1.2.3.4 127.0.0.2 1 " + port + "\r\n",
        "PROXY TCP4 1.2.3.4 127.0.0.1 1 " + otherPort + "\r\n",
        "PROXY TCP4 1.2.3.4 127.0.0.1 0 " + port + "\r\n",
        "PROXY TCP4 1.2.3.4 127.0.0.1 65536 " + port + "\r\n",
        "PROXY TCP4 1.2.3.4 127.0.0.1 1 " + port + " extra\r\n",
        "PROXY TCP4  1.2.3.4 127.0.0.1 1 " + port + "\r\n",
        "PROXY TCP4 1.2.3.4 127.0.0.1 1 " + port + "\n",
        std::string(108, 'x'),
    };
    for (const auto& line : invalid) {
        SCOPED_TRACE(line);
        Peer peer(acceptor.port());
        peer.send(line);
        EXPECT_TRUE(acceptor.poll().empty());
        EXPECT_EQ(0u, acceptor.pendingCount());
        // The refusal must be visible to the peer as a closed connection;
        // an empty poll alone would also be true of a connection not yet
        // accepted.
        EXPECT_TRUE(peer.sawEof());
    }
    Peer valid(acceptor.port());
    valid.send("PROXY TCP4 1.2.3.4 127.0.0.1 1 " + port + "\r\n");
    EXPECT_EQ(1u, acceptor.poll().size());
}

TEST(ProxyAcceptor, SlowHeaderTimesOutWithoutHoldingUpAnotherPlayer) {
    de::ProxyAcceptor acceptor(0);
    Peer slow(acceptor.port()), fast(acceptor.port());
    slow.send("PROXY ");
    fast.send(header(acceptor.port()));
    const auto now = de::ProxyAcceptor::Clock::now();
    EXPECT_EQ(1u, acceptor.poll(now).size());
    EXPECT_EQ(1u, acceptor.pendingCount());
    EXPECT_TRUE(acceptor.poll(now + std::chrono::seconds(6)).empty());
    EXPECT_EQ(0u, acceptor.pendingCount());
}

TEST(ProxyAcceptor, ClosedPartialHeaderIsDiscarded) {
    de::ProxyAcceptor acceptor(0);
    {
        Peer peer(acceptor.port());
        peer.send("PROXY TCP4 ");
        EXPECT_TRUE(acceptor.poll().empty());
        EXPECT_EQ(1u, acceptor.pendingCount());
    }
    EXPECT_TRUE(acceptor.poll().empty());
    EXPECT_EQ(0u, acceptor.pendingCount());
}

TEST(ProxyAcceptor, ConfigurationNamesThePortOrLeavesTheGatewayOff) {
    Properties config;
    EXPECT_EQ(nullptr, de::ProxyAcceptor::fromConfig(config));

    // atoi would have read the last four as 19099, 1, 19099 and 1.
    for (const char* port : {"0", "-1", "65536", "none", "", "19099abc", "1e3", "4294986395", "4294967297"}) {
        SCOPED_TRACE(port);
        config.setProperty("GatewayProxyPort", port);
        EXPECT_THROW(de::ProxyAcceptor::fromConfig(config), Error);
    }

    unsigned short freePort = 0;
    {
        de::ProxyAcceptor probe(0);
        freePort = probe.port();
    }
    // Surrounding blanks and a CRLF file's trailing \r are not part of the number.
    for (const std::string& spelling : {std::to_string(freePort), " " + std::to_string(freePort) + "\t\r"}) {
        SCOPED_TRACE(spelling);
        config.setProperty("GatewayProxyPort", spelling);
        const auto acceptor = de::ProxyAcceptor::fromConfig(config);
        ASSERT_NE(nullptr, acceptor);
        EXPECT_EQ(freePort, acceptor->port());
    }
}
