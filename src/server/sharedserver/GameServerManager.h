//////////////////////////////////////////////////////////////////////////////
// Filename    : GameServerManager.h
// Written by  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GAME_SERVER_MANAGER_H__
#define __GAME_SERVER_MANAGER_H__

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


    // select
    void select();

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

    void heartbeat();

private:
    // TCP server socket and socket descriptor
    ServerSocket* m_pServerSocket;
    SOCKET m_SocketID;

    // The set of socket descriptors of the players that belong here.
    // m_XXXXFDs[0] is the stored copy; m_XXXFDs[1] is what select() actually gets.
    // That is, [0] -> [1] must be copied before calling select().
    fd_set m_ReadFDs[2];
    fd_set m_WriteFDs[2];
    fd_set m_ExceptFDs[2];

    // Time used by select
    Timeval m_Timeout[2];

    // min_fd, max_fd
    // Used to speed the iteration after select() up.
    // Also used to compute select()'s first parameter.
    SOCKET m_MinFD;
    SOCKET m_MaxFD;

    // mutex
    mutable Mutex m_Mutex;


    // Array of game server pointers, indexed by socket descriptor.
    GameServerPlayer* m_pGameServerPlayers[nMaxGameServers];
};

// external variable declaration
extern GameServerManager* g_pGameServerManager;

#endif
