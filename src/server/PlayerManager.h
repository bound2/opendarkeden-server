//--------------------------------------------------------------------------------
//
// Filename    : PlayerManager.h
// Written by  : reiot@ewestsoft.com
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __PLAYER_MANAGER_H__
#define __PLAYER_MANAGER_H__

// include files
#include "Exception.h"
#include "Mutex.h"
#include "SocketAPI.h"
#include "Timeval.h"
#include "Types.h"

// forward declaration
class Player;
class Packet;

//--------------------------------------------------------------------------------
//
// class PlayerManager;
//
// The object that manages the players. For speed it uses an array indexed by
// socket descriptor. The array's size is the largest number of players the
// game server can handle (the largest number of sockets).
// There is some memory waste, but it is bearable.
//
// With an average of 100 players in one zone group,
//
// 		900 x 4(byte) x 10(#ZoneGroup) = 36k
//
// that much is wasted.
//
//--------------------------------------------------------------------------------

class PlayerManager {
public:
    // the size of the internal player array
    const static uint nMaxPlayers = 2000;

public:
    // constructor
    PlayerManager();

    // destructor
    virtual ~PlayerManager() noexcept(false);

    // broadcast message
    virtual void broadcastPacket(Packet* pPacket);

    // Adds a given player to the manager.
    virtual void addPlayer(Player* pPlayer);

    // Deletes a given player from the manager.
    virtual void deletePlayer(SOCKET fd);

    // Gets a given player object.
    virtual Player* getPlayer(SOCKET fd);

    // Gets the object of the player holding a given phone.
    virtual Player* getPlayerByPhoneNumber(PhoneNumber_t PhoneNumber) {
        return NULL;
    }

    // Returns the number of players currently managed.
    uint size() const {
        return m_nPlayers;
    }

    // Copy Player
    void copyPlayers();

protected:
    // The array of player pointers. The socket descriptor is the index.
    Player* m_pPlayers[nMaxPlayers];

    // number of Players
    uint m_nPlayers;

    // Where a copy of the Players is kept.
    Player* m_pCopyPlayers[nMaxPlayers];
};

#endif
