#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "Socket.h"

namespace de {

// A private listener for a gateway in the same network namespace. The public
// TCP listener never interprets forwarded identities. poll() does bounded,
// nonblocking work on the connection manager's thread before player admission.
class ProxyAcceptor {
public:
    using Clock = std::chrono::steady_clock;
    explicit ProxyAcceptor(unsigned short port);
    ~ProxyAcceptor();
    ProxyAcceptor(const ProxyAcceptor&) = delete;
    ProxyAcceptor& operator=(const ProxyAcceptor&) = delete;

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
