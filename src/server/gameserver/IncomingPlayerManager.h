//////////////////////////////////////////////////////////////////////////////
// Filename    : IncomingPlayerManager.h
// Written by  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __INCOMING_PLAYER_MANAGER_H__
#define __INCOMING_PLAYER_MANAGER_H__

#include "ConnectionInfoManager.h"
#include "DatagramSocket.h"
#include "Exception.h"
#include "GamePlayer.h"
#include "Mutex.h"
#include "PlayerManager.h"
#include "ServerSocket.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class IncomingPlayerManager;
//
// PlayerManager covers every player connected to the game server and
// ZonePlayerManager covers the players belonging to each zone group, while
// IncomingPlayerManager manages players that are connected to the game
// server but whose creature has not been loaded yet.
//
// After a new connection is authenticated, the creature is loaded and tied to the player.
// Once the creature is loaded, the player and the creature are handed to another zone group.
//////////////////////////////////////////////////////////////////////////////

class IncomingPlayerManager : public PlayerManager {
public:
    IncomingPlayerManager();
    ~IncomingPlayerManager() noexcept(false);

public:
    // initialize
    void init();

    // broadcast packet to all players
    void broadcast(Packet* pPacket);

    // The following methods are called by the ZoneThread.

    // select
    void select();

    // process all players' inputs
    void processInputs();

    // process all players' outputs
    void processOutputs();

    // process all players' exceptions
    void processExceptions();

    // process all players' commands
    void processCommands();

    // accept new connection
    bool acceptNewConnection();

    void copyPlayers();

    // add/delete player
    void addPlayer(Player* pGamePlayer);
    void addPlayer_NOBLOCKED(Player* pGamePlayer);
    void deletePlayer(SOCKET fd);
    void deletePlayer_NOBLOCKED(SOCKET fd);

    // get Player by string
    GamePlayer* getPlayer_NOBLOCKED(const string& id);
    GamePlayer* getPlayer(const string& id);
    GamePlayer* getReadyPlayer(const string& id);

    // lock/unlock
    void lock() {
        m_Mutex.lock();
    }
    void unlock() {
        m_Mutex.unlock();
    }

    // push Player to queue
    void pushPlayer(GamePlayer* pGamePlayer);

    void pushOutPlayer(GamePlayer* pGamePlayer);

    // Queue's Player Add Manager
    void heartbeat();

    void deleteQueuePlayer(GamePlayer* pGamePlayer);

    // Clear out every player.
    void clearPlayers();

private:
    // TCP server socket and socket descriptor
    ServerSocket* m_pServerSocket;
    SOCKET m_SocketID;

    // The set of socket descriptors of the players this manager owns.
    // m_XXXXFDs[0] is the stored copy; m_XXXFDs[1] is what select() is actually given.
    // So [0] must be copied to [1] before calling select().
    fd_set m_ReadFDs[2];
    fd_set m_WriteFDs[2];
    fd_set m_ExceptFDs[2];

    // Time used by select
    Timeval m_Timeout[2];

    // min_fd, max_fd
    // Used to speed up iterating after select().
    // Also used to compute the first parameter of select().
    SOCKET m_MinFD;
    SOCKET m_MaxFD;

    // mutex
    mutable Mutex m_Mutex;

    list<GamePlayer*> m_PlayerListQueue;
    list<GamePlayer*> m_PlayerOutListQueue;

    int m_CheckValue; // by sigi. for debugging. 2002.11.11

    mutable Mutex m_MutexOut;

    // Created and deleted here, registered on de::GameContext for the
    // connect handlers that look a client's IP up.
    ConnectionInfoManager* m_pConnectionInfoManager = nullptr;
};

#endif
