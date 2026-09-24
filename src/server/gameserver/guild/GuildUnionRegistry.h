//////////////////////////////////////////////////////////////////////////////
// Filename    : GuildUnionRegistry.h
// Description : the guild unions a game server holds in memory and the
//               union each guild resolves to, kept apart from the union
//               manager so the locking and the retirement can be exercised
//               with neither the union tables nor a database.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GUILD_UNION_REGISTRY_H__
#define __GUILD_UNION_REGISTRY_H__

#include <atomic>
#include <list>
#include <memory>
#include <vector>

#include <unordered_map>

#include "Mutex.h"
#include "Types.h"

class GuildUnionRegistry;

// One guild union: a master guild and the guilds that joined it. The id and
// the master are fixed for the object's life. The member list is guarded by
// the union's own mutex, a leaf: nothing else is taken while it is held.
//
// A union the registry lets go of is retired, not freed, because a thread
// that looked it up may still hold the pointer. A retired union keeps its id
// and its master -- what the reader would have read a moment earlier, and
// all a reader hands back to the manager, which resolves the id again under
// its own lock and finds nothing -- and names no guild as a member:
// hasGuild() is false for every guild and getGuildList() is empty. A reader
// that acts on the membership alone, rather than going back to the manager,
// tests isRetired().
class GuildUnion {
public:
    GuildUnion(uint unionID, GuildID_t masterGuildID, const std::list<GuildID_t>& memberGuilds = {});

    GuildUnion(const GuildUnion&) = delete;
    GuildUnion& operator=(const GuildUnion&) = delete;

    uint getUnionID() const {
        return m_UnionID;
    }
    GuildID_t getMasterGuildID() const {
        return m_MasterGuildID;
    }

    // The master guild or a member guild. False for every guild once retired.
    bool hasGuild(GuildID_t gID) const;

    // A copy of the member guilds, the master not among them. Empty once
    // retired.
    std::list<GuildID_t> getGuildList() const;

    bool isRetired() const {
        return m_Retired.load();
    }

private:
    friend class GuildUnionRegistry;

    // Registry side, under the registry's mutex.
    bool addMember(GuildID_t gID);
    bool removeMember(GuildID_t gID);
    void retire();

    const uint m_UnionID;
    const GuildID_t m_MasterGuildID;

    std::list<GuildID_t> m_Guilds; // guarded by m_Mutex
    std::atomic<bool> m_Retired{false};
    mutable Mutex m_Mutex;
};

// The live unions, by id and by the guilds they hold, and the retired ones.
// Every lookup and every change takes the registry's mutex; under it only a
// union's own mutex is taken. The pointers the lookups hand out stay valid
// until the registry is destroyed: a union taken out of the tables is
// retired (see GuildUnion), never freed.
class GuildUnionRegistry {
public:
    GuildUnionRegistry();
    ~GuildUnionRegistry();

    GuildUnionRegistry(const GuildUnionRegistry&) = delete;
    GuildUnionRegistry& operator=(const GuildUnionRegistry&) = delete;

    // The live union holding the guild, as master or member, or NULL.
    GuildUnion* unionOfGuild(GuildID_t gID) const;
    // The live union with this id, or NULL.
    GuildUnion* unionByID(uint uID) const;

    // Makes a union live: its id, its master and its members resolve to it.
    // A guild some other live union already holds resolves to this one from
    // now on, as the tables loaded in order would have it.
    GuildUnion* publish(std::unique_ptr<GuildUnion> pUnion);

    // Adds a member guild to the live union with this id. False when there
    // is no such union or the guild already resolves to a union.
    bool addMember(uint uID, GuildID_t gID);

    // Takes a member guild out of the live union with this id; the guild
    // stops resolving to it. False when there is no such union, the guild is
    // its master, or the union does not list it.
    bool removeMember(uint uID, GuildID_t gID);

    // Retires the live union with this id: neither its id nor any of its
    // guilds resolves to it any more. False when there is no such union.
    bool retire(uint uID);

    // Retires every live union and makes `fresh` live in their place, in one
    // step, so a reader finds either the old set or the new one.
    void replaceAll(std::vector<std::unique_ptr<GuildUnion>> fresh);

private:
    void publish_LOCKED(GuildUnion* pUnion);
    void retire_LOCKED(GuildUnion* pUnion);

    std::vector<std::unique_ptr<GuildUnion>> m_Live;
    std::vector<std::unique_ptr<GuildUnion>> m_Retired;
    std::unordered_map<GuildID_t, GuildUnion*> m_ByGuild;
    std::unordered_map<uint, GuildUnion*> m_ByID;

    mutable Mutex m_Mutex;
};

#endif
