//////////////////////////////////////////////////////////////////////
//
// Filename    : SharedServer.cpp
// Written By  : reiot@ewestsoft.com
// Description : Main class for the shared server
//
//////////////////////////////////////////////////////////////////////

// include files
#include "SharedServer.h"

#include <exception>

#include "Assert.h"
#include "GameServerGroupInfoManager.h"
#include "GameServerManager.h"
#include "GameWorldInfoManager.h"
#include "GuildManager.h"
#include "HeartbeatManager.h"
#include "PacketFactoryManager.h"
#include "PacketValidator.h"
#include "ResurrectLocationManager.h"
#include "ServerShutdown.h"
#include "SharedContext.h"
#include "SharedGameServerInfoManager.h"
#include "StringPool.h"
#include "database/DatabaseManager.h"
#include "types/ServerType.h"

//////////////////////////////////////////////////////////////////////
//
// constructor
//
// The system manager's constructor creates the sub-manager objects.
//
//////////////////////////////////////////////////////////////////////
SharedServer::SharedServer() {
    __BEGIN_TRY

    // create database manager
    g_pDatabaseManager = new DatabaseManager();

    // create guild manager
    m_pGuildManager = new GuildManager();
    de::sharedContext().setGuildManager(m_pGuildManager);

    // create some info managers
    m_pGameServerInfoManager = new SharedGameServerInfoManager();
    m_pGameServerGroupInfoManager = new GameServerGroupInfoManager();

    // create packet factory manager, packet validator
    // (They must be created and initialized before the client manager and the server-to-server manager.)
    g_pPacketFactoryManager = new PacketFactoryManager();
    g_pPacketValidator = new PacketValidator();

    // create inter-server communication manager
    m_pGameServerManager = new GameServerManager();
    de::sharedContext().setGameServerManager(m_pGameServerManager);

    // create client manager
    m_pHeartbeatManager = new HeartbeatManager();

    // create GameWorldInfoManager
    g_pGameWorldInfoManager = new GameWorldInfoManager();

    // create ResurrectLocationManager
    m_pResurrectLocationManager = new ResurrectLocationManager();

    m_pStringPool = new StringPool();
    de::sharedContext().setStringPool(m_pStringPool);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// destructor
//
// The system manager's destructor must delete the sub-manager objects.
//
//////////////////////////////////////////////////////////////////////
SharedServer::~SharedServer() noexcept(false) {
    __BEGIN_TRY

    SAFE_DELETE(m_pHeartbeatManager);
    SAFE_DELETE(m_pGameServerManager);
    SAFE_DELETE(g_pPacketValidator);
    SAFE_DELETE(g_pPacketFactoryManager);
    SAFE_DELETE(m_pGameServerInfoManager);
    SAFE_DELETE(m_pGameServerGroupInfoManager);
    SAFE_DELETE(m_pGuildManager);
    SAFE_DELETE(g_pDatabaseManager);
    SAFE_DELETE(g_pGameWorldInfoManager);
    SAFE_DELETE(m_pResurrectLocationManager);
    SAFE_DELETE(m_pStringPool);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// initialize game server
//
//////////////////////////////////////////////////////////////////////
void SharedServer::init() {
    __BEGIN_TRY

    cout << "SharedServer::init() start" << endl;

    // Initialize the database manager.
    g_pDatabaseManager->init();

    m_pStringPool->load();

    // Initialize the guild manager.
    m_pGuildManager->init();

    // initialize some info managers
    m_pGameServerInfoManager->init();
    m_pGameServerGroupInfoManager->init();

    g_pGameWorldInfoManager->init();

    // Initialize the packet factory manager / packet validator before the client manager.
    g_pPacketFactoryManager->init();
    g_pPacketValidator->init();

    // Initialize the server-to-server communication manager.
    m_pGameServerManager->init();

    // ResurrectLocationManager initialization
    m_pResurrectLocationManager->init();

    // Once everything is ready, initialize the client manager and so
    // be ready for networking.
    m_pHeartbeatManager->init();

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// start shared server
//
//////////////////////////////////////////////////////////////////////
void SharedServer::start() {
    __BEGIN_TRY

    cout << "---------- Start SharedServer ---------" << endl;
    // Start the server-to-server communication manager.
    m_pGameServerManager->start();

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
    m_pHeartbeatManager->start();

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// stop shared server
//
// Mind the stop order: the manager with the widest reach goes first.
// Stopping in the opposite order can leave another manager dereferencing
// something that is already gone.
//
//////////////////////////////////////////////////////////////////////
void SharedServer::stop() {
    if (m_Stopped)
        return;
    __BEGIN_TRY

    // End the main-thread heartbeat loop first, so nothing new is started.
    ServerShutdown::request();
    m_pHeartbeatManager->stop();

    // Request the stop before joining, then join while every manager the
    // worker uses (config, database, guild manager) is still alive.
    m_pGameServerManager->stop();
    m_pGameServerManager->join();
    try {
        m_pGameServerManager->rethrowFailure();
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
