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
#include "LogClient.h"
#include "PacketDispatcher.h"
#include "Properties.h"
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
        g_pDatabaseManager->addConnection((int)(long)Thread::self(), pConnection);

        while (!stopRequested()) {
            Datagram* pDatagram = NULL;
            DatagramPacket* pDatagramPacket = NULL;

            try {
                // �����ͱ׷� ��ü�� �������.
                pDatagram = m_pDatagramSocket->receive();

                if (pDatagram != NULL) // �Ϻ� exception����. by sigi. 2002.5.17
                {
                    // cout << pDatagramPacket->toString() << endl;

                    /*
                    // UDP test
                    cout << "[DatagramPacket] " << pDatagram->getHost() << ":"
                                                << pDatagram->getPort() << endl;

                    if (pDatagram->getPort()!=9997)
                    {
                        LGKickCharacter lg;
                        lg.setPCName("111");
                        lg.setID(111);

                        sendPacket(pDatagram->getHost(), pDatagram->getPort(), &lg);
                    }
                    */

                    // �����ͱ׷� ��Ŷ ��ü�� �������.
                    pDatagram->read(pDatagramPacket);

                    if (pDatagramPacket != NULL) {
                        // ����� �����ͱ׷� ��Ŷ ��ü�� �����Ѵ�.
                        PacketDispatcher::dispatch(pDatagramPacket, NULL);

                        // �����ͱ׷� ��Ŷ ��ü�� �����Ѵ�.
                        delete pDatagramPacket;
                        pDatagramPacket = NULL;
                    }

                    // �����ͱ׷� ��ü�� �����Ѵ�.
                    delete pDatagram;
                    pDatagram = NULL;
                }
            } catch (ProtocolException& pe) {
                cout << "GameServerManager::run Exception Check(ProtocolException)" << endl;
                cout << pe.toString() << endl;

                // ������ ��ſ��� �������� ������ �߻��ϸ�,
                // ���α׷��� �����̰ų� ��ŷ �õ��̴�.
                // �ϴ��� ���ڸ��� �ش�ǹǷ�.. ������ �����Ѵ�.
                // throw Error( pe.toString() );
                delete pDatagramPacket;
                delete pDatagram;
            } catch (ConnectException& ce) {
                cout << "GameServerManager::run Exception Check(ConnectException)" << endl;
                cout << ce.toString() << endl;

                // ����.. ���� �̰�..
                // �ϴ� ������..
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
        log(LOG_LOGINSERVER_ERROR, "", "", t.toString());
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
        throw ConnectException("GameServerManager::sendDatagram ������ ������");
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
        //	{
        // �����ͱ׷� ��ü�� �ϳ� �ΰ�, ������ peer �� ȣ��Ʈ��
        // ��Ʈ�� �����Ѵ�.
        Datagram datagram;

        datagram.setHost(host);
        datagram.setPort(port);

        // �����ͱ׷� ��Ŷ�� �����ͱ׷��� ����ִ´�.
        datagram.write(pPacket);

        // �����ͱ׷� ������ ���ؼ� �����ͱ׷��� �����Ѵ�.
        m_pDatagramSocket->send(&datagram);
        //	}
        //	catch ( ConnectException & t )
        //	{
        //		cout << "GameServerManager::sendDatagram Exception Check!!" << endl;
        //		cout << t.toString() << endl;
        //		throw ConnectException( "GameServerManager::sendDatagram ������ ������");
        //	}

    } catch (Throwable& t) {
        cout << "====================================================================" << endl;
        cout << t.toString() << endl;
        cout << "====================================================================" << endl;
    }

    __END_DEBUG
    __END_CATCH
}


// global variable definition
GameServerManager* g_pGameServerManager = NULL;
