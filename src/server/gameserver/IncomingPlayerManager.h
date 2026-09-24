//////////////////////////////////////////////////////////////////////////////
// Filename    : IncomingPlayerManager.h
// Written by  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __INCOMING_PLAYER_MANAGER_H__
#define __INCOMING_PLAYER_MANAGER_H__

#include "ConnectionInfoManager.h"
#include "DatagramSocket.h"
#include "DescriptorPollSet.h"
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
//
// Threads and m_Mutex. The main thread (ClientManager::run) is the only
// thread that changes the player table, the descriptor range and the poll
// set: it accepts connections, merges the players the zone threads queue
// with pushPlayer() in heartbeat(), and removes players from its own walks
// and from CGReady's handler, which it dispatches. Every one of those writes
// takes m_Mutex. The other threads only read the table, under m_Mutex:
// LoginServerManager's LG handlers look players up with getPlayer() and
// getReadyPlayer() while holding LoginServerManager's own mutex (so the
// order is LoginServerManager::m_Mutex -> m_Mutex), and a zone thread takes
// m_Mutex only to queue a player with pushPlayer() while it holds its
// group's mutex (group mutex -> m_Mutex).
//
// pollSockets() therefore follows ZonePlayerManager: fill() and collect()
// under m_Mutex, the wait between them without it. The input, output,
// exception and command walks run without m_Mutex, like
// ZonePlayerManager's: the thread walking is the only thread that writes
// what they read, so they cannot see a half-made change. Holding the mutex
// across them would also be wrong in two ways. Their removals call
// deletePlayer() and the accept path calls addPlayer(), which take the
// non-recursive m_Mutex themselves. And they disconnect and destroy
// players, which saves to the database and takes the player finder, guild
// and SharedServerManager locks: a zone thread's pushPlayer(), made under
// its group mutex, would wait behind that database work, and m_Mutex would
// become an outer lock of all of those.
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

    // The following methods are called by the main thread, once a tick.

    // Ask the kernel which of this manager's descriptors are ready.
    void pollSockets();

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

    // The socket descriptors of the players this manager owns, with what each
    // one was last reported ready for. It has a slot per player table slot,
    // so every descriptor the table can hold can be watched.
    de::DescriptorPollSet m_PollSet;

    // How long each poll waits, in milliseconds.
    int m_TimeoutMilliseconds;

    // min_fd, max_fd
    // Used to speed up iterating over the player table.
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
