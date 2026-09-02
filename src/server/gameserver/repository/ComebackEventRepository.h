#ifndef __COMEBACK_EVENT_REPOSITORY_H__
#define __COMEBACK_EVENT_REPOSITORY_H__

#include <string>

// Read-only seam for the 2005 "comeback" event tables (task 3.2, the
// Zone milestone): Event200501Main and Event200501Recommend, keyed by
// the ACCOUNT id (PlayerID), not the character name. The zone asks the
// three questions below when a character enters a zone, to nag the
// player about unclaimed event items; CGGetEventItemHandler (gameserver)
// and CLLoginHandler (loginserver) read and stamp the same tables with
// their own inline SQL — their own extractions.
//
// These reads go through the thread's "dist" connection — the second
// connection every zone thread registers, same server and DARKEDEN
// schema as the world connection in the shipped stack (see
// MySQLGoodsRepository.cpp).
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
};

// The process-wide MySQL-backed instance, wired in
// MySQLComebackEventRepository.cpp. An accessor function rather than a
// g_p* extern: ratchet R1 counts those.
ComebackEventRepository& defaultComebackEventRepository();

#endif
