#ifndef __GUILD_UNION_H__
#define __GUILD_UNION_H__

#include <list>
#include <vector>

#include "Exception.h"
#include "GCUnionOfferList.h"
#include "Mutex.h"
#include "Types.h"
#include "guild/GuildUnionJoinOffer.h"
#include "guild/GuildUnionRegistry.h"

// The guild unions of this game server, loaded from GuildUnionInfo and
// GuildUnionMember and kept in step with them. Unions are changed from the
// zone threads (the union packets), the SharedServerManager thread (a guild
// deleted) and the LoginServerManager thread (a reload another game server
// asked for), and read from all of them.
//
// Offers. A guild asking to join a union has a JOIN row in GuildUnionOffer,
// a member guild asking to leave its union a QUIT row, and a guild forced
// out of a union an ESCAPE row, its penalty; the table holds one row per
// guild. Every row lives ten days from its OfferTime
// (kUnionOfferLifetimeDays): a JOIN or QUIT offer nobody answered in that
// time lapses -- the applicant stays out, the member stays in, and either
// may ask again -- and the ESCAPE penalty ends. Expired rows are purged
// (purgeOffers) before an offer is recorded or answered, before a union's
// offers are listed to its master, and at every load, so nothing acts on an
// expired row; the purge also drops a JOIN or QUIT row naming a union that
// has gone. A guild leaving its union by any road takes its QUIT row with
// it, and a guild going away takes all its rows.
//
// A union exists for its member guilds and for its pending (unexpired) JOIN
// offers (unionIsAbandoned). Every change that takes one of them away -- a
// member expelled, quitting or deleted, an offer accepted, denied, purged
// or expired -- applies that rule (dissolveIfAbandoned_LOCKED), and only
// the loss of its master guild dissolves a union regardless. A union that
// goes takes the JOIN and QUIT rows naming it with it.
//
// Locking. Three mutexes, taken in this order and never the other way:
//
//   1. GuildUnionManager::m_Mutex serialises the changes. Each change -- a
//      union opened, a guild added or removed, an offer purged, a union
//      dissolved, the whole set reloaded -- holds it across its table
//      reads, its row writes and its registry writes, so no two changes
//      interleave.
//   2. GuildUnionRegistry's mutex guards the lookup tables. Readers take it
//      for the lookup alone, so a reader never waits on the database.
//   3. GuildUnion's mutex guards one union's member list.
//
// All three are taken after anything else the caller holds -- the zone-group
// mutex, the PC finder's critical section, the GuildManager and Guild
// mutexes, the LoginServerManager and SharedServerManager mutexes, which the
// callers hold in every combination -- and none of those is ever taken while
// one of these is held: under them the union code uses only the next mutex
// down and the thread's database connection. The guild master lookups, the
// notifications and the refresh sent to the other game servers run after
// m_Mutex is released.
//
// A union a change takes away is retired, not freed (GuildUnion), so the
// pointers getGuildUnion() and getGuildUnionByUnionID() hand out stay
// readable; a retired union resolves from neither.
class GuildUnionManager {
public:
    GuildUnionManager();
    ~GuildUnionManager();

    // Purges the offers (see purgeOffers), then brings the unions in memory
    // in line with the tables: a union whose id and master are unchanged
    // keeps its object and takes the member list the tables hold; a union
    // that vanished or changed master is retired
    // (GuildUnionRegistry::replaceAll). A table read that fails leaves the
    // old set. load() is the startup load: nobody is online to tell, and a
    // union it dissolves is owed to the other game servers until
    // sendOwedRefresh(). reload() tells the guilds and the servers at once.
    void load();
    void reload();

    // Sends the refresh a startup load owes the other game servers, once the
    // link to them is up; nothing when the load dissolved nothing.
    void sendOwedRefresh();

    // The union holding this guild, master or member, or NULL for a guild in
    // none.
    GuildUnion* getGuildUnion(GuildID_t gID) const {
        return m_Unions.unionOfGuild(gID);
    }
    GuildUnion* getGuildUnionByUnionID(uint uID) const {
        return m_Unions.unionByID(uID);
    }

    // Records `applicantGID`'s offer to join the union `masterGID` leads, or
    // refuses it; decideUnionJoinOffer (guild/GuildUnionJoinOffer.h) is the
    // rule, applied after the expired offers are purged. When the target
    // leads no union and the offer is not refused, a union is opened for it
    // first, and the other game servers are told to reload. The facts are
    // read, the rule applied and the rows written under m_Mutex, so the
    // offer cannot interleave with another offer to the same guild or with a
    // dissolve of the union it targets. `tooManyMembers` comes from the
    // guild manager, whose locks are taken before m_Mutex, never under it.
    UnionJoinOfferVerdict recordJoinOffer(GuildID_t applicantGID, GuildID_t masterGID, bool tooManyMembers);

    // Deletes every offer row past its lifetime and every JOIN or QUIT row
    // naming a union that has gone (decideUnionOfferPurge), dissolves the
    // unions an expired JOIN offer alone kept, and tells their master guilds
    // and the other game servers. Call it before reading an offer to act on.
    void purgeOffers();

