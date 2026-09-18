//////////////////////////////////////////////////////////////////////////////
// Filename    : AlignmentManager.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "AlignmentManager.h"

#include "Assert.h"
#include "DB.h"
// #include <algo.h>

//////////////////////////////////////////////////////////////////////////////
// class AlignmentManager member methods
//////////////////////////////////////////////////////////////////////////////

AlignmentManager::AlignmentManager()

    {__BEGIN_TRY __END_CATCH}

AlignmentManager::~AlignmentManager()

    {__BEGIN_TRY __END_CATCH_NO_RETHROW}

Alignment AlignmentManager::getAlignmentType(Alignment_t Alignment)

{
    __BEGIN_TRY

    if (Alignment <= -10000) {
        return LESS_EVIL;
    }
    if (Alignment >= -10000 && Alignment < -7500) {
        return LESS_EVIL;
    } else if (Alignment >= -7500 && Alignment < -2500) {
        return EVIL;
    } else if (Alignment >= -2500 && Alignment < 2500) {
        return NEUTRAL;
    } else if (Alignment >= 2500 && Alignment < 7500) {
        return GOOD;
    } else if (Alignment >= 7500 && Alignment <= 10000) {
        return MORE_GOOD;
    } else {
        return MORE_GOOD;
    }

    __END_CATCH
}

int AlignmentManager::getMultiplier(Alignment_t AttackerAlignment, Alignment_t DefenderAlignment)

{
    __BEGIN_TRY

    Alignment AAlignmentType = getAlignmentType(AttackerAlignment);
    Alignment DAlignmentType = getAlignmentType(DefenderAlignment);

    // If the attacker's alignment is GOOD or MORE_GOOD,
    if (AAlignmentType >= GOOD) {
        // if the defender's alignment is GOOD or MORE_GOOD,
        if (DAlignmentType >= GOOD) {
            // an attacker who is the better of the two gives -2,
            if (AttackerAlignment > DefenderAlignment) {
                return -200;
            }
            // an attacker who is the worse of the two gives -3.
            else if (AttackerAlignment <= DefenderAlignment) {
                return -300;
            }
        }
        // if the defender's alignment is NEUTRAL,
        else if (DAlignmentType == NEUTRAL) {
            return -100;
        }
        // if the defender's alignment is EVIL or LESS_EVIL,
        else if (DAlignmentType <= EVIL) {
            return 200;
        }
    } else if (getAlignmentType(AttackerAlignment) == NEUTRAL) {
        // if the defender's alignment is GOOD or MORE_GOOD,
        if (DAlignmentType >= GOOD) {
            return -300;
        }
        // if the defender's alignment is NEUTRAL,
        else if (DAlignmentType == NEUTRAL) {
            // an attacker who is the better of the two gives -1,
            if (AttackerAlignment > DefenderAlignment) {
                return -100;
            }
            // an attacker who is the worse of the two gives -2.
            else if (AttackerAlignment <= DefenderAlignment) {
                return -200;
            }
        }
        // if the defender's alignment is EVIL or LESS_EVIL,
        else if (DAlignmentType <= EVIL) {
            return 100;
        }
    } else if (getAlignmentType(AttackerAlignment) <= EVIL) {
        // if the defender's alignment is GOOD or MORE_GOOD,
        if (DAlignmentType >= GOOD) {
            return -300;
        }
        // if the defender's alignment is NEUTRAL,
        else if (DAlignmentType == NEUTRAL) {
            return -200;
        }
        // if the defender's alignment is EVIL or LESS_EVIL,
        else if (DAlignmentType <= EVIL) {
            // an attacker who is the better of the two gives 2,
            if (AttackerAlignment > DefenderAlignment) {
                return 200;
            }
            // an attacker who is the worse of the two gives 1.
            else if (AttackerAlignment <= DefenderAlignment) {
                return 100;
            }
        }
    } else {
        return -300;
    }

    return -300;

    __END_CATCH
}

BYTE AlignmentManager::getDropItemNum(Alignment_t Alignment, bool isPK)

{
    __BEGIN_TRY

    int Count = 0;

    if (Alignment > -10000 && Alignment < -7500) {
        Count = 2;
    } else if (Alignment >= -7500 && Alignment < -2500) {
        Count = 1;
    } else if (Alignment == -10000) {
        Count = 3;
    }

    /*
    It is not known when this changed, but players with a good alignment
    started dropping items. Something that used to be here was probably
    lost while deleting comments in PCManager.cpp's killCreature(), so
    for now this part is commented out to keep good-aligned players from
    dropping items.
    int Percent = getDropBonusPercentage(Alignment);

    if (isPK)
    {
        Count = max(0, Count - 1);
        Percent = Percent/2;
    }

    Count = max(0, Count);
    Count = min(5, Count);

    if (Random(1, 100) < Percent)
    {
        Count++;
    }
    */

    return (BYTE)Count;

    __END_CATCH
}

BYTE AlignmentManager::getDropBonusPercentage(Alignment_t Alignment)

{
    __BEGIN_TRY
    return 0;

    int Percent = (10000 - Alignment) / 400;

    Percent = max(0, Percent);
    Percent = min(50, Percent);

    return (BYTE)Percent;

    __END_CATCH
}

BYTE AlignmentManager::getMoneyDropPenalty(Alignment_t Alignment)

{
    __BEGIN_TRY

    BYTE Penalty = 0;

    if (Alignment == 10000) {
        Penalty = 0;
    } else if (Alignment >= 7500 && Alignment < 10000) {
        Penalty = 1;
    } else if (Alignment >= 2500 && Alignment < 7500) {
        Penalty = 2;
    } else if (Alignment >= -2500 && Alignment < 2500) {
        Penalty = 4;
    } else if (Alignment >= -7500 && Alignment < -2500) {
        Penalty = 8;
    } else if (Alignment >= -10000 && Alignment < -7500) {
        Penalty = 16;
    } else {
        Penalty = 32;
    }

    return Penalty;

    __END_CATCH
}

string AlignmentManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "AlignmentManager (" << ")";
    return msg.toString();

    __END_CATCH
}
