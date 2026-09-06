//////////////////////////////////////////////////////////////////////////////
// Filename    : Snapshot.h
// Description : A value that many threads read constantly and one thread
//               occasionally replaces -- the zone map of a ZoneGroup, the
//               ZoneInfo tables -- published copy-on-write behind a
//               std::shared_ptr<const T>.
//
//               Readers call load() and get an immutable snapshot they may
//               keep for as long as they like (an iteration, a tick); it is
//               never mutated under them and it stays alive until the last
//               holder lets go. Writers call update(): it copies the current
//               value, lets the caller change the copy, and publishes it
//               with one pointer swap, so a reader sees either the old table
//               or the new one, never a table mid-rehash. Writers are
//               serialised by their own mutex. The pointer slot itself is
//               guarded by a leaf mutex held only for the copy of a
//               shared_ptr (std::atomic<std::shared_ptr> is not in the
//               pinned libc++ 21); a reader is therefore never blocked on a
//               writer's work, only on another pointer copy, and no user
//               code runs under that mutex, so no lock order is created and
//               no deadlock is possible from a zone thread that holds its
//               own group mutex.
//
//               This trades the cost of a full copy per update for reads
//               that cost one uncontended mutex and one refcount. It fits
//               tables that change a handful of times per hour (a dynamic
//               zone being created) and are read thousands of times per
//               second; it does not fit anything updated per tick.
//////////////////////////////////////////////////////////////////////////////

#ifndef DARKEDEN_SNAPSHOT_H
#define DARKEDEN_SNAPSHOT_H

#include <memory>
#include <mutex>
#include <utility>

#include <type_traits>

namespace de {

template <typename T> class Snapshot {
public:
    using Value = std::shared_ptr<const T>;

    Snapshot() : m_Current(std::make_shared<const T>()) {}
    explicit Snapshot(T initial) : m_Current(std::make_shared<const T>(std::move(initial))) {}

    Snapshot(const Snapshot&) = delete;
    Snapshot& operator=(const Snapshot&) = delete;

    // The current value. Never null.
    Value load() const {
        std::lock_guard lock(m_SlotMutex);
        return m_Current;
    }

    // Copies the current value, applies `change` to the copy, publishes the
    // result. Writers are serialised; a writer never blocks a reader beyond
    // the pointer swap. `change` may throw, in which case nothing is
    // published. Returns whatever `change` returns.
    //
    // `change` runs with the writer mutex held, so it must be pure work on
    // the copy: no lock it takes may be one that is ever held while calling
    // update() on this Snapshot, and it must not call update() on this
    // Snapshot itself (the mutex is not recursive). load() is fine.
    template <typename Change> decltype(auto) update(Change&& change) {
        std::lock_guard lock(m_WriterMutex);
        // Copy from the published value, not from a cached one: a previous
        // writer's result is what the next writer must build on.
        auto next = std::make_shared<T>(*load());
        if constexpr (std::is_void_v<decltype(change(*next))>) {
            change(*next);
            publish(std::move(next));
        } else {
            auto result = change(*next);
            publish(std::move(next));
            return result;
        }
    }

private:
    void publish(Value next) {
        std::lock_guard lock(m_SlotMutex);
        m_Current = std::move(next);
    }

    mutable std::mutex m_SlotMutex; // leaf: held only to copy or swap the pointer
    Value m_Current;
    std::mutex m_WriterMutex;
};

} // namespace de

#endif // DARKEDEN_SNAPSHOT_H
