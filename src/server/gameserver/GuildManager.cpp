////////////////////////////////////////////////////////////////////////
// Filename    : GuildManager.cpp
// Written By  : 김성민
// Description :
////////////////////////////////////////////////////////////////////////

#include "GuildManager.h"

#include "Guild.h"
#include "Properties.h"
#include "StringStream.h"
#include "repository/GuildRepository.h"

#ifdef __GAME_SERVER__
#include "CastleInfoManager.h"
#include "GuildUnion.h"
#include "PlayerCreature.h"
#include "Zone.h"
#include "ZoneGroupManager.h"
#include "ZoneInfoManager.h"
#include "ZoneUtil.h"
#include "war/WarScheduler.h"
#endif
#ifdef __SHARED_SERVER__
#include "GameServerManager.h"
#include "SGExpelGuildMemberOK.h"
#include "SGGuildInfo.h"
#endif

#include "GCActiveGuildList.h"
#include "GCWaitGuildList.h"

////////////////////////////////////////////////////////////////////////
// global varible initialization
////////////////////////////////////////////////////////////////////////

GuildManager* g_pGuildManager = NULL;


////////////////////////////////////////////////////////////////////////
// class GuildManager member methods
////////////////////////////////////////////////////////////////////////

GuildManager::GuildManager()

{
    __BEGIN_TRY

    m_Mutex.setName("GuildManager");

    __END_CATCH
}

GuildManager::~GuildManager()

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    // 모든 길드 객체들을 메모리에서 삭제한다.
    unordered_map<GuildID_t, Guild*>::iterator itr = m_Guilds.begin();
    for (; itr != m_Guilds.end(); itr++) {
        Guild* pGuild = itr->second;
        SAFE_DELETE(pGuild);
    }

    m_Guilds.clear();

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH_NO_RETHROW
}


void GuildManager::init()

{
    // The body (the MaxGuildID / per-race MaxZoneID probes, then load())
    // lived here under __SHARED_SERVER__, which no build of this file
    // defines: the sharedserver compiles its own GuildManager.cpp. Gone
    // with its SQL; the gameserver's GuildManager::init() was always empty.
}


void GuildManager::load()

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    GuildRepository& repository = defaultGuildRepository();

    {
        // Read the guilds from the DB.
        vector<GuildListRow> guilds = repository.loadGuildsInStates(Guild::GUILD_STATE_WAIT, Guild::GUILD_STATE_ACTIVE);

        for (size_t g = 0; g < guilds.size(); g++) {
            const GuildListRow& row = guilds[g];
            GuildState_t state = row.state;

            // Only guilds waiting for registration or active in this world are added.
            if (state == Guild::GUILD_STATE_WAIT || state == Guild::GUILD_STATE_ACTIVE) {
                Guild* pGuild = new Guild();

                pGuild->setID(row.id);
                pGuild->setName(row.name);
                pGuild->setType(row.type);
                pGuild->setRace(row.race);
                pGuild->setState(state);
                pGuild->setServerGroupID(row.serverGroupID);
                pGuild->setZoneID(row.zoneID);
                pGuild->setMaster(row.master);
                pGuild->setDate(row.date);
                pGuild->setIntro(row.intro);

                addGuild_NOBLOCKED(pGuild);
                /*
                #ifdef __GAME_SERVER__
                                // 길드가 Active 이고 이 게임 서버에 아지트가 존재한다면 아지트 Zone을 만든다.
                                if ( pGuild->getServerGroupID() == g_pConfig->getPropertyInt("ServerID") && state ==
                Guild::GUILD_STATE_ACTIVE )
                                {
                                    //////////////
                                    // Zone Info
                                    //////////////
                                    ZoneInfo* pZoneInfo = new ZoneInfo();
                                    pZoneInfo->setZoneID( pGuild->getZoneID() );
                                    pZoneInfo->setZoneGroupID( 6 );
                                    pZoneInfo->setZoneType( "NPC_SHOP" );
                                    pZoneInfo->setZoneLevel( 0 );
                                    pZoneInfo->setZoneAccessMode( "PUBLIC" );
                                    pZoneInfo->setZoneOwnerID( "" );
                                    pZoneInfo->setPayPlay( "" );
                                    if ( pGuild->getRace() == Guild::GUILD_RACE_SLAYER )
                                    {
                                        pZoneInfo->setSMPFilename( "team_hdqrs.smp" );
                                        pZoneInfo->setSSIFilename( "team_hdqrs.ssi" );
                                        string Name = "team - " + pGuild->getName();
                                        pZoneInfo->setFullName( Name );
                                        pZoneInfo->setShortName( Name );
                                    }
                                    else if ( pGuild->getRace() == Guild::GUILD_RACE_VAMPIRE )
                                    {
                                        pZoneInfo->setSMPFilename( "clan_hdqrs.smp" );
                                        pZoneInfo->setSSIFilename( "clan_hdqrs.ssi" );
                                        string Name = "clan - " + pGuild->getName();
                                        pZoneInfo->setFullName( Name );
                                        pZoneInfo->setShortName( Name );
                                    }

                                    g_pZoneInfoManager->addZoneInfo( pZoneInfo );

                                    /////////
                                    // Zone
                                    /////////
                                    Zone* pZone = new Zone( pGuild->getZoneID() );
                                    Assert( pZone != NULL );

                                    ZoneGroup* pZoneGroup = g_pZoneGroupManager->getZoneGroup(6);
                                    Assert( pZoneGroup != NULL );

                                    pZone->setZoneGroup( pZoneGroup );
                                    pZoneGroup->addZone( pZone );
                                    pZone->init();
                                }
                #endif
                */
            }
        }

        // Read the guild members from the DB.
        vector<GuildMemberListRow> members = repository.loadActiveMembers();

        for (size_t m = 0; m < members.size(); m++) {
            GuildMember* pMember = new GuildMember();

            pMember->setGuildID(members[m].guildID);
            pMember->setName(members[m].name);
            pMember->setRank(members[m].rank);

            if (pMember->getRank() == GuildMember::GUILDMEMBER_RANK_WAIT)
                pMember->setRequestDateTime(members[m].requestDateTime);

            pMember->setLogOn(members[m].logOn);

            Guild* pGuild = getGuild_NOBLOCKED(pMember->getGuildID());

            if (pGuild != NULL)
                pGuild->addMember(pMember);
        }
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}


