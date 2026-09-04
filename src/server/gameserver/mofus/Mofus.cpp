/////////////////////////////////////////////////////////////////////////////
// Filename : Mofus.cpp
// Desc		:
/////////////////////////////////////////////////////////////////////////////

#include "Mofus.h"

#include "Exception.h"
#include "repository/MofusPointRepository.h"

// Every function here deliberately IGNORES a SQL failure and carries on
// with a zero balance ("SQL 에러는 무시한다"): the mofus link is an
// external service, and the game must not fall over when its bookkeeping
// does.
//
// The type that has to be caught to do that CHANGED with the move to the
// seam, and getting it wrong would silently turn an ignored error into a
// thrown one. These functions used to run their own statements, so a
// failure arrived as the SQLQueryException the driver raises. Now each
// repository call converts its own inside END_DB and rethrows a bare
// const char*, which is what the swallow has to name. The
// SQLQueryException catch is gone because nothing here can raise one any
// more, and so is the catch (...) that freed the Statement before
// rethrowing — its cleanup did not move into the repository, which
// therefore leaks on a non-SQLQueryException path. Unreachable for
// these statements, and recorded in the implementation.
//
// What the swallow prevents is worse than an ignored error becoming a
// thrown one, and it differs per caller. PlayerCreature::load calls
// loadPowerPoint on EVERY character login, so an escape there reaches
// GamePlayer::processCommand's catch (...) and disconnects the player
// instead of logging them in with zero points. The Restore and
// EventMorph paths call it on a zone thread, where nothing catches a
// const char* at all — std::terminate, i.e. the process. And
// MPlayerManager::processResult calls it INSIDE
// __ENTER_CRITICAL_SECTION((*g_pPCFinder)), whose
// __LEAVE_CRITICAL_SECTION catches Throwable& only: that one would
// also leave g_pPCFinder held on the way out.

int loadPowerPoint(const string& name) {
    __BEGIN_TRY

    int powerpoint = 0;

    try {
        defaultMofusPointRepository().loadPowerPoint(name, powerpoint);
    } catch (const char*) {
        // SQL 에러는 무시한다.
    }

    return powerpoint;

    __END_CATCH
}

int savePowerPoint(const string& name, int amount) {
    __BEGIN_TRY

    int powerpoint = 0;

    // One try around all three statements, as before: a failure in any of
    // them skips the rest and leaves powerpoint at 0.
    try {
        MofusPointRepository& points = defaultMofusPointRepository();

        // No row yet means a first save, which the insert creates.
        if (!points.increasePowerPoint(name, amount)) {
            points.insertPowerPoint(name, amount);
        }

        points.loadPowerPoint(name, powerpoint);
    } catch (const char*) {
        // SQL 에러는 무시한다.
    }

    return powerpoint;

    __END_CATCH
}

void logPowerPoint(const string& name, int recvPoint, int savePoint) {
    __BEGIN_TRY

    try {
        defaultMofusPointRepository().logPowerPoint(name, recvPoint, savePoint);
    } catch (const char*) {
        // SQL 에러는 무시한다.
    }

    __END_CATCH
}
