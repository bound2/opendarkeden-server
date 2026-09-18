//////////////////////////////////////////////////////////////////////////////
// Filename    : MasterLairManager.h
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __MASTER_LAIR_MANAGER_H__
#define __MASTER_LAIR_MANAGER_H__

#include <vector>

#include <unordered_map>

#include "Item.h"
#include "MonsterCounter.h"
#include "Mutex.h"
#include "Timeval.h"

//////////////////////////////////////////////////////////////////////////////
// class MasterLairManager
//////////////////////////////////////////////////////////////////////////////

class Zone;

class MasterLairManager {
public:
    enum MasterLairEvent {
        EVENT_WAITING_PLAYER,   // wait for players to come in
        EVENT_MINION_COMBAT,    // fight the summoned monsters
        EVENT_MASTER_COMBAT,    // fight the master
        EVENT_WAITING_KICK_OUT, // wait to expel the players (cleanup time once the master is killed)
        EVENT_WAITING_REGEN,    // wait for the next regen

        EVENT_MAX
    };


public:
    MasterLairManager(Zone* pZone);
    ~MasterLairManager();

    MasterLairEvent getCurrentEvent() const {
        return m_Event;
    }

    bool enterCreature(Creature* pCreature); // may the creature enter the zone?
    bool leaveCreature(Creature* pCreature); // the creature left the zone

    bool heartbeat();

    // void increaseSummonedMonsterNumber(int num) ;
    bool isMasterReady() const {
        return m_bMasterReady;
    }
    void setMasterReady(bool bReady = true) {
        m_bMasterReady = bReady;
    }

    void startEvent();
    void stopEvent();

    void lock() {
        m_Mutex.lock();
    }
    void ulnock() {
        m_Mutex.unlock();
    }

    string toString() const;

protected:
    void processEventWaitingPlayer();
    void processEventMinionCombat();
    void processEventMasterCombat();
    void processEventWaitingKickOut();
    void processEventWaitingRegen();

    void activeEventWaitingPlayer();
    void activeEventMinionCombat();
    void activeEventMasterCombat();
    void activeEventWaitingKickOut();
    void activeEventWaitingRegen();

    void deleteAllMonsters(); // delete every monster
    void kickOutPlayers();    // expel the players
    void giveKillingReward(); // reward for killing the master
    void killAllMonsters();   // kill every monster

private:
    Zone* m_pZone;
    ObjectID_t m_MasterID; // the single master
    ZoneCoord_t m_MasterX;
    ZoneCoord_t m_MasterY;

    bool m_bMasterReady; // is the master ready to fight?

    // int               m_nMaxSummonMonster; // maximum monsters the master may summon
    // int               m_nSummonedMonster;  // monsters the master has summoned

    int m_nMaxPassPlayer; // maximum number of players allowed in
    int m_nPassPlayer;    // number of players that received a pass

    MasterLairEvent m_Event; // the current event kind
    Timeval m_EventTime;     // how long the current event lasts
    int m_EventValue;        // value associated with the event

    Timeval m_RegenTime; // time at which everything is cleared and restarted


    mutable Mutex m_Mutex; // to check m_nPassPlayer reliably
};

#endif
