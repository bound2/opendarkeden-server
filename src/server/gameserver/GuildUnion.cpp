#include "GuildUnion.h"

#include <stdio.h>

#include "GCModifyInformation.h"
#include "GGCommand.h"
#include "GameServer.h"
#include "GameServerInfoManager.h"
#include "Guild.h"
#include "GuildManager.h"
#include "LoginServerManager.h"
#include "PCFinder.h"
#include "PacketUtil.h"
#include "Player.h"
#include "VariableManager.h"
#include "repository/GuildRepository.h"
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


    // 각 server로 보낸다.
    HashMapGameServerInfo** pGameServerInfos = g_pGameServerInfoManager->getGameServerInfos();


    static int myWorldID = g_pConfig->getPropertyInt("WorldID");
    static int myServerID = g_pConfig->getPropertyInt("ServerID");

    int maxWorldID = g_pGameServerInfoManager->getMaxWorldID();
    int maxServerGroupID = g_pGameServerInfoManager->getMaxServerGroupID();


    for (int worldID = 1; worldID < maxWorldID; worldID++) {
        for (int groupID = 0; groupID < maxServerGroupID; groupID++) {
            HashMapGameServerInfo& gameServerInfo = pGameServerInfos[worldID][groupID];

            if (!gameServerInfo.empty()) {
                HashMapGameServerInfo::const_iterator itr = gameServerInfo.begin();
                for (; itr != gameServerInfo.end(); itr++) {
                    GameServerInfo* pGameServerInfo = itr->second;

                    if (pGameServerInfo->getWorldID() == myWorldID) {
                        // 현재 서버가 아닌 경우에만..(위에서 처리했으므로)
                        if (pGameServerInfo->getGroupID() == myServerID) {
                        } else {
                            g_pLoginServerManager->sendPacket(pGameServerInfo->getIP(), pGameServerInfo->getUDPPort(),
                                                              &ggCommand);
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


    // 각 server로 보낸다.
    HashMapGameServerInfo** pGameServerInfos = g_pGameServerInfoManager->getGameServerInfos();


    static int myWorldID = g_pConfig->getPropertyInt("WorldID");
    static int myServerID = g_pConfig->getPropertyInt("ServerID");

    int maxWorldID = g_pGameServerInfoManager->getMaxWorldID();
    int maxServerGroupID = g_pGameServerInfoManager->getMaxServerGroupID();


    for (int worldID = 1; worldID < maxWorldID; worldID++) {
        for (int groupID = 0; groupID < maxServerGroupID; groupID++) {
            HashMapGameServerInfo& gameServerInfo = pGameServerInfos[worldID][groupID];

            if (!gameServerInfo.empty()) {
                HashMapGameServerInfo::const_iterator itr = gameServerInfo.begin();
                for (; itr != gameServerInfo.end(); itr++) {
                    GameServerInfo* pGameServerInfo = itr->second;

                    if (pGameServerInfo->getWorldID() == myWorldID) {
                        // 현재 서버가 아닌 경우에만..(위에서 처리했으므로)
                        if (pGameServerInfo->getGroupID() == myServerID) {
                        } else {
                            g_pLoginServerManager->sendPacket(pGameServerInfo->getIP(), pGameServerInfo->getUDPPort(),
                                                              &ggCommand);
                        }
                    }
                }
            }
        }
    }
}

bool GuildUnionManager::addGuild(uint uID, GuildID_t gID) {
    __BEGIN_TRY

    GuildUnion* pUnion = m_UnionIDMap[uID];
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

bool GuildUnionManager::removeMasterGuild(GuildID_t gID) {
    __BEGIN_TRY

    // If this guild is the union master and is leaving,
    // break up the union it belongs to.

    GuildUnion* pUnion = m_GuildUnionMap[gID];
    // A union this guild masters: throw every member guild out and destroy it.
    if (pUnion != NULL) {
        uint uID = pUnion->getUnionID(); // the union id

        vector<int> memberGuilds = defaultGuildRepository().loadUnionMemberGuilds(uID);

        // No members at all would be strange.
        if (memberGuilds.empty()) {
            return false;
        }

        {
            string unionMasterID = g_pGuildManager->getGuild(gID)->getMaster();
            // Remove every guild from the union; once they are all gone the
            // union dissolves itself.
            for (size_t m = 0; m < memberGuilds.size(); m++) {
                if (pUnion->removeGuild(memberGuilds[m])) {
                    m_GuildUnionMap[gID] = NULL;
                    if (pUnion->m_Guilds.empty()) {
                        list<GuildUnion*>::iterator itr =
                            find(m_GuildUnionList.begin(), m_GuildUnionList.end(), pUnion);
                        if (itr != m_GuildUnionList.end()) {
                            pUnion->destroy();
                            m_GuildUnionList.erase(itr);
                            m_GuildUnionMap.erase(pUnion->getMasterGuildID());
                            m_UnionIDMap.erase(pUnion->getUnionID());

                            SAFE_DELETE(pUnion);
                        } //
                    } // isEmpty
                    sendGCOtherModifyInfoGuildUnionByGuildID(memberGuilds[m]);
                } // if
            } // for
            // Every guild is removed; the last one cleaned up as well.

            Creature* pTargetCreature = NULL;
            __ENTER_CRITICAL_SECTION((*g_pPCFinder))

            pTargetCreature = g_pPCFinder->getCreature_LOCKED(unionMasterID);
            if (pTargetCreature != NULL) {
                GCModifyInformation gcModifyInformation2;
                makeGCModifyInfoGuildUnion(&gcModifyInformation2, pTargetCreature);
                pTargetCreature->getPlayer()->sendPacket(&gcModifyInformation2);
            }
            __LEAVE_CRITICAL_SECTION((*g_pPCFinder))

            // Tell everyone the union master changed.
            sendGCOtherModifyInfoGuildUnionByGuildID(gID);

            sendRefreshCommand();
        }
    } else // Not a union master: find which union the guild belongs to and take it out.
    {
        string unionMasterID = "";
        string guildMasterID = "";
        GuildID_t unionMasterGuildID = 0;

        int unionID = 0;
        int ownerGuildID = 0;

        // Not in any union: just leave.
        if (!defaultGuildRepository().loadUnionOfGuild(gID, unionID, ownerGuildID)) {
            return false;
        }

        {
            // In a union: find its master guild.
            int masterGuildID = 0;
            if (defaultGuildRepository().loadUnionMaster(unionID, masterGuildID)) {
                unionMasterGuildID = masterGuildID;
                unionMasterID = g_pGuildManager->getGuild(unionMasterGuildID)->getMaster();
            }

            guildMasterID = g_pGuildManager->getGuild(gID)->getMaster();

            if (removeGuild(unionID, ownerGuildID)) {
                Creature* pTargetCreature = NULL;  // the guild's master
                Creature* pTargetCreature2 = NULL; // the union guild's master

                __ENTER_CRITICAL_SECTION((*g_pPCFinder))

                pTargetCreature = g_pPCFinder->getCreature_LOCKED(guildMasterID);
                if (pTargetCreature != NULL) {
                    GCModifyInformation gcModifyInformation2;
                    makeGCModifyInfoGuildUnion(&gcModifyInformation2, pTargetCreature);
                    pTargetCreature->getPlayer()->sendPacket(&gcModifyInformation2);
                }

                pTargetCreature2 = g_pPCFinder->getCreature_LOCKED(unionMasterID);
                if (pTargetCreature != NULL) {
                    GCModifyInformation gcModifyInformation2;
                    makeGCModifyInfoGuildUnion(&gcModifyInformation2, pTargetCreature2);
                    pTargetCreature2->getPlayer()->sendPacket(&gcModifyInformation2);
                }
                __LEAVE_CRITICAL_SECTION((*g_pPCFinder))


                // Send the changed guild-master information.
                sendGCOtherModifyInfoGuildUnionByGuildID(gID);
                // A guild removed from the union because it broke up: the union master must hear of it too.
                sendGCOtherModifyInfoGuildUnionByGuildID(unionMasterGuildID);

                // The guild is removed: tell the other servers too.
                sendRefreshCommand();
            }
        }
    }

    __END_CATCH

    return true;
}

bool GuildUnionManager::removeGuild(uint uID, GuildID_t gID) {
    __BEGIN_TRY

    GuildUnion* pUnion = m_UnionIDMap[uID];
    if (pUnion == NULL)
        return false;

    if (pUnion->removeGuild(gID)) {
        m_GuildUnionMap[gID] = NULL;
        if (pUnion->m_Guilds.empty()) {
            list<GuildUnion*>::iterator itr = find(m_GuildUnionList.begin(), m_GuildUnionList.end(), pUnion);
            if (itr != m_GuildUnionList.end()) {
                pUnion->destroy();

                // m_GuildUnionMap[pUnion->getMasterGuildID()] = NULL;
                // m_UnionIDMap[pUnion->getUnionID()] = NULL;

                m_GuildUnionList.erase(itr);
                m_GuildUnionMap.erase(pUnion->getMasterGuildID());
                m_UnionIDMap.erase(pUnion->getUnionID());

                SAFE_DELETE(pUnion);
            }
        }

        sendRefreshCommand();
        return true;
    } else {
        return false;
    }

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

    Guild* pReqGuild = g_pGuildManager->getGuild(gID);
    Guild* pMasterGuild = g_pGuildManager->getGuild(masterGID);

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

    if (repository.countUnionMembers(pUnion->getUnionID()) >= g_pVariableManager->getVariable(GUILD_UNION_MAX)) {
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

    if (repository.countUnionMembers(uID) >= g_pVariableManager->getVariable(GUILD_UNION_MAX)) {
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
