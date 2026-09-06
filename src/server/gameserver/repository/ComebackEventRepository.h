#ifndef __COMEBACK_EVENT_REPOSITORY_H__
#define __COMEBACK_EVENT_REPOSITORY_H__

#include <string>

#include "Types.h"

// Seam for the 2005 event tables (task 3.2): the "comeback" event's
// Event200501Main and Event200501Recommend, keyed by the ACCOUNT id
// (PlayerID), not the character name, and — since the event-handler round
// (2026-09-06) — the New Year donation event's DonationPersonal200501 and
// DonationGuild200501, keyed by the CHARACTER name plus a world id.
//
// The zone asks the three predicates below when a character enters a
// zone, to nag the player about unclaimed event items (the Zone
// milestone's read-only part). CGGetEventItemHandler then hands the items
// out: it reads the date columns as text and compares them to
// '0000-00-00' itself, and stamps the received date with now() — those
// six statements are the loads and marks below. CGDonationMoneyHandler
// counts a character's donations before and after it records one, to
// award nicknames at the first, third and fifth — the two counts and the
// two positional INSERTs below.
//
// Not enclosed: the loginserver's CLLoginHandler.cpp reads and stamps
// Event200501Main with its own inline SQL — another binary, its own
// seam. No other SQL in the tree names these four tables.
//
// Every statement goes through the thread's "dist" connection
// (getDistConnection("PLAYER_DB") — the name is ignored; it is the
// second connection every zone thread registers, same server and
// DARKEDEN schema as the world connection in the shipped stack, see
// MySQLGoodsRepository.cpp), as both the zone and the handlers wrote it.
// Reads are typed to the getter the handler called: the dates through
// getString (the handler compares the text), UniqueID and the counts
// through getInt. Write parameters are typed as the handlers' arguments
// were.
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
    // "SELECT RecvItemDate FROM Event200501Main WHERE PlayerID = '%s'"; false
    // when the account has no row (the handler answers "not an event
    // target").
    virtual bool loadMainRecvItemDate(const std::string& playerID, std::string& recvItemDate) = 0;
    // "SELECT PayPremiumDate, RecvPremiumItemDate FROM Event200501Main WHERE
    // PlayerID = '%s'"; false when no row.
    virtual bool loadMainPremiumDates(const std::string& playerID, std::string& payPremiumDate,
                                      std::string& recvPremiumItemDate) = 0;
    // "SELECT UniqueID, RecvItemDate FROM Event200501Recommend WHERE PlayerID
    // = '%s'" — the FIRST row the ORDER-BY-less SELECT returns, as the
    // handler took it (PlayerID is not unique there: one row per
    // recommendation); false when none.
    virtual bool loadRecommendRow(const std::string& playerID, int& uniqueID, std::string& recvItemDate) = 0;
    // The three stamps, RecvItemDate / RecvPremiumItemDate / RecvItemDate =
    // now(); the recommend one keys on UniqueID, the int through a quoted
    // '%d' as written.
    virtual void markMainItemReceived(const std::string& playerID) = 0;
    virtual void markMainPremiumItemReceived(const std::string& playerID) = 0;
    virtual void markRecommendItemReceived(int uniqueID) = 0;

    // --- CGDonationMoneyHandler: the donation record --------------------------
    // "SELECT COUNT(*) FROM Donation<Personal|Guild>200501 WHERE Name = '%s'
    // AND WorldID = %d" through getInt. The handler assigned the count only
    // when next() answered and kept 0 otherwise; an aggregate always answers
    // one row, so the int is returned directly.
    virtual int countPersonalDonations(const std::string& name, int worldID) = 0;
    virtual int countGuildDonations(const std::string& name, int worldID) = 0;
    // The two INSERTs are POSITIONAL — they name no columns, so they depend
    // on each table's column order: (PlayerID, Name, WorldID, Amount,
    // DonationDateTime) and (GuildID, GuildName, PlayerID, Name, WorldID,
    // Amount, DonationDateTime), the date now(). The gold is the packet's
    // Gold_t through "%u", the guild id the character's GuildID_t through
    // "%u", the world id the handler's int through "%d" — as written.
    virtual void insertPersonalDonation(const std::string& playerID, const std::string& name, int worldID,
                                        Gold_t gold) = 0;
    virtual void insertGuildDonation(GuildID_t guildID, const std::string& guildName, const std::string& playerID,
                                     const std::string& name, int worldID, Gold_t gold) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLComebackEventRepository.cpp. An accessor function rather than a
// g_p* extern: ratchet R1 counts those.
ComebackEventRepository& defaultComebackEventRepository();

#endif
