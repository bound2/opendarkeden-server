////////////////////////////////////////////////////////////////////////
// Filename    : GuildManager.cpp
// Written By  : �輺��
// Description :
////////////////////////////////////////////////////////////////////////

#include "GuildManager.h"

#include "Guild.h"
#include "Properties.h"
#include "StringStream.h"
#include "repository/SharedGuildRepository.h"

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

GuildManager::GuildManager() noexcept {
    m_Mutex.setName("GuildManager");
}

GuildManager::~GuildManager() noexcept {
    try {
        __ENTER_CRITICAL_SECTION(m_Mutex)

        // ��� ��� ��ü���� �޸𸮿��� �����Ѵ�.
        unordered_map<GuildID_t, Guild*>::iterator itr = m_Guilds.begin();
        for (; itr != m_Guilds.end(); itr++) {
            Guild* pGuild = itr->second;
            SAFE_DELETE(pGuild);
        }

        m_Guilds.clear();

        __LEAVE_CRITICAL_SECTION(m_Mutex)
    } catch (...) {
        // destructor must not throw
    }
}


void GuildManager::init() noexcept(false) {
#ifdef __SHARED_SERVER__

    __BEGIN_TRY

    SharedGuildRepository& repo = defaultSharedGuildRepository();

    __ENTER_CRITICAL_SECTION(m_Mutex)

    // New guild ids are handed out above the largest one in the table, so
    // the manager reads that maximum once at startup. An empty table starts
    // the numbering from the configured dimension and world.
    if (repo.countGuilds() == 0) {
        Guild::setMaxGuildID(g_pConfig->getPropertyInt("Dimension") * 10000 +
                             g_pConfig->getPropertyInt("WorldID") * 3000 + 100);
    } else {
        Guild::setMaxGuildID(repo.loadMaxGuildID());
    }

    if (repo.countGuildsOfRace(Guild::GUILD_RACE_SLAYER) == 0) {
        Guild::setMaxSlayerZoneID(Guild::getMaxSlayerZoneID() + 1);
    } else {
        Guild::setMaxSlayerZoneID(
            max(repo.loadMaxGuildZoneIDOfRace(Guild::GUILD_RACE_SLAYER), Guild::getMaxSlayerZoneID() + 1));
    }

    if (repo.countGuildsOfRace(Guild::GUILD_RACE_VAMPIRE) == 0) {
        Guild::setMaxVampireZoneID(Guild::getMaxVampireZoneID() + 1);
    } else {
        Guild::setMaxVampireZoneID(
            max(repo.loadMaxGuildZoneIDOfRace(Guild::GUILD_RACE_VAMPIRE), Guild::getMaxVampireZoneID() + 1));
    }

    if (repo.countGuildsOfRace(Guild::GUILD_RACE_OUSTERS) == 0) {
        Guild::setMaxOustersZoneID(Guild::getMaxOustersZoneID() + 1);
    } else {
        Guild::setMaxOustersZoneID(
            max(repo.loadMaxGuildZoneIDOfRace(Guild::GUILD_RACE_OUSTERS), Guild::getMaxOustersZoneID() + 1));
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    load();

    __END_CATCH

#endif
}


void GuildManager::load() noexcept(false) {
    __BEGIN_TRY

    SharedGuildRepository& repo = defaultSharedGuildRepository();

    __ENTER_CRITICAL_SECTION(m_Mutex)

    // Only the waiting and the active guilds are kept in memory.
    vector<SharedGuildListRow> guilds = repo.loadGuildsInStates(Guild::GUILD_STATE_WAIT, Guild::GUILD_STATE_ACTIVE);

    for (size_t i = 0; i < guilds.size(); i++) {
        const SharedGuildListRow& row = guilds[i];
        GuildState_t state = row.state;

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
        }
    }

    // The roster: every member row whose guild is in memory joins it; a
    // member of a guild that was not loaded is never attached or freed.
    vector<SharedGuildMemberListRow> members = repo.loadActiveMembers();

    for (size_t i = 0; i < members.size(); i++) {
        const SharedGuildMemberListRow& row = members[i];
        GuildMember* pMember = new GuildMember();

        pMember->setGuildID(row.guildID);
        pMember->setName(row.name);
        pMember->setRank(row.rank);

        if (pMember->getRank() == GuildMember::GUILDMEMBER_RANK_WAIT)
            pMember->setRequestDateTime(row.requestDateTime);

        pMember->setLogOn(row.logOn);

        Guild* pGuild = getGuild_NOBLOCKED(pMember->getGuildID());

        if (pGuild != NULL)
            pGuild->addMember(pMember);
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}


void GuildManager::addGuild(Guild* pGuild) noexcept(false) {
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


void GuildManager::addGuild_NOBLOCKED(Guild* pGuild) noexcept(false) {
    __BEGIN_TRY

    Assert(pGuild != NULL);

    unordered_map<GuildID_t, Guild*>::iterator itr = m_Guilds.find(pGuild->getID());
    if (itr != m_Guilds.end())
        throw DuplicatedException();
    m_Guilds[pGuild->getID()] = pGuild;

    __END_CATCH
}


void GuildManager::deleteGuild(GuildID_t id) noexcept(false) {
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    unordered_map<GuildID_t, Guild*>::iterator itr = m_Guilds.find(id);
    if (itr == m_Guilds.end())
        throw NoSuchElementException();

    m_Guilds.erase(itr);

#ifdef __SHARED_SERVER__
    defaultSharedGuildRepository().purgeGuild(id);
#endif

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}


Guild* GuildManager::getGuild(GuildID_t id) noexcept(false) {
    __BEGIN_TRY

    // ���� �� ��� ������
    Guild* pGuild;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    unordered_map<GuildID_t, Guild*>::iterator itr = m_Guilds.find(id);

    if (itr == m_Guilds.end()) {
        return NULL;
    }

    pGuild = itr->second;

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return pGuild;

    __END_CATCH
}


Guild* GuildManager::getGuild_NOBLOCKED(GuildID_t id) noexcept(false) {
    __BEGIN_TRY

    // ���� �� ��� ������
    Guild* pGuild;

    unordered_map<GuildID_t, Guild*>::iterator itr = m_Guilds.find(id);

    if (itr == m_Guilds.end()) {
        return NULL;
    }

    pGuild = itr->second;

    return pGuild;

    __END_CATCH
}


void GuildManager::clear() noexcept(false) {
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
void GuildManager::makeSGGuildInfo(SGGuildInfo& sgGuildInfo) noexcept(false) {
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

void GuildManager::makeWaitGuildList(GCWaitGuildList& gcWaitGuildList, GuildRace_t race) noexcept(false) {
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

void GuildManager::makeActiveGuildList(GCActiveGuildList& gcActiveGuildList, GuildRace_t race) noexcept(false) {
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

void GuildManager::heartbeat() noexcept(false) {
    __BEGIN_TRY

#ifdef __SHARED_SERVER__
    Timeval currentTime;
    getCurrentTime(currentTime);

    ////////////////////////////////////////////////////////
    // ��� ���� ��û ��� �ð��� �Ѿ ����� �����.
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
                // ������ ��ҵǾ����� ���Ӽ����� �˸���.
                SGExpelGuildMemberOK sgExpelGuildMemberOK;
                sgExpelGuildMemberOK.setGuildID(pGuild->getID());
                sgExpelGuildMemberOK.setName(*itr2);
                sgExpelGuildMemberOK.setSender(pGuild->getMaster());

                g_pGameServerManager->broadcast(&sgExpelGuildMemberOK);
            }
        }

        m_WaitMemberClearTime.tv_sec = currentTime.tv_sec + 3600; // 1�ð� �ֱ�

        __LEAVE_CRITICAL_SECTION(m_Mutex)
    }
#endif

    __END_CATCH
}

string GuildManager::toString() const noexcept {
    StringStream msg;
    return msg.toString();
}

bool GuildManager::isGuildMaster(GuildID_t guildID, PlayerCreature* pPC) noexcept(false) {
    return false;
}

// ��尡 ���� ������?
bool GuildManager::hasCastle(GuildID_t guildID) noexcept(false) {
    return false;
}

// ��尡 ���� ������?
bool GuildManager::hasCastle(GuildID_t guildID, ServerID_t& serverID, ZoneID_t& zoneID) noexcept(false) {
    return false;
}

// ��尡 �����û�� �߳�?
bool GuildManager::hasWarSchedule(GuildID_t guildID) noexcept(false) {
    return false;
}

bool GuildManager::hasActiveWar(GuildID_t guildID) noexcept(false) {
    return false;
}


string GuildManager::getGuildName(GuildID_t guildID) noexcept(false) {
    __BEGIN_TRY

    Guild* pGuild = getGuild(guildID);

    if (pGuild != NULL)
        return pGuild->getName();

    return "";

    __END_CATCH
}
