#ifndef __COUPLE_REPOSITORY_H__
#define __COUPLE_REPOSITORY_H__

#include <string>

#include "Types.h"

// Persistence seam for CoupleInfo, the one table the couple system owns
// (task 3.2). A pairing is ONE row with one column per sex
// (MalePartnerName, FemalePartnerName) plus the shared Race and the
// CoupleDate the database stamps with now(). So every statement here
// names its columns from the SEX of the characters involved rather than
// from a fixed list — which is why these methods take a Sex where other
// seams would take a column name, and why two methods share one
// literal — spelled once, in the implementation's private helper —
// and differ only in how they derive its arguments.
//
// Reads are typed to the driver getter the inline code called: the three
// count(*) probes return the int their callers compared against 1, and
// the partner read returns the getString column. Race reaches the
// statements as the (uint) cast the call sites always applied.
//
// One asymmetry to know about, inherited from the inline code: all
// three DELETEs filter on Race, while none of the three count probes
// nor the partner read does. A character with pairings in two races
// therefore still reads as coupled after the pairing of their own
// race is removed. Nothing in the couple flow creates such a pair —
// WaitForMeet::canMakeCouple rejects a different race and a matching
// sex before makeCouple, which then Asserts both, and Assert is live
// in every configuration this project builds (NDEBUG is deliberately
// never defined). Nor does the schema prevent one: CoupleInfo's only
// UNIQUE key is its AUTO_INCREMENT ID, the two name indexes being
// non-unique.
//
// Not enclosed: the two "DELETE FROM CoupleInfo WHERE <column>='%s'"
// pairs that erase a deleted character's pairings from BOTH columns at
// once — CreatureUtil.cpp's and the loginserver's CLDeletePCHandler.cpp.
// Those name their columns literally rather than by sex, and the
// loginserver copy is a different binary; they join their own rounds.
class CoupleRepository {
public:
    virtual ~CoupleRepository() {}

    // CoupleManager::isCouple(pPC1, name2): is this character paired with
    // a partner of that name? The partner column is the COUNTER of the
    // caller's own sex.
    virtual int countPairingWithPartner(Sex sex, const std::string& name, const std::string& partnerName) = 0;
    // CoupleManager::isCouple(pPC1, pPC2): the same literal, but each
    // column comes from its own character's sex. The two agree whenever
    // the two sexes are the two valid values and differ, which that
    // caller guarantees by returning early
    // when they match — so this is not a second spelling, it is a second
    // derivation of the same statement, and both are kept because task
    // 3.2 does not choose between call sites.
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
    // "where" in lower case. The case of those five bytes is the only
    // textual difference from deletePairing, and it stays a separate
    // method rather than a spelling parameter because the two also
    // derive their columns differently and each is reached from one
    // CoupleManager entry point — there is nothing for an enum to
    // select between. (CoupleManager::removeCoupleForce itself has
    // three call sites; it is the repository methods that are 1:1.)
    virtual void deletePairingWithPartner(Sex sex, const std::string& name, const std::string& partnerName,
                                          uint race) = 0;
    // CoupleManager::removeCoupleForce(pPC1): every pairing naming this
    // character in its own sex's column, for this race.
    virtual void deletePairingsOf(Sex sex, const std::string& name, uint race) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLCoupleRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
CoupleRepository& defaultCoupleRepository();

#endif
