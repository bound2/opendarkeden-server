/////////////////////////////////////////////////////////////////////////////
// DynamicZoneGroup.cpp
/////////////////////////////////////////////////////////////////////////////

// include files
#include "DynamicZoneGroup.h"

#include "Assert.h"
#include "DynamicZone.h"
#include "DynamicZoneFactoryManager.h"
#include "DynamicZoneInfo.h"
#include "DynamicZoneManager.h"
#include "Zone.h"
#include "ZoneGroup.h"

///////////////////////////////////////////////////////////
// class DynamicZoneGroup
///////////////////////////////////////////////////////////
DynamicZoneGroup::DynamicZoneGroup() {
    m_MaxSize = 50;
}

DynamicZoneGroup::~DynamicZoneGroup() {
    clear();
}

void DynamicZoneGroup::clear() {
    HashMapDynamicZoneItor itr = m_DynamicZones.begin();
    HashMapDynamicZoneItor endItr = m_DynamicZones.end();

    for (; itr != endItr; ++itr) {
        SAFE_DELETE(itr->second);
    }

    m_DynamicZones.clear();
}

void DynamicZoneGroup::addDynamicZone(DynamicZone* pDynamicZone) {
    HashMapDynamicZoneItor itr = m_DynamicZones.find(pDynamicZone->getZoneID());

    if (itr != m_DynamicZones.end()) {
        cerr << "Duplicated zoneID. DynamicZoneGroup::addDynamicZone" << endl;
        Assert(false);
    }

    m_DynamicZones[pDynamicZone->getZoneID()] = pDynamicZone;
}

bool DynamicZoneGroup::canEnter() {
    std::lock_guard lock(m_Mutex);

    // 현재 있는 DynamicZone 중에서 가능한 넘을 찾는다.
    HashMapDynamicZoneItor itr = m_DynamicZones.begin();
    HashMapDynamicZoneItor endItr = m_DynamicZones.end();

    for (; itr != endItr; ++itr) {
        if (itr->second->getStatus() == DYNAMIC_ZONE_STATUS_READY) {
            return true;
        }
    }

    return getSize() < m_MaxSize;
}

DynamicZone* DynamicZoneGroup::getAvailableDynamicZone() {
    DynamicZone* pDynamicZone = NULL;

    // Under the mutex: only the choice, never the build. Loading a zone
    // (Zone::load, file I/O and a full tile grid) with this mutex held
    // would stall every other zone thread that evaluates canEnter() -- the
    // quest conditions do, per player -- for the length of the load, and
    // each of those threads holds its own group mutex while it waits.
    {
        std::lock_guard lock(m_Mutex);

        // 현재 있는 DynamicZone 중에서 가능한 넘을 찾는다.
        HashMapDynamicZoneItor itr = m_DynamicZones.begin();
        HashMapDynamicZoneItor endItr = m_DynamicZones.end();

        for (; itr != endItr; ++itr) {
            if (itr->second->getStatus() == DYNAMIC_ZONE_STATUS_READY) {
                pDynamicZone = itr->second;
                pDynamicZone->setStatus(DYNAMIC_ZONE_STATUS_RUNNING);

                // init() resets the instance's state machine, which the zone
                // group that owns the instance drives from its heartbeat.
                // This runs on the requesting player's zone thread,
                // generally another group, so the reset is handed to the
                // owner (ZoneGroup::post); it runs at the top of that
                // group's next tick, before the transported player can be
                // added to the zone. Until then the owner's heartbeat still
                // runs the machine in the previous run's terminal state --
                // every DynamicZone subclass keeps that state inert (no
                // switch case, no deadline), and a new one must too.
                // Instances are never deleted, so the pointer stays valid.
                // Should init() throw in the drain (its Assert on m_pZone),
                // the drain logs and the instance stays RUNNING for good:
                // never handed out again, never reset.
                Zone* pZone = pDynamicZone->getZone();
                Assert(pZone != NULL && pZone->getZoneGroup() != NULL);
                pZone->getZoneGroup()->post([pDynamicZone] { pDynamicZone->init(); });
                return pDynamicZone;
            }
        }

        // 현재 있는 DynamicZone 중에는 가능한 넘이 없다.
        // 새로 DynamicZone 을 만든다. Reserve it here -- RUNNING, so no one
        // else picks it, and listed, so canEnter() counts it against
        // m_MaxSize -- and build it after the lock is gone.
        pDynamicZone = g_pDynamicZoneFactoryManager->createDynamicZone(m_DynamicZoneType);

        pDynamicZone->setTemplateZoneID(m_TemplateZoneID);
        pDynamicZone->setZoneID(g_pDynamicZoneManager->getNewDynamicZoneID());
        pDynamicZone->setStatus(DYNAMIC_ZONE_STATUS_RUNNING);
        addDynamicZone(pDynamicZone);
    }

    // The build publishes the ZoneInfo and the zone into the template
    // group's map through their Snapshots (see makeDynamicZone), so the
    // readers on other threads need no lock here either.
    pDynamicZone->makeDynamicZone();

    return pDynamicZone;
}
