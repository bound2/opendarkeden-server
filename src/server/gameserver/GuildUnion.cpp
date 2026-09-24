#include "GuildUnion.h"

#include <stdio.h>

#include "GCModifyInformation.h"
#include "GGCommand.h"
#include "GameContext.h"
#include "GameServer.h"
#include "GameServerInfoManager.h"
#include "Guild.h"
#include "GuildManager.h"
#include "KernelContext.h"
#include "LoginServerManager.h"
#include "PCFinder.h"
#include "PacketUtil.h"
#include "Player.h"
#include "ServerContext.h"
#include "VariableManager.h"
#include "guild/GuildUnionTeardown.h"
#include "repository/GuildRepository.h"

namespace {

// The master character of a guild, or "" when the guild has gone. The union
// tables name guilds by id and a row can outlive the guild it names, so a
// union row pointing at a guild the GuildManager no longer holds is a stale
// row: it is logged and skipped, never dereferenced.
string guildMasterOf(GuildID_t gID) {
    Guild* pGuild = de::gameContext().guilds().getGuild(gID);

    if (pGuild == NULL) {
        filelog("GuildUnion.log", "[%u] union row names a guild that is gone.", gID);
        return "";
    }

    return pGuild->getMaster();
}

// Tell a guild its union standing changed: its master, if online, gets its
// own union info, and everyone the guild has online has the new standing
// broadcast around them. A guild that has gone has nobody to tell.
void notifyUnionChange(GuildID_t gID) {
    const string masterName = guildMasterOf(gID);

    if (!masterName.empty()) {
        PCFinder& pcFinder = de::gameContext().playerCreatures();

        __ENTER_CRITICAL_SECTION(pcFinder)

        Creature* pTargetCreature = pcFinder.getCreature_LOCKED(masterName);
        if (pTargetCreature != NULL) {
            GCModifyInformation gcModifyInformation;
            makeGCModifyInfoGuildUnion(&gcModifyInformation, pTargetCreature);
            pTargetCreature->getPlayer()->sendPacket(&gcModifyInformation);
        }

        __LEAVE_CRITICAL_SECTION(pcFinder)
    }

    sendGCOtherModifyInfoGuildUnionByGuildID(gID);
}

} // namespace

GuildUnion::~GuildUnion() {
    // cout << "GuildUnion : DELETE!!!" << endl;
}

bool GuildUnion::hasGuild(GuildID_t gID) const {
    if (gID == m_MasterGuildID)
        return true;

    if (findGuildItr(gID) != m_Guilds.end())
        return true;

    return false;
}

bool GuildUnion::addGuild(GuildID_t gID) {
    if (hasGuild(gID))
        return false;

    m_Guilds.push_back(gID);

    defaultGuildRepository().insertUnionMember(m_UnionID, gID);

    return true;
}

bool GuildUnion::removeGuild(GuildID_t gID) {
    if (m_MasterGuildID == gID)
        return false;

    list<GuildID_t>::iterator itr = findGuildItr(gID);
    if (itr == m_Guilds.end())
        return false;

    m_Guilds.erase(itr);

    if (!defaultGuildRepository().deleteUnionMember(m_UnionID, gID)) {
        filelog("GuildUnion.log", "[%u:%u] 탈퇴하려는데 해당 레코드가 없습니다.", m_UnionID, gID);
    }

    return true;
}

void GuildUnion::create() {
    __BEGIN_TRY

    GuildRepository& repository = defaultGuildRepository();

    m_UnionID = repository.insertUnion(m_MasterGuildID);

    list<GuildID_t>::iterator itr = m_Guilds.begin();

    for (; itr != m_Guilds.end(); ++itr) {
        repository.insertUnionMember(m_UnionID, (*itr));
    }

    __END_CATCH
}

void GuildUnion::destroy() {
    __BEGIN_TRY

    defaultGuildRepository().deleteUnion(m_UnionID);

    __END_CATCH
}

GuildUnionManager::GuildUnionManager() {
    m_Mutex.setName("GuildUnionManager");
}

GuildUnionManager::~GuildUnionManager() {
    list<GuildUnion*>::iterator itr = m_GuildUnionList.begin();

    for (; itr != m_GuildUnionList.end(); ++itr) {
        SAFE_DELETE((*itr));
    }
}

