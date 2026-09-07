#ifndef __SHARED_GUILD_REPOSITORY_H__
#define __SHARED_GUILD_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// The sharedserver's guild persistence: the guild rows (GuildInfo), the
// roster (GuildMember), the guild's union membership (GuildUnionMember)
// and its pending war schedules (WarScheduleInfo) that GuildManager,
// Guild and GuildMember keep, plus the character side of a membership
// change the GS handlers write — the GuildID column of the race tables
// (Slayer, Vampire, Ousters), the Gold refund when a guild registration
// is cancelled, and the Messages rows that tell the character what
// happened.
//
// The sharedserver's Guild and GuildManager are its own classes, not the
// gameserver's; the gameserver's GuildRepository covers the same
// GuildInfo / GuildMember / GuildUnionMember tables from the other binary
// with its own methods. This repository builds into the sharedserver and
// the integration tier.
//
// Every statement runs on getConnection("DARKEDEN"). The sharedserver
// registers no per-thread connection, so that name resolves to the
// process default connection for whichever thread asks: the main thread
// during SharedServer::init() (GuildManager::init/load), the
// GameServerManager worker afterwards (the GS handlers and
// GuildManager::heartbeat, which expires waiting members). The two never
// overlap: the worker starts after init() returns, and the main thread
// runs no statement after that.
//
// Reads are typed to the driver getter used: getInt → int, getString →
// std::string. Names, dates, intros and the tinysave field text are
// interpolated raw; the callers escape an intro through
// Guild::correctString before passing it, nothing else is escaped.
//
// A SQL failure is logged to DBError.log under the repository method's
// name and thrown as END_DB's const char*. No caller in the sharedserver
// catches it: on the worker it leaves the handler, processCommand,
// GameServerManager::run and ManagedThread's body, which marks the
// process failed and stops it; during init() it reaches main.cpp's
// catch (...) with the same result.
//
// Not enclosed (SQL on the same tables elsewhere in the tree):
//  - GuildInfo, GuildMember, GuildUnionMember: the gameserver's
//    MySQLGuildRepository.cpp (the other binary's own guild persistence)
//    and its MySQLSessionRepository.cpp (GuildMember.LogOn on connect and
//    disconnect).
//  - WarScheduleInfo: the gameserver's MySQLWarInfoRepository.cpp and
//    MySQLGuildRepository.cpp. The sharedserver only cancels a deleted
//    guild's schedules (purgeGuild).
//  - Messages: the gameserver's MySQLMessageRepository.cpp (the reads and
//    deletes; the gameserver inserts none).
//  - Slayer, Vampire, Ousters: every other statement on the race tables
//    is the gameserver's (Character, Gold, Stash, SkillSave, RankBonus
//    and the other per-character repositories, CharacterPurge) or the
//    loginserver's (character creation and its purges). The sharedserver
//    writes only GuildID and Gold, and all of that is here.
//  - The castle and war probes (CastleInfo, WarScheduleInfo,
//    ReinforceRegisterInfo) are the gameserver's GuildRepository's; the
//    sharedserver's GuildManager::hasCastle / hasWarSchedule /
//    hasActiveWar answer false without asking the database, and nothing
//    in the sharedserver calls them.

// Which spelling of the Messages INSERT to send: the handlers wrote the
// same statement with two different whitespace layouts. Both parse alike;
// the caller says which text goes out.
enum SharedMessageSpelling {
    // "INSERT INTO Messages (Receiver, Message ) VALUES ('%s', '%s' )"
    // — GSQuitGuildHandler, GSModifyGuildMemberHandler.
    SHARED_MESSAGE_SQL_COMPACT,
    // "INSERT INTO Messages ( Receiver, Message ) VALUES ( '%s', '%s' )"
    // — GSAddGuildMemberHandler.
    SHARED_MESSAGE_SQL_SPACED,
    SHARED_MESSAGE_SQL_SPELLING_MAX
};

// GuildMember::load — the four columns it reads back.
struct SharedGuildMemberRow {
    int guildID;
    std::string name;
    int rank;
    int logOn;
};

// GuildManager::load's roster (Rank IN (0, 1, 2, 3)). requestDateTime is
// the datetime text; the caller parses it only for waiting members.
struct SharedGuildMemberListRow {
    int guildID;
    std::string name;
    int rank;
    std::string requestDateTime;
    int logOn;
};

// Guild::load — eight columns, the Intro excluded.
struct SharedGuildRow {
    std::string name;
    int type;
    int race;
    int state;
    int serverGroupID;
    int zoneID;
    std::string master;
    std::string date;
};

// GuildManager::load — ten columns, Intro included.
struct SharedGuildListRow {
    int id;
    std::string name;
    int type;
    int race;
    int state;
    int serverGroupID;
    int zoneID;
    std::string master;
    std::string date;
    std::string intro;
};

// Guild::create / Guild::save. insertGuild writes every field; saveGuild
// writes all but the intro.
struct SharedGuildRecord {
    GuildID_t id;
    std::string name;
    GuildType_t type;
    GuildRace_t race;
    GuildState_t state;
    ServerGroupID_t serverGroupID;
    ZoneID_t zoneID;
    std::string master;
    std::string date;
    std::string intro;
};

class SharedGuildRepository {
public:
    virtual ~SharedGuildRepository() {}

