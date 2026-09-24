//--------------------------------------------------------------------------------
//
// Filename    : KeepAlive.h
// Description : When a worker next runs the query that keeps its database
//               connection open.
//
//--------------------------------------------------------------------------------

#ifndef __KEEP_ALIVE_H__
#define __KEEP_ALIVE_H__

#include <sys/time.h>

namespace de {

//--------------------------------------------------------------------------------
//
// MySQL closes a connection that stays idle longer than its wait_timeout, so
// every worker that holds one runs a throwaway query now and then. Each keeps
// an absolute deadline, starts it at the current time so the first pass runs
// the query, and after every run moves the deadline forward by an hour plus a
// random part of half an hour -- the spread keeps the workers of one process
// from all querying on the same tick.
//
// The deadline advances from itself, not from the clock, so it stays an
// absolute time a little over an hour ahead of the query just run. `draw` is
// the random number (a rand() result, never negative) the spread is taken
// from.
//
//--------------------------------------------------------------------------------

inline constexpr long kKeepAliveMinimumMinutes = 60;
inline constexpr long kKeepAliveSpreadMinutes = 30;

constexpr long keepAliveDelaySeconds(int draw) {
    return (kKeepAliveMinimumMinutes + draw % kKeepAliveSpreadMinutes) * 60;
}

inline timeval nextKeepAliveDeadline(timeval deadline, int draw) {
    deadline.tv_sec += keepAliveDelaySeconds(draw);
    return deadline;
}

} // namespace de

#endif
