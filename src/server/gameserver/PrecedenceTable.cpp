//////////////////////////////////////////////////////////////////////////////
// Filename    : PrecedenceTable.cpp
// Written by  : excel96
// Description :
// Class that computes the precedence which stops loot snatching.
// It lives inside each individual monster object, and when the monster dies
// or is drained it checks whether a player holds the right to loot an item
// or to drain blood.
//////////////////////////////////////////////////////////////////////////////

#include "PrecedenceTable.h"

#include "Creature.h"
#include "StringStream.h"

//////////////////////////////////////////////////////////////////////////////
// class PrecedenceElement member methods
//////////////////////////////////////////////////////////////////////////////

PrecedenceElement::PrecedenceElement() {
    m_Name = "";
    m_PartyID = -1;
    m_Damage = 0;

    getCurrentTime(m_Deadline);
}

void PrecedenceElement::setNextTime(void) {
    getCurrentTime(m_Deadline);
    m_Deadline.tv_sec += 30;
}

string PrecedenceElement::toString(void) const {
    StringStream msg;
    msg << "PrecedenceElement(" << "Name:" << m_Name << ",PartyID:" << m_PartyID << ",Damage:" << m_Damage << ")";
    return msg.toString();
}


//////////////////////////////////////////////////////////////////////////////
// class PrecedenceTable member methods
//////////////////////////////////////////////////////////////////////////////

PrecedenceTable::PrecedenceTable() {
    m_FirstAttackerName = "";
    m_FirstAttackerPartyID = 0;
    m_HostName = "";
    m_HostPartyID = 0;
    m_bComputeFlag = false;
}

PrecedenceTable::~PrecedenceTable() {
    // Empty the creature map.
    unordered_map<string, PrecedenceElement*>::iterator itr = m_CreatureMap.begin();
    for (; itr != m_CreatureMap.end(); itr++) {
        SAFE_DELETE(itr->second);
    }
    m_CreatureMap.clear();

    // Empty the party map.
    unordered_map<int, PrecedenceElement*>::iterator itr2 = m_PartyMap.begin();
    for (; itr2 != m_PartyMap.end(); itr2++) {
        SAFE_DELETE(itr2->second);
    }
    m_PartyMap.clear();
}
/*
void PrecedenceTable::addPrecedence(Creature* pCreature, int damage)
{
    Assert(pCreature != NULL);

    // An empty creature map means this is the first attacker.
    if (m_CreatureMap.empty())
    {
        // Set the name of the first attacker...
        m_FirstAttackerName = pCreature->getName();

        // Add the data to the creature map.
        PrecedenceElement* pElement = new PrecedenceElement;
        pElement->setName(pCreature->getName());
        pElement->setPartyID(-1);
        pElement->setDamage(damage);
        pElement->setNextTime();
        m_CreatureMap[pCreature->getName()] = pElement;
    }
    else
    {
        unordered_map<string, PrecedenceElement*>::iterator itr = m_CreatureMap.find(pCreature->getName());
        if (itr == m_CreatureMap.end())
        {
            // Create a new entry if the creature has not attacked before.
            PrecedenceElement* pElement = new PrecedenceElement;
            pElement->setName(pCreature->getName());
            pElement->setPartyID(-1);
            pElement->setDamage(damage);
            pElement->setNextTime();
            m_CreatureMap[pCreature->getName()] = pElement;
        }
        else
        {
            // Update the entry if the creature has attacked before.
            PrecedenceElement* pElement = itr->second;
            pElement->setDamage(pElement->getDamage() + damage);
            pElement->setNextTime();
        }
    }

    // The party side is computed as well.
    // A creature that is not in a party has party id 0.
    // So entry 0 holds the damage dealt by everyone without a party.
    int PartyID = pCreature->getPartyID();
    if (m_PartyMap.empty())
    {
        PrecedenceElement* pElement = new PrecedenceElement;
        pElement->setPartyID(PartyID);
        pElement->setDamage(damage);
        pElement->setNextTime();
        m_PartyMap[PartyID] = pElement;
    }
    else
    {
        unordered_map<int, PrecedenceElement*>::iterator itr = m_PartyMap.find(PartyID);
        if (itr == m_PartyMap.end())
        {
            PrecedenceElement* pElement = new PrecedenceElement;
            pElement->setPartyID(PartyID);
            pElement->setDamage(damage);
            pElement->setNextTime();
            m_PartyMap[PartyID] = pElement;
        }
        else
        {
            PrecedenceElement* pElement = itr->second;
            pElement->setDamage(pElement->getDamage() + damage);
            pElement->setNextTime();
        }
    }
}
*/

