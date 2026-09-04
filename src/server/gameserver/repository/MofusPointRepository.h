#ifndef __MOFUS_POINT_REPOSITORY_H__
#define __MOFUS_POINT_REPOSITORY_H__

#include <string>

#include "Types.h"

// Persistence seam for the two mofus tables (task 3.2): the per-account
// power-point balance (MofusPowerPoint) and the audit trail of every
// save (MofusLog). Both are keyed by an ACCOUNT id, not a character
// name — OwnerID is the player id the mofus link sends.
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

    // Point = Point + amount. Returns whether a row was affected — the
    // caller inserts when none was, which is how a first save creates
    // the row. Note the statement spells "Update" and "Insert Into" in
    // mixed case, and the insert is POSITIONAL: it names no columns and
    // so depends on MofusPowerPoint being (OwnerID, Point).
    virtual bool increasePowerPoint(const std::string& ownerID, int amount) = 0;
    virtual void insertPowerPoint(const std::string& ownerID, int amount) = 0;

    // One MofusLog row per save. SaveTime is now(), stamped by the
    // database. recvPoint and savePoint are ints reaching "%u", exactly
    // as the call site wrote them, against smallint(5) SIGNED columns.
    virtual void logPowerPoint(const std::string& ownerID, int recvPoint, int savePoint) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLMofusPointRepository.cpp. An accessor function rather than a
// g_p* extern: ratchet R1 counts those.
MofusPointRepository& defaultMofusPointRepository();

#endif
