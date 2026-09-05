//////////////////////////////////////////////////////////////////////////////
// Filename    : Mailbox.h
// Description : A queue of commands posted from any thread and run by one
//               owning thread at a point of its choosing. This is the
//               "cross-group communication via queues only" primitive of
//               the thread-ownership contract (CLAUDE.md, task 3.4 in
//               docs/RESTRUCTURING.md): a thread that must touch state it
//               does not own posts a command; the owner drains the box
//               while holding its own lock, so the command runs where the
//               mutation is legal.
//
//               The box is unbounded on purpose. Its producers are the
//               inter-server links (SharedServerManager, LoginServerManager
//               threads); blocking them on gameplay back-pressure would stall
//               every guild/login message for every zone group. size() lets
//               the owner watch the depth instead.
//////////////////////////////////////////////////////////////////////////////

#ifndef DARKEDEN_MAILBOX_H
#define DARKEDEN_MAILBOX_H

#include <cstddef>
#include <functional>
#include <mutex>
#include <utility>
#include <vector>

namespace de {

class Mailbox {
public:
    using Command = std::function<void()>;

    // Thread-safe. Never blocks on anything but the box's own mutex, which
    // is held only for the push, so a producer that already holds other
    // locks (the PCFinder lock, another group's mutex) cannot deadlock here.
    void post(Command command) {
        std::lock_guard lock(m_Mutex);
        m_Pending.push_back(std::move(command));
    }

    // Runs every command that was posted before the call, in posting order,
    // on the calling thread. Commands posted while draining (including by
    // the commands themselves) wait for the next drain. `onFailure` is
    // called with the exception in flight when a command throws, and
    // draining continues with the next command; without it an exception
    // propagates and the remaining commands of this batch are dropped.
    // Returns the number of commands run.
    template <typename OnFailure> std::size_t drain(OnFailure&& onFailure) {
        std::vector<Command> batch;
        {
            std::lock_guard lock(m_Mutex);
            batch.swap(m_Pending);
        }
        for (Command& command : batch) {
            try {
                command();
            } catch (...) {
                onFailure();
            }
        }
        return batch.size();
    }

    std::size_t drain() {
        return drain([] { throw; });
    }

    std::size_t size() const {
        std::lock_guard lock(m_Mutex);
        return m_Pending.size();
    }

private:
    mutable std::mutex m_Mutex;
    std::vector<Command> m_Pending;
};

} // namespace de

#endif // DARKEDEN_MAILBOX_H
