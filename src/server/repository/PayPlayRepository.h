#ifndef __PAY_PLAY_REPOSITORY_H__
#define __PAY_PLAY_REPOSITORY_H__

#include <string>

#include "Types.h"

// PaySystem's persistence, in ServerCore (compiled into all three
// binaries; the gameserver's GamePlayer and the loginserver's LoginPlayer
// derive from PaySystem): the account's pay-play columns on Player
// (PayType, PayPlayDate, PayPlayHours, PayPlayFlag, FamilyPayPlayDate)
// and the PC-room tables — the room a client IP belongs to (PCRoomInfo
// through PCRoomIPInfo), the room's occupants (PCRoomUserInfo) and its
// monthly minutes (PCRoomPayList).
//
// Every method runs on g_pDatabaseManager->getDistConnection("PLAYER_DB"):
// the thread's dist connection when one is registered, otherwise the
// process default connection in the loginserver and the world-default
// connection in the other two binaries (the name is ignored).
//
// Reads are typed to the driver getter used: getInt → int, getString →
// std::string. The callers cast to PayType, ObjectID_t and VSDateTime.
// The client IP and the account id are interpolated raw. The two
// PayPlayHours decrements pass an unsigned count through "%d".
//
// Not enclosed: Player's pay-play columns are also read by the
// loginserver's LoginAccountRepository (the login projections) and
// written by its extendPayPlayByWeek; PCRoomUserInfo is also deleted by
// LoginAccountRepository::deletePCRoomUser and read and written by the
// gameserver's MySQLSessionRepository.cpp. PCRoomInfo, PCRoomIPInfo and
// PCRoomPayList: nothing else.

// The room a client IP belongs to, eight columns.
struct PayPlayPCRoomRow {
    int id;
    int payType;
    std::string payStartDate;
    std::string payPlayDate;
    int payPlayHours;
    int payPlayFlag;
    int userLimit;
    int userMax;
};

// The same join, the first five columns (PaySystem::isPlayInPayPCRoom).
struct PayPlayPCRoomPeriodRow {
    int id;
    int payType;
    std::string payStartDate;
    std::string payPlayDate;
    int payPlayHours;
};

// The account's pay-play columns.
struct PayPlayAccountRow {
    int payType;
    std::string payPlayDate;
    int payPlayHours;
    int payPlayFlag;
    std::string familyPayPlayDate;
};

class PayPlayRepository {
public:
    virtual ~PayPlayRepository() {}

    // --- the PC room (PCRoomInfo, PCRoomIPInfo) ---------------------------------
    // The first room whose PCRoomIPInfo row carries this IP. False when
    // there is none; row is untouched then.
    virtual bool loadPCRoomByIP(const std::string& ip, PayPlayPCRoomRow& row) = 0;
    virtual bool loadPCRoomPeriodByIP(const std::string& ip, PayPlayPCRoomPeriodRow& row) = 0;
    // PCRoomInfo.PayPlayHours -= hours, then the column read back. False
    // when the room has no row; hours is untouched then.
    virtual bool decreasePCRoomPayPlayHours(uint hours, int roomID, int& remaining) = 0;

    // --- the room's occupants (PCRoomUserInfo) ------------------------------------
    // count(*) of the room's rows.
    virtual int loadPCRoomUserCount(int roomID) = 0;
    // INSERT IGNORE of (ID, PlayerID); (ID, PlayerID) is the primary key.
    virtual void insertPCRoomUser(int roomID, const std::string& playerID) = 0;
    // Every row of that account, whatever the room.
    virtual void deletePCRoomUser(const std::string& playerID) = 0;

    // --- the room's monthly minutes (PCRoomPayList) --------------------------------
    // Whether the room has a row for that year and month (the minute count
    // is read and not used by the caller).
    virtual bool hasPCRoomPayMonth(int roomID, int year, int month) = 0;
    virtual void addPCRoomPayMinutes(uint minutes, int roomID, int year, int month) = 0;
    virtual void insertPCRoomPayMonth(int roomID, int year, int month, uint minutes) = 0;

    // --- the account (Player) ------------------------------------------------
    // False when there is no row; row is untouched then.
    virtual bool loadAccountPayPlay(const std::string& playerID, PayPlayAccountRow& row) = 0;
    // PayPlayHours = 0, PayPlayDate = '2002-11-18 00:00:00'.
    virtual void clearAccountPayPlay(const std::string& playerID) = 0;
    // PayPlayHours -= hours.
    virtual void decreaseAccountPayPlayHours(uint hours, const std::string& playerID) = 0;
    // "SELECT PayType=0 or PayPlayDate > now()": 1 when the account plays
    // free or its period is still running, 0 otherwise. False when there
    // is no row; flag is untouched then.
    virtual bool loadAccountPayPlaying(const std::string& playerID, int& flag) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLPayPlayRepository.cpp.
PayPlayRepository& defaultPayPlayRepository();

#endif