void GuildManager::addGuild(Guild* pGuild) {
    __BEGIN_TRY

    Assert(pGuild != NULL);

    __ENTER_CRITICAL_SECTION(m_Mutex)

    unordered_map<GuildID_t, Guild*>::iterator itr = m_Guilds.find(pGuild->getID());
    if (itr != m_Guilds.end())
        throw DuplicatedException();
    m_Guilds[pGuild->getID()] = pGuild;

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}


void GuildManager::addGuild_NOBLOCKED(Guild* pGuild) {
    __BEGIN_TRY

    Assert(pGuild != NULL);

    unordered_map<GuildID_t, Guild*>::iterator itr = m_Guilds.find(pGuild->getID());
    if (itr != m_Guilds.end())
        throw DuplicatedException();
    m_Guilds[pGuild->getID()] = pGuild;

    __END_CATCH
}


void GuildManager::deleteGuild(GuildID_t id) {
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    unordered_map<GuildID_t, Guild*>::iterator itr = m_Guilds.find(id);
    if (itr == m_Guilds.end())
        throw NoSuchElementException();

#ifdef __GAME_SERVER__

    list<CastleInfo*> pGuildCastleInfoList = g_pCastleInfoManager->getGuildCastleInfos(id);

    if (!pGuildCastleInfoList.empty()) {
        // 성을 갖고 있는 길드다.. 공용성으로 바꿔줘야 된다.
        list<CastleInfo*>::iterator itr = pGuildCastleInfoList.begin();
        for (; itr != pGuildCastleInfoList.end(); itr++) {
            if ((*itr)->getRace() == RACE_SLAYER)
                g_pCastleInfoManager->modifyCastleOwner((*itr)->getZoneID(), RACE_SLAYER, 99);
            else if ((*itr)->getRace() == RACE_VAMPIRE)
                g_pCastleInfoManager->modifyCastleOwner((*itr)->getZoneID(), RACE_VAMPIRE, 0);
            else
                g_pCastleInfoManager->modifyCastleOwner((*itr)->getZoneID(), RACE_OUSTERS, 66);
        }
    }

    {
        const unordered_map<ZoneID_t, CastleInfo*>& castleInfos = g_pCastleInfoManager->getCastleInfos();

        unordered_map<ZoneID_t, CastleInfo*>::const_iterator itr = castleInfos.begin();
        unordered_map<ZoneID_t, CastleInfo*>::const_iterator endItr = castleInfos.end();

        for (; itr != endItr; ++itr) {
            Zone* pZone = getZoneByZoneID(itr->first);
            if (pZone != NULL) {
                WarScheduler* pWarScheduler = pZone->getWarScheduler();
                if (pWarScheduler != NULL && pWarScheduler->hasSchedule(id)) {
                    pWarScheduler->load();
                }
            }
        }
    }

    // GuildUnion 정보를 지워준다
/*	{

        // UnionManager->deleteGuild(xx);
        GuildUnionManager::Instance().removeMasterGuild(id);
    }
*/
#endif

    m_Guilds.erase(itr);

    // The sharedserver-only DB purge of the guild's rows lived here under
    // __SHARED_SERVER__, which no build of this file defines. Gone with its
    // SQL.

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}


