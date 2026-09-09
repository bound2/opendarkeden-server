#ifndef __FAKE_SHARED_GUILD_REPOSITORY_H__
#define __FAKE_SHARED_GUILD_REPOSITORY_H__

#include <cstdlib>
#include <string>
#include <vector>

#include "repository/SharedGuildRepository.h"

// In-memory SharedGuildRepository for the sharedserver's guild decisions.
// Only the four character-side writes those decisions ask for are modelled
// (src/server/sharedserver/repository/SharedGuildRepository.h is the
// authority on the contract; the MySQL-backed integration tier is what pins
// it): setCharacterGuildID, addCharacterGold, insertMessage and
// stampMemberRequestDateTime.
//
// Every one of them appends a line to calls, and the test's step runner
// appends the roster mutations, the guild mutations and the packets to the
// same list, so calls is the whole ordered trace of what a decision's answer
// does - which is the part of the contract the handlers have to keep.
//
// Every other method aborts: reaching one would mean a decision asked for a
// write these tests do not describe.
class FakeSharedGuildRepository : public SharedGuildRepository {
public:
    // Everything the answer did, in order.
    std::vector<std::string> calls;

    void record(const std::string& call) {
        calls.push_back(call);
    }

    // --- the character side of a membership change ---------------------------
    void setCharacterGuildID(GuildRace_t race, int guildID, const std::string& name) override {
        record("setCharacterGuildID(" + std::to_string((int)race) + "," + std::to_string(guildID) + "," + name + ")");
    }

    void addCharacterGold(GuildRace_t race, int gold, const std::string& name) override {
        record("addCharacterGold(" + std::to_string((int)race) + "," + std::to_string(gold) + "," + name + ")");
    }

    void insertMessage(SharedMessageSpelling spelling, const std::string& receiver,
                       const std::string& message) override {
        record("insertMessage(" + std::to_string((int)spelling) + "," + receiver + "," + message + ")");
    }

    void stampMemberRequestDateTime(const std::string& name) override {
        record("stampMemberRequestDateTime(" + name + ")");
    }

    // --- the rest of the interface -------------------------------------------
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
    bool loadMember(const std::string&, SharedGuildMemberRow&) override {
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
    std::vector<SharedGuildMemberListRow> loadActiveMembers() override {
        unexpected();
    }
    void insertGuild(const SharedGuildRecord&) override {
        unexpected();
    }
    bool loadGuild(GuildID_t, SharedGuildRow&) override {
        unexpected();
    }
    void saveGuild(const SharedGuildRecord&) override {
        unexpected();
    }
    void deleteGuild(GuildID_t) override {
        unexpected();
    }
    void saveGuildIntro(const std::string&, GuildID_t) override {
        unexpected();
    }
    void updateGuildFields(const std::string&, GuildID_t) override {
        unexpected();
    }
    std::vector<SharedGuildListRow> loadGuildsInStates(int, int) override {
        unexpected();
    }
    void purgeGuild(GuildID_t) override {
        unexpected();
    }
    int countGuilds() override {
        unexpected();
    }
    int loadMaxGuildID() override {
        unexpected();
    }
    int countGuildsOfRace(int) override {
        unexpected();
    }
    int loadMaxGuildZoneIDOfRace(int) override {
        unexpected();
    }

private:
    [[noreturn]] static void unexpected() {
        std::abort();
    }
};

#endif // __FAKE_SHARED_GUILD_REPOSITORY_H__