void GuildUnionManager::addGuildUnion(GuildUnion* pUnion) {
    m_GuildUnionList.push_back(pUnion);

    m_UnionIDMap[pUnion->getUnionID()] = pUnion;
    m_GuildUnionMap[pUnion->getMasterGuildID()] = pUnion;

    list<GuildID_t>::iterator itr = pUnion->m_Guilds.begin();

    for (; itr != pUnion->m_Guilds.end(); ++itr) {
        m_GuildUnionMap[*itr] = pUnion;
    }
}

void GuildUnionManager::sendModifyUnionInfo(uint gID) {
    char Msg[80];
    sprintf(Msg, "*modifyunioninfo %d", gID);

    GGCommand ggCommand;
    ggCommand.setCommand(Msg);


    // Send to each server.
    GameServerInfoManager& serverInfos = de::serverContext().serverInfos();
    HashMapGameServerInfo** pGameServerInfos = serverInfos.getGameServerInfos();


    static int myWorldID = de::kernelContext().config().getPropertyInt("WorldID");
    static int myServerID = de::kernelContext().config().getPropertyInt("ServerID");

    int maxWorldID = serverInfos.getMaxWorldID();
    int maxServerGroupID = serverInfos.getMaxServerGroupID();


    for (int worldID = 1; worldID < maxWorldID; worldID++) {
        for (int groupID = 0; groupID < maxServerGroupID; groupID++) {
            HashMapGameServerInfo& gameServerInfo = pGameServerInfos[worldID][groupID];

            if (!gameServerInfo.empty()) {
                HashMapGameServerInfo::const_iterator itr = gameServerInfo.begin();
                for (; itr != gameServerInfo.end(); itr++) {
                    GameServerInfo* pGameServerInfo = itr->second;

                    if (pGameServerInfo->getWorldID() == myWorldID) {
                        // Only for servers other than the current one (handled above).
                        if (pGameServerInfo->getGroupID() == myServerID) {
                        } else {
                            de::gameContext().loginServer().sendPacket(pGameServerInfo->getIP(),
                                                                       pGameServerInfo->getUDPPort(), &ggCommand);
                        }
                    }
                }
            }
        }
    }
}

void GuildUnionManager::sendRefreshCommand() {
    GGCommand ggCommand;
    ggCommand.setCommand("*refreshguildunion");


    // Send to each server.
    GameServerInfoManager& serverInfos = de::serverContext().serverInfos();
    HashMapGameServerInfo** pGameServerInfos = serverInfos.getGameServerInfos();


    static int myWorldID = de::kernelContext().config().getPropertyInt("WorldID");
    static int myServerID = de::kernelContext().config().getPropertyInt("ServerID");

    int maxWorldID = serverInfos.getMaxWorldID();
    int maxServerGroupID = serverInfos.getMaxServerGroupID();


    for (int worldID = 1; worldID < maxWorldID; worldID++) {
        for (int groupID = 0; groupID < maxServerGroupID; groupID++) {
            HashMapGameServerInfo& gameServerInfo = pGameServerInfos[worldID][groupID];

            if (!gameServerInfo.empty()) {
                HashMapGameServerInfo::const_iterator itr = gameServerInfo.begin();
                for (; itr != gameServerInfo.end(); itr++) {
                    GameServerInfo* pGameServerInfo = itr->second;

                    if (pGameServerInfo->getWorldID() == myWorldID) {
                        // Only for servers other than the current one (handled above).
                        if (pGameServerInfo->getGroupID() == myServerID) {
                        } else {
                            de::gameContext().loginServer().sendPacket(pGameServerInfo->getIP(),
                                                                       pGameServerInfo->getUDPPort(), &ggCommand);
                        }
                    }
                }
            }
        }
    }
}

bool GuildUnionManager::addGuild(uint uID, GuildID_t gID) {
    __BEGIN_TRY

    GuildUnion* pUnion = getGuildUnionByUnionID(uID);
    if (pUnion == NULL)
        return false;

    if (pUnion->addGuild(gID)) {
        m_GuildUnionMap[gID] = pUnion;

        sendRefreshCommand();

        return true;
    } else {
        return false;
    }

    __END_CATCH
}

