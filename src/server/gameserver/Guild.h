//////////////////////////////////////////////////////////////////////////////
// Filename		: Guild.h
// Written by	: bezz
// Description	:
//////////////////////////////////////////////////////////////////////////////

#ifndef __GUILD_H__
#define __GUILD_H__

#include <atomic>
#include <list>
#include <string>
#include <utility>
#include <vector>

#include <unordered_map>

#include "Assert.h"
#include "Exception.h"
#include "Mutex.h"
#include "Types.h"
#include "VSDateTime.h"

#ifdef __SHARED_SERVER__
class GuildInfo2;
#endif

class GuildInfo;
class GCGuildMemberList;

//////////////////////////////////////////////////////////////////////////////
// class GuildMember
// Holds the information about a guild member.
//
//////////////////////////////////////////////////////////////////////////////

const Gold_t REQUIRE_SLAYER_MASTER_GOLD = 10000000;
const Gold_t REQUIRE_VAMPIRE_MASTER_GOLD = 10000000;
const Gold_t REQUIRE_OUSTERS_MASTER_GOLD = 10000000;

const Gold_t REQUIRE_SLAYER_SUBMASTER_GOLD = 0;
const Gold_t REQUIRE_VAMPIRE_SUBMASTER_GOLD = 0;
const Gold_t REQUIRE_OUSTERS_SUBMASTER_GOLD = 0;

const Gold_t RETURN_SLAYER_MASTER_GOLD = 9000000;
const Gold_t RETURN_VAMPIRE_MASTER_GOLD = 9000000;
const Gold_t RETURN_OUSTERS_MASTER_GOLD = 9000000;

const Gold_t RETURN_SLAYER_SUBMASTER_GOLD = 0;
const Gold_t RETURN_VAMPIRE_SUBMASTER_GOLD = 0;
const Gold_t RETURN_OUSTERS_SUBMASTER_GOLD = 0;

const Fame_t REQUIRE_SLAYER_MASTER_FAME[SKILL_DOMAIN_ETC] = {
    100000, // SKILL_DOMAIN_BLADE
    100000, // SKILL_DOMAIN_SWORD
    100000, // SKILL_DOMAIN_GUN
    20000,  // SKILL_DOMAIN_HEAL
    30000   // SKILL_DOMAIN_ENCHANT
};

const Fame_t REQUIRE_SLAYER_SUBMASTER_FAME[SKILL_DOMAIN_ETC] = {
    0, // SKILL_DOMAIN_BLADE
    0, // SKILL_DOMAIN_SWORD
    0, // SKILL_DOMAIN_GUN
    0, // SKILL_DOMAIN_HEAL
    0  // SKILL_DOMAIN_ENCHANT
};

const SkillLevel_t REQUIRE_SLAYER_MASTER_SKILL_DOMAIN_LEVEL = 50;
const Level_t REQUIRE_VAMPIRE_MASTER_LEVEL = 50;
const Level_t REQUIRE_OUSTERS_MASTER_LEVEL = 50;

const SkillLevel_t REQUIRE_SLAYER_SUBMASTER_SKILL_DOMAIN_LEVEL = 30;
const Level_t REQUIRE_VAMPIRE_SUBMASTER_LEVEL = 40;
const Level_t REQUIRE_OUSTERS_SUBMASTER_LEVEL = 40;

class GuildMember {
public:
    GuildMember();

    ///// Member constants /////

public:
    enum GuildRank {
        GUILDMEMBER_RANK_NORMAL = 0, // Ordinary member
        GUILDMEMBER_RANK_MASTER,     // Guild master
        GUILDMEMBER_RANK_SUBMASTER,  // Guild sub master
        GUILDMEMBER_RANK_WAIT,       // Waiting to join the guild
        GUILDMEMBER_RANK_DENY,       // Expelled or refused
        GUILDMEMBER_RANK_LEAVE,      // Left the guild voluntarily

        GUILDMEMBER_RANK_MAX
    };


public: // DB methods
    void create();
    bool load();
    void save();
    void destroy();
    void expire();
    void leave();

    void saveIntro(const string& intro);
    string getIntro() const;


public: // identity methods
    GuildID_t getGuildID() const {
        return m_GuildID;
    }
    void setGuildID(GuildID_t guildID) {
        m_GuildID = guildID;
    }

    string getName() const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
    }

    GuildMemberRank_t getRank() const {
        return m_Rank.load(std::memory_order_relaxed);
    }
    void setRank(GuildMemberRank_t rank); // Handled in the Guild class.

    bool getLogOn() const {
        return m_bLogOn.load(std::memory_order_relaxed);
    }
    void setLogOn(bool logOn) {
        m_bLogOn.store(logOn, std::memory_order_relaxed);
    }

    ServerID_t getServerID() const {
        return m_ServerID.load(std::memory_order_relaxed);
    }
    void setServerID(ServerID_t ServerID) {
        m_ServerID.store(ServerID, std::memory_order_relaxed);
    }

    string getRequestDateTime() const;
    void setRequestDateTime(const VSDateTime& vsdatetime) {
        m_RequestDateTime = vsdatetime;
    }
    void setRequestDateTime(const string& rdatetime);
    bool isRequestDateTimeOut(const VSDateTime& currentDateTime) const;

