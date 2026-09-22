///////////////////////////////////////////////////////////////////
// General war information and the routines run when a war starts and ends
///////////////////////////////////////////////////////////////////

#include "GuildWar.h"

#include <stdio.h>

#include "Assert.h"
#include "CastleInfoManager.h"
#include "CastleShrineInfoManager.h"
#include "DB.h"
#include "GCNoticeEvent.h"
#include "GCSystemMessage.h"
#include "GCWarScheduleList.h"
#include "GameContext.h"
#include "Guild.h"
#include "GuildManager.h"
#include "GuildWarInfo.h"
#include "HolyLandRaceBonus.h"
#include "Mutex.h"
#include "PCManager.h"
#include "Properties.h"
#include "StringStream.h"
#include "WarSystem.h"
#include "Zone.h"
#include "ZoneGroup.h"
#include "ZoneGroupManager.h"
#include "ZoneInfoManager.h"
#include "ZoneUtil.h"
#include "gm/GMCommands.h"
#include "repository/WarInfoRepository.h"

//--------------------------------------------------------------------------------
//
// constructor / destructor
//
//--------------------------------------------------------------------------------
GuildWar::GuildWar(ZoneID_t castleZoneID, GuildID_t challenger, WarState warState, WarID_t warID)
    : War(warState, warID), m_CastleZoneID(castleZoneID), m_ChallangerGuildID(challenger), m_RegistrationFee(0) {
    m_bModifyCastleOwner = false;
}

GuildWar::~GuildWar() {}

//--------------------------------------------------------------------------------
//
// executeStart
//
//--------------------------------------------------------------------------------
// What has to be handled when a war starts
//
// (!) This runs in the WarScheduler attached to the Zone, so
//     handling its own Zone (the castle) needs no lock.
//--------------------------------------------------------------------------------
void GuildWar::executeStart()

{
    __BEGIN_TRY

    sendWarStartMessage();

    // Turn off the safe zone inside the castle.
    ZoneID_t guardShrineZoneID = de::gameContext().castleShrines().getGuardShrineZoneID(m_CastleZoneID);
    Zone* pZone = getZoneByZoneID(guardShrineZoneID);
    Assert(pZone != NULL);


    // This part would be better moved into CastleInfo later.
    CastleInfo* pCastleInfo = de::gameContext().castleInfos().getCastleInfo(m_CastleZoneID);
    Assert(pCastleInfo != NULL);

    GuildID_t OwnerGuildID = pCastleInfo->getGuildID();

    // Monsters are removed only when the castle is not a common one.
    if (OwnerGuildID != SlayerCommon && OwnerGuildID != VampireCommon) {
        const list<ZoneID_t>& zoneIDs = pCastleInfo->getZoneIDList();

        list<ZoneID_t>::const_iterator itr = zoneIDs.begin();
        for (; itr != zoneIDs.end(); itr++) {
            ZoneID_t targetZoneID = *itr;
            Zone* pTargetZone = getZoneByZoneID(targetZoneID);

            // Not the castle but a dungeon map..
            if (targetZoneID != m_CastleZoneID) {
                pTargetZone->killAllMonsters();
            }
        }
    }

    de::gameContext().castleShrines().removeShrineShield(pZone);

    // Record in the GuildWarHistory Table
    recordGuildWarStart();

    __END_CATCH
}

void GuildWar::recordGuildWarStart()

{
    __BEGIN_TRY

    CastleInfo* pCastleInfo = de::gameContext().castleInfos().getCastleInfo(m_CastleZoneID);

    // It cannot be NULL, but just in case
    if (pCastleInfo == NULL)
        return;

    defaultWarInfoRepository().insertGuildWarHistory(
        (int)getWarID(), getWarStartTime().toStringforWeb(), g_pConfig->getPropertyInt("ServerID"),
        pCastleInfo->getName(), (int)pCastleInfo->getGuildID(),
        g_pGuildManager->getGuildName(pCastleInfo->getGuildID()), getChallangerGuildID(),
        g_pGuildManager->getGuildName(getChallangerGuildID()));

    __END_CATCH
}

//--------------------------------------------------------------------------------
//
// executeEnd
//
//--------------------------------------------------------------------------------
// What has to be handled when a war ends
//--------------------------------------------------------------------------------
void GuildWar::executeEnd()

