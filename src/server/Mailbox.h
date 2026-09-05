//////////////////////////////////////////////////////////////////////////////
// Filename    : Mailbox.h
// Description : A queue of items posted from any thread and consumed by one
//               owning thread at a point of its choosing. This is the
//               "cross-group communication via queues only" primitive of
//               the thread-ownership contract (CLAUDE.md, task 3.4 in
//               docs/RESTRUCTURING.md): a thread that must touch state it
//               does not own posts to the owner's box; the owner drains the
//               box while holding its own lock, so the work runs where the
//               mutation is legal. ZoneGroup owns one for group-level work
//               and GamePlayer owns one for per-player work (PlayerMailbox.h).
//
//               The box is unbounded on purpose. Its producers are the
//               inter-server links (SharedServerManager, LoginServerManager
//               threads); blocking them on gameplay back-pressure would stall
//               every guild/login message for every zone group. The owner
//               watches the depth instead (size(), or the batch size drain()
//               returns) and logs when it is out of the ordinary.
//////////////////////////////////////////////////////////////////////////////

#ifndef DARKEDEN_MAILBOX_H
#define DARKEDEN_MAILBOX_H

#include <cstddef>
#include <functional>
#include <mutex>
#include <utility>
#include <vector>

namespace de {

template <typename Item> class Mailbox {
public:
    // Thread-safe. Never blocks on anything but the box's own mutex, which
    // is held only for the push, so a producer that already holds other
    // locks (the PCFinder lock, a group mutex) cannot deadlock here.
    void post(Item item) {
        std::lock_guard lock(m_Mutex);
        m_Pending.push_back(std::move(item));
    }

    // Takes every item that was posted before the call out of the box and
    // hands each to `visit`, in posting order, on the calling thread. Items
    // posted while draining (including by the visited items themselves)
    // wait for the next drain. `onFailure` is called with the exception in
    // flight when a visit throws, and draining continues with the next
    // item; without it the exception propagates and the rest of this batch
    // is dropped. Returns the number of items taken.
    template <typename Visit, typename OnFailure> std::size_t drain(Visit&& visit, OnFailure&& onFailure) {
        std::vector<Item> batch = takeBatch();
        for (Item& item : batch) {
            try {
                visit(item);
            } catch (...) {
                onFailure();
            }
        }
        return batch.size();
    }

    template <typename Visit> std::size_t drain(Visit&& visit) {
        return drain(std::forward<Visit>(visit), [] { throw; });
    }

    std::size_t size() const {
        std::lock_guard lock(m_Mutex);
        return m_Pending.size();
    }

private:
    std::vector<Item> takeBatch() {
        std::vector<Item> batch;
        std::lock_guard lock(m_Mutex);
        batch.swap(m_Pending);
        m_Pending.reserve(batch.size()); // keep the steady-state capacity
        return batch;
    }

    mutable std::mutex m_Mutex;
    std::vector<Item> m_Pending;
};

// The plain form: a queue of thunks the owner just runs.
using CommandMailbox = Mailbox<std::function<void()>>;

} // namespace de

#endif // DARKEDEN_MAILBOX_H