bool GuildUnionManager::removeGuildFromUnion(GuildID_t gID) {
    __BEGIN_TRY

    GuildRepository& guildRows = defaultGuildRepository();

    // The union holding the guild. The lookup maps answer first; a guild they
    // have forgotten may still have a member row naming it, so the tables are
    // asked before the answer is "no union".
    GuildUnion* pUnion = getGuildUnion(gID);
    uint unionID = 0;
    GuildID_t unionMasterGuildID = 0;
    bool unionKnown = false;

    if (pUnion != NULL) {
        unionKnown = true;
        unionID = pUnion->getUnionID();
        unionMasterGuildID = pUnion->getMasterGuildID();
    } else {
        int rowUnionID = 0;
        int rowOwnerGuildID = 0;
        int rowMasterGuildID = 0;

        if (guildRows.loadUnionOfGuild(gID, rowUnionID, rowOwnerGuildID) &&
            guildRows.loadUnionMaster(rowUnionID, rowMasterGuildID)) {
            unionKnown = true;
            unionID = rowUnionID;
            unionMasterGuildID = rowMasterGuildID;

            // The union object, if this server carries one for that id.
            pUnion = getGuildUnionByUnionID(unionID);
        }
    }

    if (!unionKnown)
        return false;

    // The union's member guilds, from its rows and from the union object
    // both: a guild one of them has lost is still a guild to let out.
    // decideUnionTeardown names each of them once however often it is listed.
    const vector<int> memberRows = guildRows.loadUnionMemberGuilds(unionID);

    vector<GuildID_t> memberGuilds;
    memberGuilds.reserve(memberRows.size());
    for (size_t m = 0; m < memberRows.size(); m++)
        memberGuilds.push_back(static_cast<GuildID_t>(memberRows[m]));

    if (pUnion != NULL) {
        const list<GuildID_t> knownGuilds = pUnion->getGuildList();
        for (list<GuildID_t>::const_iterator itr = knownGuilds.begin(); itr != knownGuilds.end(); ++itr)
            memberGuilds.push_back(*itr);
    }

    const UnionTeardown teardown = decideUnionTeardown(unionKnown, unionMasterGuildID, memberGuilds, gID);
    if (teardown.action == UnionTeardown::NOTHING)
        return false;

    for (size_t m = 0; m < teardown.membersToRemove.size(); m++) {
        const GuildID_t memberID = teardown.membersToRemove[m];

        if (pUnion != NULL) {
            // Takes the member row with it. The guild stops finding this
            // union whether or not the union itself survives.
            if (pUnion->removeGuild(memberID))
                m_GuildUnionMap.erase(memberID);
        } else {
            // No union object on this server: the row is all there is.
            guildRows.deleteUnionMember(unionID, memberID);
        }
    }

    if (teardown.action == UnionTeardown::DISSOLVE)
        destroyUnion(unionID);

    for (size_t n = 0; n < teardown.guildsToNotify.size(); n++)
        notifyUnionChange(teardown.guildsToNotify[n]);

    // The other game servers keep their own copy of the union tables.
    sendRefreshCommand();

    return true;

    __END_CATCH
}

void GuildUnionManager::destroyUnion(uint uID) {
    GuildUnion* pUnion = getGuildUnionByUnionID(uID);

    if (pUnion == NULL) {
        // Nothing in memory holds the union: its rows are all there is.
        defaultGuildRepository().deleteUnion(uID);
        return;
    }

    pUnion->destroy();

    // Every guild that reaches this union by id has to stop reaching it
    // before the object goes; a guild left pointing at freed memory would
    // answer a union lookup with it.
    const list<GuildID_t> guilds = pUnion->getGuildList();
    for (list<GuildID_t>::const_iterator itr = guilds.begin(); itr != guilds.end(); ++itr)
        m_GuildUnionMap.erase(*itr);

    m_GuildUnionMap.erase(pUnion->getMasterGuildID());
    m_UnionIDMap.erase(uID);

    list<GuildUnion*>::iterator itr = find(m_GuildUnionList.begin(), m_GuildUnionList.end(), pUnion);
    if (itr != m_GuildUnionList.end())
        m_GuildUnionList.erase(itr);

    SAFE_DELETE(pUnion);
}

bool GuildUnionManager::removeGuild(uint uID, GuildID_t gID) {
    __BEGIN_TRY

    GuildUnion* pUnion = getGuildUnionByUnionID(uID);
    if (pUnion == NULL)
        return false;

    if (!pUnion->removeGuild(gID))
        return false;

    m_GuildUnionMap.erase(gID);

    // The master guild alone is not a union.
    if (pUnion->m_Guilds.empty())
        destroyUnion(uID);

    sendRefreshCommand();

    return true;

    __END_CATCH
}

