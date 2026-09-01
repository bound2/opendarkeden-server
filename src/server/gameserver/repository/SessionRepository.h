#ifndef __SESSION_REPOSITORY_H__
#define __SESSION_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Seam for the player-session bookkeeping (task 3.2): the guild roster's
// online flag (GuildMember.LogOn), the account's session state and event
// counter (Player.LogOn / LastLogoutDate / SpecialEventCount), the PC-room
// records (PCRoomUserInfo, PCRoomLottoObject), the per-character IP table
// (UserIPInfo) and the per-server user count a NetMarble deployment
// publishes (USERINFO.UserStatus). Three connections, as before: Player
// and the PC-room tables go through the thread's dist connection
// ("PLAYER_DB" — DatabaseManager ignores the name and hands back the
// second per-thread socket to the same schema, see
// MySQLGoodsRepository.cpp), UserStatus through the process-wide USERINFO
// connection, the rest through the DARKEDEN connection. Write parameters
// are typed to the members/expressions each caller streamed.
//
// Not enclosed: skill/Restore.cpp's GuildMember LogOn = 0 write (the
// vampire->slayer restore, built and live, the same unfreed Statement);
// the session START side — CGConnectHandler's Player LogOn='GAME' flip
// and GuildMember LogOn = 1 writes, CGPortCheckHandler's UserIPInfo
// upsert, CGRequestIPHandler's and CGSayHandler's UserIPInfo reads — all
// handler-directory files (R3); CGSayHandler's and
// billing/CommonBillingPacket.cpp's Player.LogOn / LastLogoutDate reads;
// src/server/PaySystem.cpp's PCRoomUserInfo statements (ServerCore, every
// caller under the disabled __PAY_SYSTEM_* macros); the loginserver's
// LoginPlayerManager sweep and its copy of addLogoutPlayerData (the
// gameserver's copy was dead and is deleted); and the unbuilt
// src/server/IncomingPlayerManager.cpp fork that still carries the sweep.

class SessionRepository {
public:
    virtual ~SessionRepository() {}

    // --- guild roster (DARKEDEN) -------------------------------------------
    // GuildMember.LogOn = 0 for one character (session end, morph).
    virtual void markGuildMemberLoggedOff(const std::string& name) = 0;

    // --- the account row (dist connection) ---------------------------------
    // LogOn='LOGOFF', LastLogoutDate=now() — only for a row still in 'GAME'.
    virtual void markPlayerLoggedOff(const std::string& playerID) = 0;
    // The accounts this world/server group left in 'GAME' (boot-time sweep).
    virtual std::vector<std::string> loadPlayersInGame(int worldID, int serverGroupID) = 0;
    virtual void logOffPlayersOfServer(int worldID, int serverGroupID) = 0;
    // False when the account has no row (the caller keeps its throw).
    virtual bool loadSpecialEventCount(const std::string& playerID, DWORD& count) = 0;
    // GamePlayer::saveSpecialEventCount — the uint member through "%d".
    virtual void saveSpecialEventCount(uint count, const std::string& playerID) = 0;

    // --- PC-room records (dist connection) ---------------------------------
    virtual void deletePCRoomUser(const std::string& playerID) = 0;
    // GamePlayer::giveLotto — one lotto row per (player, character,
    // dimension, world). amount is left untouched when there is no row.
    virtual bool loadPCRoomLottoAmount(const std::string& playerID, const std::string& name, uint dimensionID,
                                       uint worldID, int& amount) = 0;
    virtual void updatePCRoomLottoAmount(int amount, const std::string& playerID, const std::string& name,
                                         uint dimensionID, uint worldID) = 0;
    // The positional INSERT ( 0, PCRoomID, PlayerID, DimensionID, WorldID,
    // Name, Race, 1 ) — the leading 0 is the AUTO_INCREMENT id, the
    // trailing 1 the first Amount.
    virtual void insertPCRoomLotto(ObjectID_t pcRoomID, const std::string& playerID, uint dimensionID, uint worldID,
                                   const std::string& name, Race_t race) = 0;

    // --- per-character IP records (DARKEDEN) -------------------------------
    virtual void deleteUserIP(const std::string& name) = 0;
    virtual void deleteUserIPsOfServer(int serverID) = 0;

    // --- the per-server user count (USERINFO connection) -------------------
    // True when a row was updated; the caller inserts otherwise.
    virtual bool updateUserStatus(uint currentUser, int worldID, int serverID) = 0;
    virtual void insertUserStatus(int worldID, int serverID, uint currentUser) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLSessionRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
SessionRepository& defaultSessionRepository();

#endif
