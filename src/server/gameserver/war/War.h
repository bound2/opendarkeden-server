///////////////////////////////////////////////////////////////////
// General war information and the routines run when a war starts and ends.
///////////////////////////////////////////////////////////////////

#ifndef __WAR_H__
#define __WAR_H__

#include "Exception.h"
#include "Types.h"
#include "VSDateTime.h"
#include "Work.h"

class Mutex;
class PlayerCreature;
class WarScheduleInfo;
class WarInfo;

class War : public Work {
public:
    enum WarState {
        WAR_STATE_WAIT,    // 0
        WAR_STATE_CURRENT, // 1
        WAR_STATE_END,     // 2
        WAR_STATE_CANCEL,  // 3

        MAX_WAR_STATE // 4
    };

public:
    War(WarState warState, WarID_t warID = 0);
    virtual ~War();

    virtual WarType_t getWarType() const = 0;
    virtual string getWarType2DBString() const = 0;
    virtual string getWarName() const = 0;

    WarID_t getWarID() const {
        return m_WarID;
    }
    void setWarID(WarID_t warID) {
        m_WarID = warID;
    }

    WarState getState() const {
        return m_State;
    }
    const string& getState2DBString() const;
    void setState(WarState warState) {
        m_State = warState;
    }

    const VSDateTime& getWarStartTime() const {
        return m_StartTime;
    }
    void setWarStartTime(VSDateTime dt) {
        m_StartTime = dt;
    }

    // A war fought over a castle names the castle's zone, the guild attacking
    // it and the fee paid to register it; a war fought over nothing answers
    // zero for all three. Both castle wars report WAR_GUILD, so the war system
    // and the war schedule match one by zone through these rather than by
    // casting to a particular castle war class.
    virtual ZoneID_t getCastleZoneID() const {
        return 0;
    }
    virtual GuildID_t getAttackerGuildID() const {
        return 0;
    }
    virtual Gold_t getRegistrationFee() const {
        return 0;
    }
    // A castle war with a single attacker has that one guild in it; a siege
    // counts its five challengers and the reinforcing guild as well.
    virtual bool isWarParticipant(GuildID_t gID) {
        return gID == getAttackerGuildID();
    }

public:
    virtual bool isModifyCastleOwner(PlayerCreature* pPC) {
        return false;
    }

    virtual void sendWarStartMessage() const;
    virtual void sendWarEndMessage() const;

public:
    static void initWarIDRegistry();

public:
    virtual void execute();
    virtual bool endWar(PlayerCreature* pPC) {
        return false;
    }

protected:
    virtual void executeStart() = 0;
    virtual void executeEnd() = 0;


public:
    virtual void makeWarScheduleInfo(WarScheduleInfo* pWSI) const = 0;
    virtual void makeWarInfo(WarInfo* pWarInfo) const = 0;

    virtual string toString() const = 0;

private:
    WarID_t m_WarID;
    WarState m_State;       // the war's current state.
    VSDateTime m_StartTime; // the war's start time

    static Mutex m_Mutex;
    static WarID_t m_WarIDRegistry;
};

#endif // __WAR_H__
