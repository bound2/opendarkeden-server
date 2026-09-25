#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "Socket.h"

class Properties;

namespace de {

// A private listener for a gateway in the same network namespace. The public
// TCP listener never interprets forwarded identities. poll() does bounded,
// nonblocking work on the connection manager's thread before player admission.
//
// Each connection must open with a PROXY protocol v1 line naming the client:
// `PROXY TCP4 <client ip> 127.0.0.1 <client port> <this listener's port>\r\n`.
// A connection that does not send one within five seconds, or sends anything
// else, is closed.
class ProxyAcceptor {
public:
    using Clock = std::chrono::steady_clock;

    // The acceptor a server's GatewayProxyPort setting asks for, or null when
    // the setting is absent. Throws Error on a port outside 1..65535 or one
    // that cannot be bound.
    static std::unique_ptr<ProxyAcceptor> fromConfig(const Properties& config);

    // Listens on 127.0.0.1:port; port 0 picks a free one (see port()).
    explicit ProxyAcceptor(unsigned short port);
    ~ProxyAcceptor();
    ProxyAcceptor(const ProxyAcceptor&) = delete;
    ProxyAcceptor& operator=(const ProxyAcceptor&) = delete;

    // Accepts waiting connections, reads what has arrived of their headers,
    // and returns the connections whose header is complete and valid. Each
    // returned socket reports the client's address through getHost(),
    // getHostIP() and getPort(), is nonblocking, and holds exactly the bytes
    // the client sent after the header.
    std::vector<std::unique_ptr<Socket>> poll(Clock::time_point now = Clock::now());
    unsigned short port() const {
        return m_Port;
    }
    size_t pendingCount() const {
        return m_Pending.size();
    }

private:
    struct Pending {
        int fd;
        std::string header;
        Clock::time_point deadline;
    };
    int m_Listener = -1;
    unsigned short m_Port = 0;
    std::vector<Pending> m_Pending;
};

} // namespace de