{
    __BEGIN_TRY

    CastleInfoManager& castleInfos = de::gameContext().castleInfos();

    //----------------------------------------------------------------------------
    // Report that the war has ended.
    //----------------------------------------------------------------------------
    sendWarEndMessage();

    //----------------------------------------------------------------------------
    // Change the castle owner
    //----------------------------------------------------------------------------
    if (m_bModifyCastleOwner) {
        castleInfos.modifyCastleOwner(m_CastleZoneID, m_WinnerRace, m_WinnerGuildID);

        if (g_pConfig->getPropertyInt("IsNetMarble") == 1) {
            char sCommand[100];
            sprintf(sCommand, "*world *command setCastleOwnerGuild %u %u", m_CastleZoneID, m_WinnerGuildID);
            de::gm::opworld(NULL, sCommand, 0, true);
        }
    } else {
        // Set WinnerGuildID to the current owner
        CastleInfo* pCastleInfo = castleInfos.getCastleInfo(m_CastleZoneID);
        m_WinnerGuildID = pCastleInfo->getGuildID();
    }

    //----------------------------------------------------------------------------
    // Give the castle symbol back.
    //----------------------------------------------------------------------------
    de::gameContext().castleShrines().returnAllCastleSymbol(m_CastleZoneID);

    //----------------------------------------------------------------------------
    // Restore the safe zone inside the castle
    //----------------------------------------------------------------------------
    // Called by the WarSystem on the ClientManager thread; the castle's zone is
    // not locked here, so this runs against the zone thread's tick.
    ZoneID_t guardShrineZoneID = de::gameContext().castleShrines().getGuardShrineZoneID(m_CastleZoneID);
    Zone* pZone = getZoneByZoneID(guardShrineZoneID);
    Assert(pZone != NULL);


    de::gameContext().castleShrines().addShrineShield(pZone);

    //----------------------------------------------------------------------------
    // The war application fee is piled onto the castle.
    // (it is assumed the castle owner changed with the war result.)
    //----------------------------------------------------------------------------
    castleInfos.increaseTaxBalance(m_CastleZoneID, m_RegistrationFee);
    m_RegistrationFee = 0;
    // tinysave("war application fee=0") <-- is that needed?

    // Record in the GuildWarHistory Table
    recordGuildWarEnd();

    __END_CATCH
}

void GuildWar::recordGuildWarEnd()

{
    __BEGIN_TRY

    defaultWarInfoRepository().updateGuildWarWinner((int)m_WinnerGuildID,
                                                    g_pGuildManager->getGuildName(m_WinnerGuildID), (int)getWarID());

    // running a script -- who would have thought the system function would be used
    char cmd[100];
    sprintf(cmd, "/home/darkeden/vs/bin/script/recordGuildWarHistory.py %d %d %d ", (int)getWarID(),
            g_pConfig->getPropertyInt("Dimension"), g_pConfig->getPropertyInt("WorldID"));

    filelog("script.log", cmd);
    system(cmd);

    __END_CATCH
}

string GuildWar::getWarName() const

{
    __BEGIN_TRY

    ZoneInfo* pZoneInfo = NULL;
    Guild* pGuild = NULL;

    try {
        pGuild = g_pGuildManager->getGuild(m_ChallangerGuildID);
        pZoneInfo = de::gameContext().zoneInfos().getZoneInfo(m_CastleZoneID);

        if (pGuild == NULL || pZoneInfo == NULL)
            return "길드 전쟁";
    } catch (Throwable& t) {
        return "길드 전쟁";
    }

    Assert(pZoneInfo != NULL);
    Assert(pGuild != NULL);

    StringStream msg;

    msg << pGuild->getName() << "길드가 ";
    msg << pZoneInfo->getFullName() << "을 공격하는 길드 전쟁";

    return msg.toString();

    __END_CATCH
}

//--------------------------------------------------------------------------------
//
//	isModifyCastleOwner( PlayerCreature* pPC )
//
//--------------------------------------------------------------------------------
// The case where the castle owner changes
//--------------------------------------------------------------------------------
bool GuildWar::isModifyCastleOwner(PlayerCreature* pPC)

{
    __BEGIN_TRY

    Assert(pPC != NULL);

    CastleInfo* pCastleInfo = de::gameContext().castleInfos().getCastleInfo(m_CastleZoneID);
    Assert(pCastleInfo != NULL);

    // common castle : attacking guild --> the attacking guild's castle
    // common castle : ordinary --> back to the original place
    // guild castle : attacking guild --> the attacking guild's castle
    // guild castle : defending guild --> back to the original place
    // guild castle : ordinary --> common castle

    // in a guild war : the guild that applied for the war, or
    // 					 an ordinary player when it is a guild castle
    if (pPC->getGuildID() == m_ChallangerGuildID ||
        (!pCastleInfo->isCommon() && pPC->getCommonGuildID() == pPC->getGuildID())) {
        return true;
    }

    return false;

    __END_CATCH
}


//--------------------------------------------------------------------------------
//
// getWinnerGuildID( PlayerCreature* pPC )
//
//--------------------------------------------------------------------------------
// Hands back the GuildID of the guild that won the war.
//--------------------------------------------------------------------------------
GuildID_t GuildWar::getWinnerGuildID(PlayerCreature* pPC)