public: // debug
    string toString() const;


    ///// operator overloadgin /////

public:
    GuildMember& operator=(GuildMember& Member);


    ///// Member data /////

protected:
    GuildID_t m_GuildID; // Guild ID
    string m_Name;       // Member name
    // Rank, log-on and server are written by the SG handlers on the
    // SharedServerManager thread and read by zone threads through
    // Guild::getMember(); each is an independent flag, so an atomic rather
    // than a field under the guild mutex.
    std::atomic<GuildMemberRank_t> m_Rank; // The member's rank
    VSDateTime m_RequestDateTime;          // Time the join request was made
    std::atomic<bool> m_bLogOn;            // Whether the member is logged on
    std::atomic<ServerID_t> m_ServerID;    // Which server the member is on
};


//////////////////////////////////////////////////////////////////////////////
// class Guild
// Holds the information about a guild.
//
// Structure of the GuildInfo table
// ----------------------------------------
// GuildID            INT
// GuildName          VARCHAR(20)
// GuildType          TINYINT
// GuildGold          INT
// ----------------------------------------
//////////////////////////////////////////////////////////////////////////////

#define MAX_GUILDMEMBER_ACTIVE_COUNT 100
#define MIN_GUILDMEMBER_COUNT 3
#define MAX_GUILDMEMBER_WAIT_COUNT 15

typedef unordered_map<string, GuildMember*> HashMapGuildMember;
typedef HashMapGuildMember::iterator HashMapGuildMemberItor;
typedef HashMapGuildMember::const_iterator HashMapGuildMemberConstItor;

class Guild {
    ///// Member constants /////

public:
    enum GuildTypes {
        GUILD_TYPE_NORMAL = 0, // Ordinary guild
        GUILD_TYPE_JUDGE,      // Judge guild
        GUILD_TYPE_ASSASSIN,   // Assassin guild

        GUILD_TYPE_MAX
    };

    enum GuildState {
        GUILD_STATE_ACTIVE = 0, // Active guild
        GUILD_STATE_WAIT,       // Guild waiting for registration
        GUILD_STATE_CANCEL,     // Cancelled guild
        GUILD_STATE_BROKEN,     // Disbanded guild

        GUILD_STATE_MAX
    };

    enum GuildRace {
        GUILD_RACE_SLAYER = 0, // Slayer guild
        GUILD_RACE_VAMPIRE,    // Vampire guild
        GUILD_RACE_OUSTERS,    // Ousters guild

        GUILD_RACE_MAX
    };

public: // constructor & destructor
    Guild();
    virtual ~Guild();


public: // DB methods
    void create();
    bool load();
    void save();
    void destroy();


public: // identity methods
    // The integral fields are independent values, so each is its own atomic
    // and is read and written relaxed: a reader gets some value the writer
    // stored, never a torn one, and no field orders another.
    GuildID_t getID() const {
        return m_ID.load(std::memory_order_relaxed);
    }
    void setID(GuildID_t id) {
        m_ID.store(id, std::memory_order_relaxed);
    }

    // The string fields are copied out of / into the object under m_Mutex.
    // m_Mutex is not recursive, so none of these four getter/setter pairs may
    // be called while this guild's m_Mutex is already held: code inside Guild
    // that runs under the mutex touches the members directly.
    string getName() const;
    void setName(const string& name);

    GuildType_t getType() const {
        return m_Type.load(std::memory_order_relaxed);
    }
    void setType(GuildType_t type) {
        m_Type.store(type, std::memory_order_relaxed);
    }

    GuildRace_t getRace() const {
        return m_Race.load(std::memory_order_relaxed);
    }
    void setRace(GuildRace_t race) {
        m_Race.store(race, std::memory_order_relaxed);
    }

    GuildState_t getState() const {
        return m_State.load(std::memory_order_relaxed);
    }
    void setState(GuildState_t state) {
        m_State.store(state, std::memory_order_relaxed);
    }

    ServerGroupID_t getServerGroupID() const {
        return m_ServerGroupID.load(std::memory_order_relaxed);
    }
    void setServerGroupID(ServerGroupID_t serverGroupID) {
        m_ServerGroupID.store(serverGroupID, std::memory_order_relaxed);
    }

    ZoneID_t getZoneID() const {
        return m_ZoneID.load(std::memory_order_relaxed);
    }
    void setZoneID(ZoneID_t zoneID) {
        m_ZoneID.store(zoneID, std::memory_order_relaxed);
    }

    string getMaster() const;
    void setMaster(const string& master);

    string getDate() const;
    void setDate(const string& Date);

    string getIntro() const;
    void setIntro(const string& intro);

#ifdef __SHARED_SERVER__
    void saveIntro(const string& intro);
    void tinysave(const char* field) const;
    void saveCount() const;
#endif


