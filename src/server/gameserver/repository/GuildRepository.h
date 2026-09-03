#ifndef __GUILD_REPOSITORY_H__
#define __GUILD_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Persistence seam for the guild tables the gameserver owns (task 3.2):
// the guilds (GuildInfo), their members (GuildMember), the guild unions
// (GuildUnionInfo, GuildUnionMember) and the union join/quit offers
// (GuildUnionOffer), plus the guild-scoped reads of the war tables
// (CastleInfo, WarScheduleInfo, ReinforceRegisterInfo) GuildManager makes
// before a guild may register a war or be dissolved. Reads are typed to
// the driver getter the inline code called (getInt → int, getString →
// std::string); the writes' parameters to the members/expressions each
// caller streamed, so the varargs bytes reaching the format strings are
// unchanged (GuildID_t is a WORD, the rank/type/race/state BYTEs, all
// promoted the same way as before).
//
// Not enclosed: the sharedserver's own Guild.cpp / GuildManager.cpp (a
// separate copy with its own SQL, WarScheduleInfo statements included),
// CGSayHandler's GM guild commands, and the SG*Guild* handlers that
// mutate guild rows from the SharedServerManager thread. war/'s own
// WarScheduleInfo writes are no longer among them — WarInfoRepository
// took the last of them with the war-scheduler round. The five
// guild-union handlers were never on this list; their statements are
// enclosed as of this round.

// Which spelling of the union handlers' two shared statements to write.
// CGDenyUnionHandler backticks every identifier where the two quit
// handlers do not; the bytes differ and task 3.2 moves statements
// without rewriting them, so the caller says which it used. The two
// parse to the same statement for these identifiers — neither
// GuildUnionMember, GuildUnionInfo nor UnionID is reserved — and the
// case of count() is not significant either. Collapsing the two later
// is a small change, the same one MessageRepository.h describes for
// its three.
enum UnionStatementSpelling {
    // CGQuitUnionHandler, CGQuitUnionAcceptHandler.
    UNION_SQL_PLAIN,
    // CGDenyUnionHandler.
    UNION_SQL_QUOTED,
    UNION_SQL_SPELLING_MAX
};

// GuildMember::load — the four columns it reads back.
struct GuildMemberRow {
    int guildID;
    std::string name;
    int rank;
    int logOn;
};

// GuildManager::load's member list (Rank IN (0, 1, 2, 3)).
struct GuildMemberListRow {
    int guildID;
    std::string name;
    int rank;
    std::string requestDateTime;
    int logOn;
};

// Guild::load — eight columns, the Intro excluded.
struct GuildRow {
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
struct GuildListRow {
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

// Guild::create / Guild::save — the members as streamed (save does not
// write the intro; create writes correctString(intro)).
struct GuildRecord {
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

struct UnionRow {
    int unionID;
    int masterGuildID;
};

// GuildUnionOfferManager::makeOfferList — OfferType+0 (the enum ordinal),
// the offering guild, and DATE_FORMAT(OfferTime, '%y%m%d') read as an int.
struct UnionOfferRow {
    int offerType;
    int ownerGuildID;
    int date;
};

class GuildRepository {
public:
    virtual ~GuildRepository() {}

    // --- members (GuildMember) ----------------------------------------------
    // GuildMember::create: does a row for this name exist already?
    virtual bool memberExists(const std::string& name) = 0;
    virtual void insertMember(GuildID_t guildID, const std::string& name, GuildMemberRank_t rank) = 0;
    virtual void insertWaitingMember(GuildID_t guildID, const std::string& name, GuildMemberRank_t rank,
                                     const std::string& requestDateTime) = 0;
    // The re-join UPDATEs: GuildID, Rank and a cleared ExpireDate (the
    // waiting variant also stamps RequestDateTime).
    virtual void rejoinMember(GuildID_t guildID, GuildMemberRank_t rank, const std::string& name) = 0;
    virtual void rejoinWaitingMember(GuildID_t guildID, GuildMemberRank_t rank, const std::string& requestDateTime,
                                     const std::string& name) = 0;
    // False unless exactly one row (Name is the primary key).
    virtual bool loadMember(const std::string& name, GuildMemberRow& row) = 0;
    virtual void saveMember(GuildID_t guildID, GuildMemberRank_t rank, const std::string& name) = 0;
    virtual void deleteMember(const std::string& name) = 0;
    // GuildMember::expire / leave — a GuildRank enumerator through "%d" and
    // the caller's "%03d%02d%02d" date text.
    virtual void setMemberRankAndExpireDate(int rank, const std::string& expireDate, const std::string& name) = 0;
    virtual void saveMemberIntro(const std::string& intro, const std::string& name) = 0;
    virtual bool loadMemberIntro(const std::string& name, std::string& intro) = 0;
    virtual std::vector<GuildMemberListRow> loadActiveMembers() = 0;

