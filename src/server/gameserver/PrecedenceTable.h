//////////////////////////////////////////////////////////////////////////////
// Filename    : PrecedenceTable.h
// Written by  : excel96
// Description :
// Class that computes the precedence used to prevent loot stealing.
// It lives inside each monster object and, when the monster dies or is
// drained, checks whether a player has the right to take its items or
// to drain it.
//////////////////////////////////////////////////////////////////////////////

#ifndef __PRECEDENCETABLE_H__
#define __PRECEDENCETABLE_H__

#include <unordered_map>

#include "Timeval.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// Forward declaration
//////////////////////////////////////////////////////////////////////////////
class Creature;

//////////////////////////////////////////////////////////////////////////////
// class PrecedenceElement;
// An object stored in PrecedenceTable. It records the amount of damage each
// player dealt to the monster, the time of the last damage, and information
// about that player.
//////////////////////////////////////////////////////////////////////////////

class PrecedenceElement {
public:
    PrecedenceElement();

public:
    string getName(void) const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
    }

    int getPartyID(void) const {
        return m_PartyID;
    }
    void setPartyID(int PartyID) {
        m_PartyID = PartyID;
    }

    int getDamage(void) const {
        return m_Damage;
    }
    void setDamage(int Damage) {
        m_Damage = Damage;
    }

    Timeval getDeadline(void) const {
        return m_Deadline;
    }
    void setDeadline(const Timeval& deadline) {
        m_Deadline = deadline;
    }
    void setNextTime(void);

    string toString(void) const;

public:
    string m_Name;
    int m_PartyID;
    int m_Damage;
    Timeval m_Deadline;
};

//////////////////////////////////////////////////////////////////////////////
// class PrecedenceTable;
// A collection of PrecedenceElement, held inside a monster object by
// composition. addPrecedence records the damage each player dealt, and
// compute decides and stores who has precedence over the items this
// monster drops or over draining it.
//////////////////////////////////////////////////////////////////////////////

class PrecedenceTable {
public:
    PrecedenceTable();
    ~PrecedenceTable();

public:
    //	void addPrecedence(Creature* pCreature, int damage);
    void addPrecedence(const string& Name, int PartyID, int damage);
    void heartbeat(const Timeval& currentTime);
    void compute(void);

public:
    bool canLoot(Creature* pCreature) const;
    bool canDrainBlood(Creature* pCreature) const;
    bool canGainRankExp(Creature* pCreature) const;

    string getHostName(void) const {
        return m_HostName;
    }
    int getHostPartyID(void) const {
        return m_HostPartyID;
    }

    bool getComputeFlag(void) const {
        return m_bComputeFlag;
    }
    void setComputeFlag(bool bFlag) {
        m_bComputeFlag = bFlag;
    }

    string getQuestHostName() const {
        return m_QuestHostName;
    }
    void setQuestHostName(const string& name) {
        m_QuestHostName = name;
    }

    double getDamagePercent(const string& Name, int PartyID) const;

    string toString(void) const;

protected:
    unordered_map<string, PrecedenceElement*> m_CreatureMap;
    unordered_map<int, PrecedenceElement*> m_PartyMap;

    string m_FirstAttackerName; // Name of the one who attacked first
    int m_FirstAttackerPartyID; // Party ID of the one who attacked first

    string m_HostName; // Name of the owner
    int m_HostPartyID; // Party ID of the owner

    string m_QuestHostName; // Name of the owner of the quest item

    bool m_bComputeFlag; // Has the computation finished?

    Damage_t m_TotalDamage; // Total damage
};

#endif
