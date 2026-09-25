#include "ProxyAcceptor.h"

#include <fcntl.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <charconv>
#include <cstring>

#include <arpa/inet.h>
#include <string_view>
#include <sys/socket.h>

#include "Properties.h"

namespace de {
namespace {
// Connections still waiting for their header; later ones are refused.
constexpr size_t MaxPending = 128;
// A PROXY v1 line is at most 107 bytes, so a longer one is never valid.
constexpr size_t MaxHeader = 108;

bool setNonBlockingCloseOnExec(int fd) {
    const int flags = ::fcntl(fd, F_GETFL, 0);
    return flags >= 0 && ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0 && ::fcntl(fd, F_SETFD, FD_CLOEXEC) == 0;
}

bool parsePort(std::string_view text, unsigned short& port) {
    unsigned int value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || value == 0 || value > 65535)
        return false;
    port = static_cast<unsigned short>(value);
    return true;
}

bool parseHeader(const std::string& header, unsigned short localPort, sockaddr_in& peer) {
    // inet_pton reads a C string, so an embedded NUL would end an address
    // early and let trailing bytes through unchecked.
    if (!header.ends_with("\r\n") || header.find('\0') != std::string::npos)
        return false;
    std::string_view rest(header.data(), header.size() - 2);
    std::array<std::string_view, 6> fields;
    for (size_t i = 0; i < fields.size(); ++i) {
        const size_t end = rest.find(' ');
        if ((i == fields.size() - 1) != (end == std::string_view::npos))
            return false;
        fields[i] = rest.substr(0, end);
        if (fields[i].empty())
            return false;
        if (end != std::string_view::npos)
            rest.remove_prefix(end + 1);
    }
    if (fields[0] != "PROXY" || fields[1] != "TCP4")
        return false;
    in_addr destination{};
    if (::inet_pton(AF_INET, std::string(fields[2]).c_str(), &peer.sin_addr) != 1 ||
        ::inet_pton(AF_INET, std::string(fields[3]).c_str(), &destination) != 1 ||
        destination.s_addr != htonl(INADDR_LOOPBACK))
        return false;
    unsigned short sourcePort = 0, destinationPort = 0;
    if (!parsePort(fields[4], sourcePort) || !parsePort(fields[5], destinationPort) || destinationPort != localPort)
        return false;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(sourcePort);
    return true;
}

class ForwardedSocket : public SocketImpl {
public:
    ForwardedSocket(int fd, const sockaddr_in& peer) {
        m_SockAddr = peer;
        char address[INET_ADDRSTRLEN]{};
        ::inet_ntop(AF_INET, &peer.sin_addr, address, sizeof(address));
        m_Host = address;
        m_Port = ntohs(peer.sin_port);
        // Assign ownership after every operation that can throw.
        m_SocketID = fd;
    }
};
} // namespace

std::unique_ptr<ProxyAcceptor> ProxyAcceptor::fromConfig(const Properties& config) {
    if (!config.hasKey("GatewayProxyPort"))
        return nullptr;
    const int port = config.getPropertyInt("GatewayProxyPort");
    if (port < 1 || port > 65535)
        throw Error("GatewayProxyPort must be between 1 and 65535");
    return std::make_unique<ProxyAcceptor>(static_cast<unsigned short>(port));
}

ProxyAcceptor::ProxyAcceptor(unsigned short port) {
    m_Pending.reserve(MaxPending);
    m_Listener = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_Listener < 0)
        throw Error("cannot create gateway listener");
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port);
    const int reuse = 1;
    socklen_t length = sizeof(address);
    if (!setNonBlockingCloseOnExec(m_Listener) ||
        ::setsockopt(m_Listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0 ||
        ::bind(m_Listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 ||
        ::listen(m_Listener, 64) != 0 ||
        ::getsockname(m_Listener, reinterpret_cast<sockaddr*>(&address), &length) != 0) {
        const std::string reason = std::strerror(errno);
        ::close(m_Listener);
        m_Listener = -1;
        throw Error("cannot listen on 127.0.0.1:" + std::to_string(port) + " for the gateway: " + reason);
    }
    m_Port = ntohs(address.sin_port);
}

ProxyAcceptor::~ProxyAcceptor() {
    for (const auto& pending : m_Pending)
        ::close(pending.fd);
    if (m_Listener >= 0)
        ::close(m_Listener);
}

std::vector<std::unique_ptr<Socket>> ProxyAcceptor::poll(Clock::time_point now) {
    // Called every tick and nearly always empty, so nothing is reserved.
    std::vector<std::unique_ptr<Socket>> ready;
    for (int count = 0; count < 32; ++count) {
        sockaddr_in address{};
        socklen_t length = sizeof(address);
        const int fd = ::accept(m_Listener, reinterpret_cast<sockaddr*>(&address), &length);
        if (fd < 0)
            break;
        if (address.sin_addr.s_addr != htonl(INADDR_LOOPBACK) || m_Pending.size() == MaxPending ||
            !setNonBlockingCloseOnExec(fd)) {
            ::close(fd);
            continue;
        }
        m_Pending.push_back({fd, {}, now + std::chrono::seconds(5)});
    }

    for (auto it = m_Pending.begin(); it != m_Pending.end();) {
        bool discard = now >= it->deadline;
        // One byte at a time, so the client's first packet bytes after the
        // header stay in the socket for the player's own input stream.
        while (!discard && it->header.size() < MaxHeader) {
            char byte;
            const auto received = ::recv(it->fd, &byte, 1, MSG_DONTWAIT);
            if (received < 0 && errno == EINTR)
                continue;
            if (received < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
                break;
            if (received <= 0) {
                discard = true;
                break;
            }
            it->header += byte;
            if (byte == '\n') {
                sockaddr_in peer{};
                if (parseHeader(it->header, m_Port, peer)) {
                    auto impl = std::make_unique<ForwardedSocket>(it->fd, peer);
                    it->fd = -1;
                    auto socket = std::make_unique<Socket>(impl.get());
                    impl.release();
                    ready.push_back(std::move(socket));
                }
                discard = true;
                break;
            }
        }
        if (discard || it->header.size() == MaxHeader) {
            if (it->fd >= 0)
                ::close(it->fd);
            it = m_Pending.erase(it);
        } else {
            ++it;
        }
    }
    return ready;
}

} // namespace de
