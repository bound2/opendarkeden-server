#ifndef __MOFUS_POINT_REPOSITORY_H__
#define __MOFUS_POINT_REPOSITORY_H__

#include <string>

#include "Types.h"

// The two mofus tables: the power-point balance (MofusPowerPoint) and
// the MofusLog rows the mofus link writes when it credits points.
// Credits are logged; spends (CGUsePowerPointHandler, through
// savePowerPoint) are not.
//
// Both are keyed by CHARACTER NAME: every caller passes getName(), and
// MJob's account id (m_UserID) never reaches these methods. The column
// widths disagree — MofusPowerPoint.OwnerID is varchar(30), MofusLog.OwnerID
// varchar(20) — so a name over 20 characters would be truncated in the
// log but not the balance; names are varchar(10) elsewhere, so neither
// is reachable. Nothing deletes MofusLog; the loginserver erases a
// deleted character's balance.
//
// Every mofus call site deliberately SWALLOWS SQL errors ("SQL 에러는
// 무시한다" — ignore SQL errors) and carries on with a zero balance: the
// mofus link is an external service, and the game must not fall over
// when its bookkeeping does. That swallow lives at the call sites, not
// here — these methods raise like every other repository method
// (END_DB's const char*), and Mofus.cpp catches.
class MofusPointRepository {
public:
    virtual ~MofusPointRepository() {}

    // The balance for an account. False when the account has no row,
    // leaving point untouched; the callers start it at 0 and read that
    // as "no points".
    virtual bool loadPowerPoint(const std::string& ownerID, int& point) = 0;

    // Point = Point + amount. Returns whether a row was AFFECTED, not
    // whether one matched: the connection sets no CLIENT_FOUND_ROWS, so
    // an amount of 0 against an existing row also returns false and
    // sends the caller into a duplicate-key insert the swallow eats.
    // The caller inserts when nothing was affected, which is how a
    // first save creates the row. Note the statement spells "Update" and "Insert Into" in
    // mixed case, and the insert is POSITIONAL: it names no columns and
    // so depends on MofusPowerPoint being (OwnerID, Point).
    virtual bool increasePowerPoint(const std::string& ownerID, int amount) = 0;
    virtual void insertPowerPoint(const std::string& ownerID, int amount) = 0;

    // One MofusLog row per point transfer RECEIVED from the mofus link.
    // SaveTime is now(), stamped by the database. recvPoint and savePoint
    // are ints reaching "%u", against smallint(5) SIGNED columns.
    virtual void logPowerPoint(const std::string& ownerID, int recvPoint, int savePoint) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLMofusPointRepository.cpp.
MofusPointRepository& defaultMofusPointRepository();

#endif