{
    __BEGIN_TRY

    Assert(pPC != NULL);

    // in a guild war : pPC's GuildID when the applying guild is pPC's guild
    // 					 otherwise the original castle owner's GuildID when it matches that
    //					 otherwise COMMON_GUILD_ID
    CastleInfo* pCastleInfo = de::gameContext().castleInfos().getCastleInfo(m_CastleZoneID);
    Assert(pCastleInfo != NULL);

    if (m_ChallangerGuildID == pPC->getGuildID() || pPC->getGuildID() == pCastleInfo->getGuildID()) {
        return pPC->getGuildID();
    }

    return pPC->getCommonGuildID();

    __END_CATCH
}

bool GuildWar::endWar(PlayerCreature* pPC)

{
    __BEGIN_TRY

    Assert(pPC != NULL);

    // < castle owner change >
    if (isModifyCastleOwner(pPC)) {
        m_WinnerRace = pPC->getRace();
        m_WinnerGuildID = getWinnerGuildID(pPC);
        m_bModifyCastleOwner = true;

        return true;
    }

    return false;

    __END_CATCH
}

//--------------------------------------------------------------------------------
// When the war ends
//--------------------------------------------------------------------------------
void GuildWar::sendWarEndMessage() const

{
    __BEGIN_TRY

    War::sendWarEndMessage();

    // The packet that confirms the safe zone release?
    GCNoticeEvent gcNoticeEvent;
    gcNoticeEvent.setCode(NOTICE_EVENT_WAR_OVER);
    gcNoticeEvent.setParameter(m_CastleZoneID);
    de::gameContext().zoneGroups().broadcast(&gcNoticeEvent);

    __END_CATCH
}

void GuildWar::makeWarScheduleInfo(WarScheduleInfo* pWSI) const

{
    __BEGIN_TRY

    pWSI->warType = getWarType();
    pWSI->challengerGuildID[0] = getChallangerGuildID();
    for (int i = 1; i < 5; ++i)
        pWSI->challengerGuildID[i] = 0;
    pWSI->reinforceGuildID = 0;

    pWSI->challengerGuildName[0] = g_pGuildManager->getGuildName(getChallangerGuildID());

    __END_CATCH
}

void GuildWar::makeWarInfo(WarInfo* pWarInfo) const

{
    __BEGIN_TRY

    Assert(pWarInfo != NULL);
    Assert(pWarInfo->getWarType() == WAR_GUILD);

    GuildWarInfo* pGuildWarInfo = dynamic_cast<GuildWarInfo*>(pWarInfo);
    Assert(pGuildWarInfo != NULL);

    //---------------------------------------------------
    // Get the current castle owner
    //---------------------------------------------------
    CastleInfo* pCastleInfo = de::gameContext().castleInfos().getCastleInfo(getCastleZoneID());
    if (pCastleInfo == NULL) {
        filelog("WarError.log", "CastleInfo가 없다(%d)", getCastleZoneID());
        return;
    }

    GuildID_t ownGuildID = pCastleInfo->getGuildID();
    GuildID_t challangerGuildID = getChallangerGuildID();

    pGuildWarInfo->addJoinGuild(ownGuildID);        // the current castle owner
    pGuildWarInfo->addJoinGuild(challangerGuildID); // the guild attacking the castle
    pGuildWarInfo->setCastleID(getCastleZoneID());

    // The attacking guild's name
    static const string commonSlayerGuild("없음");
    static const string commonVampireGuild("없음");

    string attackGuildName;
    string defenseGuildName;

    if (challangerGuildID == SlayerCommon)
        attackGuildName = commonSlayerGuild;
    else if (challangerGuildID == VampireCommon)
        attackGuildName = commonVampireGuild;
    else
        attackGuildName = g_pGuildManager->getGuildName(getChallangerGuildID());

    if (ownGuildID == SlayerCommon)
        defenseGuildName = commonSlayerGuild;
    else if (ownGuildID == VampireCommon)
        defenseGuildName = commonVampireGuild;
    else
        defenseGuildName = g_pGuildManager->getGuildName(ownGuildID);

    pGuildWarInfo->setAttackGuildName(attackGuildName);
    pGuildWarInfo->setDefenseGuildName(defenseGuildName);

    __END_CATCH
}


string GuildWar::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GuildWar(" << "WarID:" << (int)getWarID() << ",State:" << (int)getState()
        << ",CastleZoneID:" << (int)m_CastleZoneID << ",WarType:" << getWarType2DBString()
        << ",ChallengerGuildID:" << (int)m_ChallangerGuildID << ",RegistrationFee:" << (int)m_RegistrationFee << ")";

    return msg.toString();

    __END_CATCH
}
