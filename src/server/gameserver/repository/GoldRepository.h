#ifndef __GOLD_REPOSITORY_H__
#define __GOLD_REPOSITORY_H__

#include <string>

#include "CharacterRace.h"
#include "Types.h"

// Persistence seam for the RELATIVE carried-gold writes and the
// integrity read (task 3.2). Like stash, Gold is a column ON the three
// race tables — but unlike stash, every operation targets ONLY the
// character's own table, and the writes are relative (Gold = Gold ±
// delta, arithmetic done by the database against whatever the row
// holds). The gameplay clamps (MAX_MONEY on the way up, zero on the way
// down) happen in the calling creature against its in-memory balance
// BEFORE the delta reaches this seam — the repository applies the
// already-clamped delta, exactly as the inline SQL did.
//
// This seam does NOT enclose every writer of the Gold column: setGoldEx
// in all three races writes it ABSOLUTELY through a tinysave fragment,
// and the sharedserver binary's GSQuitGuildHandler.cpp writes it from
// another process entirely. Those join their own extractions.
//
// SGAddGuildMemberOKHandler's DB-side-clamped fee is no longer among
// them — it is decreaseGoldClamped below.
class GoldRepository {
public:
    virtual ~GoldRepository() {}

    // Gold = Gold + delta on the character's own race table.
    virtual void increaseGold(const std::string& ownerName, CharacterRace race, Gold_t delta) = 0;

    // Gold = Gold - delta on the character's own race table. The caller
    // clamps delta to its in-memory balance; if the ROW holds less than
    // delta (integrity drift), the unsigned subtraction is the same
    // ER_DATA_OUT_OF_RANGE territory as GoodsRepository::takeOne — see
    // the MySQL implementation's quirk notes.
    virtual void decreaseGold(const std::string& ownerName, CharacterRace race, Gold_t delta) = 0;

    // SGAddGuildMemberOKHandler's guild-registration fee, charged to a
    // character who is NOT logged in: the guild was approved while the
    // payer was offline, so there is no in-memory balance to clamp
    // against and the clamp is written into the statement instead —
    // note that the handler's ONLINE path does not use decreaseGold
    // either: it clamps in memory and then writes ABSOLUTELY through
    // setGoldEx's tinysave fragment.
    // SET Gold = IF (fee > Gold, 0, Gold - fee). Two consequences worth
    // naming. It cannot raise the ER_DATA_OUT_OF_RANGE that
    // decreaseGold can, because a row holding less than the fee is
    // zeroed rather than driven negative; and the zeroing is silent, so
    // a character short of the fee pays everything they have and the
    // caller cannot tell. Both are the inline statement's behaviour,
    // kept.
    //
    // The WHERE names Name, not the uppercase NAME the two relative
    // writes above use. MySQL folds neither — column names are always
    // case-insensitive — but the bytes are the call site's.
    virtual void decreaseGoldClamped(const std::string& ownerName, CharacterRace race, Gold_t fee) = 0;

    // Read back the stored gold (the checkGoldIntegrity flow). Returns
    // false when the table has no row for the name; on true, gold carries
    // the column as the driver's getInt returned it.
    virtual bool loadGold(const std::string& ownerName, CharacterRace race, int& gold) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLGoldRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
GoldRepository& defaultGoldRepository();

#endif