Guild* GuildManager::getGuild(GuildID_t id)

{
    __BEGIN_TRY

    // 리턴 할 길드 포인터
    Guild* pGuild;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    unordered_map<GuildID_t, Guild*>::iterator itr = m_Guilds.find(id);

    if (itr == m_Guilds.end()) {
        m_Mutex.unlock();

        return NULL;
    }

    pGuild = itr->second;

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return pGuild;

    __END_CATCH
}


Guild* GuildManager::getGuild_NOBLOCKED(GuildID_t id)

{
    __BEGIN_TRY

    // 리턴 할 길드 포인터
    Guild* pGuild;

    unordered_map<GuildID_t, Guild*>::iterator itr = m_Guilds.find(id);

    if (itr == m_Guilds.end()) {
        return NULL;
    }

    pGuild = itr->second;

    return pGuild;

    __END_CATCH
}


void GuildManager::clear()

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    HashMapGuildItor itr = m_Guilds.begin();
    for (; itr != m_Guilds.end(); itr++) {
        SAFE_DELETE(itr->second);
    }

    m_Guilds.clear();

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

void GuildManager::clear_NOBLOCKED() {
    __BEGIN_TRY

    HashMapGuildItor itr = m_Guilds.begin();
    for (; itr != m_Guilds.end(); itr++) {
        SAFE_DELETE(itr->second);
    }

    m_Guilds.clear();

    __END_CATCH
}

#ifdef __SHARED_SERVER__
void GuildManager::makeSGGuildInfo(SGGuildInfo& sgGuildInfo)

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    HashMapGuildConstItor itr = m_Guilds.begin();
    for (; itr != m_Guilds.end(); itr++) {
        GuildInfo2* pGuildInfo = new GuildInfo2();
        itr->second->makeInfo(pGuildInfo);
        sgGuildInfo.addGuildInfo(pGuildInfo);
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}
#endif

