#ifndef __RANK_BONUS_REPOSITORY_H__
#define __RANK_BONUS_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// The RankBonusData table. A row is just (OwnerID, Type): the point/rank
// values are re-derived from RankBonusInfoManager on load, never stored.
// The character-deletion sweeps (CharacterPurgeRepository here, the
// loginserver's CLDeletePCHandler) DELETE from this table as part of
// their multi-table purge.
class RankBonusRepository {
public:
    virtual ~RankBonusRepository() {}

    // Every stored Type for a character, Type ascending: the query has no
    // ORDER BY, but the covering index (OwnerID, Type) fully serves it, so
    // InnoDB's index scan returns Type order deterministically. The
    // table has no unique key, so
    // duplicates can come back; the in-memory book dedups.
    virtual std::vector<DWORD> loadTypes(const std::string& ownerName) = 0;

    // Record a learned bonus. Plain INSERT into a keyless table: inserting
    // a (owner, type) pair twice stores two rows.
    virtual void insert(const std::string& ownerName, DWORD type) = 0;

    // Remove every row of one type for a character.
    virtual void deleteOne(const std::string& ownerName, DWORD type) = 0;

    // Remove every row for a character.
    virtual void deleteAll(const std::string& ownerName) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLRankBonusRepository.cpp.
RankBonusRepository& defaultRankBonusRepository();

#endif
