///////////////////////////////////////////////////////////////////
// General war information and the routines run when a war starts and ends
///////////////////////////////////////////////////////////////////

#include "RaceWar.h"

#include <stdio.h>

#include "Assert.h"
#include "DB.h"
#include "Mutex.h"
#include "Properties.h"
#include "WarSystem.h"
#include "ZoneGroupManager.h"
#include "repository/WarInfoRepository.h"
// #include "HolyLandRaceBonus.h"
#include "CastleInfoManager.h"
#include "DragonEyeManager.h"
#include "GCNoticeEvent.h"
#include "GCSystemMessage.h"
#include "GCWarScheduleList.h"
#include "GameContext.h"
#include "HolyLandManager.h"
#include "PCManager.h"
#include "RaceWarInfo.h"
#include "RaceWarLimiter.h"
#include "RegenZoneManager.h"
#include "ShrineInfoManager.h"
#include "StringStream.h"
#include "VariableManager.h"
#include "Zone.h"
#include "ZoneGroup.h"
#include "ZoneInfoManager.h"
#include "ZoneUtil.h"
#include "gm/GMCommands.h"

//--------------------------------------------------------------------------------
//
// constructor / destructor
//
//--------------------------------------------------------------------------------
RaceWar::RaceWar(WarState warState, WarID_t warID) : War(warState, warID) {}

RaceWar::~RaceWar() {}

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
void RaceWar::executeStart()

{
    __BEGIN_TRY

    sendWarStartMessage();

    // Broadcasting the blood bible positions and sending out the players who
    // are not taking part belong to WarSystem::addWar(): hasActiveRaceWar()
    // is not set yet while this runs.

    // During a war, fighting inside the castle is free
    de::gameContext().castleInfos().releaseAllSafeZone();

    // Every guardian shrine shield disappears.
    g_pShrineInfoManager->removeAllShrineShield();


    // Fix the time across Adam's holy land.
    g_pHolyLandManager->fixTimeband(de::gameContext().variables().getVariable(RACE_WAR_TIMEBAND));

    g_pHolyLandManager->killAllMonsters();

    RegenZoneManager::getInstance()->putTryingPosition();
    RegenZoneManager::getInstance()->broadcastStatus();

    // Put the dragon eye items at their initial positions.
    de::gameContext().dragonEyes().addAllDragonEyesToZone();


    // Record in the RaceWarHistory Table
    recordRaceWarStart();

    __END_CATCH
}

void RaceWar::recordRaceWarStart()

{
    __BEGIN_TRY

    vector<RaceCurrentNumRow> raceNums = defaultWarInfoRepository().loadRaceWarCurrentNums();

    uint slayerSum = 0;
    uint vampireSum = 0;
    uint oustersSum = 0;

    string slayerOld;
    string vampireOld;
    string oustersOld;

    for (size_t r = 0; r < raceNums.size(); r++) {
        uint race = raceNums[r].race;
        uint num = raceNums[r].currentNum;

        if (race == 0)
            slayerSum = num;
        else if (race == 1)
            vampireSum = num;
        else if (race == 2)
            oustersSum = num;
    }

    vector<ShrineOwnerRow> owners = defaultWarInfoRepository().loadShrineOwners();

    for (size_t r = 0; r < owners.size(); r++) {
        uint id = owners[r].id;
        uint ownerRace = owners[r].ownerRace;

        if (ownerRace == 0)
            slayerOld = slayerOld + itos(id) + "|";
        else if (ownerRace == 1)
            vampireOld = vampireOld + itos(id) + "|";
        else if (ownerRace == 2)
            oustersOld = oustersOld + itos(id) + "|";
    }

    defaultWarInfoRepository().insertRaceWarHistory(getWarStartTime().toStringforWeb(), slayerSum, vampireSum,
                                                    oustersSum, slayerOld, vampireOld, oustersOld);

    __END_CATCH
}

//--------------------------------------------------------------------------------
//
// executeEnd
//
//--------------------------------------------------------------------------------
// What has to be handled when a war ends
//--------------------------------------------------------------------------------
void RaceWar::executeEnd()