void GuildUnionManager::reload() {
    __ENTER_CRITICAL_SECTION(m_Mutex)

    list<GuildUnion*>::iterator itr = m_GuildUnionList.begin();
    list<GuildUnion*>::iterator endItr = m_GuildUnionList.end();

    for (; itr != endItr; ++itr) {
        GuildUnion* pUnion = *itr;
        SAFE_DELETE(pUnion);
    }
    m_GuildUnionList.clear();

    m_GuildUnionMap.clear();
    m_UnionIDMap.clear();

    load();

    __LEAVE_CRITICAL_SECTION(m_Mutex)
}

void GuildUnionManager::load() {
    __BEGIN_TRY

    GuildRepository& repository = defaultGuildRepository();

    vector<UnionRow> unions = repository.loadUnions();

    for (size_t u = 0; u < unions.size(); u++) {
        uint uID = unions[u].unionID;
        GuildID_t gID = unions[u].masterGuildID;

        GuildUnion* pUnion = new GuildUnion(gID);
        pUnion->setUnionID(uID);

        vector<int> memberGuilds = repository.loadUnionMemberGuilds(uID);

        for (size_t m = 0; m < memberGuilds.size(); m++) {
            GuildID_t gID2 = memberGuilds[m];
            pUnion->m_Guilds.push_back(gID2);
            //					pUnion->addGuild( gID2 );
        }

        addGuildUnion(pUnion);
    }

    __END_CATCH
}

uint GuildUnionOfferManager::offerJoin(GuildID_t gID, GuildID_t masterGID) {
    __BEGIN_TRY

    if (GuildUnionManager::Instance().getGuildUnion(gID) != NULL)
        return ALREADY_IN_UNION;
    GuildUnion* pUnion = GuildUnionManager::Instance().getGuildUnion(masterGID);

    Guild* pReqGuild = de::gameContext().guilds().getGuild(gID);
    Guild* pMasterGuild = de::gameContext().guilds().getGuild(masterGID);

    if (pReqGuild != NULL && pMasterGuild != NULL) {
        if (pReqGuild->getActiveMemberCount() > MAX_GUILDMEMBER_ACTIVE_COUNT ||
            pMasterGuild->getActiveMemberCount() > MAX_GUILDMEMBER_ACTIVE_COUNT) {
            return TOO_MANY_MEMBER;
        }
    }

    //
    if (pUnion == NULL) {
        pUnion = new GuildUnion(masterGID);
        pUnion->create();
        GuildUnionManager::Instance().addGuildUnion(pUnion);
    } else if (pUnion->getMasterGuildID() != masterGID) {
        return TARGET_IS_NOT_MASTER;
    }

    if (hasOffer(gID)) {
        return ALREADY_OFFER_SOMETHING;
    }

    GuildRepository& repository = defaultGuildRepository();

    // Was the guild forced out of a union in the last ten days? Then it is penalised.
    if (repository.countRecentEscapes(gID) > 0) {
        return YOU_HAVE_PENALTY;
    }

    if (repository.countUnionMembers(pUnion->getUnionID()) >=
        de::gameContext().variables().getVariable(GUILD_UNION_MAX)) {
        return NOT_ENOUGH_SLOT;
    }

    // Drop offers older than ten days.
    repository.deleteStaleOffers(gID);
    repository.insertJoinOffer(pUnion->getUnionID(), gID);

    return OK;

    __END_CATCH
}

uint GuildUnionOfferManager::offerQuit(GuildID_t gID) {
    __BEGIN_TRY

    GuildUnion* pUnion = GuildUnionManager::Instance().getGuildUnion(gID);

    if (pUnion == NULL) {
        return NOT_IN_UNION;
    } else if (pUnion->getMasterGuildID() == gID) {
        return MASTER_CANNOT_QUIT;
    }

    if (hasOffer(gID)) {
        return ALREADY_OFFER_SOMETHING;
    }

    defaultGuildRepository().insertQuitOffer(pUnion->getUnionID(), gID);

    return OK;

    __END_CATCH
}

