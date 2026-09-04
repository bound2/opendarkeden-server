#ifndef __MOFUS_POINT_REPOSITORY_H__
#define __MOFUS_POINT_REPOSITORY_H__

#include <string>

#include "Types.h"

// Persistence seam for the two mofus tables (task 3.2): the power-point
// balance (MofusPowerPoint) and the MofusLog rows the mofus link
// writes when it credits points. NOT an audit trail of every save, as
// an earlier version of this comment said: logPowerPoint has one
// caller, and CGUsePowerPointHandler's spend goes through
// savePowerPoint without it. Credits are logged; spends are not.
//
// Not enclosed: the loginserver's
// "DELETE FROM MofusPowerPoint WHERE OwnerID='%s'" in
// CLDeletePCHandler.cpp, which erases a deleted character's balance —
// a different binary, so it joins its own round. Nothing anywhere
// deletes MofusLog. (That delete is also independent evidence for the
// paragraph below: it sits among the per-CHARACTER cleanups, so an
// account-keyed balance would be wiped by deleting one character.)
//
// Both are keyed by CHARACTER NAME. An earlier version of this comment
// said account id, which is wrong: every caller passes getName() —
// PlayerCreature::load, MPlayerManager::processResult,
// CGUsePowerPointHandler, and PKTPowerPointHandler through
// MJob::m_Name, whose own comment is "character name". MJob carries the
// account id separately as m_UserID, and it never reaches these
// methods. The seeded MofusLog rows are character names too. Note the
// column widths disagree with each other — MofusPowerPoint.OwnerID is
// varchar(30), MofusLog.OwnerID is varchar(20) — so a name over 20
// characters would be truncated in the log but not the balance. Names
// are varchar(10) elsewhere, so neither is reachable.
//
// A note on failure that matters more here than in most seams. Every
// mofus call site deliberately SWALLOWS SQL errors ("SQL 에러는
// 무시한다" — ignore SQL errors) and carries on with a zero balance:
// the mofus link is an external service, and the game must not fall
// over when its bookkeeping does. That swallow lives at the call
// sites, not here — these methods raise like every other repository
// method, and Mofus.cpp catches. What changed with the move is the
// TYPE it has to catch: see the comment there.
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
    // Its only caller is PKTPowerPointHandler; the in-game spend calls
    // savePowerPoint and never this, so the table is a record of
    // credits, not of balance changes. SaveTime is now(), stamped by the
    // database. recvPoint and savePoint are ints reaching "%u", exactly
    // as the call site wrote them, against smallint(5) SIGNED columns.
    virtual void logPowerPoint(const std::string& ownerID, int recvPoint, int savePoint) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLMofusPointRepository.cpp. An accessor function rather than a
// g_p* extern: ratchet R1 counts those.
MofusPointRepository& defaultMofusPointRepository();

#endif
