//////////////////////////////////////////////////////////////////////////////
// Filename    : EventShutdown.cpp
// Written by  : bezz
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "EventShutdown.h"

#include "BillingInfo.h"
#include "IncomingPlayerManager.h"
#include "VariableManager.h"
#include "ZoneGroupManager.h"
#include "ZonePlayerManager.h"
#include "repository/ItemObjectRepository.h"
#include "repository/SystemAvailabilityRepository.h"
#include "signal.h"
//////////////////////////////////////////////////////////////////////////////
// class EventShutdown member methods
//////////////////////////////////////////////////////////////////////////////

EventShutdown::EventShutdown(GamePlayer* pGamePlayer)

    : Event(pGamePlayer) {}

void EventShutdown::activate()

{
    __BEGIN_TRY

    try {
        const unordered_map<ZoneGroupID_t, ZoneGroup*>& zoneGroups = g_pZoneGroupManager->getZoneGroups();
        unordered_map<ZoneGroupID_t, ZoneGroup*>::const_iterator itr = zoneGroups.begin();

        for (; itr != zoneGroups.end(); itr++) {
            ZonePlayerManager* pZonePlayerManager = itr->second->getZonePlayerManager();

            pZonePlayerManager->clearPlayers();
        }

        g_pIncomingPlayerManager->clearPlayers();
    } catch (Throwable& t) {
        // 무시
    }

    // 프로세스 종료. 꺄꺄~ 죽어라~~ 꺄꺄~
    if (g_pVariableManager->isKillDaemonCtl() == 1) {
        kill(getppid(), 9);
    }

    //	kill( getppid(), 9 );

    kill(getpid(), 9);

    __END_CATCH
}

string EventShutdown::toString() const

{
    StringStream msg;
    msg << "EventShutdown(" << ")";
    return msg.toString();
}
