//////////////////////////////////////////////////////////////////////////////
// Filename    : GameServerManager.h
// Written by  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GAME_SERVER_MANAGER_H__
#define __GAME_SERVER_MANAGER_H__

#include "DescriptorPollSet.h"
#include "Exception.h"
#include "GameServerPlayer.h"
#include "ManagedThread.h"
#include "Mutex.h"
#include "ServerSocket.h"
#include "Timeval.h"
#include "Types.h"


//////////////////////////////////////////////////////////////////////////////
//
// class GameServerManager;
//
//////////////////////////////////////////////////////////////////////////////

class GameServerManager : public ManagedThread {
public:
    GameServerManager();
    ~GameServerManager() noexcept;

public:
    // Maximum number of game servers the shared server accepts
    const static uint nMaxGameServers = 100;

    // initialize
    void init();

    void run() override;

    // broadcast packet to all players
    void broadcast(Packet* pPacket);
    void broadcast(Packet* pPacket, Player* pPlayer);


    // Ask the kernel which of this manager's descriptors are ready.
    void pollSockets();

    // process all inputs
    void processInputs();

    // process all outputs
    void processOutputs();

    // process all exceptions
    void processExceptions();

    // process all commands
    void processCommands();

    // accept new connection
    void acceptNewConnection();

    // add/delete player
    void addGameServerPlayer(GameServerPlayer* pGameServerPlayer);
    void deleteGameServerPlayer(SOCKET fd);

    // lock/unlock
    void lock() {
        m_Mutex.lock();
    }
    void unlock() {
        m_Mutex.unlock();
    }

private:
    // TCP server socket and socket descriptor
    ServerSocket* m_pServerSocket;
    SOCKET m_SocketID;

    // The socket descriptors of the game servers that belong here, with what
    // each one was last reported ready for. It has a slot per game server
    // table slot, so every descriptor the table can hold can be watched.
    de::DescriptorPollSet m_PollSet;

    // How long each poll waits, in milliseconds.
    int m_TimeoutMilliseconds;

    // min_fd, max_fd
    // Used to speed the iteration over the game server table up.
    SOCKET m_MinFD;
    SOCKET m_MaxFD;

    // mutex
    mutable Mutex m_Mutex;


    // Array of game server pointers, indexed by socket descriptor. Every
    // slot starts empty: the loops read slots no connection has filled.
    GameServerPlayer* m_pGameServerPlayers[nMaxGameServers] = {};
};

#endif
