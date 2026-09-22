//////////////////////////////////////////////////////////////////////////////
// Filename    : MonsterFunctionObject.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __MONSTER_FUNCTION_OBJECT_H__
#define __MONSTER_FUNCTION_OBJECT_H__

#include "Monster.h"
#include "Ousters.h"
#include "Slayer.h"
#include "Vampire.h"
#include "Zone.h"

//////////////////////////////////////////////////////////////////////////////
//
// class StrongerSlayer, WeakerSlayer, StrongerVampire, WeakerVampire;
//
// These classes are used as function objects by Monster::addEnemy(), which
// adds an enemy to a monster, to find the right insertion position for the
// new enemy in the zone's creature manager.
//
// ex> Assume the monster's AttackOrder is ATTACK_WEAKEST. This
// monster's m_Enemies is sorted from the weakest. Because the
// current spec only has vampire monsters, when such a monster is
// attacked by a vampire and a slayer at the same time it attacks
// the slayer first in certain situations. So m_Enemies holds the
// slayers' OIDs at the front and the vampires' OIDs at the
// back.
//
// Now suppose a slayer with SkillDomainLevelSum == 5000 attacks this
// monster and is about to be marked as an enemy. To insert this slayer
// at the right position, the position of the weakest slayer must be found.
// (If there is no slayer, the position of the first vampire must be found.)
//
// Using the STL find_if algorithm keeps the code simple: passing
// WeakerSlayer(5000) as its parameter finds the position.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// class StrongerSlayer;
// The enemy list is sorted from the weakest.
// So operator() must return true for anything that is not a slayer, or for a
// slayer stronger than self. Inserting before that node keeps the weakest-first order.
//////////////////////////////////////////////////////////////////////////////

class StrongerSlayer {
public:
    StrongerSlayer(Zone* pZone, SkillLevel_t skillLevelSum) {
        m_pZone = pZone;
        m_SkillDomainLevelSum = skillLevelSum;
    }

    bool operator()(ObjectID_t objectID) {
        Assert(m_pZone != NULL); // by sigi

        Creature* pCreature = NULL;

        try {
            pCreature = m_pZone->getCreature(objectID);
        } catch (NoSuchElementException& nsee) {
            // cout << nsee.toString() << endl;
            pCreature = NULL;
        }

        if (pCreature == NULL)
            return false; // by sigi

        if (!pCreature->isSlayer())
            return true;

        Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);

        return pSlayer->getSkillDomainLevelSum() > m_SkillDomainLevelSum;
    }

private:
    Zone* m_pZone;
    SkillLevel_t m_SkillDomainLevelSum;
};


//////////////////////////////////////////////////////////////////////////////
// class WeakerSlayer;
// The enemy list is sorted from the strongest.
// So it must return true for anything that is not a slayer, or
// for a slayer weaker than self. Inserting before that node
// keeps the strongest-first order.
//////////////////////////////////////////////////////////////////////////////

class WeakerSlayer {
public:
    // constructor
    WeakerSlayer(Zone* pZone, SkillLevel_t skillLevelSum) {
        m_pZone = pZone;
        m_SkillDomainLevelSum = skillLevelSum;
    }

    bool operator()(ObjectID_t objectID) {
        Assert(m_pZone != NULL); // by sigi

        Creature* pCreature = NULL;

        try {
            pCreature = m_pZone->getCreature(objectID);
        } catch (NoSuchElementException& nsee) {
            // cout << nsee.toString() << endl;
            pCreature = NULL;
        }

        if (pCreature == NULL)
            return false; // by sigi

        if (!pCreature->isSlayer())
            return true;

        Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);

        return pSlayer->getSkillDomainLevelSum() < m_SkillDomainLevelSum;
    }

private:
    Zone* m_pZone;
    SkillLevel_t m_SkillDomainLevelSum;
};


//////////////////////////////////////////////////////////////////////////////
// class StrongerVampire;
// The enemy list is sorted from the weakest.
// So it must return true only for a vampire stronger than self.
// Inserting before that node keeps the weakest-first order.
//////////////////////////////////////////////////////////////////////////////

class StrongerVampire {
public:
    StrongerVampire(Zone* pZone, Level_t level) {
        m_pZone = pZone;
        m_Level = level;
    }

    bool operator()(ObjectID_t objectID) {
        Assert(m_pZone != NULL); // by sigi

        Creature* pCreature = NULL;

        try {
            pCreature = m_pZone->getCreature(objectID);
        } catch (NoSuchElementException& nsee) {
            // cout << nsee.toString() << endl;
            pCreature = NULL;
        }

        if (pCreature == NULL)
            return false; // by sigi

        if (!pCreature->isVampire())
            return false;

        Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);

        return pVampire->getLevel() > m_Level;
    }

private:
    Zone* m_pZone;
    Level_t m_Level;
};


