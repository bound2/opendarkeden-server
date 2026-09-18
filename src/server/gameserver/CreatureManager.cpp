//////////////////////////////////////////////////////////////////////////////
// Filename    : CreatureManager.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CreatureManager.h"

#include <algorithm> // find_if ()

#include "Assert.h"
#include "GCCreatureDied.h"
#include "GamePlayer.h"
#include "IncomingPlayerManager.h"
#include "LevelWarZoneInfoManager.h"
#include "LogClient.h"
#include "Player.h"
#include "Slayer.h"
#include "SweeperBonusManager.h"
#include "Tile.h"
#include "Vampire.h"
#include "Zone.h"
#include "ZoneGroupManager.h"
#include "ZoneInfo.h"
#include "ZoneInfoManager.h"
#include "ZonePlayerManager.h"

class isSameNAME {
public:
    isSameNAME(const string& Name) : m_Name(Name) {}

    bool operator()(pair<const long unsigned int, Creature*> itr) {
        return (itr.second)->getName() == m_Name;
    }

private:
    string m_Name;
};

//////////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////////
CreatureManager::CreatureManager()

{
    __BEGIN_TRY

    m_Mutex.setName("CreatureManager");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////////////
CreatureManager::~CreatureManager()

{
    __BEGIN_TRY

    unordered_map<ObjectID_t, Creature*>::iterator itr = m_Creatures.begin();

    for (; itr != m_Creatures.end(); itr++) {
        Creature* pCreature = itr->second;
        SAFE_DELETE(pCreature);
    }

    m_Creatures.clear();

    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////////////
// Register a new creature object in the creature manager.
//////////////////////////////////////////////////////////////////////////////
void CreatureManager::addCreature(Creature* pCreature) {
    __BEGIN_TRY

    Assert(pCreature != NULL);

    unordered_map<ObjectID_t, Creature*>::iterator itr = m_Creatures.find(pCreature->getObjectID());

    if (itr != m_Creatures.end())
        throw DuplicatedException();

    m_Creatures[pCreature->getObjectID()] = pCreature;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// Delete a specific creature object from the creature manager.
//////////////////////////////////////////////////////////////////////////////
void CreatureManager::deleteCreature(ObjectID_t objectID){
    __BEGIN_TRY

        __ENTER_CRITICAL_SECTION(m_Mutex)

    // try
    {unordered_map<ObjectID_t, Creature*>::iterator itr = m_Creatures.find(objectID);

if (itr == m_Creatures.end()) {
    // throw NoSuchElementException();
    //  A rough look showed that nothing outside does anything special
    //  with NoSuchElementException().
    //  Some places were even broken because it was not handled.
    //   If NoSuch.. ever shows up in Zone::deletePC() from EventMorph..
    //  Removed anyway because of the speed cost.
    return; // by sigi 2002.5.2
}

// Only the unordered_map node is erased; the creature itself is not deleted.
m_Creatures.erase(itr);
}
// catch (Throwable & t)
//{
// cerr << t.toString() << endl;
//}

__LEAVE_CRITICAL_SECTION(m_Mutex)

__END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// Return a specific creature object from the creature manager.
//////////////////////////////////////////////////////////////////////////////
Creature* CreatureManager::getCreature(ObjectID_t objectID) const {
    __BEGIN_TRY

    unordered_map<ObjectID_t, Creature*>::const_iterator itr = m_Creatures.find(objectID);

    if (itr == m_Creatures.end()) {
        // throw NoSuchElementException();
        return NULL; // by sigi 2002.5.2
    }

    return itr->second;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Return a specific creature object from the creature manager.
//////////////////////////////////////////////////////////////////////////////
Creature* CreatureManager::getCreature(const string& Name) const {
    __BEGIN_TRY

    unordered_map<ObjectID_t, Creature*>::const_iterator itr =
        find_if(m_Creatures.begin(), m_Creatures.end(), isSameNAME(Name));

    if (itr == m_Creatures.end()) {
        // throw NoSuchElementException();
        return NULL; // by sigi 2002.5.2
    }

    return itr->second;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Run the AI actions of the monsters and NPCs owned by the creature manager.
//////////////////////////////////////////////////////////////////////////////
void CreatureManager::processCreatures()

{
    __BEGIN_TRY

    unordered_map<ObjectID_t, Creature*>::iterator before = m_Creatures.end();
    unordered_map<ObjectID_t, Creature*>::iterator current = m_Creatures.begin();

    Timeval currentTime;
    getCurrentTime(currentTime);

    while (current != m_Creatures.end()) {
        Assert(current->second != NULL);

        if (current->second->isAlive()) {
            current->second->act(currentTime);
            before = current++;
        } else {
            killCreature(current->second);

            if (before == m_Creatures.end()) {
                m_Creatures.erase(current);
                current = m_Creatures.begin();
            } else {
                m_Creatures.erase(current);
                current = before;
                current++;
            }
        }
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Handle a dead creature.
//////////////////////////////////////////////////////////////////////////////
void CreatureManager::killCreature(Creature* pDeadCreature)

{
    __BEGIN_TRY

    throw UnsupportedError();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// broadcast packet
//////////////////////////////////////////////////////////////////////////////
void CreatureManager::broadcastPacket(Packet* pPacket, Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pPacket != NULL);

    unordered_map<ObjectID_t, Creature*>::iterator itr = m_Creatures.begin();

    for (; itr != m_Creatures.end(); itr++) {
        Assert(itr->second != NULL);
        Assert(itr->second->isPC());

        if (itr->second != pCreature) {
            Creature* pCreature = itr->second;
            Assert(pCreature != NULL);

            Player* pPlayer = pCreature->getPlayer();
            Assert(pPlayer != NULL);

            pPlayer->sendPacket(pPacket);
        }
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// broadcast DarkLight packet
//////////////////////////////////////////////////////////////////////////////
void CreatureManager::broadcastDarkLightPacket(Packet* pPacket1, Packet* pPacket2, Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pPacket1 != NULL);
    Assert(pPacket2 != NULL);

    Creature* pTargetCreature = NULL;

    unordered_map<ObjectID_t, Creature*>::iterator itr = m_Creatures.begin();

    for (; itr != m_Creatures.end(); itr++) {
        Assert(itr->second != NULL);
        Assert(itr->second->isPC());

        pTargetCreature = itr->second;
        Assert(pTargetCreature != NULL);

        Zone* pZone = pTargetCreature->getZone();
        Assert(pZone != NULL);

        // ZoneCoord_t X = pTargetCreature->getX();
        // ZoneCoord_t Y = pTargetCreature->getY();

        if (pTargetCreature != pCreature) {
            if (pTargetCreature->isSlayer()) {
                bool bYellowPoisonEffected = pTargetCreature->isFlag(Effect::EFFECT_CLASS_YELLOW_POISON_TO_CREATURE);
                bool bLightnessEffected = pTargetCreature->isFlag(Effect::EFFECT_CLASS_LIGHTNESS);

                // Commented out
                // bool bDarknessEffected = (tile.getEffect(Effect::EFFECT_CLASS_DARKNESS) == NULL) ? false : true;

                if (!bYellowPoisonEffected && !bLightnessEffected) // && !bDarknessEffected)  // Commented out
                {
                    Player* pPlayer = pTargetCreature->getPlayer();
                    Assert(pPlayer != NULL);
                    pPlayer->sendPacket(pPacket1);
                }
            } else if (pTargetCreature->isVampire()) {
                Player* pPlayer = pTargetCreature->getPlayer();
                pPlayer->sendPacket(pPacket2);
            }
        }
    }

    __END_CATCH
}

void CreatureManager::broadcastLevelWarBonusPacket(Packet* pPacket, Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pPacket != NULL);

    unordered_map<ObjectID_t, Creature*>::iterator itr = m_Creatures.begin();

    for (; itr != m_Creatures.end(); itr++) {
        Assert(itr->second != NULL);
        Assert(itr->second->isPC());

        if (itr->second != pCreature) {
            Creature* pCreature = itr->second;
            Assert(pCreature != NULL);

            Player* pPlayer = pCreature->getPlayer();
            Assert(pPlayer != NULL);

            if (g_pLevelWarZoneInfoManager->isCreatureBonusZone(pCreature, pCreature->getZoneID())) {
                pPlayer->sendPacket(pPacket);
                pCreature->setFlag(Effect::EFFECT_CLASS_INIT_ALL_STAT);
            }
        }
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// get debug string
//////////////////////////////////////////////////////////////////////////////
string CreatureManager::toString() const

{
    StringStream msg;
    msg << "CreatureManager(" << "Size:" << (int)m_Creatures.size();
    for (unordered_map<ObjectID_t, Creature*>::const_iterator itr = m_Creatures.begin(); itr != m_Creatures.end();
         itr++) {
        msg << itr->second->toString();
    }
    msg << ")";
    return msg.toString();
}
