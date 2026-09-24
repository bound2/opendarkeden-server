#ifndef __GUILD_UNION_H__
#define __GUILD_UNION_H__

#include <list>

#include "Exception.h"
#include "GCUnionOfferList.h"
#include "Mutex.h"
#include "Types.h"
#include "guild/GuildUnionRegistry.h"

// The guild unions of this game server, loaded from GuildUnionInfo and
// GuildUnionMember and kept in step with them. Unions are changed from the
// zone threads (the union packets), the SharedServerManager thread (a guild
// deleted) and the LoginServerManager thread (a reload another game server
// asked for), and read from all of them.
//
// Locking. Three mutexes, taken in this order and never the other way:
//
//   1. GuildUnionManager::m_Mutex serialises the changes. Each change -- a
//      union opened, a guild added or removed, a union dissolved, the whole
//      set reloaded -- holds it across its table reads, its row writes and
//      its registry writes, so no two changes interleave.
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

    // Replaces the unions in memory with the ones in the tables; the unions
    // replaced are retired. A table read that fails leaves the old set.
    void load();
    void reload();

    // The union holding this guild, master or member, or NULL for a guild in
    // none.
    GuildUnion* getGuildUnion(GuildID_t gID) const {
        return m_Unions.unionOfGuild(gID);
    }
    GuildUnion* getGuildUnionByUnionID(uint uID) const {
        return m_Unions.unionByID(uID);
    }

    // The union the guild resolves to; a union of its own, row and all, when
    // it resolves to none. The union answered may have the guild as a member
    // rather than as its master.
    GuildUnion* openUnion(GuildID_t masterGID);

    bool addGuild(uint uID, GuildID_t gID);

    // Settle a guild's union standing because the guild is going away. A
    // member guild leaves and the union carries on; a master guild takes the
    // union with it, since mastery cannot be handed on. The rule is
    // decideUnionTeardown's (guild/GuildUnionTeardown.h); this performs it
    // over the union tables, the lookup maps and the guilds still online.
    // Call it while the guild is still in the GuildManager: the guild
    // masters it notifies are read from there, and a guild that has already
    // gone is logged to GuildUnion.log and skipped.
    bool removeGuildFromUnion(GuildID_t gID);

    bool removeGuild(uint uID, GuildID_t gID);

    void sendRefreshCommand();
    void sendModifyUnionInfo(uint gID);

    static GuildUnionManager& Instance() {
        static GuildUnionManager theInstance;
        return theInstance;
    }

private:
    // Dissolve a union: its own row and its member rows go, and no guild
    // resolves to it any more. The caller holds m_Mutex.
    void destroyUnion_LOCKED(uint uID);

    GuildUnionRegistry m_Unions;

    // Serialises the changes; see the class comment.
    mutable Mutex m_Mutex;
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

    uint acceptJoin(GuildID_t gID); // Accept a union join
    uint acceptQuit(GuildID_t gID); // Accept a union withdrawal

    uint denyJoin(GuildID_t gID); // Deny a union join
    uint denyQuit(GuildID_t gID); // Deny a union withdrawal

    void clearOffer(GuildID_t gID);
    bool hasOffer(GuildID_t gID);

    bool makeOfferList(uint uID, GCUnionOfferList& offerList); // Request the list

    static GuildUnionOfferManager& Instance() {
        static GuildUnionOfferManager theInstance;
        return theInstance;
    }
};

#endif