{
    __BEGIN_TRY

    //----------------------------------------------------------------------------
    // Report that the war has ended.
    //----------------------------------------------------------------------------
    sendWarEndMessage();


    //----------------------------------------------------------------------------
    // Give the blood bible fragments back.
    //----------------------------------------------------------------------------
    g_pShrineInfoManager->returnAllBloodBible();

    g_pShrineInfoManager->addAllShrineShield();

    de::gameContext().castleInfos().resetAllSafeZone();

    de::gameContext().castleInfos().transportAllOtherRace();


    // Broadcast the blood bible positions across Adam's holy land.
    g_pShrineInfoManager->broadcastBloodBibleStatus();

    // Let the time that was fixed across Adam's holy land run again.
    g_pHolyLandManager->resumeTimeband();

    // Remove every entry from the war participant list.
    RaceWarLimiter::clearPCList();

    // Set the participant count back to 0.
    RaceWarLimiter::getInstance()->clearCurrent();
    RegenZoneManager::getInstance()->deleteTryingPosition();
    RegenZoneManager::getInstance()->reload();

    // Remove the Flag from every character too.
    de::gameContext().zoneGroups().removeFlag(Effect::EFFECT_CLASS_RACE_WAR_JOIN_TICKET);

    de::gm::opworld(NULL, "*world *load blood_bible_owner", 0, true);

    // Remove the dragon eye items.
    de::gameContext().dragonEyes().removeAllDragonEyes();

    // Record in the RaceWarHistory Table
    recordRaceWarEnd();

    __END_CATCH
}

void RaceWar::recordRaceWarEnd()

{
    __BEGIN_TRY

    vector<ShrineOwnerRow> owners = defaultWarInfoRepository().loadShrineOwners();

    string slayerNew;
    string vampireNew;
    string oustersNew;

    for (size_t r = 0; r < owners.size(); r++) {
        uint id = owners[r].id;
        uint ownerRace = owners[r].ownerRace;

        if (ownerRace == 0)
            slayerNew = slayerNew + itos(id) + "|";
        else if (ownerRace == 1)
            vampireNew = vampireNew + itos(id) + "|";
        else if (ownerRace == 2)
            oustersNew = oustersNew + itos(id) + "|";
    }

    defaultWarInfoRepository().updateRaceWarBloodBibles(slayerNew, vampireNew, oustersNew,
                                                        getWarStartTime().toStringforWeb());

    // running a script -- who would have thought the system function would be used
    char cmd[100];
    sprintf(cmd, "/home/darkeden/vs/bin/script/recordRaceWarHistory.py %s %d %d ",
            getWarStartTime().toStringforWeb().c_str(), g_pConfig->getPropertyInt("Dimension"),
            g_pConfig->getPropertyInt("WorldID"));

    filelog("script.log", cmd);
    system(cmd);

    __END_CATCH
}

string RaceWar::getWarName() const

{
    __BEGIN_TRY

    return "蘆痢쇌濫轢";

    __END_CATCH
}

//--------------------------------------------------------------------------------
// When the war ends
//--------------------------------------------------------------------------------
void RaceWar::sendWarEndMessage() const

{
    __BEGIN_TRY

    War::sendWarEndMessage();

    // The packet that confirms the safe zone release?
    GCNoticeEvent gcNoticeEvent;
    gcNoticeEvent.setCode(NOTICE_EVENT_RACE_WAR_OVER);
    de::gameContext().zoneGroups().broadcast(&gcNoticeEvent);

    __END_CATCH
}


void RaceWar::makeWarScheduleInfo(WarScheduleInfo* pWSI) const

{
    __BEGIN_TRY

    pWSI->warType = getWarType();

    __END_CATCH
}

void RaceWar::makeWarInfo(WarInfo* pWarInfo) const

{
    __BEGIN_TRY

    Assert(pWarInfo != NULL);
    Assert(pWarInfo->getWarType() == WAR_RACE);

    RaceWarInfo* pRaceWarInfo = dynamic_cast<RaceWarInfo*>(pWarInfo);
    Assert(pRaceWarInfo != NULL);

    const unordered_map<ZoneID_t, CastleInfo*>& castleInfos = de::gameContext().castleInfos().getCastleInfos();

    unordered_map<ZoneID_t, CastleInfo*>::const_iterator itr = castleInfos.begin();

    for (; itr != castleInfos.end(); itr++) {
        CastleInfo* pCastleInfo = itr->second;
        pRaceWarInfo->addCastleID(pCastleInfo->getZoneID());
    }

    __END_CATCH
}


string RaceWar::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "RaceWar(" << "WarID:" << (int)getWarID() << ",State:" << (int)getState() << ")";

    return msg.toString();

    __END_CATCH
}
