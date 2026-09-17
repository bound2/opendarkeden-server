//////////////////////////////////////////////////////////////////////////////
// Filename    : GameServerManager.cpp
// Written by  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GameServerManager.h"

#include <stdio.h>

#include <algorithm>
#include <chrono>

#include "Assert.h"
#include "DB.h"
#include "Guild.h"
#include "GuildManager.h"
#include "Packet.h"
#include "Properties.h"
#include "ServerShutdown.h"
#include "Socket.h"
#include "SocketAPI.h"


//////////////////////////////////////////////////////////////////////////////
// constructor
// Delete the sub-managers and the data members.
//////////////////////////////////////////////////////////////////////////////

GameServerManager::GameServerManager() : m_pServerSocket(NULL), m_SocketID(INVALID_SOCKET), m_MinFD(-1), m_MaxFD(-1) {
    __BEGIN_TRY

    m_Mutex.setName("GameServerManager");

    try {
        // create  server socket
        while (!ServerShutdown::isRequested()) {
            try {
                m_pServerSocket = new ServerSocket(g_pConfig->getPropertyInt("TCPPort"));
                break;
            } catch (BindException& b) {
                SAFE_DELETE(m_pServerSocket);
                cout << "GameServerManager(" << g_pConfig->getPropertyInt("TCPPort") << ") : " << b.toString() << endl;
                sleep(1);
            }
        }

        if (m_pServerSocket == NULL)
            throw Error("shutdown requested during TCP listener startup");

        m_pServerSocket->setNonBlocking();

        // Set the server socket descriptor.
        m_SocketID = m_pServerSocket->getSOCKET();
    } catch (NoSuchElementException& nsee) {
        // When the configuration file has no such element
        throw Error(nsee.toString());
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////////////

GameServerManager::~GameServerManager() noexcept {
    // The worker owns the listening socket and the per-descriptor player
    // table, so it must be joined before those members go away. A base
    // destructor would run too late.
    stop();
    join();
}


//////////////////////////////////////////////////////////////////////////////
// Initialize the sub-managers and its own members.
//////////////////////////////////////////////////////////////////////////////

void GameServerManager::init() {
    __BEGIN_TRY

    // Zero the fd_sets.
    FD_ZERO(&m_ReadFDs[0]);
    FD_ZERO(&m_WriteFDs[0]);
    FD_ZERO(&m_ExceptFDs[0]);

    //  Turn the server socket's bit on. (write need not be checked.)
    FD_SET(m_SocketID, &m_ReadFDs[0]);
    FD_SET(m_SocketID, &m_ExceptFDs[0]);

    // set min/max fd
    m_MinFD = m_MaxFD = m_SocketID;

    // Initialize m_Timeout.
    // This period should become an option later as well.
    m_Timeout[0].tv_sec = 0;
    m_Timeout[0].tv_usec = 0;

    __END_CATCH
}


void GameServerManager::run() {
    __BEGIN_TRY
    __BEGIN_DEBUG

    try {
        Timeval dummyQueryTime;
        getCurrentTime(dummyQueryTime);

        while (!stopRequested()) {
            try {
                // Stop-aware idle: a shutdown request wakes this immediately
                // instead of costing another polling interval.
                pauseFor(std::chrono::milliseconds(1));

                select();

                processInputs();

                processOutputs();
            } catch (Throwable& t) {
                filelog("SSGSManager.txt", "%s", t.toString().c_str());
            }

            processCommands();

            g_pGuildManager->heartbeat();

            Timeval currentTime;
            getCurrentTime(currentTime);

            if (dummyQueryTime < currentTime) {
                g_pDatabaseManager->executeDummyQuery(g_pDatabaseManager->getConnection("DARKEDEN"));

                dummyQueryTime.tv_sec = (60 + rand() % 30) * 60;
            }
        }

    } catch (Throwable& t) {
        filelog("sharedserverBug.txt", "%s", t.toString().c_str());
        throw;
    }

    __END_DEBUG
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void GameServerManager::broadcast(Packet* pPacket) {
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    try {
        for (int i = m_MinFD; i <= m_MaxFD; i++) {
            if (i != m_SocketID && m_pGameServerPlayers[i] != NULL)
                m_pGameServerPlayers[i]->sendPacket(pPacket);
        }
    } catch (const ProtocolException& e) {
        filelog("SSException.log", "%s\n%s", e.toString().c_str(), pPacket->toString().c_str());
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void GameServerManager::broadcast(Packet* pPacket, Player* pPlayer) {
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    for (int i = m_MinFD; i <= m_MaxFD; i++) {
        if (i != m_SocketID && m_pGameServerPlayers[i] != NULL && m_pGameServerPlayers[i] != pPlayer)
            m_pGameServerPlayers[i]->sendPacket(pPacket);
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// call select() system call
// A TimeoutException from select means no player needs processing.
//////////////////////////////////////////////////////////////////////////////
void GameServerManager::select() {
    __BEGIN_TRY

    //__ENTER_CRITICAL_SECTION(m_Mutex)

    // Copy m_Timeout[0] into m_Timeout[1].
    m_Timeout[1].tv_sec = m_Timeout[0].tv_sec;
    m_Timeout[1].tv_usec = m_Timeout[0].tv_usec;

    // Copy m_XXXFDs[0] into m_XXXFDs[1].
    m_ReadFDs[1] = m_ReadFDs[0];
    m_WriteFDs[1] = m_WriteFDs[0];
    m_ExceptFDs[1] = m_ExceptFDs[0];

    try {
        // Now call select() with m_XXXFDs[1].
        SocketAPI::select_ex(m_MaxFD + 1, &m_ReadFDs[1], &m_WriteFDs[1], &m_ExceptFDs[1], &m_Timeout[1]);
    } catch (InterruptedException& ie) {
        // No signal can arrive here.
        // log(LOG_GAMESERVER_ERROR, "", "", ie.toString());
    }

    //__LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// process all players' inputs
// When the server socket's read flag is set a new connection arrived,
// so handle it; when another socket's read flag is set a new packet
// arrived, so call that player's processInput().
//////////////////////////////////////////////////////////////////////////////
void GameServerManager::processInputs() {
    __BEGIN_TRY

    //__ENTER_CRITICAL_SECTION(m_Mutex)

    if (m_MinFD == -1 && m_MaxFD == -1) // no player exist
    {
        // m_Mutex.unlock();
        return;
    }

    for (int i = m_MinFD; i <= m_MaxFD; i++) {
        if (FD_ISSET(i, &m_ReadFDs[1])) {
            if (i == m_SocketID) {
                //  The server socket means a new connection arrived.
                acceptNewConnection();
            } else {
                if (m_pGameServerPlayers[i] != NULL) {
                    GameServerPlayer* pGameServerPlayer = m_pGameServerPlayers[i];
                    Assert(pGameServerPlayer != NULL);
                    Assert(m_pGameServerPlayers[i] != NULL);

                    if (pGameServerPlayer->getSocket()->getSockError()) {
                        try {
                            // The connection is already gone, so the output buffer must not be flushed.
                            pGameServerPlayer->disconnect(DISCONNECTED);
                        } catch (Throwable& t) {
                            cerr << t.toString() << endl;
                        }

                        deleteGameServerPlayer(i);

                        delete pGameServerPlayer;
                    } else {
                        try {
                            pGameServerPlayer->processInput();
                        } catch (ConnectException& ce) {
                            // The socket is blocking, so no exception other than ConnectException
                            // and Error
                            // is thrown. On a connection error, log it, save the player's
                            // information (if it was loaded) and then delete the player object.
                            try {
                                pGameServerPlayer->disconnect();
                            } catch (Throwable& t) {
                                cerr << t.toString() << endl;
                            }

                            deleteGameServerPlayer(i);

                            delete pGameServerPlayer;
                        }
                    } // else
                } // else
            } // if
        }
    }

    //	__LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// process all players' commands
//////////////////////////////////////////////////////////////////////////////

void GameServerManager::processCommands() {
    __BEGIN_TRY
    __BEGIN_DEBUG

    //__ENTER_CRITICAL_SECTION(m_Mutex)

    if (m_MinFD == -1 && m_MaxFD == -1) // no player exist
    {
        // m_Mutex.unlock();
        return;
    }

    // copyPlayers();

    for (int i = m_MinFD; i <= m_MaxFD; i++) {
        if (i != m_SocketID && m_pGameServerPlayers[i] != NULL) {
            GameServerPlayer* pGameServerPlayer = m_pGameServerPlayers[i];
            Assert(pGameServerPlayer != NULL);
            Assert(m_pGameServerPlayers[i] != NULL);

            if (pGameServerPlayer->getSocket()->getSockError()) {
                try {
                    // The connection is already gone, so the output buffer must not be flushed.
                    pGameServerPlayer->disconnect();
                } catch (Throwable& t) {
                    cerr << t.toString() << endl;
                }

                deleteGameServerPlayer(i);

                delete pGameServerPlayer;
            } else {
                try {
                    pGameServerPlayer->processCommand();
                } catch (ProtocolException& pe) {
                    try {
                        pGameServerPlayer->disconnect();
                        cout << pe.toString().c_str() << endl;
                    } catch (Throwable& t) {
                        cerr << t.toString() << endl;
                    }

                    deleteGameServerPlayer(i);

                    delete pGameServerPlayer;
                }
            }
        }
    }

    //__LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_DEBUG
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// process all players' outputs
//////////////////////////////////////////////////////////////////////////////

void GameServerManager::processOutputs() {
    __BEGIN_TRY

    //__ENTER_CRITICAL_SECTION(m_Mutex)

    if (m_MinFD == -1 && m_MaxFD == -1) // no player exist
    {
        // m_Mutex.unlock();
        return;
    }

    // copyPlayers();

    for (int i = m_MinFD; i <= m_MaxFD; i++) {
        if (FD_ISSET(i, &m_WriteFDs[1])) {
            if (i == m_SocketID)
                throw IOException("server socket's write bit is selected.");

            if (m_pGameServerPlayers[i] != NULL) {
                GameServerPlayer* pGameServerPlayer = m_pGameServerPlayers[i];

                Assert(pGameServerPlayer != NULL);
                Assert(m_pGameServerPlayers[i] != NULL);

                if (pGameServerPlayer->getSocket()->getSockError()) {
                    try {
                        // The connection is already gone, so the output buffer must not be flushed.
                        pGameServerPlayer->disconnect(DISCONNECTED);
                    } catch (Throwable& t) {
                        cerr << t.toString() << endl;
                    }

                    GameServerPlayer* pGameServerPlayer = pGameServerPlayer;

                    deleteGameServerPlayer(i);

                    delete pGameServerPlayer;
                } else {
                    try {
                        pGameServerPlayer->processOutput();
                    } catch (ConnectException& ce) {
                        StringStream msg;
                        msg << "DISCONNECT " << pGameServerPlayer->getID() << "(" << ce.toString() << ")";
                        // log(LOG_GAMESERVER_ERROR, "", "", msg.toString());

                        try {
                            // The connection is already gone, so the output buffer must not be flushed.
                            pGameServerPlayer->disconnect(DISCONNECTED);
                        } catch (Throwable& t) {
                            cerr << t.toString() << endl;
                        }

                        deleteGameServerPlayer(i);

                        delete pGameServerPlayer;
                    } catch (ProtocolException& cp) {
                        StringStream msg;
                        msg << "DISCONNECT " << pGameServerPlayer->getID() << "(" << cp.toString() << ")";
                        // log(LOG_GAMESERVER_ERROR, "", "", cp.toString());

                        // The connection is already gone, so the output buffer must not be flushed.

                        try {
                            pGameServerPlayer->disconnect(DISCONNECTED);
                        } catch (Throwable& t) {
                            cerr << t.toString() << endl;
                        }

                        deleteGameServerPlayer(i);

                        delete pGameServerPlayer;
                    }
                }
            }
        }
    }

    //__LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// process all players' exceptions
// There is no plan to send OOB data at the moment.
// So if OOB data does arrive, treat it as an error and cut the connection.
//////////////////////////////////////////////////////////////////////////////

void GameServerManager::processExceptions() {
    __BEGIN_TRY

    //__ENTER_CRITICAL_SECTION(m_Mutex)

    if (m_MinFD == -1 && m_MaxFD == -1) // no player exist
    {
        // m_Mutex.unlock();
        return;
    }

    // copyPlayers();

    for (int i = m_MinFD; i <= m_MaxFD; i++) {
        if (FD_ISSET(i, &m_ExceptFDs[1])) {
            if (i != m_SocketID) {
                if (m_pGameServerPlayers[i] != NULL) {
                    GameServerPlayer* pGameServerPlayer = m_pGameServerPlayers[i];
                    Assert(pGameServerPlayer != NULL);
                    Assert(i != m_SocketID);
                    Assert(m_pGameServerPlayers[i] != NULL);
                    StringStream msg;
                    msg << "OOB from " << pGameServerPlayer->toString();

                    try {
                        pGameServerPlayer->disconnect();
                    } catch (Throwable& t) {
                        // cerr << t.toString() << endl;
                    }

                    deleteGameServerPlayer(i);

                    delete pGameServerPlayer;
                }
            } else {
                // cerr << "Exception in Loginserver to Gameserver" << endl;
            }
        }
    }

    //__LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// A select-based design does not use nonblocking sockets.
//////////////////////////////////////////////////////////////////////////////
void GameServerManager::acceptNewConnection() {
    __BEGIN_TRY

    // When waiting for a connection in blocking mode
    // the returned value can never be NULL.
    // NonBlockingIOException cannot be thrown either.
    Socket* client = NULL;

    try {
        client = m_pServerSocket->accept();
    } catch (Throwable& t) {
    }

    if (client == NULL) {
        return;
    }

    try {
        // Put in for error handling; the cause still has to be found..
        // Probably something goes wrong in Thread's socket handling
        // Temporary guard until Thread's error handling is fixed.
        if (client->getSockError())
            throw Error();
        client->setNonBlocking(true);

        // Put in for error handling; the cause still has to be found..
        // Probably something goes wrong in Thread's socket handling
        // Temporary guard until Thread's error handling is fixed.
        if (client->getSockError())
            throw Error();
        // set socket option (!NonBlocking, NoLinger)
        client->setLinger(0);

        // StringStream msg;
        // cout << "NEW CONNECTION FROM " << client->getHost() << ":" << client->getPort();

        // Create the player object with the client socket as parameter.
        GameServerPlayer* pGameServerPlayer = new GameServerPlayer(client);

        // Register it with the IPM.
        try {
            addGameServerPlayer(pGameServerPlayer);
        } catch (DuplicatedException&) {
            client->close();
            SAFE_DELETE(client);
            SAFE_DELETE(pGameServerPlayer);
            return;
        }
    } catch (NoSuchElementException&) {
        StringStream msg2;
        msg2 << "ILLEGAL ACCESS FROM " << client->getHost() << ":" << client->getPort();
        // log(LOG_GAMESERVER, "", "", msg2.toString());

        // The connection is not authenticated, so cut it.
        client->send("Error : Unauthorized access", 27);
        client->close();
        SAFE_DELETE(client);
    } catch (Throwable& t) {
        try {
            if (client != NULL) {
                //				client->close();
                SAFE_DELETE(client);
            }
        } catch (Throwable& t) {
        } catch (...) {
        }
    } catch (exception& e) {
    } catch (...) {
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// Add the player object for the new connection to the IPM.
//
//////////////////////////////////////////////////////////////////////
void GameServerManager::addGameServerPlayer(GameServerPlayer* pGameServerPlayer) {
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    SOCKET fd = pGameServerPlayer->getSocket()->getSOCKET();

    // Readjust m_MinFD and m_MaxFD.
    m_MinFD = min(fd, m_MinFD);
    m_MaxFD = max(fd, m_MaxFD);

    // Turn the fd bit on in every fd_set.
    // m_XXXFDs[1] can be dealt with on the next round.
    FD_SET(fd, &m_ReadFDs[0]);
    FD_SET(fd, &m_WriteFDs[0]);
    FD_SET(fd, &m_ExceptFDs[0]);

    m_pGameServerPlayers[fd] = pGameServerPlayer;

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// Remove a given player from the IPM.
//
//////////////////////////////////////////////////////////////////////
void GameServerManager::deleteGameServerPlayer(SOCKET fd) {
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    m_pGameServerPlayers[fd] = NULL;

    // Readjust m_MinFD and m_MaxFD.
    // The fd == m_MinFD && fd == m_MaxFD case is handled by the first if.
    if (fd == m_MinFD) {
        // Find the smallest fd from the front.
        // Note that the m_MinFD slot is NULL at this point.
        int i = m_MinFD;
        for (i = m_MinFD; i <= m_MaxFD; i++) {
            if (m_pGameServerPlayers[i] != NULL || i == m_SocketID) {
                m_MinFD = i;
                break;
            }
        }

        // When no suitable m_MinFD was found,
        // this is the m_MinFD == m_MaxFD case.
        // Set both to -1 then.
        if (i > m_MaxFD)
            m_MinFD = m_MaxFD = -1;
    } else if (fd == m_MaxFD) {
        // Find the largest fd from the back.
        // Watch out for SocketID! (for SocketID the Player pointer is NULL.)
        int i = m_MaxFD;
        for (i = m_MaxFD; i >= m_MinFD; i--) {
            if (m_pGameServerPlayers[i] != NULL || i == m_SocketID) {
                m_MaxFD = i;
                break;
            }
        }

        // When no suitable m_MinFD was found,
        if (i < m_MinFD) {
            throw UnknownError("m_MinFD & m_MaxFD problem.");
        }
    }

    // Turn the fd bit off in every fd_set.
    // m_XXXFDs[1] has to be fixed too, because otherwise an object that is gone
    // could still be processed later.
    FD_CLR(fd, &m_ReadFDs[0]);
    FD_CLR(fd, &m_ReadFDs[1]);
    FD_CLR(fd, &m_WriteFDs[0]);
    FD_CLR(fd, &m_WriteFDs[1]);
    FD_CLR(fd, &m_ExceptFDs[0]);
    FD_CLR(fd, &m_ExceptFDs[1]);

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

void GameServerManager::heartbeat(){__BEGIN_TRY

                                        __ENTER_CRITICAL_SECTION(m_Mutex)

                                            __LEAVE_CRITICAL_SECTION(m_Mutex)

                                                __END_CATCH}

// external variable definition
GameServerManager* g_pGameServerManager = NULL;
