//////////////////////////////////////////////////////////////////////////////
// Filename : ExchangeService.h
// Written by : Exchange System
// Description : Core business logic service for Exchange System
//////////////////////////////////////////////////////////////////////////////

#ifndef __EXCHANGE_SERVICE_H__
#define __EXCHANGE_SERVICE_H__

#include <memory>
#include <string>
#include <vector>

#include "ExchangeDecision.h"
#include "ExchangePurchase.h"
#include "Outcome.h"
#include "repository/ExchangeRepository.h"

using namespace std;

class PlayerCreature;
class Item;

//////////////////////////////////////////////////////////////////////////////
// Exchange Claim Info
//////////////////////////////////////////////////////////////////////////////

struct ExchangeClaim {
    int64_t id;      // OrderID for buyer, ListingID for seller
    string itemName; // Item name for display
    int pricePoint;
    uint8_t type;   // 0=buyer claim, 1=seller claim
    uint8_t status; // Order status or Listing status
};

//////////////////////////////////////////////////////////////////////////////
// What the mutating operations produce on the success side
//////////////////////////////////////////////////////////////////////////////

// The listing row a create wrote.
struct ExchangeListingCreated {
    int64_t listingID = 0;
};

// The ledger balance a point adjustment left behind.
struct ExchangePointsAdjusted {
    int balanceAfter = 0;
};

//////////////////////////////////////////////////////////////////////////////
// Exchange Service
//////////////////////////////////////////////////////////////////////////////

class ExchangeService {
public:
    // Status constants
    static const uint8_t LISTING_STATUS_ACTIVE = 0;
    static const uint8_t LISTING_STATUS_SOLD = 1;
    static const uint8_t LISTING_STATUS_CANCELLED = 2;
    static const uint8_t LISTING_STATUS_EXPIRED = 3;

    static const uint8_t ORDER_STATUS_PAID = 0;
    static const uint8_t ORDER_STATUS_DELIVERED = 1;
    static const uint8_t ORDER_STATUS_CANCELLED = 2;

    // Point ledger reasons
    static const uint8_t POINT_REASON_BUY = 0;
    static const uint8_t POINT_REASON_SALE = 1;
    static const uint8_t POINT_REASON_TAX = 2;
    static const uint8_t POINT_REASON_REFUND = 3;
    static const uint8_t POINT_REASON_ADJUST = 4;

    ////////////////////////////////////////////////////////////////////
    // Browse operations
    ////////////////////////////////////////////////////////////////////

    // The server id every game server lists into and browses: one market,
    // whatever this game server's configured ServerID is.
    static int16_t getMarketServerID();

    // One page of active listings, narrowed by the filter
    static vector<ExchangeListing> getListings(int16_t serverID, int page = 1, int pageSize = 20,
                                               const ExchangeListingFilter& filter = ExchangeListingFilter());

    // Get total count matching filters
    static int getListingsCount(int16_t serverID, const ExchangeListingFilter& filter = ExchangeListingFilter());

    // Get specific listing, or NULL when the id names none
    static unique_ptr<ExchangeListing> getListing(int64_t listingID);

    ////////////////////////////////////////////////////////////////////
    // Listing operations
    ////////////////////////////////////////////////////////////////////

    // Create a new listing: the seller's item moves to exchange storage and
    // the new listing id comes back.
    [[nodiscard]] static Outcome<ExchangeListingCreated, ExchangeRejection>
    createListing(PlayerCreature* pSeller, Item* pItem, int pricePoint, int durationHours = 72);

    // Withdraw one of the seller's own active listings. The item stays in
    // exchange storage until the seller claims it back.
    [[nodiscard]] static Outcome<void, ExchangeRejection> cancelListing(PlayerCreature* pSeller, int64_t listingID);

    // Get seller's listings
    static vector<ExchangeListing> getSellerListings(const string& sellerAccount,
                                                     uint8_t status = LISTING_STATUS_ACTIVE);

    ////////////////////////////////////////////////////////////////////
    // Buying operations
    ////////////////////////////////////////////////////////////////////

    // Buy a listing: the listing is marked sold, the order is written and
    // points move both ways, all inside one transaction pair
    // (completeExchangePurchase). A refusal or a failed write comes back as
    // a rejection with the pair rolled back; the decision's reads, which run
    // before the pair opens, still throw their DatabaseError.
    [[nodiscard]] static Outcome<ExchangePurchase, ExchangeRejection>
    buyListing(PlayerCreature* pBuyer, int64_t listingID, const string& idempotencyKey);

    // Get buyer's orders
    static vector<ExchangeOrder> getBuyerOrders(const string& buyerPlayer, uint8_t status = ORDER_STATUS_PAID);

    // Get seller's fulfilled orders
    static vector<ExchangeOrder> getSellerOrders(const string& sellerPlayer, uint8_t status = ORDER_STATUS_DELIVERED);

    ////////////////////////////////////////////////////////////////////
    // Claim operations
    ////////////////////////////////////////////////////////////////////

    // Prepare claim list for a player (both buyer and seller items)
    static vector<ExchangeClaim> prepareClaimList(PlayerCreature* pPlayer);

    // Claim item (for buyer: deliver order, for seller: return cancelled item).
    // Ok means the claim was permitted; the item transfer itself is still
    // unimplemented on both branches.
    [[nodiscard]] static Outcome<void, ExchangeRejection> claimItem(PlayerCreature* pPlayer, int64_t orderOrListingID,
                                                                    bool isBuyerClaim);

    ////////////////////////////////////////////////////////////////////
    // Point operations
    ////////////////////////////////////////////////////////////////////

    // Get point balance
    static int getPointBalance(const string& account);

    // Adjust points with ledger record. The repository refuses a duplicate
    // idempotency key and a balance that would go below zero with the same
    // answer, so the rejection cannot name which of the two it was.
    [[nodiscard]] static Outcome<ExchangePointsAdjusted, ExchangeRejection>
    adjustPoints(const string& account, int delta, uint8_t reason, int64_t refListingID = 0, int64_t refOrderID = 0,
                 const string& idempotencyKey = "");

    ////////////////////////////////////////////////////////////////////
    // Maintenance operations
    ////////////////////////////////////////////////////////////////////

    // Scan and expire listings
    static void scanExpiredListings();

    ////////////////////////////////////////////////////////////////////
    // Configuration
    ////////////////////////////////////////////////////////////////////

    static void setTaxRate(uint8_t rate) {
        m_TaxRate = rate;
    }
    static uint8_t getTaxRate() {
        return m_TaxRate;
    }

    static void setListingDuration(int days) {
        m_ListingDurationDays = days;
    }
    static int getListingDuration() {
        return m_ListingDurationDays;
    }

private:
    ////////////////////////////////////////////////////////////////////
    // Helper methods
    ////////////////////////////////////////////////////////////////////

    // Get current timestamp string
    static string getCurrentTimestamp();

    // Move item to exchange storage
    static bool moveItemToExchangeStorage(PlayerCreature* pPlayer, Item* pItem);

    // Move item from exchange storage to player
    static bool moveItemFromExchangeStorage(PlayerCreature* pPlayer, int64_t listingID, Item* pItem);

    // Create item snapshot for UI display
    static void createItemSnapshot(Item* pItem, ExchangeListing& listing);

    // Check if inventory has space
    static bool checkInventorySpace(PlayerCreature* pPlayer);

    // Static configuration
    static uint8_t m_TaxRate;
    static int m_ListingDurationDays;
};

#endif // __EXCHANGE_SERVICE_H__