    // --- guilds (GuildInfo) ---------------------------------------------------
    virtual void insertGuild(const GuildRecord& record) = 0;
    // False unless exactly one row.
    virtual bool loadGuild(GuildID_t id, GuildRow& row) = 0;
    virtual void saveGuild(const GuildRecord& record) = 0;
    // Guild::destroy — the GuildInfo row and the guild's GuildUnionMember rows.
    virtual void deleteGuild(GuildID_t id) = 0;
    // GuildManager::load — GuildState IN (%d, %d).
    virtual std::vector<GuildListRow> loadGuildsInStates(int stateA, int stateB) = 0;
    // makeOfferList's per-offer lookup; false when the guild has no row.
    virtual bool loadGuildNameAndMaster(int guildID, std::string& name, std::string& master) = 0;

    // --- castles and wars (GuildManager's guild-scoped reads) ---------------------
    virtual int countCastlesOfGuild(int guildID) = 0;
    virtual bool loadCastleOfGuild(int guildID, int& serverID, int& zoneID) = 0;
    // WarScheduleInfo rows in WAIT/START naming the guild in any of the five
    // attacker slots.
    virtual int countWarSchedulesOfAttacker(int guildID) = 0;
    virtual int countReinforceRegistrations(int guildID) = 0;
    virtual int countStartedWarsAtCastle(ServerID_t serverID, ZoneID_t zoneID) = 0;
    virtual int countStartedWarsOfAttacker(int guildID) = 0;

    // --- unions (GuildUnionInfo, GuildUnionMember) ------------------------------
    // Returns the AUTO_INCREMENT UnionID.
    virtual uint insertUnion(GuildID_t masterGuildID) = 0;
    virtual void insertUnionMember(uint unionID, GuildID_t guildID) = 0;
    // False when no row matched (the caller logs it).
    virtual bool deleteUnionMember(uint unionID, GuildID_t guildID) = 0;
    virtual void deleteUnion(uint unionID) = 0;
    virtual std::vector<UnionRow> loadUnions() = 0;
    virtual std::vector<int> loadUnionMemberGuilds(uint unionID) = 0;
    // The first (UnionID, OwnerGuildID) row of a guild; false when none.
    virtual bool loadUnionOfGuild(GuildID_t guildID, int& unionID, int& ownerGuildID) = 0;
    virtual bool loadUnionMaster(int unionID, int& masterGuildID) = 0;
    // "WHERE UnionID='%u'" — the quoted numeric key, kept.
    virtual int countUnionMembers(uint unionID) = 0;
    // The union handlers' own copy of that count, spelled with a
    // lowercase count() rather than COUNT(). Deliberately NOT an overload
    // of countUnionMembers: an unscoped enumerator converts to uint, so
    // a one-argument call with a spelling would compile and silently
    // count union 0.
    //
    // Each caller USED to call next() without checking it, which a COUNT
    // always satisfies, and still compares the result against 0 to decide
    // whether the union is now empty; the next() lives in countOf now.
    // Note the direction of failure that implies: the
    // inline code threw out of the handler if the row were ever missing
    // (getField logs to ResultBug.log and raises Error on a NULL row),
    // where this returns 0 and the caller reads that as "empty" and
    // DELETEs the union. Unreachable for a COUNT on a live connection,
    // but it is the first destructive statement this helper gates.
    virtual int countUnionMembersSpelled(UnionStatementSpelling spelling, uint unionID) = 0;
    // DELETE FROM GuildUnionInfo alone. NOT deleteUnion(), which also
    // clears the union's GuildUnionMember rows: these callers drop the
    // info row only, having just found the member table empty.
    virtual void deleteUnionInfoOnly(UnionStatementSpelling spelling, uint unionID) = 0;

    // --- union offers (GuildUnionOffer) ---------------------------------------------
    // ESCAPE offers of the last ten days (the join penalty).
    virtual int countRecentEscapes(GuildID_t guildID) = 0;
    virtual void deleteStaleOffers(GuildID_t guildID) = 0;
    virtual void insertJoinOffer(uint unionID, GuildID_t guildID) = 0;
    virtual void insertQuitOffer(uint unionID, GuildID_t guildID) = 0;
    // CGQuitUnionHandler's ESCAPE offer, the row countRecentEscapes
    // later counts. Written POSITIONALLY, unlike the two above — it
    // names no columns and so depends on GuildUnionOffer's column
    // order. Kept as it was.
    virtual void insertEscapeOffer(uint unionID, GuildID_t guildID) = 0;
    virtual std::vector<UnionOfferRow> loadOffers(uint unionID) = 0;
    virtual bool loadJoinOfferUnion(GuildID_t guildID, int& unionID) = 0;
    virtual bool loadQuitOfferUnion(GuildID_t guildID, int& unionID) = 0;
    virtual void deleteOffers(GuildID_t guildID) = 0;
    virtual int countOffers(GuildID_t guildID) = 0;
};

// The process-wide MySQL-backed instance, wired in MySQLGuildRepository.cpp.
// An accessor function rather than a g_p* extern: ratchet R1 counts those.
GuildRepository& defaultGuildRepository();

#endif
