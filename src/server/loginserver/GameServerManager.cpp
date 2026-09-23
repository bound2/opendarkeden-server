//////////////////////////////////////////////////////////////////////
//
// Filename    : GameServerManager.cpp
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#include "GameServerManager.h"

#include <unistd.h>

#include <chrono>

#include "DB.h"
#include "Datagram.h"
#include "DatagramPacket.h"
#include "LGKickCharacter.h"
#include "PacketDispatcher.h"
#include "Properties.h"
#include "ServerContext.h"
#include "ServerShutdown.h"
#include "SocketAPI.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GameServerManager::GameServerManager() : m_pDatagramSocket(NULL) {
    __BEGIN_TRY

    // create datagram server socket
    while (!ServerShutdown::isRequested()) {
        try {
            m_pDatagramSocket = new DatagramSocket(g_pConfig->getPropertyInt("LoginServerUDPPort"));
            // A blocking recvfrom() would hold the worker inside the kernel
            // for as long as the game servers stay quiet, so a shutdown
            // request could not be observed. recvfrom_ex() maps EWOULDBLOCK
            // to "no datagram", which is what the loop below already expects.
            SocketAPI::setsocketnonblocking_ex(m_pDatagramSocket->getSOCKET(), true);
            break;
        } catch (BindException& be) {
            SAFE_DELETE(m_pDatagramSocket);
            cout << be.toString() << endl;
            sleep(1);
        }
    }

    if (m_pDatagramSocket == NULL)
        throw Error("shutdown requested during UDP listener startup");

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GameServerManager::~GameServerManager() noexcept {
    // The worker touches m_pDatagramSocket, so it must be joined before the
    // socket below is destroyed. A base destructor would run too late.
    stop();
    join();

    __BEGIN_TRY

    if (m_pDatagramSocket != NULL) {
        delete m_pDatagramSocket;
        m_pDatagramSocket = NULL;
    }

    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////
// stop thread
//////////////////////////////////////////////////////////////////////
void GameServerManager::stop() {
    __BEGIN_TRY

    ManagedThread::stop();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// main method
//////////////////////////////////////////////////////////////////////
void GameServerManager::run() {
    try {
        string host = g_pConfig->getProperty("DB_HOST");
        string db = g_pConfig->getProperty("DB_DB");
        string user = g_pConfig->getProperty("DB_USER");
        string password = g_pConfig->getProperty("DB_PASSWORD");
        uint port = 0;
        if (g_pConfig->hasKey("DB_PORT"))
            port = g_pConfig->getPropertyInt("DB_PORT");

        Connection* pConnection = new Connection(host, db, user, password, port);
        de::serverContext().database().addConnection((int)(long)Thread::self(), pConnection);

        while (!stopRequested()) {
            Datagram* pDatagram = NULL;
            DatagramPacket* pDatagramPacket = NULL;

            try {
                // Pull the datagram object out.
                pDatagram = m_pDatagramSocket->receive();

                if (pDatagram != NULL) // guards against some exceptions
                {
                    // Pull the datagram packet object out.
                    pDatagram->read(pDatagramPacket);

                    if (pDatagramPacket != NULL) {
                        // Process the datagram packet object that arrived.
                        PacketDispatcher::dispatch(pDatagramPacket, NULL);

                        // Delete the datagram packet object.
                        delete pDatagramPacket;
                        pDatagramPacket = NULL;
                    }

                    // Delete the datagram object.
                    delete pDatagram;
                    pDatagram = NULL;
                }
            } catch (ProtocolException& pe) {
                cout << "GameServerManager::run Exception Check(ProtocolException)" << endl;
                cout << pe.toString() << endl;

                // A protocol error in server-to-server communication means
                // a programming error or a hacking attempt.
                // For now the error is simply ignored.
                // throw Error( pe.toString() );
                delete pDatagramPacket;
                delete pDatagram;
            } catch (ConnectException& ce) {
                cout << "GameServerManager::run Exception Check(ConnectException)" << endl;
                cout << ce.toString() << endl;

                // Hmm, what is this..
                // Treat it as an error for now.
                // throw Error( ce.toString() );
                delete pDatagramPacket;
                delete pDatagram;
            } catch (Throwable& t) {
                cout << "GameServerManager::run Exception Check(ConnectException)" << endl;
                cout << t.toString() << endl;
                delete pDatagramPacket;
                delete pDatagram;
            }
            // Stop-aware idle: a shutdown request wakes this immediately
            // instead of costing another polling interval.
            pauseFor(std::chrono::milliseconds(1));
        }

        cout << "GameServerManager thread exiting... " << endl;
        //::exit(1);
    } catch (Throwable& t) {
        cout << "GameServerManager thread exiting... : " << t.toString() << endl;
    }
}


//////////////////////////////////////////////////////////////////////
// send datagram to datagram-socket
//////////////////////////////////////////////////////////////////////
void GameServerManager::sendDatagram(Datagram* pDatagram) {
    __BEGIN_TRY

    try {
        m_pDatagramSocket->send(pDatagram);
    } catch (ConnectException& t) {
        cout << "GameServerManager::sendDatagram Exception Check!!" << endl;
        cout << t.toString() << endl;
        throw ConnectException("GameServerManager::sendDatagram failed");
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// send datagram-packet to datagram-socket
//////////////////////////////////////////////////////////////////////
void GameServerManager::sendPacket(string host, uint port, DatagramPacket* pPacket) {
    __BEGIN_TRY
    __BEGIN_DEBUG

    try {
        //	try
        // Keep one datagram object and set the destination peer's host
        // and port on it.
        Datagram datagram;

        datagram.setHost(host);
        datagram.setPort(port);

        // Put the datagram packet into the datagram.
        datagram.write(pPacket);

        // Send the datagram through the datagram socket.
        m_pDatagramSocket->send(&datagram);
    } catch (Throwable& t) {
        cout << "====================================================================" << endl;
        cout << t.toString() << endl;
        cout << "====================================================================" << endl;
    }

    __END_DEBUG
    __END_CATCH
}
