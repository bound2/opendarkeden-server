#ifndef DARKEDEN_COOPERATIVE_THREAD_H
#define DARKEDEN_COOPERATIVE_THREAD_H

#include <stdexcept>
#include <thread>
#include <utility>

// Owns a C++20 cooperative worker. Destruction requests cancellation and
// joins, so a worker cannot outlive the object whose state it uses.
class CooperativeThread {
public:
    CooperativeThread() = default;
    ~CooperativeThread() = default;

    CooperativeThread(const CooperativeThread&) = delete;
    CooperativeThread& operator=(const CooperativeThread&) = delete;
    CooperativeThread(CooperativeThread&&) = delete;
    CooperativeThread& operator=(CooperativeThread&&) = delete;

    template <typename Function> void start(Function&& function) {
        if (m_Thread.joinable())
            throw std::logic_error("cooperative thread is already active");

        m_Thread = std::jthread(std::forward<Function>(function));
    }

    bool requestStop() noexcept {
        return m_Thread.request_stop();
    }

    void join() {
        if (m_Thread.joinable())
            m_Thread.join();
    }

    bool joinable() const noexcept {
        return m_Thread.joinable();
    }

    std::jthread::native_handle_type nativeHandle() {
        return m_Thread.native_handle();
    }

private:
    std::jthread m_Thread;
};

#endif