//////////////////////////////////////////////////////////////////////////////
// class WeakerVampire;
// The enemy list is sorted from the strongest.
// So it must return true only for a vampire weaker than self.
// Inserting before that node keeps the weakest-first order.
//////////////////////////////////////////////////////////////////////////////

class WeakerVampire {
public:
    WeakerVampire(Zone* pZone, Level_t level) {
        m_pZone = pZone;
        m_Level = level;
    }

    bool operator()(ObjectID_t objectID) {
        Assert(m_pZone != NULL); // by sigi

        Creature* pCreature = NULL;

        try {
            pCreature = m_pZone->getCreature(objectID);
        } catch (NoSuchElementException& nsee) {
            // cout << nsee.toString() << endl;
            pCreature = NULL;
        }

        if (pCreature == NULL)
            return false; // by sigi

        if (!pCreature->isVampire())
            return false;

        Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);

        return pVampire->getLevel() < m_Level;
    }

private:
    Zone* m_pZone;
    Level_t m_Level;
};

class StrongerOusters {
public:
    StrongerOusters(Zone* pZone, Level_t level) {
        m_pZone = pZone;
        m_Level = level;
    }

    bool operator()(ObjectID_t objectID) {
        Assert(m_pZone != NULL); // by sigi

        Creature* pCreature = NULL;

        try {
            pCreature = m_pZone->getCreature(objectID);
        } catch (NoSuchElementException& nsee) {
            // cout << nsee.toString() << endl;
            pCreature = NULL;
        }

        if (pCreature == NULL)
            return false; // by sigi

        if (!pCreature->isOusters())
            return false;

        Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);

        return pOusters->getLevel() > m_Level;
    }

private:
    Zone* m_pZone;
    Level_t m_Level;
};


//////////////////////////////////////////////////////////////////////////////
// class WeakerOusters;
// The enemy list is sorted from the strongest.
// So it must return true only for an Ousters weaker than self.
// Inserting before that node keeps the weakest-first order.
//////////////////////////////////////////////////////////////////////////////

class WeakerOusters {
public:
    WeakerOusters(Zone* pZone, Level_t level) {
        m_pZone = pZone;
        m_Level = level;
    }

    bool operator()(ObjectID_t objectID) {
        Assert(m_pZone != NULL); // by sigi

        Creature* pCreature = NULL;

        try {
            pCreature = m_pZone->getCreature(objectID);
        } catch (NoSuchElementException& nsee) {
            // cout << nsee.toString() << endl;
            pCreature = NULL;
        }

        if (pCreature == NULL)
            return false; // by sigi

        if (!pCreature->isOusters())
            return false;

        Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);

        return pOusters->getLevel() < m_Level;
    }

private:
    Zone* m_pZone;
    Level_t m_Level;
};

//////////////////////////////////////////////////////////////////////////////
// class StrongerMonster;
// The enemy list is sorted from the weakest.
// So it must return true only for a monster stronger than self.
// Inserting before that node keeps the weakest-first order.
//////////////////////////////////////////////////////////////////////////////

class StrongerMonster {
public:
    StrongerMonster(Zone* pZone, Level_t level) {
        m_pZone = pZone;
        m_Level = level;
    }

    bool operator()(ObjectID_t objectID) {
        Assert(m_pZone != NULL); // by sigi

        Creature* pCreature = NULL;

        try {
            pCreature = m_pZone->getCreature(objectID);
        } catch (NoSuchElementException& nsee) {
            // cout << nsee.toString() << endl;
            pCreature = NULL;
        }

        if (pCreature == NULL)
            return false; // by sigi

        if (!pCreature->isMonster())
            return false;

        Monster* pMonster = dynamic_cast<Monster*>(pCreature);

        return pMonster->getLevel() > m_Level;
    }

private:
    Zone* m_pZone;
    Level_t m_Level;
};


//////////////////////////////////////////////////////////////////////////////
// class WeakerMonster;
// The enemy list is sorted from the strongest.
// So it must return true only for a monster weaker than self.
// Inserting before that node keeps the weakest-first order.
//////////////////////////////////////////////////////////////////////////////

class WeakerMonster {
public:
    WeakerMonster(Zone* pZone, Level_t level) {
        m_pZone = pZone;
        m_Level = level;
    }

    bool operator()(ObjectID_t objectID) {
        Assert(m_pZone != NULL); // by sigi

        Creature* pCreature = NULL;

        try {
            pCreature = m_pZone->getCreature(objectID);
        } catch (NoSuchElementException& nsee) {
            // cout << nsee.toString() << endl;
            pCreature = NULL;
        }

        if (pCreature == NULL)
            return false; // by sigi

        if (!pCreature->isMonster())
            return false;

        Monster* pMonster = dynamic_cast<Monster*>(pCreature);

        return pMonster->getLevel() < m_Level;
    }

private:
    Zone* m_pZone;
    Level_t m_Level;
};


#endif
