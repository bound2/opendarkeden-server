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
    // The member list becomes `memberGuilds`, in one step under the union's
    // mutex: a reader copies either the old list or the new one.
    void replaceMembers(const std::list<GuildID_t>& memberGuilds);
    void retire();

    const uint m_UnionID;
    const GuildID_t m_MasterGuildID;

    std::list<GuildID_t> m_Guilds; // guarded by m_Mutex
    std::atomic<bool> m_Retired{false};
    mutable Mutex m_Mutex;
};

// The live unions, by id and by the guilds they hold, and the retired ones.
// A union is retired when it is dissolved or when a reload no longer finds
// it with the same master; a reload keeps every other union's object.
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

    // Makes the live set the one `fresh` describes, in one step, so a lookup
    // finds either the old set or the new one. A live union whose id and
    // master `fresh` still names is kept -- the same object, a reader's
    // pointer to it stays live -- with its member list replaced by the fresh
    // one; the fresh copy is dropped unpublished. A live union `fresh` no
    // longer names, or names with another master, is retired, and a fresh
    // union no live one matches is published. The guild and id entries are
    // rebuilt from the result in `fresh`'s order, so a guild two unions list
    // resolves to the later one, as publish() would have it.
    void replaceAll(std::vector<std::unique_ptr<GuildUnion>> fresh);

    // How many unions have been retired over the registry's life. Each one
    // stays allocated until the registry goes, so this is what a reload that
    // changes nothing must leave unchanged.
    size_t retiredCount() const;

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
