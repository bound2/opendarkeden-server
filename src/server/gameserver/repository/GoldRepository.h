#ifndef __GOLD_REPOSITORY_H__
#define __GOLD_REPOSITORY_H__

#include <string>

#include "CharacterRace.h"
#include "Types.h"

// The relative carried-gold writes and the integrity read. Gold is a
// column on the three race tables; every operation here targets only
// the character's own table, and the writes are relative (Gold = Gold ±
// delta, arithmetic done by the database against whatever the row
// holds). The gameplay clamps (MAX_MONEY on the way up, zero on the way
// down) happen in the calling creature against its in-memory balance
// before the delta reaches the repository. Not every writer of the
// column comes through here: setGoldEx in all three races writes it
// absolutely through a tinysave fragment.
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

    // The guild-registration fee, charged to a character who is NOT
    // logged in: there is no in-memory balance to clamp against, so the
    // clamp is in the statement: SET Gold = IF (fee > Gold, 0, Gold - fee).
    // It cannot raise the ER_DATA_OUT_OF_RANGE that decreaseGold can,
    // because a row holding less than the fee is zeroed rather than
    // driven negative; and the zeroing is silent, so a character short of
    // the fee pays everything they have and the caller cannot tell.
    virtual void decreaseGoldClamped(const std::string& ownerName, CharacterRace race, Gold_t fee) = 0;

    // Read back the stored gold (the checkGoldIntegrity flow). Returns
    // false when the table has no row for the name; on true, gold carries
    // the column as the driver's getInt returned it.
    virtual bool loadGold(const std::string& ownerName, CharacterRace race, int& gold) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLGoldRepository.cpp.
GoldRepository& defaultGoldRepository();

#endif