void PrecedenceTable::addPrecedence(const string& Name, int PartyID, int damage) {
    // An empty creature map means this is the first attacker.
    if (m_CreatureMap.empty()) {
        // Set the name of the first attacker...
        m_FirstAttackerName = Name;

        // Add the data to the creature map.
        PrecedenceElement* pElement = new PrecedenceElement;
        pElement->setName(Name);
        pElement->setPartyID(-1);
        pElement->setDamage(damage);
        pElement->setNextTime();
        m_CreatureMap[Name] = pElement;
    } else {
        unordered_map<string, PrecedenceElement*>::iterator itr = m_CreatureMap.find(Name);
        if (itr == m_CreatureMap.end()) {
            // Create a new entry if the creature has not attacked before.
            PrecedenceElement* pElement = new PrecedenceElement;
            pElement->setName(Name);
            pElement->setPartyID(-1);
            pElement->setDamage(damage);
            pElement->setNextTime();
            m_CreatureMap[Name] = pElement;
        } else {
            // Update the entry if the creature has attacked before.
            PrecedenceElement* pElement = itr->second;
            pElement->setDamage(pElement->getDamage() + damage);
            pElement->setNextTime();
        }
    }

    // The party side is computed as well.
    // A creature that is not in a party has party id 0.
    // So entry 0 holds the damage dealt by everyone without a party.
    //	int PartyID = PartyID;
    if (m_PartyMap.empty()) {
        PrecedenceElement* pElement = new PrecedenceElement;
        pElement->setPartyID(PartyID);
        pElement->setDamage(damage);
        pElement->setNextTime();
        m_PartyMap[PartyID] = pElement;
    } else {
        unordered_map<int, PrecedenceElement*>::iterator itr = m_PartyMap.find(PartyID);
        if (itr == m_PartyMap.end()) {
            PrecedenceElement* pElement = new PrecedenceElement;
            pElement->setPartyID(PartyID);
            pElement->setDamage(damage);
            pElement->setNextTime();
            m_PartyMap[PartyID] = pElement;
        } else {
            PrecedenceElement* pElement = itr->second;
            pElement->setDamage(pElement->getDamage() + damage);
            pElement->setNextTime();
        }
    }

    // Force the precedence to be computed again.
    m_bComputeFlag = false;
}

void PrecedenceTable::heartbeat(const Timeval& currentTime) {
    unordered_map<string, PrecedenceElement*>::iterator c_before = m_CreatureMap.end();
    unordered_map<string, PrecedenceElement*>::iterator c_current = m_CreatureMap.begin();

    while (c_current != m_CreatureMap.end()) {
        PrecedenceElement* pElement = c_current->second;
        Assert(pElement != NULL);

        if (pElement->getDeadline() < currentTime) {
            if (c_before == m_CreatureMap.end()) {
                m_CreatureMap.erase(c_current);
                c_current = m_CreatureMap.begin();
            } else {
                m_CreatureMap.erase(c_current);
                c_current = c_before;
                c_current++;
            }
        } else {
            c_before = c_current++;
        }
    }

    unordered_map<string, PrecedenceElement*>::iterator p_before = m_CreatureMap.end();
    unordered_map<string, PrecedenceElement*>::iterator p_current = m_CreatureMap.begin();

    while (p_current != m_CreatureMap.end()) {
        PrecedenceElement* pElement = p_current->second;
        Assert(pElement != NULL);

        if (pElement->getDeadline() < currentTime) {
            if (p_before == m_CreatureMap.end()) {
                m_CreatureMap.erase(p_current);
                p_current = m_CreatureMap.begin();
            } else {
                m_CreatureMap.erase(p_current);
                p_current = p_before;
                p_current++;
            }
        } else {
            p_before = p_current++;
        }
    }
}

