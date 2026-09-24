//////////////////////////////////////////////////////////////////////////////
// Filename    : SpeedHackDecision.h
// Description : the gameserver's client-heartbeat plausibility rule, separated
//               from GamePlayer so it can be exercised without a socket, a
//               creature or a clock.
//////////////////////////////////////////////////////////////////////////////

#ifndef __SPEED_HACK_DECISION_H__
#define __SPEED_HACK_DECISION_H__

namespace de {

// The client sends a heartbeat packet on a fixed interval, and a client whose
// clock runs fast sends it early. The rule watches how early: one heartbeat
// that arrives sooner than the interval allows is tolerated, a run of them is
// not, and a heartbeat that arrives on time pays one of them back.
struct HeartbeatVerifyParams {
    // The interval the client sends its heartbeat on, in seconds.
    long checkDelaySec = 0;
    // How far ahead of that interval a heartbeat may still arrive.
    long maxTimeGapSec = 0;
    // How many early heartbeats are tolerated before the rule refuses.
    int maxEarlyCount = 0;
};

// What the rule carries between two heartbeats of one session.
struct HeartbeatVerifyState {
    // When the next heartbeat is expected, in seconds. Zero until the first
    // one arrives, which is the only heartbeat with nothing to compare to.
    long nextVerifySec = 0;
    // How many early heartbeats this session is currently carrying.
    int earlyCount = 0;
};

// Answers whether a heartbeat that arrived at nowSec is plausible, and folds
// the arrival into state. The first heartbeat of a session is always
// plausible. Every heartbeat, early or not, sets the next expected time from
// its own arrival, so a client that has been refused once is measured from
// where it is rather than from where it should have been.
bool verifyHeartbeat(const HeartbeatVerifyParams& params, long nowSec, HeartbeatVerifyState& state);

} // namespace de

#endif // __SPEED_HACK_DECISION_H__
