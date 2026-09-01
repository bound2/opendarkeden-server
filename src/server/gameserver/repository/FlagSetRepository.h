#ifndef __FLAG_SET_REPOSITORY_H__
#define __FLAG_SET_REPOSITORY_H__

#include <string>

// Persistence seam for the FlagSet table (task 3.2): one row per
// character holding its flag bits as a '0'/'1' text (FlagData,
// varchar(24) — FLAG_SIZE_MAX bits). The FlagSet class owns the
// bit<->text encoding; this seam moves the text.
//
// The row is normally created by the loginserver at character creation
// (CLCreatePCHandler, with a race-specific default pattern) and purged
// with the character (CreatureUtil.cpp / CLDeletePCHandler) — neither
// is enclosed here. The gameserver's own create() is the newbie-item
// path's fallback.
class FlagSetRepository {
public:
    virtual ~FlagSetRepository() {}

    // FlagSet::create — a fresh row with the given text.
    virtual void insert(const std::string& ownerName, const std::string& flagData) = 0;

    // FlagSet::load's fallback when the owner has no row: an INSERT
    // IGNORE of an EMPTY FlagData — the primary key makes it a no-op if
    // a row appeared meanwhile.
    virtual void insertEmptyIfMissing(const std::string& ownerName) = 0;

    // Returns false when the owner has no row; on true, flagData carries
    // the column text as the driver's getString returned it.
    virtual bool load(const std::string& ownerName, std::string& flagData) = 0;

    // FlagSet::save.
    virtual void update(const std::string& ownerName, const std::string& flagData) = 0;

    // FlagSet::destroy.
    virtual void remove(const std::string& ownerName) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLFlagSetRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
FlagSetRepository& defaultFlagSetRepository();

#endif
