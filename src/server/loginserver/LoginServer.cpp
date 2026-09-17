//////////////////////////////////////////////////////////////////////
//
// Filename    : LoginServer.cpp
// Written By  : reiot@ewestsoft.com
// Description : Main class for the login server
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
#include "LogClient.h"
#include "PacketFactoryManager.h"
#include "PacketValidator.h"
#include "ServerShutdown.h"
#include "UserInfoManager.h"
#include "ZoneGroupInfoManager.h"
#include "ZoneInfoManager.h"
#include "database/DatabaseManager.h"

//////////////////////////////////////////////////////////////////////
//
// constructor
//
// The system manager's constructor creates the sub-manager objects.
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
    // (They must be created and initialized before the client manager and the server-to-server manager.)
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

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// destructor
//
// The system manager's destructor must delete the sub-manager objects.
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

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// initialize game server
//
//////////////////////////////////////////////////////////////////////
void LoginServer::init() {
    __BEGIN_TRY

    // Initialize the database manager.
    g_pDatabaseManager->init();

    // initialize some info managers
    g_pGameServerInfoManager->init();
    g_pGameServerGroupInfoManager->init();
    g_pZoneInfoManager->init();
    g_pZoneGroupInfoManager->init();

    g_pGameWorldInfoManager->init();

    // Initialize the packet factory manager / packet validator before the client manager.
    g_pPacketFactoryManager->init();
    g_pPacketValidator->init();

    g_pUserInfoManager->init();

    // Initialize the server-to-server communication manager.
    g_pGameServerManager->init();

    // Once everything is ready, initialize the client manager and so
    // be ready for networking.
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

    // Start the server-to-server communication manager.
    g_pGameServerManager->start();

    //
    // Start the client manager.
    //
    // *Reiot's Notes*
    //
    // It must run last, because it is not multi-thread based but
    // a function with an infinite loop. If another function were
    // called after it, then unless the loop ends (that is, unless an error occurs)
    // the other managers' processing loops would never run.
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

    __END_CATCH
}
