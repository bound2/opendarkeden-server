#ifndef __FAKE_GUILD_REPOSITORY_H__
#define __FAKE_GUILD_REPOSITORY_H__

#include <cstdlib>
#include <map>
#include <string>
#include <vector>

#include "repository/GuildRepository.h"

// In-memory GuildRepository for the guild join/registration decisions.
// Only the four calls those decisions make are modelled
// (src/server/gameserver/repository/GuildRepository.h is the authority on
// the contract; the MySQL-backed integration tier is what pins it):
//
//  - loadMemberExpireDate, loadMemberRankExpireDate and
//    loadMemberGuildRankExpireDate each answer false when the name has no
//    GuildMember row. They read the same row through different SELECTs, so
//    the fake keeps one row per name and hands out the columns each asks
//    for. A SQL NULL ExpireDate reads back as the empty string, which is
//    what an unseeded expire date is here.
//  - deleteMemberSpelled drops the row, whichever spelling it is given.
//
// Every call is appended to calls in order, so a test can pin that the
// decisions read what the handlers used to read, in the same sequence.
//
// Every other method of the interface aborts: reaching one from a decision
// would mean the decision took a path these tests do not describe.
class FakeGuildRepository : public GuildRepository {
public:
    struct MemberRow {
        int guildID = 0;
        int rank = 0;
        std::string expireDate;
    };

    // --- test seeding -------------------------------------------------------
    void addMember(const std::string& name, int guildID, int rank, const std::string& expireDate = std::string()) {
        MemberRow row;
        row.guildID = guildID;
        row.rank = rank;
        row.expireDate = expireDate;
        m_Members[name] = row;
    }

    void addGuildName(const std::string& guildName) {
        m_GuildNames.push_back(guildName);
    }

    bool hasMember(const std::string& name) const {
        return m_Members.find(name) != m_Members.end();
    }

    // The calls the decision made, in order.
    std::vector<std::string> calls;

    // --- the calls the decisions make ---------------------------------------
    bool loadMemberExpireDate(const std::string& name, std::string& expireDate) override {
        calls.push_back("loadMemberExpireDate(" + name + ")");

        std::map<std::string, MemberRow>::const_iterator itr = m_Members.find(name);
        if (itr == m_Members.end())
            return false;

        expireDate = itr->second.expireDate;
        return true;
    }

    bool loadMemberRankExpireDate(const std::string& name, int& rank, std::string& expireDate) override {
        calls.push_back("loadMemberRankExpireDate(" + name + ")");

        std::map<std::string, MemberRow>::const_iterator itr = m_Members.find(name);
        if (itr == m_Members.end())
            return false;

        rank = itr->second.rank;
        expireDate = itr->second.expireDate;
        return true;
    }

    bool loadMemberGuildRankExpireDate(const std::string& name, int& guildID, int& rank,
                                       std::string& expireDate) override {
        calls.push_back("loadMemberGuildRankExpireDate(" + name + ")");

        std::map<std::string, MemberRow>::const_iterator itr = m_Members.find(name);
        if (itr == m_Members.end())
            return false;

        guildID = itr->second.guildID;
        rank = itr->second.rank;
        expireDate = itr->second.expireDate;
        return true;
    }

    bool guildNameInUse(const std::string& guildName) override {
        calls.push_back("guildNameInUse(" + guildName + ")");

        for (const auto& name : m_GuildNames) {
            if (name == guildName)
                return true;
        }
        return false;
    }

    void deleteMemberSpelled(GuildMemberDeleteSpelling spelling, const std::string& name) override {
        calls.push_back("deleteMemberSpelled(" + std::to_string(static_cast<int>(spelling)) + "," + name + ")");
        m_Members.erase(name);
    }