    ///// GuildMember get/add/delete/modify /////
    GuildMember* getMember(const string& name) const;
    GuildMember* getMember_NOLOCKED(const string& name) const;
    void addMember(GuildMember* pMember);
    void deleteMember(const string& name);
    void modifyMember(GuildMember& Member);

    void modifyMemberRank(const string& name, GuildMemberRank_t rank);

    // The live map, for the one thread that owns the writes only: the
    // SharedServerManager thread, where the SG handlers run. That thread
    // may walk it while adding or deleting members; nobody else may hold a
    // reference to it. A zone thread answering a client takes a copy
    // under the guild mutex instead.
    HashMapGuildMember& getMembers_NOLOCKED() {
        return m_Members;
    }
    std::vector<std::string> getMemberNames() const;

    // Guild teardown (SGDeleteGuildOK): empties the member map under the
    // mutex and returns each member's name and rank for the handler to act
    // on. The GuildMember objects are retired, not freed -- see
    // m_RetiredMembers.
    std::vector<std::pair<std::string, GuildMemberRank_t>> retireAllMembers();

    int getActiveMemberCount() const {
        return m_ActiveMemberCount.load(std::memory_order_relaxed);
    }
    int getWaitMemberCount() const {
        return m_WaitMemberCount.load(std::memory_order_relaxed);
    }

#ifdef __GAME_SERVER__
    void addCurrentMember(const string& name);
    void deleteCurrentMember(const string& name);
    list<string> getCurrentMembers();
#endif

#ifdef __SHARED_SERVER__
    void makeInfo(GuildInfo2* pGulidInfo);
#endif

    void makeInfo(GuildInfo* pGuildInfo);
    void makeMemberInfo(GCGuildMemberList& gcGuildMemberList);


public: // static
    static GuildID_t getMaxGuildID() {
        return m_MaxGuildID;
    }
    static void setMaxGuildID(GuildID_t id) {
        m_MaxGuildID = id;
    }

    static ZoneID_t getMaxSlayerZoneID() {
        return m_MaxSlayerZoneID;
    }
    static void setMaxSlayerZoneID(ZoneID_t zoneID) {
        m_MaxSlayerZoneID = zoneID;
    }

    static ZoneID_t getMaxVampireZoneID() {
        return m_MaxVampireZoneID;
    }
    static void setMaxVampireZoneID(ZoneID_t zoneID) {
        m_MaxVampireZoneID = zoneID;
    }

    static ZoneID_t getMaxOustersZoneID() {
        return m_MaxOustersZoneID;
    }
    static void setMaxOustersZoneID(ZoneID_t zoneID) {
        m_MaxOustersZoneID = zoneID;
    }

public:
    void expireTimeOutWaitMember(VSDateTime currentDateTime, list<string>& mList);

public: // debug
    string toString() const;

    static string correctString(const string& str);


    ///// Member data /////

protected:
    // The identity fields are written by the SG handlers on the
    // SharedServerManager thread and read by zone threads through
    // GuildManager::getGuild(). The integral ones are atomics; the strings
    // are guarded by m_Mutex, which the accessors above take for the length
    // of the copy and nothing more.
    std::atomic<GuildID_t> m_ID;                  // guild ID
    string m_Name;                                // guild name, guarded by m_Mutex
    std::atomic<GuildType_t> m_Type;              // guild type
    std::atomic<GuildRace_t> m_Race;              // guild race
    std::atomic<GuildState_t> m_State;            // guild state
    std::atomic<ServerGroupID_t> m_ServerGroupID; // ID of the server group hosting the guild zone
    std::atomic<ZoneID_t> m_ZoneID;               // guild zone ID
    string m_Master;                              // guild master, guarded by m_Mutex
    string m_Date;                                // guild expire / registration date, guarded by m_Mutex
    string m_Intro;                               // guild introduction, guarded by m_Mutex

    HashMapGuildMember m_Members; // Map of guild member pointers
    // Members removed from the map are parked here until the guild is
    // destroyed rather than deleted: getMember() hands its GuildMember* out
    // after releasing m_Mutex, so a zone thread may still be reading one
    // while the SharedServerManager thread removes it. A stale read is
    // harmless; a freed one is not. Departures come from players and from
    // the sharedserver's hourly expiry of stale join requests (one
    // SGExpelGuildMemberOK each), so the list grows at the pace of join
    // requests, about a hundred bytes per member.
    std::vector<GuildMember*> m_RetiredMembers;
    std::atomic<int> m_ActiveMemberCount; // Active Member Count
    std::atomic<int> m_WaitMemberCount;   // Wait Member Count

    static GuildID_t m_MaxGuildID;      // Maximum guild ID
    static ZoneID_t m_MaxSlayerZoneID;  // Maximum slayer guild zone ID
    static ZoneID_t m_MaxVampireZoneID; // Maximum vampire guild zone ID
    static ZoneID_t m_MaxOustersZoneID; // Maximum ousters guild zone ID

    mutable Mutex m_Mutex; // Mutex for Guild

#ifdef __GAME_SERVER__
    list<string> m_CurrentMembers; // Members currently logged on
#endif
};

#endif
