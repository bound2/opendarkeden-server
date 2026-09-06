#ifndef __COMEBACK_EVENT_REPOSITORY_H__
#define __COMEBACK_EVENT_REPOSITORY_H__

#include <string>

#include "Types.h"

// The 2005 event tables: the "comeback" event's Event200501Main and
// Event200501Recommend, keyed by the ACCOUNT id (PlayerID), not the
// character name, and the New Year donation event's
// DonationPersonal200501 and DonationGuild200501, counted by CHARACTER
// name plus world id (neither table has a key; the only index on each
// starts with PlayerID, so those counts match no index prefix).
//
// The zone asks the three predicates below when a character enters a
// zone, to nag the player about unclaimed event items.
// CGGetEventItemHandler then hands the items out: it reads the date
// columns as text and compares them to '0000-00-00' itself, and stamps
// the received date with now(). CGDonationMoneyHandler counts a
// character's donations before and after it records one, to award
// nicknames at the first, third and fifth.
//
// Every statement goes through the thread's dist connection (the second
// connection every zone thread registers; same server and DARKEDEN schema
// as the world connection in the shipped stack). Reads are typed to the
// getter used: the dates through getString, UniqueID and the counts
// through getInt.
class ComebackEventRepository {
public:
    virtual ~ComebackEventRepository() {}

    // A main-event row whose RecvItemDate is still the zero date.
    virtual bool hasUnclaimedItem(const std::string& playerID) = 0;

    // A main-event row that has PAID for premium (PayPremiumDate set)
    // but not yet received the premium item.
    virtual bool hasUnclaimedPremiumItem(const std::string& playerID) = 0;

    // A recommend-event row whose RecvItemDate is still the zero date.
    virtual bool hasUnclaimedRecommendItem(const std::string& playerID) = 0;

    // --- CGGetEventItemHandler: the hand-out ---------------------------------
    // Event200501Main.RecvItemDate as text; false when the account has no
    // row.
    virtual bool loadMainRecvItemDate(const std::string& playerID, std::string& recvItemDate) = 0;
    // Event200501Main.PayPremiumDate and RecvPremiumItemDate as text; false
    // when no row.
    virtual bool loadMainPremiumDates(const std::string& playerID, std::string& payPremiumDate,
                                      std::string& recvPremiumItemDate) = 0;
    // The FIRST Event200501Recommend row the ORDER-BY-less SELECT returns
    // for the account (PlayerID is not unique there: one row per
    // recommendation); false when none.
    virtual bool loadRecommendRow(const std::string& playerID, int& uniqueID, std::string& recvItemDate) = 0;
    // The three stamps, RecvItemDate / RecvPremiumItemDate / RecvItemDate =
    // now(); the recommend one keys on UniqueID.
    virtual void markMainItemReceived(const std::string& playerID) = 0;
    virtual void markMainPremiumItemReceived(const std::string& playerID) = 0;
    virtual void markRecommendItemReceived(int uniqueID) = 0;

    // --- CGDonationMoneyHandler: the donation record --------------------------
    // COUNT(*) of Donation<Personal|Guild>200501 rows for (Name, WorldID).
    virtual int countPersonalDonations(const std::string& name, int worldID) = 0;
    virtual int countGuildDonations(const std::string& name, int worldID) = 0;
    // The two INSERTs are POSITIONAL — they name no columns, so they depend
    // on each table's column order: (PlayerID, Name, WorldID, Amount,
    // DonationDateTime) and (GuildID, GuildName, PlayerID, Name, WorldID,
    // Amount, DonationDateTime), the date now().
    virtual void insertPersonalDonation(const std::string& playerID, const std::string& name, int worldID,
                                        Gold_t gold) = 0;
    virtual void insertGuildDonation(GuildID_t guildID, const std::string& guildName, const std::string& playerID,
                                     const std::string& name, int worldID, Gold_t gold) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLComebackEventRepository.cpp.
ComebackEventRepository& defaultComebackEventRepository();

#endif
