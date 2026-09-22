//////////////////////////////////////////////////////////////////////////////
// Filename    : GameServer.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GameServer.h"

#include <signal.h>
#include <time.h>
#include <unistd.h>

#include "Assert.h"
#include "ClientManager.h"
#include "DatabaseManager.h"
#include "GameContext.h"
#include "GameServerInfoManager.h"
#include "LoginServerManager.h"
#include "ObjectManager.h"
#include "PacketFactoryManager.h"
#include "PacketValidator.h"
#include "Properties.h"
#include "SharedServerManager.h"
#include "SystemAPI.h"
#include "ThreadManager.h"
#include "mofus/Mofus.h"
#ifdef __MOFUS__
#include "mofus/MPacketManager.h"
#include "mofus/MPlayerManager.h"
#endif

#include "GDRLairManager.h"
#include "SMSServiceThread.h"


////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////

GameServer::GameServer()

{
    __BEGIN_TRY

    try {
        // The configuration is loaded before the server object exists, so it
        // is registered first; every manager below registers as it is created.
        de::gameContext().setConfig(g_pConfig);

        // create database manager
        g_pDatabaseManager = new DatabaseManager();
        de::gameContext().setDatabaseManager(g_pDatabaseManager);

        // create object manager
        m_pObjectManager = new ObjectManager();

        // create packet factory manager , packet validator
        g_pPacketFactoryManager = new PacketFactoryManager();
        g_pPacketValidator = new PacketValidator();

        // create thread manager
        m_pThreadManager = new ThreadManager();

        // create login server manager
        g_pLoginServerManager = new LoginServerManager();

        // create shared server manager
        g_pSharedServerManager = new SharedServerManager();

#ifdef __MOFUS__
        g_pMPlayerManager = new MPlayerManager();
        g_pMPacketManager = new MPacketManager();
#endif

        // create client manager
        m_pClientManager = new ClientManager();
        de::gameContext().setClientManager(m_pClientManager);

        // create login server manager
        g_pGameServerInfoManager = new GameServerInfoManager();

    } catch (Throwable& t) {
        // cout << t.toString() << endl;
        throw;
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// destructor
// Must check for the game server terminating without going through stop().
//////////////////////////////////////////////////////////////////////////////

GameServer::~GameServer()

{
    __BEGIN_TRY

    stop();
    // Zone workers must stop while their zone and database dependencies are
    // still alive.
    SAFE_DELETE(m_pThreadManager);
    SAFE_DELETE(m_pClientManager);
    SAFE_DELETE(m_pObjectManager);
    SAFE_DELETE(g_pPacketValidator);
    SAFE_DELETE(g_pPacketFactoryManager);
    SAFE_DELETE(g_pLoginServerManager);
    SAFE_DELETE(g_pSharedServerManager);
#ifdef __MOFUS__
    SAFE_DELETE(g_pMPlayerManager);
    SAFE_DELETE(g_pMPacketManager);
#endif
    SAFE_DELETE(g_pGameServerInfoManager);
    SAFE_DELETE(g_pDatabaseManager);

    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////////////
// initialize game server
//////////////////////////////////////////////////////////////////////////////

void GameServer::init()

{
    __BEGIN_TRY

    sysinit();
    cout << "GameServer::init() : System Initialization Success..." << endl;

    // Set gCurrentTime.
    setCurrentTime();

    // Initialize the database manager.
    g_pDatabaseManager->init();
    cout << "GameServer::init() : DatabaseManager Initialization Success..." << endl;

    // Initialize the object manager through the database manager.
    m_pObjectManager->init();
    m_pObjectManager->load();
    cout << "GameServer::init() : ObjectManager Initialization Success..." << endl;

    // Initialize the thread manager on top of the object manager.
    // (In particular ZoneThreadPool requires ZoneGroupManager to be initialized first.)
    m_pThreadManager->init();
    cout << "GameServer::init() : ThreadManager Initialization Success..." << endl;

    // Initialize the packet factory manager and packet validator before the client manager.
    g_pPacketFactoryManager->init();
    cout << "GameServer::init() : PacketFactoryManager Initialization Success..." << endl;

    g_pPacketValidator->init();
    cout << "GameServer::init() : PacketValidator Initialization Success..." << endl;

    // Now prepare the inter-server communication.
    g_pLoginServerManager->init();
    cout << "GameServer::init() : LoginServerManager Initialization Success..." << endl;

    // Prepare the communication with the shared server.
    g_pSharedServerManager->init();
    cout << "GameServer::init() : SharedServerManager Initialization Success..." << endl;

#ifdef __MOFUS__
    g_pMPacketManager->init();
    cout << "GameServer::init() : MPacketManager Initialization Success..." << endl;

    g_pMPlayerManager->init();
    cout << "GameServer::init() : MPlayerManager Initialization Success..." << endl;
#endif

    g_pGameServerInfoManager->init();
    cout << "GameServer::init() : GameServerInfoManager Initialization Success..." << endl;

    // Once everything else is ready, initialize the client manager to
    // prepare for networking.
    m_pClientManager->init();
    cout << "GameServer::init() : ClientManager Initialization Success..." << endl;

    // When initialization is done, stop the console output and go to the background.
    // goBackground();

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// start game server
//////////////////////////////////////////////////////////////////////////////

void GameServer::start()

{
    __BEGIN_TRY

    cout << ">>> STARTING THREAD MANAGER..." << endl;
    m_pThreadManager->start();

    cout << ">>> STARTING LOGIN SERVER MANAGER..." << endl;
    g_pLoginServerManager->start();

    cout << ">>> STARTING SHARED SERVER MANAGER..." << endl;
    g_pSharedServerManager->start();

#ifdef __MOFUS__
    g_pMPlayerManager->start();
    cout << ">>> STARTING MOFUS PLAYER MANAGER..." << endl;
#endif

    // add by zdj
    // cout << ">>> STARTING SMS SERVICE THREAD..." << endl;
    // SMSServiceThread::Instance().start();

    //	cout << ">>> STARTING Gilles De Rais Lair Manager THREAD..." << endl;
    GDRLairManager::Instance().init();
    GDRLairManager::Instance().start();

    // Start the client manager.
    // *Reiot's Notes*
    // It must run last, because it is a function with an infinite loop
    // rather than something multithreaded. Any function called after it
    // does not run until the loop ends, that is, until an error
    // occurs.
    cout << ">>> ALL INITIALIZATIONS ARE COMPLETED SUCCESSFULLY." << endl;
    cout << ">>> STARTING ClientManager->start() INFINITE LOOP..." << endl;

    try {
        m_pClientManager->start();

    } catch (Throwable& t) {
        filelog("GameServerError.txt", "%s", t.toString().c_str());
        throw;
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// stop game server
//
// Mind the stop order. The managers with the widest effect must be
// stopped first. Stopping them in the reverse order can produce null
// pointer problems.
//
// So the thread-related managers must be destroyed first.
//////////////////////////////////////////////////////////////////////////////

void GameServer::stop()

{
    if (m_Stopped)
        return;
    __BEGIN_TRY

    //
    // stop client manager
    //
    // Destroy the client manager first so that no new connections are
    // accepted any more.
    //
    ServerShutdown::request();
    m_pClientManager->stop();
    // Request every auxiliary stop before any join. All shared dependencies
    // remain alive until BOTH auxiliary and zone workers have finished.
    std::vector<ManagedThread*> workers{g_pLoginServerManager, g_pSharedServerManager, &GDRLairManager::Instance()};
#ifdef __MOFUS__
    workers.push_back(g_pMPlayerManager);
#endif
    for (auto* worker : workers)
        worker->stop();

    //
    // stop thread manager
    //
    // Then destroy the thread manager, which stops processing the existing
    // users and throws them off the game server. The stop run by the thread
    // manager's thread pools has to work properly here.
    //
    //
    m_pThreadManager->stop();
    for (auto* worker : workers) {
        worker->join();
        try {
            worker->rethrowFailure();
        } catch (Throwable& error) {
            cerr << worker->getName() << ": " << error.toString() << endl;
        } catch (const std::exception& error) {
            cerr << worker->getName() << ": " << error.what() << endl;
        } catch (...) {
            cerr << worker->getName() << ": unknown worker failure" << endl;
        }
    }
    m_Stopped = true;

    // stop object manager
    //
    // Now that every user connection is closed, save the remaining zones and
    // the various game settings to the database.
    //
    // m_pObjectManager->save();

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// System level initialization
//////////////////////////////////////////////////////////////////////////////

void GameServer::sysinit()

{
    __BEGIN_TRY

    // Initialization for rand().
    srand(time(0));

    signal(SIGPIPE, SIG_IGN); // This one is likely to happen now and then
    signal(SIGALRM, SIG_IGN); // No alarms are used; set for good measure
    signal(SIGCHLD, SIG_IGN); // No fork is used; set for good measure

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// Call this function once the server is stable enough that console
// output is no longer needed.
//////////////////////////////////////////////////////////////////////////////

void GameServer::goBackground()

{
    __BEGIN_TRY

    int forkres = SystemAPI::fork_ex();

    if (forkres == 0) {
        // case of child process
        close(0);
        close(1);
        close(2);
    } else {
        // case of parent process
        exit(0);
    }

    __END_CATCH
}
