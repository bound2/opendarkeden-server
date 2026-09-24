#include <unistd.h>

#include <chrono>
#include <string>

#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <sys/socket.h>

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
    for (const auto& line : {std::string("PROXY UNKNOWN\r\n"), std::string("PROXY TCP6 ::1 ::1 1 2\r\n"),
                             std::string("PROXY TCP4 999.0.0.1 127.0.0.1 1 2\r\n"),
                             std::string("PROXY TCP4 1.2.3.4 127.0.0.1 65536 2\r\n"),
                             std::string("PROXY TCP4 1.2.3.4 127.0.0.1 1 2 extra\r\n"), std::string(108, 'x')}) {
        Peer peer(acceptor.port());
        peer.send(line);
        EXPECT_TRUE(acceptor.poll().empty());
        EXPECT_EQ(0u, acceptor.pendingCount());
    }
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
