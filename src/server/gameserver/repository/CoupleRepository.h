#ifndef __COUPLE_REPOSITORY_H__
#define __COUPLE_REPOSITORY_H__

#include <string>

#include "Types.h"

// CoupleInfo, the one table the couple system owns. A pairing is ONE row
// with one column per sex (MalePartnerName, FemalePartnerName) plus the
// shared Race and the CoupleDate the database stamps with now(). Every
// statement here names its columns from the SEX of the characters
// involved, which is why these methods take a Sex where another
// repository would take a column name.
//
// The three count(*) probes return the int their callers compare against
// 1; the partner read returns the getString column.
//
// One asymmetry: all three DELETEs filter on Race, while none of the
// three count probes nor the partner read does. A character with
// pairings in two races therefore still reads as coupled after the
// pairing of their own race is removed. Nothing in the couple flow
// creates such a pair — WaitForMeet::canMakeCouple rejects a different
// race and a matching sex before makeCouple, which then Asserts both —
// and the schema does not prevent one: CoupleInfo's only UNIQUE key is
// its AUTO_INCREMENT ID.
//
// The character-deletion purge erases a deleted character's pairings from
// BOTH columns at once through CharacterPurgeRepository.
class CoupleRepository {
public:
    virtual ~CoupleRepository() {}

    // CoupleManager::isCouple(pPC1, name2): is this character paired with
    // a partner of that name? The partner column is the COUNTER of the
    // caller's own sex.
    virtual int countPairingWithPartner(Sex sex, const std::string& name, const std::string& partnerName) = 0;
    // CoupleManager::isCouple(pPC1, pPC2): the same statement, but each
    // column comes from its own character's sex. The two agree whenever
    // the two sexes differ, which that caller guarantees by returning
    // early when they match.
    virtual int countPairing(Sex sex1, const std::string& name1, Sex sex2, const std::string& name2) = 0;
    // CoupleManager::hasCouple: is this character paired with anyone?
    virtual int countPairingsOf(Sex sex, const std::string& name) = 0;
    // CoupleManager::getPartnerName: the counter column of the first row
    // naming this character. False when there is no such row, leaving
    // partnerName untouched.
    virtual bool loadPartnerName(Sex sex, const std::string& name, std::string& partnerName) = 0;

    // CoupleManager::makeCouple. CoupleDate is now(), stamped by the
    // database; the row's ID is the table's AUTO_INCREMENT and is read
    // by nothing.
    virtual void insertCouple(Sex sex1, const std::string& name1, Sex sex2, const std::string& name2, uint race) = 0;
    // CoupleManager::removeCouple: both columns from their own
    // character's sex, and the statement spells WHERE in upper case.
    virtual void deletePairing(Sex sex1, const std::string& name1, Sex sex2, const std::string& name2, uint race) = 0;
    // CoupleManager::removeCoupleForce(pPC1, strPC2): the partner column
    // is the counter of the caller's sex, and this statement spells
    // "where" in lower case.
    virtual void deletePairingWithPartner(Sex sex, const std::string& name, const std::string& partnerName,
                                          uint race) = 0;
    // CoupleManager::removeCoupleForce(pPC1): every pairing naming this
    // character in its own sex's column, for this race.
    virtual void deletePairingsOf(Sex sex, const std::string& name, uint race) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLCoupleRepository.cpp.
CoupleRepository& defaultCoupleRepository();

#endif
