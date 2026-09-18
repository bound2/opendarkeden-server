//////////////////////////////////////////////////////////////////////
//
// Filename    : ZonePlayerManager.h
// Written by  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __ZONE_PLAYER_MANAGER_H__
#define __ZONE_PLAYER_MANAGER_H__

// include files
#include "Effect.h"
#include "Exception.h"
#include "Mutex.h"
#include "Player.h"
#include "PlayerManager.h"
#include "Socket.h"
#include "Types.h"

class GamePlayer;
class BroadcastFilter;

//////////////////////////////////////////////////////////////////////
//
// class ZonePlayerManager;
//
// Manages the players that belong to this ZoneGroup.
//
//////////////////////////////////////////////////////////////////////

class ZonePlayerManager : public PlayerManager {
public:
    typedef pair<BroadcastFilter*, SocketOutputStream*> PairFilterStream;

public:
    // constructor
    ZonePlayerManager();

    // destructor
    ~ZonePlayerManager() noexcept;

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

    // broadcast packet
    void broadcastPacket(Packet* pPacket);
    void broadcastPacket_NOBLOCKED(Packet* pPacket);
    void pushBroadcastPacket(Packet* pPacket, BroadcastFilter* pFilter = NULL);
    void flushBroadcastPacket();

    // add player to zone player manager
    void addPlayer(GamePlayer* pGamePlayer);
    void addPlayer_NOBLOCKED(GamePlayer* pGamePlayer);

    // delete player from zone player manager
    void deletePlayer(SOCKET fd);
    void deletePlayer_NOBLOCKED(SOCKET fd);
    void deletePlayer(Player* pPlayer) {
        deletePlayer(pPlayer->getSocket()->getSOCKET());
    }

    // get player
    Player* getPlayer(SOCKET fd);

    // get Player by PhoneNumber
    Player* getPlayerByPhoneNumber(PhoneNumber_t PhoneNumber);

    // All Member Save
    void save();

    void copyPlayers();

    // push Player to queue
    void pushPlayer(GamePlayer* pGamePlayer);
    void pushOutPlayer(GamePlayer* pGamePlayer);
    void processPlayerListQueue();

    // Queue's Player Add Manager
    void heartbeat();

    // delete Queue Player
    void deleteQueuePlayer(GamePlayer* pGamePlayer);

    void removeFlag(Effect::EffectClass EC);

    void lock() {
        m_Mutex.lock();
    }
    void unlock() {
        m_Mutex.unlock();
    }

    // Clear out every player.
    void clearPlayers();

    void setZGID(ZoneGroupID_t id) {
        m_ZGID = id;
    }
    ZoneGroupID_t getZGID() const {
        return m_ZGID;
    }

private:
    // Set of socket descriptors of the players in this group.
    // m_XXXXFDs[0] is the stored copy; m_XXXFDs[1] is what select() is given,
    // so [0] must be copied into [1] before every select().
    fd_set m_ReadFDs[2];
    fd_set m_WriteFDs[2];
    fd_set m_ExceptFDs[2];

    // Time used by select
    Timeval m_Timeout[2];

    // min_fd, max_fd
    // Used to speed up iteration after select(),
    // and to compute the first parameter of select().
    SOCKET m_MinFD;
    SOCKET m_MaxFD;

    // mutex
    mutable Mutex m_Mutex;
    mutable Mutex m_Mutex2;
    mutable Mutex m_MutexBroadcast;

    list<GamePlayer*> m_PlayerListQueue;
    list<GamePlayer*> m_PlayerOutListQueue;
    list<PairFilterStream> m_BroadcastQueue;

    ZoneGroupID_t m_ZGID;
};

#endif
