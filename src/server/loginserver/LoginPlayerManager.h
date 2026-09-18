//////////////////////////////////////////////////////////////////////
//
// Filename    : LoginPlayerManager.h
// Written by  : reiot@ewestsoft.com
// Description : Login player manager for the login server
//
//////////////////////////////////////////////////////////////////////

#ifndef __LOGIN_PLAYER_MANAGER_H__
#define __LOGIN_PLAYER_MANAGER_H__

// include files
#include "Exception.h"
#include "PlayerManager.h"
#include "ServerSocket.h"
#include "Types.h"

class LoginPlayer;
class ReconnectLoginInfoManager;

//////////////////////////////////////////////////////////////////////
//
// class LoginPlayerManager;
//
// Manages every player connected to the login server.
//
//////////////////////////////////////////////////////////////////////

class LoginPlayerManager : public PlayerManager {
public:
    // constructor
    LoginPlayerManager();

    // destructor
    ~LoginPlayerManager() noexcept;

public:
    // Initialize the client manager.
    void init();

    // accept new connection
    void acceptNewConnection();

    // Uses the select() system call for I/O multiplexing.
    void select();

    // Copy the input of every connected user into the input buffer.
    void processInputs();

    // Send the output of every connected user to the client.
    void processOutputs();

    // Process the packets of every connected user.
    void processCommands();

    // Handle OOB data.
    void processExceptions();

public:
    // Deliver a given packet to every player connected to the login server.
    void broadcastPacket(Packet* pPacket);

    // Deliver a given packet to the player with a given id.
    void sendPacket(const string& id, Packet* pPacket);

    // Add a player object.
    void addPlayer(Player* pPlayer);
    void addPlayer_NOLOCKED(Player* pPlayer);

    // Delete the player object.
    void deletePlayer(SOCKET fd);
    void deletePlayer_NOLOCKED(SOCKET fd);

    // Access a player object.
    LoginPlayer* getPlayer(const string& PCName) const;
    LoginPlayer* getPlayer_NOLOCKED(const string& PCName) const;

    // lock/unlock
    void lock() {
        m_Mutex.lock();
    }
    void unlock() {
        m_Mutex.unlock();
    }

    // get debug string
    string toString() const;

private:
    // Server socket
    ServerSocket* m_pServerSocket;

    // Server socket descriptor ( for fast reference )
    SOCKET m_ServerFD;

    // The set of socket descriptors of the players that belong here.
    // m_XXXXFDs[0] is the stored copy; m_XXXFDs[1] is what select() actually gets.
    // That is, [0] -> [1] must be copied before calling select().
    fd_set m_ReadFDs[2];
    fd_set m_WriteFDs[2];
    fd_set m_ExceptFDs[2];

    // Time used by select
    Timeval m_Timeout[2];

    // min_fd , max_fd
    // Used to speed the iteration after select() up.
    // Also used to compute select()'s first parameter.
    SOCKET m_MinFD;
    SOCKET m_MaxFD;

    // The login server's main loop is single threaded,
    // so it looks as though no mutex were needed..
    // but the routine that receives and handles datagrams from the game servers
    // runs as its own thread. That thread can manipulate LPM's
    // player array, so a race condition is quite
    // likely.
    mutable Mutex m_Mutex;

    // Owned here and registered on de::loginContext(), which is how the
    // packet handlers reach it.
    ReconnectLoginInfoManager* m_pReconnectLoginInfoManager = nullptr;
};

#endif
