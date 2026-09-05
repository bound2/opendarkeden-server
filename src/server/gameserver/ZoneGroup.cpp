//////////////////////////////////////////////////////////////////////////////
// Filename    : ZoneGroup.cc
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "ZoneGroup.h"

#include <cstdlib>
#include <exception>

#include "Assert.h"
#include "Profile.h"
#include "VSDateTime.h"
#include "ZonePlayerManager.h"

// #define __FULL_PROFILE__

#ifndef __FULL_PROFILE__
#undef beginProfileEx
#define beginProfileEx(name) ((void)0)
#undef endProfileEx
#define endProfileEx(name) ((void)0)
#endif

//////////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////////
ZoneGroup::ZoneGroup(ZoneGroupID_t zoneGroupID)

    : m_ZoneGroupID(zoneGroupID), m_pZonePlayerManager(NULL) {
    __BEGIN_TRY

#ifdef DE_OWNERSHIP_CHECKS
    m_LockHolder = TID();
    m_LockHolderValid = false;
    m_OwnershipArmed = false;
#endif

    m_Mutex.setName("ZoneGroupMutex");

    Assert(m_ZoneGroupID > 0);

    m_TickTime.tv_sec = 0;
    m_TickTime.tv_usec = 0;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////////////
ZoneGroup::~ZoneGroup()

{
    __BEGIN_TRY

    // 해쉬맵안에 있는 모든 pair 들을 삭제한다.
    m_Zones.clear();

    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////////////
// Debug-build (DE_OWNERSHIP_CHECKS) check of the thread-ownership contract:
// zone-group state may only be touched with this group's mutex held (see
// CLAUDE.md, "Thread ownership"). Armed by ZoneGroupThread::run() once the
// thread exists, so single-threaded startup/loading is exempt. The holder
// fields are written only under the mutex, so a false FIRE cannot occur for
// the actual holder; a reader that does not hold the mutex may read them
// racily — and that reader is exactly the violation being detected.
//
// A violation ABORTS. It must not throw: AssertionError is a Throwable, and
// the catch(Throwable&) blocks sitting on these very paths (e.g. the empty
// one in GamePlayer::disconnect) would swallow it, converting a detected
// cross-thread race into a silently half-applied mutation — quieter and
// more destructive than the race itself. abort() cannot be caught, leaves a
// core, and the filelog line says which group and which threads.
//////////////////////////////////////////////////////////////////////////////
#ifdef DE_OWNERSHIP_CHECKS
void ZoneGroup::assertOwned() const {
    if (!m_OwnershipArmed)
        return;
    TID holder = m_LockHolder;
    if (!m_LockHolderValid || !pthread_equal(holder, Thread::self())) {
        filelog("threadOwnership.log",
                "ZoneGroup %d state touched without holding the group mutex (tid=%lu, holder=%lu, holderValid=%d) — "
                "aborting",
                (int)m_ZoneGroupID, (unsigned long)Thread::self(), (unsigned long)holder, (int)m_LockHolderValid);
        abort();
    }
}
#endif

//////////////////////////////////////////////////////////////////////////////
// initialize zone group
//////////////////////////////////////////////////////////////////////////////
void ZoneGroup::init()

{
    __BEGIN_TRY

    // init == load
    load();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// load from database
//////////////////////////////////////////////////////////////////////////////
void ZoneGroup::load()

{
    __BEGIN_TRY

    throw UnsupportedError(__PRETTY_FUNCTION__);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// save to database
//////////////////////////////////////////////////////////////////////////////
void ZoneGroup::save()

{
    __BEGIN_TRY

    throw UnsupportedError(__PRETTY_FUNCTION__);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// process all players in zone player manager
//////////////////////////////////////////////////////////////////////////////
void ZoneGroup::processPlayers()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    //__ENTER_CRITICAL_SECTION(m_Mutex)

    try {
        // m_pZonePlayerManager->copyPlayers();
        //__ENTER_CRITICAL_SECTION(m_pZonePlayerManager)

        beginProfileEx("ZPM_SELECT");
        m_pZonePlayerManager->select();
        endProfileEx("ZPM_SELECT");

        beginProfileEx("ZPM_EXCEPTION");
        m_pZonePlayerManager->processExceptions();
        endProfileEx("ZPM_EXCEPTION");

        beginProfileEx("ZPM_INPUT");
        m_pZonePlayerManager->processInputs();
        endProfileEx("ZPM_INPUT");

        beginProfileEx("ZPM_OUTPUT");
        m_pZonePlayerManager->processOutputs();
        endProfileEx("ZPM_OUTPUT");

        //__LEAVE_CRITICAL_SECTION(m_pZonePlayerManager)
    } catch (TimeoutException&) {
        // timeout 이 발생하면, 입력, 출력, OOB 처리 어느 것이나 할 게 없당..
        // 잘못된 FD가 있을 경우 짜르기 위하여 시행한다 -_-;
        // m_pZonePlayerManager->processOutputs();
    } catch (InterruptedException& ie) {
        // throw Error(ie.toString());
    } catch (IOException& ioe) {
        // throw Error(ioe.toString());
    } catch (Error& er) {
        filelog("errorLog.txt", "%s", er.toString().c_str());

        // Assert(false);
    }

    try {
        // 모든 플레이어의 명령을 처리한다.
        beginProfileEx("ZPM_COMMAND");
        //	__ENTER_CRITICAL_SECTION(m_pZonePlayerManager)
        m_pZonePlayerManager->processCommands();
        //	__LEAVE_CRITICAL_SECTION(m_pZonePlayerManager)
        endProfileEx("ZPM_COMMAND");

    } catch (Error& er) {
        filelog("errorLog.txt", "%s", er.toString().c_str());

        // Assert(false);
    } catch (Throwable&) {
    }

    try {
        beginProfileEx("ZPM_HEARTBEAT");
        m_pZonePlayerManager->heartbeat(); // 내부에서 lock건다.
        endProfileEx("ZPM_HEARTBEAT");
    } catch (Error& er) {
        filelog("errorLog.txt", "%s", er.toString().c_str());

        // Assert(false);
    } catch (Throwable&) {
    }

    //__LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_DEBUG
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// Run the commands other threads posted for this group. Called by the
// ZoneGroupThread at the top of its tick, with the group mutex held, so a
// command sees the same ownership guarantees as a CG handler. One failing
// command is logged, not fatal: the others still run and the tick goes on.
//////////////////////////////////////////////////////////////////////////////
std::size_t ZoneGroup::drainMailbox() {
    assertOwned();

    std::size_t ran = m_Mailbox.drain(
        [](std::function<void()>& command) { command(); },
        [this] {
            try {
                throw;
            } catch (Throwable& t) {
                filelog("errorLog.txt", "ZoneGroup %u mailbox command failed: %s", (unsigned)m_ZoneGroupID,
                        t.toString().c_str());
            } catch (std::exception& e) {
                filelog("errorLog.txt", "ZoneGroup %u mailbox command failed: %s", (unsigned)m_ZoneGroupID, e.what());
            } catch (...) {
                filelog("errorLog.txt", "ZoneGroup %u mailbox command failed: unknown exception",
                        (unsigned)m_ZoneGroupID);
            }
        });
    if (ran > kMailboxDepthWarning)
        filelog("errorLog.txt", "ZoneGroup %u mailbox drained %u commands in one tick: a producer outran the tick",
                (unsigned)m_ZoneGroupID, (unsigned)ran);
    return ran;
}


//////////////////////////////////////////////////////////////////////////////
// process all npc, monsters, ... in zones
//////////////////////////////////////////////////////////////////////////////
void ZoneGroup::heartbeat()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    // VSTime vstime;
    // vstime.start();
    //__ENTER_CRITICAL_SECTION(m_Mutex)

    // now process each zones' NPCs, MOBs, weather, quest, ...
    for (unordered_map<ZoneID_t, Zone*>::iterator itr = m_Zones.begin(); itr != m_Zones.end(); itr++) {
        Zone* pZone = itr->second;
        pZone->heartbeat();
    }

    //__LEAVE_CRITICAL_SECTION(m_Mutex)

    // filelog("ZoneGroupHeartbeat.txt", "ZoneGroupID[%d]ZoneGroupHeartbeat:%d", m_ZoneGroupID, vstime.elapsed());

    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// process all npc, monsters, ... in zones
//////////////////////////////////////////////////////////////////////////////
void ZoneGroup::makeZoneUserInfo(GMServerInfo& gmServerInfo)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    // VSTime vstime;
    // vstime.start();

    // now process each zones' NPCs, MOBs, weather, quest, ...
    for (unordered_map<ZoneID_t, Zone*>::iterator itr = m_Zones.begin(); itr != m_Zones.end(); itr++) {
        Zone* pZone = itr->second;

        gmServerInfo.addZoneUserData(pZone->getZoneID(), pZone->getPCCount());
    }

    // filelog("ZoneGroupHeartbeat.txt", "ZoneGroupID[%d]ZoneGroupHeartbeat:%d", m_ZoneGroupID, vstime.elapsed());

    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// add zone to zone group
//////////////////////////////////////////////////////////////////////////////
void ZoneGroup::addZone(Zone* pZone)

{
    __BEGIN_TRY

    // 일단 같은 아이디의 존이 있는지 체크해본다.
    unordered_map<ZoneID_t, Zone*>::iterator itr = m_Zones.find(pZone->getZoneID());

    if (itr != m_Zones.end())
        // 똑같은 아이디가 이미 존재한다는 소리다. - -;
        throw Error("duplicated zone id");

    m_Zones[pZone->getZoneID()] = pZone;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Delete zone from zone group
//////////////////////////////////////////////////////////////////////////////
void ZoneGroup::deleteZone(ZoneID_t zoneID) {
    __BEGIN_TRY

    unordered_map<ZoneID_t, Zone*>::iterator itr = m_Zones.find(zoneID);

    if (itr != m_Zones.end()) {
        // 존을 삭제한다.
        SAFE_DELETE(itr->second);

        // pair를 삭제한다.
        m_Zones.erase(itr);
    } else {
        // 그런 존 아이디를 찾을 수 없었을 때
        StringStream msg;
        msg << "ZoneID : " << zoneID;
        throw NoSuchElementException(msg.toString());
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Remove zone from zone group
// delete하지 않고 node만 지워준다.
//////////////////////////////////////////////////////////////////////////////
Zone* ZoneGroup::removeZone(ZoneID_t zoneID) {
    __BEGIN_TRY

    unordered_map<ZoneID_t, Zone*>::iterator itr = m_Zones.find(zoneID);

    if (itr != m_Zones.end()) {
        // 존을 삭제한다.
        // SAFE_DELETE(itr->second);
        Zone* pZone = itr->second;

        // pair를 삭제한다.
        m_Zones.erase(itr);

        return pZone;
    } else {
        // 그런 존 아이디를 찾을 수 없었을 때
        StringStream msg;
        msg << "ZoneID : " << zoneID;
        throw NoSuchElementException(msg.toString());
    }

    return NULL;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// get zone from zone group
//////////////////////////////////////////////////////////////////////////////
Zone* ZoneGroup::getZone(ZoneID_t zoneID) const {
    __BEGIN_TRY

    Zone* pZone = NULL;

    unordered_map<ZoneID_t, Zone*>::const_iterator itr = m_Zones.find(zoneID);

    if (itr != m_Zones.end()) {
        pZone = itr->second;
    } else {
        // 그런 존 아이디를 찾을 수 없었을 때
        StringStream msg;
        msg << "ZoneID : " << zoneID;
        throw NoSuchElementException(msg.toString());
    }

    return pZone;

    __END_CATCH
}

// #ifdef __NO_COMBAT__
Zone* ZoneGroup::getCombatZone(ZoneID_t zoneID) const

{
    Zone* pZone = NULL;

    __BEGIN_TRY

    unordered_map<ZoneID_t, Zone*>::const_iterator itr = m_Zones.find(zoneID);

    if (itr != m_Zones.end()) {
        pZone = itr->second;
        return pZone;
    }

    __END_CATCH

    return NULL;
}
// #endif

void ZoneGroup::initLoadValue() {
    Zone* pZone = NULL;

    __BEGIN_TRY

    unordered_map<ZoneID_t, Zone*>::const_iterator itr = m_Zones.begin();

    while (itr != m_Zones.end()) {
        pZone = itr->second;

        pZone->initLoadValue();

        itr++;
    }

    __END_CATCH
}

DWORD
ZoneGroup::getLoadValue() const {
    Zone* pZone = NULL;
    DWORD loadValue = 0;

    __BEGIN_TRY

    unordered_map<ZoneID_t, Zone*>::const_iterator itr = m_Zones.begin();

    while (itr != m_Zones.end()) {
        pZone = itr->second;

        loadValue += pZone->getLoadValue();

        itr++;
    }

    __END_CATCH

    return loadValue;
}


//////////////////////////////////////////////////////////////////////////////
// get debug string
//////////////////////////////////////////////////////////////////////////////
string ZoneGroup::toString() const

{
    StringStream msg;
    msg << "ZoneGroup(" << "ZoneGroupID:" << (int)m_ZoneGroupID << "GameTime:" << m_GameTime.toString() << ")";
    return msg.toString();
}
