#ifndef DARKEDEN_COOPERATIVE_THREAD_H
#define DARKEDEN_COOPERATIVE_THREAD_H

#include <exception>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

#include <condition_variable>
#include <system_error>

// Owns a C++20 cooperative worker. Destruction requests cancellation and
// joins, so a worker cannot outlive the object whose state it uses.
class CooperativeThread {
public:
    CooperativeThread() = default;
    ~CooperativeThread() {
        requestStop();
        join();
    }

    CooperativeThread(const CooperativeThread&) = delete;
    CooperativeThread& operator=(const CooperativeThread&) = delete;
    CooperativeThread(CooperativeThread&&) = delete;
    CooperativeThread& operator=(CooperativeThread&&) = delete;

    template <typename Function> void start(Function&& function) {
        std::lock_guard lock(m_Mutex);
        if (m_Started || m_Stopped)
            throw std::logic_error("cooperative thread is already started or stopped");
        m_Thread = std::jthread([this, fn = std::forward<Function>(function)](std::stop_token token) mutable {
            try {
                fn(token);
            } catch (...) {
                std::lock_guard failureLock(m_Mutex);
                m_Failure = std::current_exception();
            }
        });
        m_StopSource = m_Thread.get_stop_source();
        m_ThreadID = m_Thread.get_id();
        m_Started = true;
    }

    bool requestStop() noexcept {
        std::stop_source source{std::nostopstate};
        {
            std::lock_guard lock(m_Mutex);
            m_Stopped = true;
            source = m_StopSource;
        }
        // Stop callbacks may run synchronously; never invoke them under our lock.
        return source.request_stop();
    }

    void join() {
        std::unique_lock lock(m_Mutex);
        if ((m_Thread.joinable() || m_Joining) && m_ThreadID == std::this_thread::get_id())
            throw std::system_error(std::make_error_code(std::errc::resource_deadlock_would_occur));
        m_Stopped = true;
        m_Joined.wait(lock, [this] { return !m_Joining; });
        if (!m_Thread.joinable())
            return;
        m_Joining = true;
        std::jthread thread = std::move(m_Thread);
        lock.unlock();
        try {
            thread.join();
        } catch (...) {
            lock.lock();
            m_Thread = std::move(thread);
            m_Joining = false;
            m_Joined.notify_all();
            throw;
        }
        lock.lock();
        m_Joining = false;
        m_Joined.notify_all();
    }

    bool joinable() const noexcept {
        std::lock_guard lock(m_Mutex);
        return m_Thread.joinable() || m_Joining;
    }

    std::jthread::native_handle_type nativeHandle() {
        std::lock_guard lock(m_Mutex);
        return m_Thread.native_handle();
    }

    void rethrowFailure() const {
        std::lock_guard lock(m_Mutex);
        if (m_Failure)
            std::rethrow_exception(m_Failure);
    }

private:
    mutable std::mutex m_Mutex;
    std::condition_variable m_Joined;
    std::stop_source m_StopSource{std::nostopstate};
    std::exception_ptr m_Failure;
    std::thread::id m_ThreadID;
    bool m_Started = false;
    bool m_Stopped = false;
    bool m_Joining = false;
    std::jthread m_Thread;
};

#endif
