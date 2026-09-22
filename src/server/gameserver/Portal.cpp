//////////////////////////////////////////////////////////////////////////////
// Filename    : NormalPortal.cpp
// Written by  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "Portal.h"

#include "GCMoveOK.h"
#include "GCUpdateInfo.h"
#include "GamePlayer.h"
#include "Guild.h"
#include "GuildManager.h"
#include "IncomingPlayerManager.h"
#include "LogClient.h"
#include "PCSlayerInfo2.h"
#include "PCVampireInfo2.h"
#include "PacketUtil.h"
#include "Properties.h"
#include "Slayer.h"
#include "TimeManager.h"
#include "Vampire.h"
#include "WeatherManager.h"
#include "Zone.h"
#include "ZoneGroup.h"
#include "ZoneGroupManager.h"
#include "ZoneInfo.h"
#include "ZoneInfoManager.h"
#include "ZonePlayerManager.h"
#include "ZoneUtil.h"
#include "quest/Trigger.h"


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class isSameZoneID {
public:
    // constructor
    isSameZoneID(ZoneID_t ZoneID) : m_ZoneID(ZoneID) {}

    // operator
    bool operator()(PortalTargetInfo* pPortalTargetInfo) {
        return pPortalTargetInfo->getZoneID() == m_ZoneID;
    }

private:
    // EffectClass identifier
    ZoneID_t m_ZoneID;
};


//////////////////////////////////////////////////////////////////////////////
//
// class PrivatePortal member methods
//
//////////////////////////////////////////////////////////////////////////////

PrivatePortal::PrivatePortal(ObjectID_t objectID)

    : Portal(objectID) {
    __BEGIN_TRY

    m_pTarget = new PortalTargetInfo();

    __END_CATCH
}

PrivatePortal::~PrivatePortal()

{
    __BEGIN_TRY

    SAFE_DELETE(m_pTarget);

    __END_CATCH_NO_RETHROW
}

ZoneID_t PrivatePortal::getZoneID() const

{
    __BEGIN_TRY

    Assert(m_pTarget != NULL);
    return m_pTarget->getZoneID();

    __END_CATCH
}

void PrivatePortal::setZoneID(ZoneID_t zoneID)

{
    __BEGIN_TRY

    Assert(m_pTarget != NULL);
    m_pTarget->setZoneID(zoneID);

    __END_CATCH
}


ZoneCoord_t PrivatePortal::getX() const

{
    __BEGIN_TRY

    Assert(m_pTarget != NULL);
    return m_pTarget->getX();

    __END_CATCH
}

void PrivatePortal::setX(ZoneCoord_t x)

{
    __BEGIN_TRY

    Assert(m_pTarget != NULL);
    m_pTarget->setX(x);

    __END_CATCH
}

ZoneCoord_t PrivatePortal::getY() const

{
    __BEGIN_TRY

    Assert(m_pTarget != NULL);
    return m_pTarget->getY();

    __END_CATCH
}

void PrivatePortal::setY(ZoneCoord_t y)

{
    __BEGIN_TRY

    Assert(m_pTarget != NULL);
    m_pTarget->setY(y);

    __END_CATCH
}


bool PrivatePortal::activate(Creature* pCreature)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    Assert(pCreature != NULL);
    Assert(m_pTarget != NULL);
    Assert(pCreature->isPC());

    transportCreature(pCreature, m_pTarget->getZoneID(), m_pTarget->getX(), m_pTarget->getY(), true);

    return true;

    __END_DEBUG
    __END_CATCH
}

string PrivatePortal::toString() const

{
    StringStream msg;

    msg << "NormalPortal(" << "Type:" << (int)m_PortalType << "ZoneID:" << (int)m_pTarget->getZoneID()
        << ",X:" << (int)m_pTarget->getX() << ",Y:" << (int)m_pTarget->getY() << ")";

    return msg.toString();
}


//////////////////////////////////////////////////////////////////////////////
//
// class NormalPortal member methods
//
//////////////////////////////////////////////////////////////////////////////

bool NormalPortal::activate(Creature* pCreature)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    Assert(pCreature != NULL);
    Assert(m_pTarget != NULL);
    Assert(pCreature->isPC());

    switch (getObjectType()) {
    case PORTAL_SLAYER:
        if (!pCreature->isSlayer() && !pCreature->isFlag(Effect::EFFECT_CLASS_CAN_ENTER_SLAYER_PORTAL))
            return false;
        if (pCreature->isSlayer() && dynamic_cast<Slayer*>(pCreature)->hasRideMotorcycle())
            return false;
        break;
    case PORTAL_VAMPIRE:
        if (!pCreature->isVampire() && !pCreature->isFlag(Effect::EFFECT_CLASS_CAN_ENTER_VAMPIRE_PORTAL))
            return false;
        break;
    case PORTAL_OUSTERS:
        if (!pCreature->isOusters() && !pCreature->isFlag(Effect::EFFECT_CLASS_CAN_ENTER_OUSTERS_PORTAL))
            return false;
        break;
    case PORTAL_NORMAL:
        break;
    default:
        return false;
    }

    transportCreature(pCreature, m_pTarget->getZoneID(), m_pTarget->getX(), m_pTarget->getY(), true);

    return true;

    __END_DEBUG
    __END_CATCH
}