void GuildManager::makeWaitGuildList(GCWaitGuildList& gcWaitGuildList, GuildRace_t race)

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    HashMapGuildConstItor itr = m_Guilds.begin();
    for (; itr != m_Guilds.end(); itr++) {
        Guild* pGuild = itr->second;
        if (pGuild->getState() == Guild::GUILD_STATE_WAIT && pGuild->getRace() == race) {
            GuildInfo* pGuildInfo = new GuildInfo();
            pGuild->makeInfo(pGuildInfo);

            gcWaitGuildList.addGuildInfo(pGuildInfo);
        }
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

void GuildManager::makeActiveGuildList(GCActiveGuildList& gcActiveGuildList, GuildRace_t race)

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    HashMapGuildConstItor itr = m_Guilds.begin();
    for (; itr != m_Guilds.end(); itr++) {
        Guild* pGuild = itr->second;
        if (pGuild->getState() == Guild::GUILD_STATE_ACTIVE && pGuild->getRace() == race) {
            GuildInfo* pGuildInfo = new GuildInfo();
            pGuild->makeInfo(pGuildInfo);

            gcActiveGuildList.addGuildInfo(pGuildInfo);
        }
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

void GuildManager::heartbeat()

{
    __BEGIN_TRY

#ifdef __SHARED_SERVER__
    Timeval currentTime;
    getCurrentTime(currentTime);

    ////////////////////////////////////////////////////////
    // 길드 가입 신청 대기 시간이 넘어간 멤버를 지운다.
    ////////////////////////////////////////////////////////
    if (currentTime > m_WaitMemberClearTime) {
        __ENTER_CRITICAL_SECTION(m_Mutex)

        VSDateTime currentDateTime = VSDateTime::currentDateTime();

        HashMapGuildConstItor itr = m_Guilds.begin();
        for (; itr != m_Guilds.end(); itr++) {
            Guild* pGuild = itr->second;

            list<string> mList;

            pGuild->expireTimeOutWaitMember(currentDateTime, mList);

            list<string>::const_iterator itr2 = mList.begin();

            for (; itr2 != mList.end(); itr2++) {
                // 가입이 취소되었음을 게임서버에 알린다.
                SGExpelGuildMemberOK sgExpelGuildMemberOK;
                sgExpelGuildMemberOK.setGuildID(pGuild->getID());
                sgExpelGuildMemberOK.setName(*itr2);
                sgExpelGuildMemberOK.setSender(pGuild->getMaster());

                g_pGameServerManager->broadcast(&sgExpelGuildMemberOK);
            }
        }

        m_WaitMemberClearTime.tv_sec = currentTime.tv_sec + 3600; // 1시간 주기

        __LEAVE_CRITICAL_SECTION(m_Mutex)
    }
#endif

    __END_CATCH
}

string GuildManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    return msg.toString();

    __END_CATCH
}

bool GuildManager::isGuildMaster(GuildID_t guildID, PlayerCreature* pPC)

{
#ifdef __GAME_SERVER__
    __BEGIN_TRY

    Guild* pGuild = getGuild(guildID);

    if (pGuild == NULL)
        return false;

    //	cout << "isGuildMaster : " << pGuild->getMaster() << ", " << pPC->getName() << endl;
    return (pGuild->getMaster() == pPC->getName());

    __END_CATCH
#else
    return false;
#endif
}

// 길드가 성을 가졌나?
bool GuildManager::hasCastle(GuildID_t guildID)

{
    __BEGIN_TRY

    bool bHasCastle = false;

#ifdef __GAME_SERVER__

    if (defaultGuildRepository().countCastlesOfGuild((int)guildID) > 0) {
        bHasCastle = true;
    }

#endif

    return bHasCastle;

    __END_CATCH
}

// 길드가 성을 가졌나?
bool GuildManager::hasCastle(GuildID_t guildID, ServerID_t& serverID, ZoneID_t& zoneID)

{
    __BEGIN_TRY

    bool bHasCastle = false;

#ifdef __GAME_SERVER__

    int castleServerID = 0;
    int castleZoneID = 0;
    if (defaultGuildRepository().loadCastleOfGuild((int)guildID, castleServerID, castleZoneID)) {
        serverID = castleServerID;
        zoneID = castleZoneID;

        bHasCastle = true;
    }

#endif

    return bHasCastle;

    __END_CATCH
}

// 길드가 전쟁신청을 했나?
bool GuildManager::hasWarSchedule(GuildID_t guildID)

{
    __BEGIN_TRY

    bool bHasWarSchedule = false;

#ifdef __GAME_SERVER__

    // The __OLD_GUILD_WAR__ single-slot variant that lived here is gone: the
    // macro is commented out in Types.h and defined nowhere, so only the
    // five-slot read ever compiled.
    if (defaultGuildRepository().countWarSchedulesOfAttacker((int)guildID) > 0) {
        bHasWarSchedule = true;
    }

    if (defaultGuildRepository().countReinforceRegistrations((int)guildID) > 0) {
        bHasWarSchedule = true;
    }

#endif

    return bHasWarSchedule;

    __END_CATCH
}

bool GuildManager::hasActiveWar(GuildID_t guildID)

{
    __BEGIN_TRY

    bool bHasActiveWar = false;

#ifdef __GAME_SERVER__

    ServerID_t serverID;
    ZoneID_t zoneID;

    if (hasCastle(guildID, serverID, zoneID)) {
        // The guild owns a castle: is a guild war against that castle running?
        if (defaultGuildRepository().countStartedWarsAtCastle(serverID, zoneID) > 0) {
            bHasActiveWar = true;
        }
    } else {
        // No castle: is the guild attacking some castle in a running war?
        if (defaultGuildRepository().countStartedWarsOfAttacker((int)guildID) > 0) {
            bHasActiveWar = true;
        }
    }

#endif

    return bHasActiveWar;

    __END_CATCH
}


string GuildManager::getGuildName(GuildID_t guildID)

{
    __BEGIN_TRY

    Guild* pGuild = getGuild(guildID);

    if (pGuild != NULL)
        return pGuild->getName();

    return "";

    __END_CATCH
}
