#ifndef DARKEDEN_SERVER_SHUTDOWN_H
#define DARKEDEN_SERVER_SHUTDOWN_H

#include <unistd.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <string>
#include <thread>

#include <string_view>

namespace ServerShutdown {
// Lock-free atomic access is signal-safe and also safe when a signal is
// delivered to a worker instead of the main thread.
static_assert(std::atomic<bool>::is_always_lock_free);
inline std::atomic<bool> requested{false};
inline std::atomic<bool> failed{false};

inline void request(int = 0) noexcept {
    requested.store(true);
}
inline bool isRequested() noexcept {
    return requested.load();
}
inline void fail() noexcept {
    failed.store(true);
    request();
}

// Covers blocked startup, worker joins, and dependency destruction. Never
// detach a worker and free its state to satisfy a shutdown deadline.
class Deadline {
public:
    // The message is built by the init-capture below, on the constructing
    // thread, so the watcher only writes bytes that already exist when it
    // force-exits the process.
    explicit Deadline(std::chrono::milliseconds timeout = std::chrono::seconds(30), std::string_view process = "server")
        : m_Watcher([timeout, message = std::string(process) +
                                        " shutdown deadline exceeded; forcing process exit\n"](std::stop_token token) {
              while (!token.stop_requested() && !isRequested())
                  std::this_thread::sleep_for(std::chrono::milliseconds(10));
              const auto end = std::chrono::steady_clock::now() + timeout;
              while (!token.stop_requested()) {
                  if (std::chrono::steady_clock::now() >= end) {
                      (void)::write(STDERR_FILENO, message.data(), message.size());
                      std::_Exit(EXIT_FAILURE);
                  }
                  std::this_thread::sleep_for(std::chrono::milliseconds(10));
              }
          }) {}

private:
    std::jthread m_Watcher;
};
} // namespace ServerShutdown

#endif