bool GuildUnionOfferManager::makeOfferList(uint uID, GCUnionOfferList& offerList) {
    GuildRepository& repository = defaultGuildRepository();

    vector<UnionOfferRow> offers = repository.loadOffers(uID);

    if (offers.empty()) {
        return false;
    }

    for (size_t o = 0; o < offers.size(); o++) {
        SingleGuildUnionOffer* offer = new SingleGuildUnionOffer;

        offer->setGuildType(offers[o].offerType);
        offer->setGuildID(offers[o].ownerGuildID);

        DWORD dwDate = offers[o].date;
        offer->setDate(dwDate * 100);

        string guildName;
        string guildMaster;
        if (!repository.loadGuildNameAndMaster(offers[o].ownerGuildID, guildName, guildMaster)) {
            delete offer;
            return false;
        }

        offer->setGuildName(guildName);
        offer->setGuildMaster(guildMaster);

        offerList.addUnionOfferList(offer);
    }

    // cout << "make offerlist success!" << endl;
    return true;
}

uint GuildUnionOfferManager::acceptJoin(GuildID_t gID) {
    __BEGIN_TRY

    GuildRepository& repository = defaultGuildRepository();

    int unionID = 0;
    if (!repository.loadJoinOfferUnion(gID, unionID))
        return NO_TARGET_UNION;

    clearOffer(gID);

    GuildUnion* pUnion = GuildUnionManager::Instance().getGuildUnion(gID);
    if (pUnion != NULL) {
        return ALREADY_IN_UNION;
    }

    uint uID = unionID;
    pUnion = GuildUnionManager::Instance().getGuildUnionByUnionID(uID);
    if (pUnion == NULL) {
        return NO_TARGET_UNION;
    }

    if (repository.countUnionMembers(uID) >= de::gameContext().variables().getVariable(GUILD_UNION_MAX)) {
        return NOT_ENOUGH_SLOT;
    }

    GuildUnionManager::Instance().addGuild(uID, gID);

    return OK;

    __END_CATCH
}

uint GuildUnionOfferManager::acceptQuit(GuildID_t gID) {
    __BEGIN_TRY

    int unionID = 0;
    if (defaultGuildRepository().loadQuitOfferUnion(gID, unionID)) {
        clearOffer(gID);

        GuildUnion* pUnion = GuildUnionManager::Instance().getGuildUnion(gID);
        if (pUnion == NULL) {
            return NOT_IN_UNION;
        }

        uint uID = unionID;
        if (uID != pUnion->getUnionID()) {
            return NOT_YOUR_UNION;
        }

        pUnion = GuildUnionManager::Instance().getGuildUnionByUnionID(uID);
        if (pUnion == NULL) {
            return NO_TARGET_UNION;
        }

        GuildUnionManager::Instance().removeGuild(uID, gID);
    }

    return OK;

    __END_CATCH
}

uint GuildUnionOfferManager::denyJoin(GuildID_t gID) {
    __BEGIN_TRY

    int unionID = 0;
    if (defaultGuildRepository().loadJoinOfferUnion(gID, unionID)) {
        clearOffer(gID);

        GuildUnion* pUnion = GuildUnionManager::Instance().getGuildUnion(gID);
        if (pUnion != NULL) {
            return ALREADY_IN_UNION;
        }

        uint uID = unionID;
        pUnion = GuildUnionManager::Instance().getGuildUnionByUnionID(uID);
        if (pUnion == NULL) {
            return NO_TARGET_UNION;
        }
    }

    return OK;

    __END_CATCH
}

uint GuildUnionOfferManager::denyQuit(GuildID_t gID) {
    __BEGIN_TRY

    int unionID = 0;
    if (defaultGuildRepository().loadQuitOfferUnion(gID, unionID)) {
        clearOffer(gID);

        GuildUnion* pUnion = GuildUnionManager::Instance().getGuildUnion(gID);
        if (pUnion == NULL) {
            return NOT_IN_UNION;
        }

        uint uID = unionID;
        if (uID != pUnion->getUnionID()) {
            return NOT_YOUR_UNION;
        }

        pUnion = GuildUnionManager::Instance().getGuildUnionByUnionID(uID);
        if (pUnion == NULL) {
            return NO_TARGET_UNION;
        }
    }

    return OK;

    __END_CATCH
}

void GuildUnionOfferManager::clearOffer(GuildID_t gID) {
    __BEGIN_TRY

    defaultGuildRepository().deleteOffers(gID);

    __END_CATCH
}

bool GuildUnionOfferManager::hasOffer(GuildID_t gID) {
    __BEGIN_TRY

    if (defaultGuildRepository().countOffers(gID) > 0) {
        return true;
    }

    return false;

    __END_CATCH
}
