#ifndef __SESSION_REPOSITORY_H__
#define __SESSION_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Seam for the player-session bookkeeping (task 3.2): the guild roster's
// online flag (GuildMember.LogOn), the account row the connect path
// reads WHOLE (Player.LogOn, LastLogoutDate and SpecialEventCount, plus
// the server group and the billing columns loadPlayerSession takes), the
// PC-room
// records (PCRoomUserInfo, PCRoomLottoObject), the per-character IP table
// (UserIPInfo) and the per-server user count a NetMarble deployment
// publishes (USERINFO.UserStatus). Three connections, as before: Player
// and the PC-room tables go through the thread's dist connection
// (mostly under the name "PLAYER_DB", though loadPlayerLocation asks
// for "USERINFO" — DatabaseManager ignores the name and hands back the
// second per-thread socket to the same schema, see
// MySQLGoodsRepository.cpp), UserStatus through the process-wide USERINFO
// connection, the rest through the DARKEDEN connection. Write parameters
// are typed to the members/expressions each caller streamed.
//
// The session START side joined this seam on 2026-09-04:
// CGConnectHandler's account read, its Player LogOn='GAME' flip and
// its three GuildMember LogOn = 1 writes are loadPlayerSession,
// markPlayerLoggedOn and markGuildMemberLoggedOn below.
//
// Not enclosed: CGPortCheckHandler's UserIPInfo upsert,
// CGRequestIPHandler's and CGSayHandler's UserIPInfo reads — all
// handler-directory files (R3); CGSayHandler's and
// billing/CommonBillingPacket.cpp's Player.LogOn / LastLogoutDate reads,
// and CGSayHandler's "UPDATE Player set Access='DENY'" — a Player WRITE
// to a column this seam does not name, which the list missed twice;
// src/server/PaySystem.cpp's PCRoomUserInfo statements (ServerCore, every
// caller under the disabled __PAY_SYSTEM_* macros); the loginserver's
// LoginPlayerManager sweep and its copy of addLogoutPlayerData (the
// gameserver's copy was dead and is deleted); and the unbuilt
// src/server/IncomingPlayerManager.cpp fork that still carries the sweep.

// CGConnectHandler's account read. The columns are named in the order
// the statement selects them, and each is typed to the driver getter
// that call site used: getString, getInt, getDWORD for the event
// count. payType is the int the caller casts to PayType.
struct PlayerSessionRow {
    std::string playerID;
    int serverGroupID;
    std::string logOn;
    DWORD specialEventCount;
    int payType;
    std::string payPlayDate;
    int payPlayHours;
    int payPlayFlag;
    int billingUserKey;
    std::string familyPayPlayDate;
    // __THAILAND_SERVER__ only; empty elsewhere.
    std::string birthday;
};

class SessionRepository {
public:
    virtual ~SessionRepository() {}

    // --- guild roster (DARKEDEN) -------------------------------------------
    // GuildMember.LogOn = 0 for one character. Two callers: the session
    // end, and skill/Restore.cpp's vampire–>slayer morph, whose own copy
    // of the write never freed its Statement.
    virtual void markGuildMemberLoggedOff(const std::string& name) = 0;

    // GuildMember.LogOn = 1, the mirror of the write above. Three
    // identical call sites in CGConnectHandler, one per race, none of
    // which freed its Statement.
    virtual void markGuildMemberLoggedOn(const std::string& name) = 0;

    // --- the account row (dist connection) ---------------------------------
    // The connect-time account read. False unless EXACTLY one row: the
    // caller treats none and several alike, logging to
    // connectDB_BUG.txt and throwing ProtocolException, so the
    // distinction never reaches game code.
    //
    // Under __THAILAND_SERVER__ the statement selects an eleventh
    // column, Birthday, which that build reads to set the adult
    // permission. birthday is left empty in every other build, where
    // the column is not selected at all.
    virtual bool loadPlayerSession(const std::string& playerID, PlayerSessionRow& row) = 0;
    // LogOn='GAME', but only for a row still in 'LOGOFF'. Returns
    // whether a row was affected — the caller reads false as "someone
    // else got there first" and throws. This is the mirror of
    // markPlayerLoggedOff, which does not report its row count. (Its
    // caller did ask, into an empty if block the session round deleted;
    // "never asked" was the wrong way to put it.)
    virtual bool markPlayerLoggedOn(const std::string& playerID) = 0;
    // CGWhisperHandler's cross-server lookup: where is this account,
    // and is it in game? False when the account has no row, leaving
    // both out-parameters untouched. Note the connection name: that
    // call site asks getDistConnection for "USERINFO" where every
    // other Player statement THAT GOES THROUGH getDistConnection asks
    // for "PLAYER_DB". Plenty do not go through it at all —
    // CGSayHandler's "UPDATE Player set Access='DENY'" uses
    // getConnection("DARKEDEN"), as does every Player statement in the
    // loginserver — so "every other Player statement" would be wrong.
    // DatabaseManager ignores the name and hands back the same
    // per-thread socket, so the two reach the same schema; the name is
    // kept because it is what that call site wrote, and the integration
    // tier now pins that a write through the PLAYER_DB-named path lands
    // in the row this one reads.
    virtual bool loadPlayerLocation(const std::string& playerID, int& serverGroupID, std::string& logOn) = 0;
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
