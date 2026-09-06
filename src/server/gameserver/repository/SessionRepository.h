#ifndef __SESSION_REPOSITORY_H__
#define __SESSION_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Player-session bookkeeping: the guild roster's online flag
// (GuildMember.LogOn), the account row (Player: LogOn, LastLogoutDate,
// SpecialEventCount, server group and billing columns), the PC-room
// records (PCRoomUserInfo, PCRoomLottoObject), the per-character IP table
// (UserIPInfo), the client's self-reports (SpeedHackPlayer, CrashReportLog),
// the GM commands' report tables (BugReportLog, CrashLog) and the
// per-server user count a NetMarble deployment publishes
// (USERINFO.UserStatus).
//
// Three connections: Player and the PC-room tables go through the
// thread's dist connection (DatabaseManager ignores the name asked for
// and hands back the second per-thread socket to the same schema),
// UserStatus through the process-wide USERINFO connection, the rest
// through the DARKEDEN connection. The one exception is denyAccount,
// which writes Player on the DARKEDEN connection.

// The connect-time account read. Columns in the order the statement
// selects them, typed to the driver getter used for each: getString,
// getInt, getDWORD for the event count. payType is cast to PayType by the
// caller.
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
    // GuildMember.LogOn = 0 for one character.
    virtual void markGuildMemberLoggedOff(const std::string& name) = 0;
    // GuildMember.LogOn = 1 for one character.
    virtual void markGuildMemberLoggedOn(const std::string& name) = 0;

    // --- the account row (dist connection) ---------------------------------
    // False unless EXACTLY one row; the caller treats none and several
    // alike (logs to connectDB_BUG.txt and throws ProtocolException).
    //
    // Under __THAILAND_SERVER__ the statement selects an eleventh column,
    // Birthday, used to set the adult permission. In every other build the
    // column is not selected and birthday is left empty.
    virtual bool loadPlayerSession(const std::string& playerID, PlayerSessionRow& row) = 0;
    // LogOn='GAME', but only for a row still in 'LOGOFF'. Returns whether a
    // row was affected; the caller reads false as "someone else got there
    // first" and throws.
    virtual bool markPlayerLoggedOn(const std::string& playerID) = 0;
    // Where is this account, and is it in game? False when the account has
    // no row, leaving both out-parameters untouched.
    virtual bool loadPlayerLocation(const std::string& playerID, int& serverGroupID, std::string& logOn) = 0;
    // LogOn='LOGOFF', LastLogoutDate=now() — only for a row still in 'GAME'.
    // Does not report whether a row was affected.
    virtual void markPlayerLoggedOff(const std::string& playerID) = 0;
    // The accounts this world/server group left in 'GAME' (boot-time sweep).
    virtual std::vector<std::string> loadPlayersInGame(int worldID, int serverGroupID) = 0;
    virtual void logOffPlayersOfServer(int worldID, int serverGroupID) = 0;
    // False when the account has no row.
    virtual bool loadSpecialEventCount(const std::string& playerID, DWORD& count) = 0;
    virtual void saveSpecialEventCount(uint count, const std::string& playerID) = 0;

    // --- PC-room records (dist connection) ---------------------------------
    virtual void deletePCRoomUser(const std::string& playerID) = 0;
    // One lotto row per (player, character, dimension, world). amount is
    // left untouched when there is no row.
    virtual bool loadPCRoomLottoAmount(const std::string& playerID, const std::string& name, uint dimensionID,
                                       uint worldID, int& amount) = 0;
    virtual void updatePCRoomLottoAmount(int amount, const std::string& playerID, const std::string& name,
                                         uint dimensionID, uint worldID) = 0;
    // Positional INSERT ( 0, PCRoomID, PlayerID, DimensionID, WorldID, Name,
    // Race, 1 ): the leading 0 is the AUTO_INCREMENT id, the trailing 1 the
    // first Amount.
    virtual void insertPCRoomLotto(ObjectID_t pcRoomID, const std::string& playerID, uint dimensionID, uint worldID,
                                   const std::string& name, Race_t race) = 0;

    // --- per-character IP records (DARKEDEN) -------------------------------
    virtual void deleteUserIP(const std::string& name) = 0;
    virtual void deleteUserIPsOfServer(int serverID) = 0;
    // INSERT IGNORE of (Name, IP, Port, ServerID); when that changed no row,
    // UPDATE IP and Port for the name instead. A SQL failure in either
    // statement is logged to DBError.log and thrown as END_DB's const
    // char*, which the caller swallows.
    virtual void recordUserIP(const std::string& name, DWORD ip, uint port, int serverID) = 0;
    // IP and Port for one character; false when there is no row.
    virtual bool loadUserIP(const std::string& name, DWORD& ip, DWORD& port) = 0;

    // --- what the client reports about itself (dist and DARKEDEN) ------------
    // The per-account speed-hack counter, on the dist connection: an UPDATE
    // of IP/NAME/WorldID/ServerGroupID/Date=now()/Count+1, then, when it
    // changed no row, an INSERT IGNORE of a first row with Count 1.
    virtual void recordSpeedHack(const std::string& playerID, const std::string& ip, const std::string& name,
                                 int worldID, int serverGroupID) = 0;
    // The crash report row (DARKEDEN), ReportTime = now(). The seven texts
    // are interpolated unescaped.
    virtual void insertCrashReport(const std::string& playerID, const std::string& name,
                                   const std::string& executableTime, WORD version, const std::string& address,
                                   const std::string& message, const std::string& os, const std::string& callStack) = 0;
    // Player.LastLogoutDate as the datetime text; false when no row. On the
    // dist connection. CommonBillingPacket.cpp is also compiled into the
    // loginserver, which does not link this repository, so its call sits
    // under __GAME_SERVER__.
    virtual bool loadLastLogoutDate(const std::string& playerID, std::string& lastLogoutDate) = 0;

    // --- the GM commands (CGSayHandler) ---------------------------------------
    // UserIPInfo.ServerID for one character; false when no row.
    virtual bool loadUserServerID(const std::string& name, int& serverID) = 0;
    // Count of Player rows with LogOn 'GAME' or 'LOGON', on the dist
    // connection.
    virtual int countPlayersOnline() = 0;
    // Player.Access='DENY' for one account. On the DARKEDEN connection, not
    // the dist one the other Player statements use.
    virtual void denyAccount(const std::string& playerID) = 0;
    // BugReportLog row, ReportTime = now(). The caller replaces the first
    // quote and the first backslash in the text with '_' before calling;
    // nothing else is escaped.
    virtual void insertBugReport(const std::string& playerID, const std::string& name, const std::string& report) = 0;
    // CrashLog row (not CrashReportLog): every value text, ReportTime =
    // now().
    virtual void insertCrashLog(const std::string& playerID, const std::string& name, const std::string& executableTime,
                                const std::string& version, const std::string& address, const std::string& message) = 0;

    // --- the per-server user count (USERINFO connection) -------------------
    // True when a row was updated; the caller inserts otherwise.
    virtual bool updateUserStatus(uint currentUser, int worldID, int serverID) = 0;
    virtual void insertUserStatus(int worldID, int serverID, uint currentUser) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLSessionRepository.cpp.
SessionRepository& defaultSessionRepository();

#endif
