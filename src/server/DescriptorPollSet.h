//--------------------------------------------------------------------------------
//
// Filename    : DescriptorPollSet.h
// Description : The readiness set the connection managers multiplex their
//               descriptor-indexed player tables with.
//
//--------------------------------------------------------------------------------

#ifndef __DESCRIPTOR_POLL_SET_H__
#define __DESCRIPTOR_POLL_SET_H__

#include <poll.h>

#include <cstddef>
#include <vector>

namespace de {

//--------------------------------------------------------------------------------
//
// The connection managers keep their players in a fixed array indexed by
// socket descriptor and ask the kernel, once a tick, which of those
// descriptors are ready. A DescriptorPollSet is that question and its answer,
// held per descriptor so a manager can keep asking it the way it walks its
// table: by index.
//
// The set has the same number of slots as the table it serves, so a
// descriptor the table cannot hold is one the set does not watch either --
// the managers refuse such a connection before they reach here.
//
// A tick is three steps, kept apart so the wait can run without whatever lock
// guards the manager's membership:
//
//   fill()    builds one pollfd per watched descriptor and drops the previous
//             tick's results, so nothing is reported ready until new results
//             arrive.
//   wait(ms)  hands that array to poll() for at most `ms` milliseconds and
//             records whether the call failed.
//   collect() moves the results into the per-descriptor table.
//
// pollOnce() is the three in a row, for a manager that holds one lock across
// the whole call.
//
// A descriptor whose membership changed between fill() and collect() keeps no
// result: the answer was about the connection that has since gone, not about
// whichever one now holds the descriptor.
//
// A failed wait leaves every descriptor unready, so the manager processes
// nothing this tick and asks again on the next one.
//
//--------------------------------------------------------------------------------

class DescriptorPollSet {
public:
    // What a manager asks for. The read and write interests are the two
    // directions a manager drives; the urgent interest is out-of-band data,
    // which the managers treat as an error and cut the connection for.
    static constexpr short kRead = POLLIN;
    static constexpr short kWrite = POLLOUT;
    static constexpr short kUrgent = POLLPRI;

    // What the kernel reports without being asked: a pending socket error, a
    // peer that hung up, and a descriptor that is not open at all. Each makes
    // the descriptor ready in every direction the manager asked about, so the
    // manager's own read or write runs and fails, and its existing disconnect
    // path takes the connection down.
    static constexpr short kFailure = POLLERR | POLLHUP | POLLNVAL;

    explicit DescriptorPollSet(int tableSize)
        : m_Events(tableSize > 0 ? static_cast<size_t>(tableSize) : 0, 0),
          m_Revents(tableSize > 0 ? static_cast<size_t>(tableSize) : 0, 0),
          m_Generation(tableSize > 0 ? static_cast<size_t>(tableSize) : 0, 0) {}

    int tableSize() const {
        return static_cast<int>(m_Events.size());
    }

    // True for the descriptors this set has a slot for.
    bool holds(int fd) const {
        return fd >= 0 && fd < tableSize();
    }

    // Start watching a descriptor for the given interests. A descriptor the
    // set has no slot for is ignored. The descriptor is not ready until the
    // next wait answers for it.
    void watch(int fd, short events) {
        if (!holds(fd))
            return;

        m_Events[fd] = events;
        m_Revents[fd] = 0;
        m_Generation[fd]++;
    }

    // Stop watching a descriptor and drop any result already collected for
    // it, so a connection removed part way through a tick is not processed by
    // the steps that follow.
    void unwatch(int fd) {
        if (!holds(fd))
            return;

        m_Events[fd] = 0;
        m_Revents[fd] = 0;
        m_Generation[fd]++;
    }

    bool watched(int fd) const {
        return holds(fd) && m_Events[fd] != 0;
    }

    short events(int fd) const {
        return holds(fd) ? m_Events[fd] : static_cast<short>(0);
    }

    short revents(int fd) const {
        return holds(fd) ? m_Revents[fd] : static_cast<short>(0);
    }

    // Readiness, reported only in the directions the descriptor was watched
    // for: a manager that never asked to write to its listening socket is
    // never told it can.
    bool isReadable(int fd) const {
        return ready(fd, kRead, static_cast<short>(kRead | kFailure));
    }

    bool isWritable(int fd) const {
        return ready(fd, kWrite, static_cast<short>(kWrite | kFailure));
    }

    // Out-of-band data alone. A hangup or a socket error is not urgent data
    // and is reported through the read and write directions instead.
    bool isUrgent(int fd) const {
        return ready(fd, kUrgent, kUrgent);
    }

    // Build the array the wait hands the kernel and drop the previous tick's
    // results.
    void fill() {
        m_Failed = false;
        m_Entries.clear();
        m_EntryGenerations.clear();

        for (int fd = 0; fd < tableSize(); fd++) {
            m_Revents[fd] = 0;

            if (m_Events[fd] == 0)
                continue;

            pollfd entry;
            entry.fd = fd;
            entry.events = m_Events[fd];
            entry.revents = 0;
            m_Entries.push_back(entry);
            m_EntryGenerations.push_back(m_Generation[fd]);
        }
    }

    // The array the wait hands the kernel, which writes each entry's revents.
    std::vector<pollfd>& entries() {
        return m_Entries;
    }

    const std::vector<pollfd>& entries() const {
        return m_Entries;
    }

    // Wait up to timeoutMilliseconds for one of the filled entries to become
    // ready and return what poll() returned: the number of ready descriptors,
    // zero on timeout, or a negative value on failure. A failure, including
    // the interrupted call a signal causes, leaves the tick with nothing to
    // process.
    int wait(int timeoutMilliseconds) {
        const int result = m_Entries.empty()
                               ? 0
                               : ::poll(m_Entries.data(), static_cast<nfds_t>(m_Entries.size()), timeoutMilliseconds);
        m_Failed = result < 0;
        return result;
    }

    // Move the results into the per-descriptor table.
    void collect() {
        if (m_Failed)
            return;

        for (size_t i = 0; i < m_Entries.size(); i++) {
            const int fd = m_Entries[i].fd;

            // The descriptor was added or removed while the wait ran, so the
            // answer is about a connection this set no longer watches.
            if (!watched(fd) || m_Generation[fd] != m_EntryGenerations[i])
                continue;

            m_Revents[fd] = m_Entries[i].revents;
        }
    }

    int pollOnce(int timeoutMilliseconds) {
        fill();
        const int result = wait(timeoutMilliseconds);
        collect();
        return result;
    }

private:
    bool ready(int fd, short interest, short reported) const {
        if (!holds(fd))
            return false;

        return (m_Events[fd] & interest) != 0 && (m_Revents[fd] & reported) != 0;
    }

    // Per descriptor: what the manager asked for, what the kernel last
    // answered, and a counter bumped on every membership change so a result
    // cannot outlive the connection it describes.
    std::vector<short> m_Events;
    std::vector<short> m_Revents;
    std::vector<unsigned int> m_Generation;

    // The filled entries and the generation each one was filled at.
    std::vector<pollfd> m_Entries;
    std::vector<unsigned int> m_EntryGenerations;

    bool m_Failed = false;
};

} // namespace de

#endif