void PrecedenceTable::compute(void) {
    // Do nothing if it has already been computed.
    if (m_bComputeFlag)
        return;

    int MaxDamage = 0;
    string MaxDamageName = "";
    string SecondDamageName = "";
    int MaxDamagePartyID = 0;
    int SecondDamagePartyID = 0;

    Damage_t TotalDamage = 0;

    // Search the creature map first.
    unordered_map<string, PrecedenceElement*>::const_iterator itr = m_CreatureMap.begin();
    for (; itr != m_CreatureMap.end(); itr++) {
        PrecedenceElement* pElement = itr->second;
        Assert(pElement != NULL);

        if (MaxDamage < pElement->getDamage()) {
            if (MaxDamageName == "") {
                MaxDamageName = pElement->getName();
                MaxDamage = pElement->getDamage();
            } else {
                SecondDamageName = MaxDamageName;
                MaxDamageName = pElement->getName();
                MaxDamage = pElement->getDamage();
            }
        }

        TotalDamage += pElement->getDamage();
    }

    m_TotalDamage = TotalDamage;

    if (MaxDamageName != "") {
        // If the top damage dealer is also the first attacker,
        // 40+20 = 60 makes that one the owner.
        if (MaxDamageName == m_FirstAttackerName) {
            m_HostName = MaxDamageName;
        }
        // If the top damage dealer is not the first attacker but the
        // second highest damage dealer is, that one gets 30+20 and becomes the owner.
        // If the second highest damage dealer is not the first attacker either,
        // the top damage dealer becomes the owner.
        else {
            if (SecondDamageName != "" && SecondDamageName == m_FirstAttackerName) {
                m_HostName = SecondDamageName;
            } else {
                m_HostName = MaxDamageName;
            }
        }
    } else {
        // There should always be a top damage dealer, but
        // should there be none, the first attacker becomes the owner.
        m_HostName = m_FirstAttackerName;
    }

    // The party information has to be computed too.
    // Note that party id 0 is included in this computation,
    // that is, the damage of everyone who is not in a party.
    // If the party id with the highest damage is 0 -- that is, the players
    // without a party dealt the most damage -- HostPartyID becomes 0.
    // In that case everyone without a party must not gain the right,
    // so canLoot and canDrainBlood have to check that the party id is not 0.
    MaxDamage = 0;
    MaxDamagePartyID = -1;
    SecondDamagePartyID = -1;

    unordered_map<int, PrecedenceElement*>::const_iterator itr2 = m_PartyMap.begin();
    for (; itr2 != m_PartyMap.end(); itr2++) {
        PrecedenceElement* pElement = itr2->second;
        Assert(pElement != NULL);

        if (MaxDamage < pElement->getDamage()) {
            if (MaxDamagePartyID == -1) {
                MaxDamagePartyID = pElement->getPartyID();
                MaxDamage = pElement->getDamage();
            } else {
                SecondDamagePartyID = MaxDamagePartyID;
                MaxDamagePartyID = pElement->getPartyID();
                MaxDamage = pElement->getDamage();
            }
        }
    }

    if (MaxDamagePartyID != -1) {
        // If the top damage party is also the first attacking party,
        // 40+20 = 60 makes it the owner.
        if (MaxDamagePartyID == m_FirstAttackerPartyID) {
            m_HostPartyID = MaxDamagePartyID;
        }
        // If the top damage party is not the first attacking party but the
        // second highest damage party is, it gets 30+20 and becomes the owner.
        // If the second highest damage party is not the first attacking party either,
        // the top damage party becomes the owner.
        else {
            if (SecondDamagePartyID != -1 && SecondDamagePartyID == m_FirstAttackerPartyID) {
                m_HostPartyID = SecondDamagePartyID;
            } else {
                m_HostPartyID = MaxDamagePartyID;
            }
        }
    } else {
        // There should always be a top damage dealer, but
        // should there be none, the first attacker becomes the owner.
        m_HostPartyID = m_FirstAttackerPartyID;
    }

    m_bComputeFlag = true;
}

bool PrecedenceTable::canLoot(Creature* pCreature) const {
    // The holder of the precedence may loot.
    if (m_HostName == pCreature->getName())
        return true;

    // A member of the party holding the precedence may loot.
    int PartyID = pCreature->getPartyID();
    if (PartyID != 0 && m_HostPartyID == PartyID)
        return true;

    return false;
}

bool PrecedenceTable::canDrainBlood(Creature* pCreature) const {
    // The holder of the precedence may drain blood.
    if (m_HostName == pCreature->getName())
        return true;

    // A member of the party holding the precedence may drain blood.
    int PartyID = pCreature->getPartyID();
    if (PartyID != 0 && m_HostPartyID == PartyID)
        return true;

    return false;
}

bool PrecedenceTable::canGainRankExp(Creature* pCreature) const {
    // Rank exp is granted for dealing more than 1/4 of the total damage the monster took.
    unordered_map<string, PrecedenceElement*>::const_iterator itr = m_CreatureMap.find(pCreature->getName());
    if (itr == m_CreatureMap.end())
        return false;

    return (m_TotalDamage >> 2) < itr->second->getDamage();
}

double PrecedenceTable::getDamagePercent(const string& Name, int PartyID) const {
    if (m_TotalDamage == 0)
        return 0.0;

    double ownDamage = 0.0;
    unordered_map<string, PrecedenceElement*>::const_iterator itr = m_CreatureMap.find(Name);
    if (itr != m_CreatureMap.end()) {
        ownDamage = (double)(itr->second->getDamage());
    }

    double partyDamage = 0.0;
    if (PartyID != 0) {
        unordered_map<int, PrecedenceElement*>::const_iterator itr = m_PartyMap.find(PartyID);
        if (itr != m_PartyMap.end()) {
            partyDamage = (double)(itr->second->getDamage());
        }
    }

    double maxDamage = (ownDamage > partyDamage ? ownDamage : partyDamage);

    return maxDamage / (double)m_TotalDamage;
}

string PrecedenceTable::toString(void) const {
    StringStream msg;
    msg << "PrecedenceTable(" << "FirstAttackerName:" << m_FirstAttackerName
        << ",FirstAttackerPartyID:" << m_FirstAttackerPartyID << ",HostName:" << m_HostName
        << ",HostPartyID:" << m_HostPartyID << ",ComputeFlag:" << m_bComputeFlag;

    msg << "\n,CreatureMap:\n";

    unordered_map<string, PrecedenceElement*>::const_iterator itr1 = m_CreatureMap.begin();
    for (; itr1 != m_CreatureMap.end(); itr1++) {
        msg << itr1->second->toString() << ",";
    }

    msg << "\n,PartyMap:\n";

    unordered_map<int, PrecedenceElement*>::const_iterator itr2 = m_PartyMap.begin();
    for (; itr2 != m_PartyMap.end(); itr2++) {
        msg << itr2->second->toString() << ",";
    }

    msg << ")";

    return msg.toString();
}
