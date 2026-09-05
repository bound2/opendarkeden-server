//////////////////////////////////////////////////////////////////////
//
// Filename    : ZoneGroup.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __ZONE_GROUP_H__
#define __ZONE_GROUP_H__

// include files
#include <cstddef>
#include <functional>

#include <unordered_map>

#include "Exception.h"
#include "GMServerInfo.h"
#include "GameTime.h"
#include "Mailbox.h"
#include "Thread.h"
#include "Types.h"
#include "Zone.h"

// forward declaration
class ZonePlayerManager;

// type redefinition
// typedef unordered_map<ZoneID_t,Zone*> ZONE_HASHMAP;

//////////////////////////////////////////////////////////////////////
//
// class ZoneGroup;
//
//////////////////////////////////////////////////////////////////////
class ZoneGroup {
    // friend declaration

public:
    // constructor
    ZoneGroup(ZoneGroupID_t zoneGroupID);

    // destructor
    ~ZoneGroup();


public:
    // initialize zone group
    void init();

    // load sub zones from database
    void load();

    // save sub zones to database
    void save();

    //
    void processPlayers();
    void heartbeat();

    // Cross-thread mailbox (CLAUDE.md, "Thread ownership"). Group-level work
    // that originates on another thread -- a sibling zone thread, a manager
    // thread -- is posted here and run by this group's ZoneGroupThread at
    // the start of its next tick, under the group mutex. post() never takes
    // the group mutex, so a caller holding the PCFinder lock or its own
    // group's mutex cannot deadlock on it. Work aimed at one player goes
    // through the player's own box instead (PlayerMailbox.h), which follows
    // the player between groups and is drained only by its current owner.
    void post(std::function<void()> command) {
        m_Mailbox.post(std::move(command));
    }
    // Runs the posted commands. The caller must hold the group mutex
    // (asserted under DE_OWNERSHIP_CHECKS); a command that throws -- any
    // type, Throwable or not -- is logged to errorLog.txt and the rest still
    // run, so a command must leave the state consistent at every throw
    // point. A batch deeper than kMailboxDepthWarning is logged as well:
    // the box is unbounded, so that is the sign a producer outran this
    // group's tick. Returns how many ran.
    static constexpr std::size_t kMailboxDepthWarning = 1000;
    std::size_t drainMailbox();
    std::size_t mailboxSize() const {
        return m_Mailbox.size();
    }

public:
    // add zone to zone group
    void addZone(Zone* pZone);

    // delete zone from zone group
    void deleteZone(ZoneID_t zoneID);
    Zone* removeZone(ZoneID_t zoneID);

    // get zone from zone group
    Zone* getZone(ZoneID_t zoneID) const;

    // #ifdef __NO_COMBAT__
    Zone* getCombatZone(ZoneID_t zoneID) const; // getZone과 같은 일을 수행하나 NULL을 리턴하는 것이 가능, 김경석
    // #endif

    //--------------------------------------------------
    // get/set methods
    //--------------------------------------------------
public:
    // get/set zone group id
    ZoneGroupID_t getZoneGroupID() const {
        return m_ZoneGroupID;
    }
    void setZoneGroupID(ZoneGroupID_t zoneGroupID) {
        m_ZoneGroupID = zoneGroupID;
    }

    // get zone player manager
    ZonePlayerManager* getZonePlayerManager() {
        return m_pZonePlayerManager;
    }
    void setZonePlayerManager(ZonePlayerManager* pZonePlayerManager) {
        m_pZonePlayerManager = pZonePlayerManager;
    }

    // get/set game time
    GameTime getGameTime() const {
        return m_GameTime;
    }
    void setGameTime(const GameTime& gameTime) {
        m_GameTime = gameTime;
    }

    void makeZoneUserInfo(GMServerInfo& gmServerInfo);

    const unordered_map<ZoneID_t, Zone*>& getZones() const {
        return m_Zones;
    }

    // get debug string
    string toString() const;

public:
    // Thread-ownership contract (see "Thread ownership" in CLAUDE.md):
    // zone-group state may only be touched while this group's mutex is
    // held — the group's ZoneGroupThread holds it for its whole tick, and
    // any other thread (e.g. GDRLairManager) must take it explicitly.
    //
    // Under DE_OWNERSHIP_CHECKS (defined only for Debug builds — this
    // project deliberately never defines NDEBUG, so gating on our own
    // macro is the only way the checks truly vanish from release),
    // lock()/unlock() record the holding thread and assertOwned() ABORTS
    // on a violation. abort(), not a throw: an AssertionError is a
    // Throwable, and the legacy catch(Throwable&) blocks on exactly these
    // paths would swallow it — turning a detected race into a silently
    // half-applied mutation (e.g. GamePlayer::disconnect's empty catch
    // would skip the character save). The check is armed by the
    // ZoneGroupThread when it starts; before that, single-threaded
    // startup/loading passes vacuously.
    //
    // Invariant that makes false FIRES impossible: m_LockHolder and
    // m_LockHolderValid are written only while the mutex is held (the
    // mutex is non-recursive — Mutex::lock() throws on self-relock before
    // the holder is written — so clearing on unlock is sound). A racing
    // reader that does not hold the mutex may see stale values, but that
    // is precisely the caller that must fail.
    void lock() {
        m_Mutex.lock();
#ifdef DE_OWNERSHIP_CHECKS
        m_LockHolder = Thread::self();
        m_LockHolderValid = true;
#endif
    }
    void unlock() {
#ifdef DE_OWNERSHIP_CHECKS
        m_LockHolderValid = false;
#endif
        m_Mutex.unlock();
    }

#ifdef DE_OWNERSHIP_CHECKS
    void armOwnershipAssert() {
        m_OwnershipArmed = true;
    }
    void assertOwned() const;
#else
    // Release: empty inlines so the gateway call sites compile away.
    void armOwnershipAssert() {}
    void assertOwned() const {}
#endif

    void initLoadValue();
    DWORD getLoadValue() const;


    //--------------------------------------------------
    // data members
    //--------------------------------------------------
private:
    // zone group id
    ZoneGroupID_t m_ZoneGroupID;

    // zone 의 해쉬맵
    unordered_map<ZoneID_t, Zone*> m_Zones;

    // zone player manager
    ZonePlayerManager* m_pZonePlayerManager;

    // game time
    GameTime m_GameTime;

    // Tick Time
    Timeval m_TickTime;

    DWORD m_LoadValue;

    mutable Mutex m_Mutex;

    de::CommandMailbox m_Mailbox;

#ifdef DE_OWNERSHIP_CHECKS
    // Debug-only ownership tracking (see lock()/unlock()/assertOwned()).
    // pthread_t is opaque — compared with pthread_equal(), never ==, and a
    // separate valid flag stands in for the old "zero tid" sentinel POSIX
    // never promised. Members exist only in checking builds; ZoneGroup's
    // layout differs between configs, which is fine for a type never
    // serialized or shared across differently-configured TUs.
    volatile TID m_LockHolder;
    volatile bool m_LockHolderValid;
    volatile bool m_OwnershipArmed;
#endif
};

#endif
