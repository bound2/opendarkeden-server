/////////////////////////////////////////////////////////////////////
// General capture-the-flag information and the routines run when it starts and ends.
/////////////////////////////////////////////////////////////////////

#ifndef __FLAG_WAR_H__
#define __FLAG_WAR_H__

#include <memory>
#include <vector>

#include "GameContext.h"
#include "VSDateTime.h"
#include "ctf/FlagWarPlan.h"
#include "war/Work.h"

class FlagManager;

// Runs on the main thread, from FlagManager's schedule. Every change it
// makes to a zone is posted to the zone's group (war/WarZoneWork.h): the
// flags a start drops, the returns that take them back at the end, the pole
// sweeps and the summons of a newbie war's end.
class FlagWar : public Work {
public:
    FlagWar(FlagManager& flagManager, de::GameContext& context) : m_FlagManager(flagManager), m_Context(context) {
        m_State = STATE_WAIT;
    }
    virtual void execute();

    virtual VSDateTime getNextFlagWarTime();
    virtual int getWarTime() const;

protected:
    virtual void executeReady();
    virtual void executeStart();
    virtual void executeFinish();
    virtual void executeEnd();

    // The flags a start drops, zone by zone.
    virtual std::vector<de::ctf::FlagDrop> flagDrops() const;

    string toString() const {
        return "FlagWar";
    }

    FlagManager& m_FlagManager;
    de::GameContext& m_Context;

private:
    enum State { STATE_WAIT, STATE_READY, STATE_START, STATE_FINISH, STATE_END };

    State m_State;

    State getState() const {
        return m_State;
    }
    void setState(State state) {
        m_State = state;
    }

    // The ids of the flags this round dropped, filled in by the drops as
    // they run on the zones' threads.
    std::shared_ptr<de::ctf::FlagLedger> m_pFlags = std::make_shared<de::ctf::FlagLedger>();
};

#endif // __FLAG_WAR_H__