string NormalPortal::toString() const

{
    StringStream msg;

    msg << "NormalPortal(" << "Type:" << (int)m_PortalType << "ZoneID:" << (int)m_pTarget->getZoneID()
        << ",X:" << (int)m_pTarget->getX() << ",Y:" << (int)m_pTarget->getY() << ")";

    return msg.toString();
}


//////////////////////////////////////////////////////////////////////////////
//
// class GuildPortal member methods
//
//////////////////////////////////////////////////////////////////////////////

bool GuildPortal::activate(Creature* pCreature)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    Assert(pCreature != NULL);
    Assert(m_pTarget != NULL);
    Assert(pCreature->isPC());

    static const uint TEAM_ZONEID = 20001;
    static const uint CLAN_ZONEID = 20001;

    PlayerCreature* pPlayerCreature = dynamic_cast<PlayerCreature*>(pCreature);
    Assert(pPlayerCreature != NULL);

    Guild* pGuild = g_pGuildManager->getGuild(pPlayerCreature->getGuildID());

    // Ignore if the player has no guild.
    if (pGuild == NULL)
        return false;

    // Ignore if the guild's hideout is not on this server.
    if (pGuild->getServerGroupID() != g_pConfig->getPropertyInt("ServerID"))
        return false;

    // Ignore if the guild is not in the active state.
    if (pGuild->getState() != Guild::GUILD_STATE_ACTIVE)
        return false;

    // The Guild Portal ought to come in Slayer and Vampire kinds; for now the Zone ID tells them apart.
    // Check that the portal belongs to the player's own race.
    if ((pGuild->getRace() == Guild::GUILD_RACE_SLAYER && m_pTarget->getZoneID() == TEAM_ZONEID) ||
        (pGuild->getRace() == Guild::GUILD_RACE_VAMPIRE && m_pTarget->getZoneID() == CLAN_ZONEID)) {
        transportCreature(pCreature, pGuild->getZoneID(), m_pTarget->getX(), m_pTarget->getY(), true);
    } else {
        return false;
    }

    return true;

    __END_DEBUG
    __END_CATCH
}

string GuildPortal::toString() const

{
    StringStream msg;

    msg << "GuildPortal(" << "Type:" << (int)m_PortalType << ",X:" << (int)m_pTarget->getX()
        << ",Y:" << (int)m_pTarget->getY() << ")";

    return msg.toString();
}


//////////////////////////////////////////////////////////////////////////////
//
// class MultiPortal member methods
//
//////////////////////////////////////////////////////////////////////////////

bool MultiPortal::activate(Creature* pCreature, ZoneID_t ZoneID)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    Assert(pCreature->isPC());

    list<PortalTargetInfo*>::iterator itr = find_if(m_Targets.begin(), m_Targets.end(), isSameZoneID(ZoneID));

    if (itr == m_Targets.end()) {
        // Not found.
        return false;
    }

    Assert((*itr) != NULL);

    PortalTargetInfo* pPortalTargetInfo = *itr; // by sigi. 2002.5.8

    transportCreature(pCreature, pPortalTargetInfo->getZoneID(), pPortalTargetInfo->getX(), pPortalTargetInfo->getY(),
                      true);

    return true;

    __END_DEBUG
    __END_CATCH
}

string MultiPortal::toString() const

{
    StringStream msg;

    msg << "MultiPortal(" << "Type:" << (int)m_PortalType;
    for (list<PortalTargetInfo*>::const_iterator itr = m_Targets.begin(); itr != m_Targets.end(); itr++) {
        msg << ",ZoneID:" << (int)(*itr)->getZoneID() << ",X:" << (int)(*itr)->getX() << ",Y:" << (int)(*itr)->getY();
    }
    msg << ")";

    return msg.toString();
}


//////////////////////////////////////////////////////////////////////////////
//
// class TriggeredPortal member methods
//
//////////////////////////////////////////////////////////////////////////////

bool TriggeredPortal::activate(Creature* pCreature)

{
    __BEGIN_TRY

    list<Trigger*>& triggers = m_TriggerManager.getTriggers();
    list<Trigger*>::iterator itr = triggers.begin();

    // Looping here is in fact pointless:
    // if the very first trigger fails its condition, the counter action
    // runs and the function returns.
    for (; itr != triggers.end(); itr++) {
        Trigger* pTrigger = *itr;

        if (pTrigger->isAllSatisfied(Trigger::PASSIVE_TRIGGER, NULL, pCreature)) {
            if (pTrigger->activate(NULL, pCreature)) {
                return true;
            }
        } else {
            if (pTrigger->activateCounter(NULL, pCreature)) {
                return false;
            }
        }
    }

    return false;

    __END_CATCH
}

void TriggeredPortal::load(ZoneID_t zoneid, int left, int top, int right, int bottom)

{
    __BEGIN_TRY

    m_TriggerManager.load(zoneid, left, top, right, bottom);

    __END_CATCH
}

string TriggeredPortal::toString() const {
    __BEGIN_TRY

    StringStream msg;
    return msg.toString();

    __END_CATCH
}
