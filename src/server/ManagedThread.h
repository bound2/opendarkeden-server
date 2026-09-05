#ifndef DARKEDEN_MANAGED_THREAD_H
#define DARKEDEN_MANAGED_THREAD_H

#include <mutex>

#include <condition_variable>

#include "CooperativeThread.h"
#include "ServerShutdown.h"
#include "Thread.h"

// Opt-in backend for loops that observe stopRequested(). The owning manager
// must join before destroying dependencies; each derived destructor must also
// stop/join BEFORE destroying its own members. A base destructor is too late.
class ManagedThread : public Thread {
public:
    void start() override {
        std::lock_guard lock(m_Lifecycle);
        if (getStatus() != READY || m_JoinRequested)
            throw ThreadException("invalid thread status");
        m_Worker.start([this](std::stop_token token) {
            m_Token = token; // Read only by this worker in run()/pauseFor().
            try {
                if (!stopRequested())
                    run();
                if (!stopRequested())
                    throw std::runtime_error("managed worker exited unexpectedly");
            } catch (...) {
                ServerShutdown::fail();
                std::lock_guard finished(m_Lifecycle);
                setStatus(EXIT);
                throw; // CooperativeThread retains the failure for the owner.
            }
            std::lock_guard finished(m_Lifecycle);
            setStatus(EXIT);
        });
        setTID(m_Worker.nativeHandle());
        setStatus(RUNNING);
    }

    void stop() override {
        std::lock_guard lock(m_Lifecycle);
        m_Worker.requestStop();
        if (getStatus() != EXIT)
            setStatus(m_Worker.joinable() ? EXITING : EXIT);
    }

    void join() override {
        // Serializes against start while still allowing stop during a join.
        {
            std::lock_guard lock(m_Lifecycle);
            m_JoinRequested = true;
        }
        m_Worker.join();
        setStatus(EXIT);
    }

    void detach() override {
        throw UnsupportedError("managed worker must remain owned");
    }
    void rethrowFailure() const {
        m_Worker.rethrowFailure();
    }

protected:
    bool stopRequested() const noexcept {
        return m_Token.stop_requested() || ServerShutdown::isRequested();
    }

    bool pauseFor(std::chrono::microseconds delay) {
        std::unique_lock lock(m_WaitMutex);
        m_Wake.wait_for(lock, m_Token, delay, [] { return ServerShutdown::isRequested(); });
        return !stopRequested();
    }

private:
    std::mutex m_Lifecycle;
    bool m_JoinRequested = false;
    std::mutex m_WaitMutex;
    std::condition_variable_any m_Wake;
    std::stop_token m_Token;
    CooperativeThread m_Worker;
};

#endif
