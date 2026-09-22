//////////////////////////////////////////////////////////////////////////////
// Filename    : PCFinder.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __PC_FINDER_H__
#define __PC_FINDER_H__

#include <map>

#include <unordered_map>

#include "Creature.h"
#include "Exception.h"
#include "Mutex.h"
#include "NPC.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class PCFinder;
// Global manager object of the game server that gives access to a PC object by
// PC name. Internally uses an unordered_map to speed up the lookup.
//////////////////////////////////////////////////////////////////////////////

class PCFinder {
public:
    PCFinder();
    ~PCFinder();

public:
    // add creature to unordered_map
    // execute just once at PC's login
    void addCreature(Creature* pCreature);

    // delete creature from unordered_map
    // execute just once at PC's logout
    void deleteCreature(const string& name); // NoSuchElementException, Error);

    // get creature with PC-name
    Creature* getCreature(const string& name) const; // NoSuchElementException, Error);

    // get creature with PC-name
    Creature* getCreature_LOCKED(const string& name) const; // NoSuchElementException, Error);

    // PlayerID. for BillingServer. by sigi. 2002.11.18
    Creature* getCreatureByID(const string& ID) const;        // NoSuchElementException, Error);
    Creature* getCreatureByID_LOCKED(const string& ID) const; // NoSuchElementException, Error);

    // add NPC to unordered_map
    void addNPC(NPC* npc);

    // delete NPC from unordered_map
    void deleteNPC(const string& name);

    // get NPC
    NPC* getNPC(const string& name) const;
    NPC* getNPC_LOCKED(const string& name) const;

    // get creature's IP address
    IP_t getIP(const string& name) const;

    list<Creature*> getGuildCreatures(GuildID_t gID, uint Num);

    void lock() {
        m_Mutex.lock();
    }
    void unlock() {
        m_Mutex.unlock();
    }

private:
    unordered_map<string, Creature*> m_PCs;
    unordered_map<string, Creature*> m_IDs; // PlayerID. for BillingServer. by sigi. 2002.11.18
    unordered_map<string, NPC*> m_NPCs;     // NPCs.. for NPC trace ;; by DEW  2003. 04. 16
                                            //	multimap< GuildID_t, Creature* >	m_GuildMap;
    mutable Mutex m_Mutex;
};

#endif