    // --- the roster (GuildMember) --------------------------------------------
    // Does a row for this name exist? Name is the primary key.
    virtual bool memberExists(const std::string& name) = 0;
    virtual void insertMember(GuildID_t guildID, const std::string& name, GuildMemberRank_t rank) = 0;
    // The waiting variant also writes RequestDateTime.
    virtual void insertWaitingMember(GuildID_t guildID, const std::string& name, GuildMemberRank_t rank,
                                     const std::string& requestDateTime) = 0;
    // The re-join UPDATEs for a name that already has a row: GuildID, Rank
    // and a cleared ExpireDate (the waiting variant also stamps
    // RequestDateTime).
    virtual void rejoinMember(GuildID_t guildID, GuildMemberRank_t rank, const std::string& name) = 0;
    virtual void rejoinWaitingMember(GuildID_t guildID, GuildMemberRank_t rank, const std::string& requestDateTime,
                                     const std::string& name) = 0;
    // False unless exactly one row, leaving row untouched.
    virtual bool loadMember(const std::string& name, SharedGuildMemberRow& row) = 0;
    virtual void saveMember(GuildID_t guildID, GuildMemberRank_t rank, const std::string& name) = 0;
    virtual void deleteMember(const std::string& name) = 0;
    // GuildMember::expire / leave — a GuildRank enumerator through "%d"
    // and the caller's "%03d%02d%02d" date text (seven characters; the
    // column is varchar(7)).
    virtual void setMemberRankAndExpireDate(int rank, const std::string& expireDate, const std::string& name) = 0;
    virtual void saveMemberIntro(const std::string& intro, const std::string& name) = 0;
    // False when the name has no row, leaving intro untouched.
    virtual bool loadMemberIntro(const std::string& name, std::string& intro) = 0;
    // RequestDateTime = now() — GSAddGuildMemberHandler stamps every
    // member when a waiting guild becomes active.
    virtual void stampMemberRequestDateTime(const std::string& name) = 0;
    // Every row with Rank IN (0, 1, 2, 3): normal, master, submaster,
    // waiting. Denied and left members (4, 5) are not loaded.
    virtual std::vector<SharedGuildMemberListRow> loadActiveMembers() = 0;

    // --- the guilds (GuildInfo) ----------------------------------------------
    virtual void insertGuild(const SharedGuildRecord& record) = 0;
    // False unless exactly one row, leaving row untouched.
    virtual bool loadGuild(GuildID_t id, SharedGuildRow& row) = 0;
    virtual void saveGuild(const SharedGuildRecord& record) = 0;
    // Guild::destroy: the GuildInfo row and the guild's GuildUnionMember
    // rows, on one Statement.
    virtual void deleteGuild(GuildID_t id) = 0;
    virtual void saveGuildIntro(const std::string& intro, GuildID_t id) = 0;
    // Guild::tinysave: "UPDATE GuildInfo SET <assignments> WHERE GuildID =
    // <id>". The assignment text is a SQL fragment the caller builds
    // (GSModifyGuildMemberHandler passes "Master='<name>'") and is
    // interpolated raw.
    virtual void updateGuildFields(const std::string& assignments, GuildID_t id) = 0;
    // GuildState IN (%d, %d).
    virtual std::vector<SharedGuildListRow> loadGuildsInStates(int stateA, int stateB) = 0;
    // GuildManager::deleteGuild's four statements on one Statement, in
    // this order: DELETE the GuildInfo row, DELETE the guild's GuildMember
    // rows, DELETE its GuildUnionMember rows, and UPDATE every
    // WarScheduleInfo row it attacks in (AttackGuildID only, not the
    // AttackGuildID2..5 slots) to Status 'CANCEL'. No transaction: a
    // failure part-way leaves the earlier deletes done.
    virtual void purgeGuild(GuildID_t id) = 0;

    // --- GuildManager::init's id probes -----------------------------------------
    // COUNT(*) over GuildInfo.
    virtual int countGuilds() = 0;
    // MAX(GuildID) through getInt. The caller asks only after countGuilds()
    // returned non-zero; on an empty table MySQL answers one NULL row and
    // getInt (atoi over a NULL field) would crash the process.
    virtual int loadMaxGuildID() = 0;
    // COUNT(*) of the guilds of one race (Guild::GuildRace through "%d").
    virtual int countGuildsOfRace(int race) = 0;
    // MAX(GuildZoneID) of the guilds of one race through getInt. Same
    // NULL caveat as loadMaxGuildID; the caller asks only after
    // countGuildsOfRace() returned non-zero.
    virtual int loadMaxGuildZoneIDOfRace(int race) = 0;

    // --- the character side of a membership change (the GS handlers) ----------
    // "UPDATE <race table> SET GuildID = %d WHERE Name = '%s'": the table
    // is chosen by race (Guild::GuildRace — 0 Slayer, 1 Vampire, 2
    // Ousters). A race outside those three runs no statement. The callers
    // pass the guild's id on joining, and the race's no-guild id on
    // leaving (99 for Slayer, 0 for Vampire, 66 for Ousters — except that
    // GSQuitGuildHandler's dissolve-on-quit branch passes 0 for Ousters).
    virtual void setCharacterGuildID(GuildRace_t race, int guildID, const std::string& name) = 0;
    // "UPDATE <race table> SET Gold = Gold + %d WHERE Name = '%s'" — the
    // registration-fee refund when a waiting guild's master quits. Same
    // race-to-table choice as setCharacterGuildID.
    virtual void addCharacterGold(GuildRace_t race, int gold, const std::string& name) = 0;
    // A Messages row (Receiver, Message); Sender is left at its default
    // ''. The message text is the StringPool's, interpolated raw.
    virtual void insertMessage(SharedMessageSpelling spelling, const std::string& receiver,
                               const std::string& message) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLSharedGuildRepository.cpp.
SharedGuildRepository& defaultSharedGuildRepository();

#endif
