//////////////////////////////////////////////////////////////////////////////
// Filename    : SpeedHackDecision.cpp
// Description : the gameserver's client-heartbeat plausibility rule.
//////////////////////////////////////////////////////////////////////////////

#include "SpeedHackDecision.h"

namespace de {

bool verifyHeartbeat(const HeartbeatVerifyParams& params, long nowSec, HeartbeatVerifyState& state) {
    // The first heartbeat of the session has nothing to be early against: it
    // only opens the window the next one is measured in.
    if (state.nextVerifySec == 0) {
        state.nextVerifySec = nowSec + params.checkDelaySec;
        return true;
    }

    const bool onTime = nowSec > state.nextVerifySec - params.maxTimeGapSec;

    state.nextVerifySec = nowSec + params.checkDelaySec;

    if (onTime) {
        if (state.earlyCount > 0)
            state.earlyCount--;
        return true;
    }

    const bool accepted = state.earlyCount <= params.maxEarlyCount;
    state.earlyCount++;

    return accepted;
}

} // namespace de
