//////////////////////////////////////////////////////////////////////
//
// Filename    : SharedServerManager.cpp
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "SharedServerManager.h"

#include <unistd.h>

#include "Assert.h"
#include "DB.h"
#include "GSRequestGuildInfo.h"
#include "Properties.h"
#include "SharedServerClient.h"
#include "ThreadManager.h"
#include "ThreadPool.h"
#include "Timeval.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
SharedServerManager::SharedServerManager()

{
    __BEGIN_TRY

    m_pSharedServerClient = NULL;

    m_Mutex.setName("SharedServerManager");

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
SharedServerManager::~SharedServerManager() noexcept

{
    stop();
    join();
    __BEGIN_TRY

    SAFE_DELETE(m_pSharedServerClient);

    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////
// stop thread
//////////////////////////////////////////////////////////////////////
void SharedServerManager::stop()

{
    __BEGIN_TRY

    ManagedThread::stop();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// main method
//////////////////////////////////////////////////////////////////////
void SharedServerManager::run()

{
    try {
        string host = g_pConfig->getProperty("DB_HOST");
        string db = g_pConfig->getProperty("DB_DB");
        string user = g_pConfig->getProperty("DB_USER");
        string password = g_pConfig->getProperty("DB_PASSWORD");
        uint port = 0;
        if (g_pConfig->hasKey("DB_PORT"))
            port = g_pConfig->getPropertyInt("DB_PORT");

        Connection* pConnection = new Connection(host, db, user, password, port);
        g_pDatabaseManager->addConnection((int)(long)Thread::self(), pConnection);
        cout << "************************************************************************" << endl;
        cout << "OPEN LOGIN DB" << endl;
        cout << "************************************************************************" << endl;

        Timeval dummyQueryTime;
        getCurrentTime(dummyQueryTime);

        while (!stopRequested()) {
            usleep(1000); // FIX: reduce CPU usage

            // Try to connect if not connected.
            if (m_pSharedServerClient == NULL) {
                Socket* pSocket = NULL;

                try {
                    string SharedServerIP = g_pConfig->getProperty("SharedServerIP");
                    uint SharedServerPort = g_pConfig->getPropertyInt("SharedServerPort");

                    // create socket
                    pSocket = new Socket(SharedServerIP, SharedServerPort);

                    // connect
                    pSocket->connect();

                    // make nonblocking socket
                    pSocket->setNonBlocking(true);

                    // make no-linger socket
                    pSocket->setLinger(0);

                    __ENTER_CRITICAL_SECTION(m_Mutex)
                    m_pSharedServerClient = new SharedServerClient(pSocket);
                    __LEAVE_CRITICAL_SECTION(m_Mutex)

                    pSocket = NULL;

                    cout << "connection to sharedserver established" << endl;

                    // Request the guild information.
                    GSRequestGuildInfo gsRequestGuildInfo;
                    m_pSharedServerClient->sendPacket(&gsRequestGuildInfo);
                } catch (Throwable& t) {
                    cout << "connect to sharedserver fail" << endl;

                    try {
                        SAFE_DELETE(pSocket);
                    } catch (Throwable& t) {
                        filelog("sharedServerClient.txt", "[0]%s", t.toString().c_str());
                    }

                    __ENTER_CRITICAL_SECTION(m_Mutex)

                    try {
                        SAFE_DELETE(m_pSharedServerClient);
                    } catch (Throwable& t) {
                        filelog("sharedServerClient.txt", "[1]%s", t.toString().c_str());
                    }
                    __LEAVE_CRITICAL_SECTION(m_Mutex)

                    // Wait before the next connection attempt.
                    usleep(500000);
                }
            }

            // Process I/O if the socket is connected.
            __ENTER_CRITICAL_SECTION(m_Mutex)

            if (m_pSharedServerClient != NULL) {
                try {
                    m_pSharedServerClient->processInput();
                    m_pSharedServerClient->processOutput();
                    m_pSharedServerClient->processCommand();
                } catch (Throwable& t) {
                    cout << t.toString().c_str() << endl;

                    try {
                        SAFE_DELETE(m_pSharedServerClient);
                    } catch (Throwable& t) {
                        filelog("sharedServerClient.txt", "[2]%s", t.toString().c_str());
                    }
                }
            }

            __LEAVE_CRITICAL_SECTION(m_Mutex)

            // dummy query
            Timeval currentTime;
            getCurrentTime(currentTime);

            if (dummyQueryTime < currentTime) {
                g_pDatabaseManager->executeDummyQuery(pConnection);

                // Schedule the dummy query between 1 hour and 1 hour 30 minutes out,
                // so the connection does not time out.
                dummyQueryTime.tv_sec += (60 + rand() % 30) * 60;
            }
        }

    } catch (Throwable& t) {
        filelog("SHAREDSERVERMANAGER.log", "SharedServerManager::run() 4 : %s", t.toString().c_str());

        cerr << t.toString() << endl;
    }
}

//////////////////////////////////////////////////////////////////////
// send packet to shared server
//////////////////////////////////////////////////////////////////////
void SharedServerManager::sendPacket(Packet* pPacket) {
    __ENTER_CRITICAL_SECTION(m_Mutex)

    if (m_pSharedServerClient != NULL) {
        m_pSharedServerClient->sendPacket(pPacket);
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)
}