    // Dissolves the union if nobody is left for it -- no member row and no
    // pending join offer (unionIsAbandoned) -- and tells its master guild and
    // the other game servers. For the paths that take away a union's last
    // offer. True when the union went.
    bool dissolveIfAbandoned(uint uID);

    bool addGuild(uint uID, GuildID_t gID);

    // Settle a guild's union standing because the guild is going away. Its
    // own offer rows go, a JOIN among them under the abandoned-union rule. A
    // member guild leaves and the union carries on while a member or a
    // pending join offer is left; a master guild takes the union with it,
    // since mastery cannot be handed on. The membership rule is
    // decideUnionTeardown's (guild/GuildUnionTeardown.h); this performs it
    // over the union tables, the lookup maps and the guilds still online.
    // Call it while the guild is still in the GuildManager: the guild
    // masters it notifies are read from there, and a guild that has already
    // gone is logged to GuildUnion.log and skipped.
    bool removeGuildFromUnion(GuildID_t gID);

    // Takes a member guild out of the union: its member row and its QUIT row
    // go, and the union goes too when that leaves it abandoned
    // (unionIsAbandoned). False when the union does not hold the guild as a
    // member. `pDissolved`, when given, says whether the union went.
    bool removeGuild(uint uID, GuildID_t gID, bool* pDissolved = NULL);

    void sendRefreshCommand();
    void sendModifyUnionInfo(uint gID);

    static GuildUnionManager& Instance() {
        static GuildUnionManager theInstance;
        return theInstance;
    }

private:
    // What a change owes the world once m_Mutex is released: the guilds
    // whose union standing it changed, and whether the other game servers
    // must reload.
    struct UnionChanges {
        std::vector<GuildID_t> guildsToNotify;
        bool refresh = false;
    };

    // Tells each guild once, then the other game servers. The caller does
    // not hold m_Mutex.
    void publish(const UnionChanges& changes);

    // Dissolve a union: its own row, its member rows and the JOIN and QUIT
    // rows naming it go, and no guild resolves to it any more. The caller
    // holds m_Mutex.
    void destroyUnion_LOCKED(uint uID);
    // The abandoned-union rule: dissolves the union when it has no member
    // row and no pending join offer, recording its master guild and the
    // refresh in `changes`. False, and nothing done, for a union still held
    // or one that does not exist. The caller holds m_Mutex.
    bool dissolveIfAbandoned_LOCKED(uint uID, UnionChanges& changes);
    // purgeOffers' work. The caller holds m_Mutex.
    void purgeOffers_LOCKED(UnionChanges& changes);
    // load's work. The caller holds m_Mutex.
    void load_LOCKED(UnionChanges& changes);

    GuildUnionRegistry m_Unions;

    // Serialises the changes; see the class comment.
    mutable Mutex m_Mutex;

    // A startup load dissolved a union the other game servers still hold.
    // Guarded by m_Mutex.
    bool m_RefreshOwed = false;
};

class GuildUnionOffer {
public:
    enum OfferType { JOIN, QUIT };
    BYTE m_Type;
    GuildID_t m_GuildID;
    string m_MasterID;
    string m_GuildName;
    string m_MasterName;
};

class GuildUnionOfferManager {
public:
    enum ErrorType {
        OK = 0,
        ALREADY_IN_UNION,
        ALREADY_OFFER_SOMETHING,
        TARGET_IS_NOT_MASTER, // The other side is not a master
        NOT_IN_UNION,
        MASTER_CANNOT_QUIT, // A union master guild cannot leave on its own (automatic once no sub-guild is left)
        NO_TARGET_UNION,
        NOT_YOUR_UNION,
        SOURCE_IS_NOT_MASTER, // The applicant is not a master
        YOU_HAVE_PENALTY,     // Has a record of a forced withdrawal and cannot apply
        NOT_ENOUGH_SLOT,      // On OfferJoin: the joinable slots are full
        TOO_MANY_MEMBER       // Cannot join because there are 50 or more members


    };
    uint offerJoin(GuildID_t gID, GuildID_t MasterGID); // Apply to join a union
    uint offerQuit(GuildID_t gID);                      // Apply to leave a union

    // A union master answering guild gID's pending offer. `answeringUnionID`
    // is the union the master leads; an offer naming another union is
    // refused with NOT_YOUR_UNION and left for its own master
    // (decideUnionOfferAnswer), and a guild with no pending offer of that
    // kind -- none made, already answered, or expired -- is answered
    // NO_TARGET_UNION. The expired offers are purged first.
    uint acceptJoin(GuildID_t gID, uint answeringUnionID); // Accept a union join
    uint acceptQuit(GuildID_t gID, uint answeringUnionID); // Accept a union withdrawal

    uint denyJoin(GuildID_t gID, uint answeringUnionID); // Deny a union join
    uint denyQuit(GuildID_t gID, uint answeringUnionID); // Deny a union withdrawal

    void clearOffer(GuildID_t gID);
    bool hasOffer(GuildID_t gID);

    // The offers naming the union, for its master, after the expired ones are
    // purged. False when there is none, or the purge dissolved the union.
    bool makeOfferList(uint uID, GCUnionOfferList& offerList);

    static GuildUnionOfferManager& Instance() {
        static GuildUnionOfferManager theInstance;
        return theInstance;
    }
};

#endif
