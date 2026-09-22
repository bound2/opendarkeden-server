//////////////////////////////////////////////////////////////////////////////
// Filename    : GuildManager.h
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GUILDMANAGER_H__
#define __GUILDMANAGER_H__

#include <vector>

#include <unordered_map>

#include "Assert.h"
#include "Exception.h"
#include "Mutex.h"
#include "Timeval.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GuildManager
// Keeps the active guilds and the guilds waiting for registration in maps in
// memory, and handles registering and deleting guilds.
//
//////////////////////////////////////////////////////////////////////////////

class Guild;

typedef unordered_map<GuildID_t, Guild*> HashMapGuild;
typedef unordered_map<GuildID_t, Guild*>::iterator HashMapGuildItor;
typedef unordered_map<GuildID_t, Guild*>::const_iterator HashMapGuildConstItor;

#ifdef __SHARED_SERVER__
class SGGuildInfo;
#endif

class GCWaitGuildList;
class GCActiveGuildList;
class PlayerCreature;

class GuildManager {
    ///// Member methods /////

public: // constructor & destructor
    GuildManager();
    ~GuildManager();


public: // initializing related methods
    void init();
    void load();


public: // memory related methods
    void addGuild(Guild* pGuild);
    void addGuild_NOBLOCKED(Guild* pGuild);
    void deleteGuild(GuildID_t id);
    Guild* getGuild(GuildID_t id);
    Guild* getGuild_NOBLOCKED(GuildID_t id);

    void clear();
    void clear_NOBLOCKED();
    void retireAll_NOBLOCKED(); // what clear() does: retire, never free


public: // misc methods
    ushort getGuildSize() const {
        return m_Guilds.size();
    }
    HashMapGuild& getGuilds() {
        return m_Guilds;
    }
    const HashMapGuild& getGuilds_const() const {
        return m_Guilds;
    }

#ifdef __SHARED_SERVER__
public:
    void makeSGGuildInfo(SGGuildInfo& sgGuildInfo);
#endif

    void makeWaitGuildList(GCWaitGuildList& gcWaitGuildList, GuildRace_t race);
    void makeActiveGuildList(GCActiveGuildList& gcWaitGuildList, GuildRace_t race);

public:
    void lock() {
        m_Mutex.lock();
    }
    void unlock() {
        m_Mutex.unlock();
    }


public:
    void heartbeat();

public:
    bool isGuildMaster(GuildID_t guildID, PlayerCreature* pPC);

    string getGuildName(GuildID_t guildID);

    // Does the guild own a castle?
    bool hasCastle(GuildID_t guildID);
    bool hasCastle(GuildID_t guildID, ServerID_t& serverID, ZoneID_t& zoneID);

    // Has the guild applied for a war?
    bool hasWarSchedule(GuildID_t guildID);

    // Is there a war in progress?
    bool hasActiveWar(GuildID_t guidlID);

public: // debug
    string toString(void) const;


    ///// Member data /////

protected:
    unordered_map<GuildID_t, Guild*> m_Guilds; // Map of guild pointers
    // Guilds taken out of the map -- by deleteGuild(), or by the whole-table
    // clear() that the sharedserver resync triggers -- are parked here until
    // the destructor rather than deleted: getGuild() returns its Guild* after releasing m_Mutex, so a
    // zone thread may still be inside one -- reading it, holding its mutex --
    // while the SharedServerManager thread tears it down. A retired guild
    // stays readable with an empty member map; a freed one is a crash and,
    // with its mutex, undefined behaviour. Guild deletions are rare,
    // human-paced events, so this leaks nothing that matters.
    std::vector<Guild*> m_RetiredGuilds;

    Timeval m_WaitMemberClearTime; // When heartbeat clears out waiting guild members

    // mutex
    mutable Mutex m_Mutex;
};

#endif // __GUILDINFO_H__
