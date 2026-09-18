//////////////////////////////////////////////////////////////////////////////
// Filename    : Event.h
// Written by  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EVENT_H__
#define __EVENT_H__

#include "Exception.h"
#include "Timeval.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class Event;
// Associated with the player class;
// makes a specific action run after a set amount of time.
//////////////////////////////////////////////////////////////////////////////

class GamePlayer;

class Event {
public:
    enum EventClass {
        EVENT_CLASS_RESURRECT,                // Resurrect a creature after it dies
        EVENT_CLASS_MORPH,                    // Slayer -> Vampire transformation
        EVENT_CLASS_RESTORE,                  // Vampire -> Slayer transformation
        EVENT_SAVE,                           // Save creature information periodically
        EVENT_CLASS_REGENERATION,             // Recover a Vampire periodically
        EVENT_CLASS_RELOAD_INFO,              // Reload info from the DB
        EVENT_CLASS_TRANSPORT,                // Move between zones
        EVENT_CLASS_KICK,                     // Kick after a while
        EVENT_CLASS_SYSTEM_MESSAGE,           // Hold a system message received while incoming and show it later
        EVENT_CLASS_REFRESH_HOLY_LAND_PLAYER, // Refresh Holy Land player stats when the Holy Land Race Bonus changes
        EVENT_CLASS_SHUTDOWN,                 // Clear out all users and shut down
        EVENT_CLASS_HEAD_COUNT,               // Count severed heads once every 30 minutes
        EVENT_CLASS_AUTH,                     // Nprotect authentication
        EVENT_CLASS_MAX
    };

public:
    Event(GamePlayer* pGamePlayer);
    virtual ~Event();

public:
    virtual EventClass getEventClass() const = 0;

    // get event life-cycle
    // By default every event is one-shot.
    virtual bool isTemporary() const {
        return true;
    }
    virtual bool isPermanent() const {
        return false;
    }

    virtual void activate() = 0;

    Timeval getDeadline() const {
        return m_Deadline;
    }
    void setDeadline(Turn_t delay);

    virtual string toString() const = 0;

protected:
    GamePlayer* m_pGamePlayer; // Game player object
    Timeval m_Deadline;        // Time it runs
};

#endif
