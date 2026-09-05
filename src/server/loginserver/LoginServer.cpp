//////////////////////////////////////////////////////////////////////
//
// Filename    : LoginServer.cpp
// Written By  : reiot@ewestsoft.com
// Description : 로그인 서버용 메인 클래스
//
//////////////////////////////////////////////////////////////////////

// include files
#include "LoginServer.h"

#include <exception>

#include "Assert.h"
#include "ClientManager.h"
#include "GameServerGroupInfoManager.h"
#include "GameServerInfoManager.h"
#include "GameServerManager.h"
#include "GameWorldInfoManager.h"
#include "ItemDestroyer.h"
#include "PacketFactoryManager.h"
#include "PacketValidator.h"
#include "ServerShutdown.h"
#include "UserInfoManager.h"
#include "ZoneGroupInfoManager.h"
#include "ZoneInfoManager.h"
#include "database/DatabaseManager.h"
// #include "gameserver/billing/BillingPlayerManager.h"
#include "LogClient.h"

#ifdef __THAILAND_SERVER__

#include "TimeChecker.h"

#endif

//////////////////////////////////////////////////////////////////////
//
// constructor
//
// 시스템 매니저의 constructor에서는 하위 매니저 객체를 생성한다.
//
//////////////////////////////////////////////////////////////////////
LoginServer::LoginServer() {
    __BEGIN_TRY

    // create database manager
    g_pDatabaseManager = new DatabaseManager();

    // create some info managers
    g_pGameServerInfoManager = new GameServerInfoManager();
    g_pGameServerGroupInfoManager = new GameServerGroupInfoManager();

    g_pZoneInfoManager = new ZoneInfoManager();
    g_pZoneGroupInfoManager = new ZoneGroupInfoManager();

    // create packet factory manager, packet validator
    // (클라이언트 매니저와 서버간통신매니저보다 먼저 생성, 초기화되어야 한다.)
    g_pPacketFactoryManager = new PacketFactoryManager();
    g_pPacketValidator = new PacketValidator();

    // create inter-server communication manager
    g_pGameServerManager = new GameServerManager();

    // create client manager
    g_pClientManager = new ClientManager();

    // create ItemDestroyer
    g_pItemDestroyer = new ItemDestroyer();

    // create ItemDestroyer
    g_pUserInfoManager = new UserInfoManager();

    // create GameWorldInfoManager
    g_pGameWorldInfoManager = new GameWorldInfoManager();

    // login 서버에서는 빌링을 빼기로 한다.
    // 애드빌 요청. by bezz 2003.04.22
    // #ifdef __CONNECT_BILLING_SYSTEM__
    //  create GameWorldInfoManager
    // g_pBillingPlayerManager = new BillingPlayerManager();
    // #endif

#ifdef __THAILAND_SERVER__

    g_pTimeChecker = new TimeChecker();
#endif

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// destructor
//
// 시스템 매니저의 destructor에서는 하위 매니저 객체를 삭제해야 한다.
//
//////////////////////////////////////////////////////////////////////
LoginServer::~LoginServer() noexcept(false) {
    __BEGIN_TRY

    if (g_pClientManager != NULL) {
        delete g_pClientManager;
        g_pClientManager = NULL;
    }

    if (g_pGameServerManager != NULL) {
        delete g_pGameServerManager;
        g_pGameServerManager = NULL;
    }

    if (g_pPacketValidator != NULL) {
        delete g_pPacketValidator;
        g_pPacketValidator = NULL;
    }

    if (g_pPacketFactoryManager != NULL) {
        delete g_pPacketFactoryManager;
        g_pPacketFactoryManager = NULL;
    }

    if (g_pZoneGroupInfoManager != NULL) {
        delete g_pZoneGroupInfoManager;
        g_pZoneGroupInfoManager = NULL;
    }

    if (g_pZoneInfoManager != NULL) {
        delete g_pZoneInfoManager;
        g_pZoneInfoManager = NULL;
    }

    if (g_pGameServerInfoManager != NULL) {
        delete g_pGameServerInfoManager;
        g_pGameServerInfoManager = NULL;
    }

    if (g_pGameServerGroupInfoManager != NULL) {
        delete g_pGameServerGroupInfoManager;
        g_pGameServerGroupInfoManager = NULL;
    }
    if (g_pDatabaseManager != NULL) {
        delete g_pDatabaseManager;
        g_pDatabaseManager = NULL;
    }
    if (g_pUserInfoManager != NULL) {
        delete g_pUserInfoManager;
        g_pUserInfoManager = NULL;
    }
    if (g_pGameWorldInfoManager != NULL) {
        delete g_pGameWorldInfoManager;
        g_pGameWorldInfoManager = NULL;
    }

    // login 서버에서는 빌링을 빼기로 한다.
    // 애드빌 요청. by bezz 2003.04.22
    // #ifdef __CONNECT_BILLING_SYSTEM__
    // if ( g_pBillingPlayerManager != NULL ) {
    // delete g_pBillingPlayerManager;
    // g_pBillingPlayerManager = NULL;
    //}
    // #endif

#ifdef __THAILAND_SERVER__
    if (g_pTimeChecker != NULL) {
        delete g_pTimeChecker;
        g_pTimeChecker = NULL;
    }

#endif


    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// initialize game server
//
//////////////////////////////////////////////////////////////////////
void LoginServer::init() {
    __BEGIN_TRY

    // 데이타베이스매니저를 초기화한다.
    g_pDatabaseManager->init();

    // initialize some info managers
    g_pGameServerInfoManager->init();
    g_pGameServerGroupInfoManager->init();
    g_pZoneInfoManager->init();
    g_pZoneGroupInfoManager->init();

    g_pGameWorldInfoManager->init();

    // 클라이언트매니저를 초기화하기 전에, 패킷팩토리매니저/패킷발리데이터를 초기화한다.
    g_pPacketFactoryManager->init();
    g_pPacketValidator->init();

    g_pUserInfoManager->init();

    // 서버간 통신 매니저를 초기화한다.
    g_pGameServerManager->init();

    // login 서버에서는 빌링을 빼기로 한다.
    // 애드빌 요청. by bezz 2003.04.22
    // #ifdef __CONNECT_BILLING_SYSTEM__
    //  빌링 서버 접속 준비
    // g_pBillingPlayerManager->init();
    // #endif

#ifdef __THAILAND_SERVER__
    // for Thailand ChildGuard System
    g_pTimeChecker->init();
#endif


    // 만반의 준비가 끝이 나면 이제 클라이언트매니저를 초기화함으로써,
    // 네트워킹에 대비한다.
    g_pClientManager->init();

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// start login server
//
//////////////////////////////////////////////////////////////////////
void LoginServer::start() {
    __BEGIN_TRY

    // 서버간 통신 매니저를 시작한다.
    g_pGameServerManager->start();

    // login 서버에서는 빌링을 빼기로 한다.
    // 애드빌 요청. by bezz 2003.04.22
    // #ifdef __CONNECT_BILLING_SYSTEM__
    // g_pBillingPlayerManager->start();
    // #endif

    //
    // 클라이언트 매니저를 시작한다.
    //
    // *Reiot's Notes*
    //
    // 가장 나중에 실행되어야 한다. 왜냐하면 멀티쓰레드기반이 아닌
    // 무한루프를 가진 함수이기 때문이다. 만일 이 다음에 다른 함수를
    // 호출할 경우, 루프가 끝나지 않는한(즉 에러가 발생하지 않는한)
    // 다른 매니저의 처리 루프는 실행되지 않는다.
    //
    g_pClientManager->start();

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// stop login server
//
// Mind the stop order: the manager with the widest reach goes first.
// Stopping in the opposite order can leave another manager dereferencing
// something that is already gone.
//
//////////////////////////////////////////////////////////////////////
void LoginServer::stop() {
    if (m_Stopped)
        return;
    __BEGIN_TRY

    // Stop the client manager first so no further connection is accepted.
    ServerShutdown::request();
    g_pClientManager->stop();

    // Request the stop before joining, then join while every manager the
    // worker uses (config, database, packet factory) is still alive.
    g_pGameServerManager->stop();
    g_pGameServerManager->join();
    try {
        g_pGameServerManager->rethrowFailure();
    } catch (Throwable& error) {
        cerr << "GameServerManager: " << error.toString() << endl;
    } catch (const std::exception& error) {
        cerr << "GameServerManager: " << error.what() << endl;
    } catch (...) {
        cerr << "GameServerManager: unknown worker failure" << endl;
    }
    m_Stopped = true;

    // login 서버에서는 빌링을 빼기로 한다.
    // 애드빌 요청. by bezz 2003.04.22
    // #ifdef __CONNECT_BILLING_SYSTEM__
    // g_pBillingPlayerManager->stop();
    // #endif

    __END_CATCH
}


//////////////////////////////////////////////////
// global variable declaration
//////////////////////////////////////////////////
LoginServer* g_pLoginServer = NULL;
