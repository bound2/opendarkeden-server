//////////////////////////////////////////////////////////////////////////////
// Filename    : PCManager.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __PC_MANANGER_H__
#define __PC_MANANGER_H__

#include <vector>

#include "CreatureManager.h"

//////////////////////////////////////////////////////////////////////////////
// class PCManager
//////////////////////////////////////////////////////////////////////////////
const BYTE defaultRaceValue = 0xFF;

class PCManager : public CreatureManager {
public:
    PCManager();
    virtual ~PCManager();

public:
    // Register a new creature object with the creature manager.
    // virtual void addCreature(Creature* pCreature) ;

    // Delete a particular creature object from the creature manager.
    // virtual void deleteCreature(ObjectID_t objectID) ;

    // Return a particular creature object from the creature manager.
    // Creature* getCreature(ObjectID_t objectID) const ;

    // Process the creatures (NPC, Monster) that belong to this manager.
    virtual void processCreatures();

    // Handle a dead creature.
    virtual void killCreature(Creature* pDeadCreature);

    // Broadcast, when this is a PC manager.
    // void broadcastPacket(Packet* pPacket, Creature* owner) ;

    // Move every player somewhere else.
    void transportAllCreatures(ZoneID_t ZoneID, ZoneCoord_t ZoneX = 0xffff, ZoneCoord_t ZoneY = 0xffff,
                               Race_t race = defaultRaceValue, Turn_t delay = 10) const;

    // get debug string
    string toString() const;

    // Refresh players when the Holy Land race bonus changes
    void setRefreshHolyLandPlayer(bool bRefresh) {
        m_bRefreshHolyLandPlayer = bRefresh;
    }
    //	void setRefreshLevelWarBonusZonePlayer( bool bRefresh ) { m_bRefreshLevelWarBonusZonePlayer = bRefresh; }

    vector<uint> getPCNumByRace() const;

private:
    bool m_bRefreshHolyLandPlayer;
    //	bool m_bRefreshLevelWarBonusZonePlayer;
};

#endif
