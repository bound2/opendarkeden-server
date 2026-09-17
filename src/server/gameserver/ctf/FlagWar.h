/////////////////////////////////////////////////////////////////////
// General capture-the-flag information and the routines run when it starts and ends.
/////////////////////////////////////////////////////////////////////

#ifndef __FLAG_WAR_H__
#define __FLAG_WAR_H__

#include <list>
#include <vector>

#include "GameContext.h"
#include "VSDateTime.h"
#include "war/Work.h"

class FlagManager;

class FlagWar : public Work {
    struct FlagGenZone {
        vector<ZoneID_t> ZoneIDs;
        uint FlagCount;
    };

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

    virtual void addFlags();

    void addFlagsRandom(ZoneID_t zoneID, uint no);

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

    vector<ItemID_t> m_Flags;
    static list<FlagGenZone> m_FlagGenInfo;
};

#endif // __FLAG_WAR_H__
