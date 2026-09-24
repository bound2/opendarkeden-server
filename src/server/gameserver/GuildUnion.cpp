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

GuildUnionManager::GuildUnionManager() {
    m_Mutex.setName("GuildUnionManager");
}

// The registry frees the unions, live and retired.
GuildUnionManager::~GuildUnionManager() = default;

UnionJoinOfferVerdict GuildUnionManager::recordJoinOffer(GuildID_t applicantGID, GuildID_t masterGID,
                                                         bool tooManyMembers) {
    UnionJoinOfferVerdict verdict = UnionJoinOfferVerdict::ALREADY_IN_UNION;
    bool opened = false;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    GuildRepository& repository = defaultGuildRepository();

    GuildUnion* pUnion = m_Unions.unionOfGuild(masterGID);

    UnionJoinOfferFacts facts;
    facts.applicantInUnion = m_Unions.unionOfGuild(applicantGID) != NULL;
    facts.tooManyMembers = tooManyMembers;
    if (pUnion == NULL)
        facts.targetStanding = UnionJoinOfferFacts::TARGET_IN_NO_UNION;
    else if (pUnion->getMasterGuildID() == masterGID)
        facts.targetStanding = UnionJoinOfferFacts::TARGET_LEADS_UNION;
    else
        facts.targetStanding = UnionJoinOfferFacts::TARGET_IS_MEMBER;
    facts.applicantHasOffer = repository.countOffers(applicantGID) > 0;
    // Forced out of a union in the last ten days: penalised.
    facts.applicantHasPenalty = repository.countRecentEscapes(applicantGID) > 0;
    facts.unionMemberCount = pUnion != NULL ? repository.countUnionMembers(pUnion->getUnionID()) : 0;
    facts.unionMemberLimit = de::gameContext().variables().getVariable(GUILD_UNION_MAX);

    verdict = decideUnionJoinOffer(facts);

    uint targetUnionID = pUnion != NULL ? pUnion->getUnionID() : 0;

    if (verdict == UnionJoinOfferVerdict::OPEN_UNION_AND_RECORD)
        targetUnionID = repository.insertUnion(masterGID);

    if (verdict == UnionJoinOfferVerdict::OPEN_UNION_AND_RECORD || verdict == UnionJoinOfferVerdict::RECORD_OFFER) {
        try {
            // Drop offers older than ten days.
            repository.deleteStaleOffers(applicantGID);
            repository.insertJoinOffer(targetUnionID, applicantGID);
        } catch (...) {
            // A union row with no offer behind it would keep its master in a
            // union nobody can reach, so the row goes with the failed offer.
            if (verdict == UnionJoinOfferVerdict::OPEN_UNION_AND_RECORD)
                repository.deleteUnion(targetUnionID);
            throw;
        }

        // Published only once the offer that justifies it is on record.
        if (verdict == UnionJoinOfferVerdict::OPEN_UNION_AND_RECORD) {
            pUnion = m_Unions.publish(std::make_unique<GuildUnion>(targetUnionID, masterGID));
            opened = true;
        }
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    // The other game servers keep their own copy of the union tables; without
    // this, an offer to the same guild made on one of them would open a
    // second union there.
    if (opened)
        sendRefreshCommand();

    return verdict;
}

bool GuildUnionManager::dissolveIfAbandoned(uint uID) {
    bool dissolved = false;
    GuildID_t masterGuildID = 0;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    GuildUnion* pUnion = m_Unions.unionByID(uID);
    if (pUnion != NULL)
        masterGuildID = pUnion->getMasterGuildID();

    dissolved = dissolveIfAbandoned_LOCKED(uID);

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    if (dissolved) {
        // The master guild was the only guild in the union.
        if (masterGuildID != 0)
            notifyUnionChange(masterGuildID);

        sendRefreshCommand();
    }

    return dissolved;
}

bool GuildUnionManager::dissolveIfAbandoned_LOCKED(uint uID) {
    GuildRepository& repository = defaultGuildRepository();

    const int memberRows = repository.countUnionMembers(uID);
    if (memberRows > 0)
        return false;

    // GuildUnionOffer.OfferType read as a number: JOIN is the enum's first
    // value. A QUIT row belongs to a member and an ESCAPE row is a former
    // member's penalty; neither keeps a union.
    const int kJoinOfferType = 1;

    const vector<UnionOfferRow> offers = repository.loadOffers(uID);
    int pendingJoinOffers = 0;
    for (size_t o = 0; o < offers.size(); o++) {
        if (offers[o].offerType == kJoinOfferType)
            pendingJoinOffers++;
    }

    if (!unionIsAbandoned(memberRows, pendingJoinOffers))
        return false;

    destroyUnion_LOCKED(uID);
    return true;
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

    __ENTER_CRITICAL_SECTION(m_Mutex)

    if (!m_Unions.addMember(uID, gID))
        return false;

    defaultGuildRepository().insertUnionMember(uID, gID);

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    sendRefreshCommand();

    return true;

    __END_CATCH
}

bool GuildUnionManager::removeGuildFromUnion(GuildID_t gID) {
    __BEGIN_TRY

    UnionTeardown teardown;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    GuildRepository& guildRows = defaultGuildRepository();

    // The union holding the guild. The lookup maps answer first; a guild they
    // have forgotten may still have a member row naming it, so the tables are
    // asked before the answer is "no union".
    GuildUnion* pUnion = m_Unions.unionOfGuild(gID);
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
            pUnion = m_Unions.unionByID(unionID);
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

    teardown = decideUnionTeardown(unionKnown, unionMasterGuildID, memberGuilds, gID);
    if (teardown.action == UnionTeardown::NOTHING)
        return false;

    for (size_t m = 0; m < teardown.membersToRemove.size(); m++) {
        const GuildID_t memberID = teardown.membersToRemove[m];

        // The guild stops resolving to this union whether or not the union
        // itself survives, and its member row goes. A guild the union object
        // did not list only has the row to lose.
        const bool listed = m_Unions.removeMember(unionID, memberID);

        if (!guildRows.deleteUnionMember(unionID, memberID) && listed)
            filelog("GuildUnion.log", "[%u:%u] no member row to remove.", unionID, memberID);
    }

    if (teardown.action == UnionTeardown::DISSOLVE)
        destroyUnion_LOCKED(unionID);

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    for (size_t n = 0; n < teardown.guildsToNotify.size(); n++)
        notifyUnionChange(teardown.guildsToNotify[n]);

    // The other game servers keep their own copy of the union tables.
    sendRefreshCommand();

    return true;

    __END_CATCH
}

void GuildUnionManager::destroyUnion_LOCKED(uint uID) {
    // The rows go whether or not this server carries the union in memory.
    defaultGuildRepository().deleteUnion(uID);

    // Every guild that resolved to the union stops resolving to it. A thread
    // still holding the pointer finds it retired rather than freed.
    m_Unions.retire(uID);
}

bool GuildUnionManager::removeGuild(uint uID, GuildID_t gID) {
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    if (!m_Unions.removeMember(uID, gID))
        return false;

    if (!defaultGuildRepository().deleteUnionMember(uID, gID))
        filelog("GuildUnion.log", "[%u:%u] no member row to remove.", uID, gID);

    // The master guild alone is not a union.
    GuildUnion* pUnion = m_Unions.unionByID(uID);
    if (pUnion != NULL && pUnion->getGuildList().empty())
        destroyUnion_LOCKED(uID);

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    sendRefreshCommand();

    return true;

    __END_CATCH
}

void GuildUnionManager::reload() {
    load();
}

void GuildUnionManager::load() {
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    GuildRepository& repository = defaultGuildRepository();

    const vector<UnionRow> unions = repository.loadUnions();

    vector<std::unique_ptr<GuildUnion>> fresh;
    fresh.reserve(unions.size());

    for (size_t u = 0; u < unions.size(); u++) {
        const uint uID = unions[u].unionID;

        list<GuildID_t> memberGuilds;
        const vector<int> memberRows = repository.loadUnionMemberGuilds(uID);
        for (size_t m = 0; m < memberRows.size(); m++)
            memberGuilds.push_back(static_cast<GuildID_t>(memberRows[m]));

        const GuildID_t masterGuildID = static_cast<GuildID_t>(unions[u].masterGuildID);
        fresh.push_back(std::make_unique<GuildUnion>(uID, masterGuildID, memberGuilds));
    }

    m_Unions.replaceAll(std::move(fresh));

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

uint GuildUnionOfferManager::offerJoin(GuildID_t gID, GuildID_t masterGID) {
    __BEGIN_TRY

    // The guild sizes are read here, outside the union manager's lock, which
    // the guild manager's locks may not be taken under.
    Guild* pReqGuild = de::gameContext().guilds().getGuild(gID);
    Guild* pMasterGuild = de::gameContext().guilds().getGuild(masterGID);

    const bool tooManyMembers = pReqGuild != NULL && pMasterGuild != NULL &&
                                (pReqGuild->getActiveMemberCount() > MAX_GUILDMEMBER_ACTIVE_COUNT ||
                                 pMasterGuild->getActiveMemberCount() > MAX_GUILDMEMBER_ACTIVE_COUNT);

    switch (GuildUnionManager::Instance().recordJoinOffer(gID, masterGID, tooManyMembers)) {
    case UnionJoinOfferVerdict::RECORD_OFFER:
    case UnionJoinOfferVerdict::OPEN_UNION_AND_RECORD:
        return OK;
    case UnionJoinOfferVerdict::ALREADY_IN_UNION:
        return ALREADY_IN_UNION;
    case UnionJoinOfferVerdict::TOO_MANY_MEMBER:
        return TOO_MANY_MEMBER;
    case UnionJoinOfferVerdict::TARGET_IS_NOT_MASTER:
        return TARGET_IS_NOT_MASTER;
    case UnionJoinOfferVerdict::ALREADY_OFFER_SOMETHING:
        return ALREADY_OFFER_SOMETHING;
    case UnionJoinOfferVerdict::YOU_HAVE_PENALTY:
        return YOU_HAVE_PENALTY;
    case UnionJoinOfferVerdict::NOT_ENOUGH_SLOT:
        return NOT_ENOUGH_SLOT;
    }

    throw Error("unknown union join offer verdict");

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

    GuildUnionManager& unions = GuildUnionManager::Instance();
    const uint uID = unionID;
    uint result = OK;

    if (unions.getGuildUnion(gID) != NULL) {
        result = ALREADY_IN_UNION;
    } else if (unions.getGuildUnionByUnionID(uID) == NULL) {
        result = NO_TARGET_UNION;
    } else if (repository.countUnionMembers(uID) >= de::gameContext().variables().getVariable(GUILD_UNION_MAX)) {
        result = NOT_ENOUGH_SLOT;
    } else if (!unions.addGuild(uID, gID)) {
        // Another change came between the checks above and this one: the
        // guild joined some union, or this union was dissolved.
        result = unions.getGuildUnion(gID) != NULL ? ALREADY_IN_UNION : NO_TARGET_UNION;
    }

    // The offer is gone whether or not the join went through, so a union
    // opened for it that got no member goes too.
    if (result != OK)
        unions.dissolveIfAbandoned(uID);

    return result;

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

        // The guild may have left, or the union been dissolved, since the
        // checks above.
        if (!GuildUnionManager::Instance().removeGuild(uID, gID))
            return NOT_IN_UNION;
    }

    return OK;

    __END_CATCH
}

uint GuildUnionOfferManager::denyJoin(GuildID_t gID) {
    __BEGIN_TRY

    uint result = OK;

    int unionID = 0;
    if (defaultGuildRepository().loadJoinOfferUnion(gID, unionID)) {
        clearOffer(gID);

        GuildUnionManager& unions = GuildUnionManager::Instance();
        const uint uID = unionID;

        if (unions.getGuildUnion(gID) != NULL)
            result = ALREADY_IN_UNION;
        else if (unions.getGuildUnionByUnionID(uID) == NULL)
            result = NO_TARGET_UNION;

        // A union opened for this offer that has no member and no other
        // offer pending goes with the offer, on every game server.
        unions.dissolveIfAbandoned(uID);
    }

    return result;

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