    // --- the rest of the interface ------------------------------------------
    bool memberExists(const std::string&) override {
        unexpected();
    }
    void insertMember(GuildID_t, const std::string&, GuildMemberRank_t) override {
        unexpected();
    }
    void insertWaitingMember(GuildID_t, const std::string&, GuildMemberRank_t, const std::string&) override {
        unexpected();
    }
    void rejoinMember(GuildID_t, GuildMemberRank_t, const std::string&) override {
        unexpected();
    }
    void rejoinWaitingMember(GuildID_t, GuildMemberRank_t, const std::string&, const std::string&) override {
        unexpected();
    }
    bool loadMember(const std::string&, GuildMemberRow&) override {
        unexpected();
    }
    bool loadMemberGuildID(const std::string&, int&) override {
        unexpected();
    }
    void saveMember(GuildID_t, GuildMemberRank_t, const std::string&) override {
        unexpected();
    }
    void deleteMember(const std::string&) override {
        unexpected();
    }
    void setMemberRankAndExpireDate(int, const std::string&, const std::string&) override {
        unexpected();
    }
    void saveMemberIntro(const std::string&, const std::string&) override {
        unexpected();
    }
    bool loadMemberIntro(const std::string&, std::string&) override {
        unexpected();
    }
    std::vector<GuildMemberListRow> loadActiveMembers() override {
        unexpected();
    }
    void insertGuild(const GuildRecord&) override {
        unexpected();
    }
    bool loadGuild(GuildID_t, GuildRow&) override {
        unexpected();
    }
    void saveGuild(const GuildRecord&) override {
        unexpected();
    }
    void deleteGuild(GuildID_t) override {
        unexpected();
    }
    std::vector<GuildListRow> loadGuildsInStates(int, int) override {
        unexpected();
    }
    bool loadGuildNameAndMaster(int, std::string&, std::string&) override {
        unexpected();
    }
    int countCastlesOfGuild(int) override {
        unexpected();
    }
    bool loadCastleOfGuild(int, int&, int&) override {
        unexpected();
    }
    int countWarSchedulesOfAttacker(int) override {
        unexpected();
    }
    int countReinforceRegistrations(int) override {
        unexpected();
    }
    int countStartedWarsAtCastle(ServerID_t, ZoneID_t) override {
        unexpected();
    }
    int countStartedWarsOfAttacker(int) override {
        unexpected();
    }
    uint insertUnion(GuildID_t) override {
        unexpected();
    }
    void insertUnionMember(uint, GuildID_t) override {
        unexpected();
    }
    bool deleteUnionMember(uint, GuildID_t) override {
        unexpected();
    }
    void deleteUnion(uint) override {
        unexpected();
    }
    std::vector<UnionRow> loadUnions() override {
        unexpected();
    }
    std::vector<int> loadUnionMemberGuilds(uint) override {
        unexpected();
    }
    bool loadUnionOfGuild(GuildID_t, int&, int&) override {
        unexpected();
    }
    bool loadUnionMaster(int, int&) override {
        unexpected();
    }
    int countUnionMembers(uint) override {
        unexpected();
    }
    int countUnionMembersSpelled(UnionStatementSpelling, uint) override {
        unexpected();
    }
    void deleteUnionInfoOnly(UnionStatementSpelling, uint) override {
        unexpected();
    }
    int countRecentEscapes(GuildID_t) override {
        unexpected();
    }
    void deleteStaleOffers(GuildID_t) override {
        unexpected();
    }
    void insertJoinOffer(uint, GuildID_t) override {
        unexpected();
    }
    void insertQuitOffer(uint, GuildID_t) override {
        unexpected();
    }
    void insertEscapeOffer(uint, GuildID_t) override {
        unexpected();
    }
    std::vector<UnionOfferRow> loadOffers(uint) override {
        unexpected();
    }
    bool loadJoinOfferUnion(GuildID_t, int&) override {
        unexpected();
    }
    bool loadQuitOfferUnion(GuildID_t, int&) override {
        unexpected();
    }
    void deleteOffers(GuildID_t) override {
        unexpected();
    }
    int countOffers(GuildID_t) override {
        unexpected();
    }

private:
    [[noreturn]] static void unexpected() {
        std::abort();
    }

    std::map<std::string, MemberRow> m_Members;
    std::vector<std::string> m_GuildNames;
};

#endif // __FAKE_GUILD_REPOSITORY_H__
