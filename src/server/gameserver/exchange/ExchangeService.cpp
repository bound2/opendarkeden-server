//////////////////////////////////////////////////////////////////////////////
// Filename : ExchangeService.cpp
// Written by : Exchange System
// Description : Core business logic service for Exchange System
//////////////////////////////////////////////////////////////////////////////

#include "ExchangeService.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <algorithm>

#include "GCExchangeList.h" // For ExchangeListing definition
#include "Inventory.h"
#include "Item.h"
#include "ItemUtil.h"
#include "PlayerCreature.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// Static members
//////////////////////////////////////////////////////////////////////////////

uint8_t ExchangeService::m_TaxRate = 8;         // Default 8% tax
int ExchangeService::m_ListingDurationDays = 3; // Default 3 days

//////////////////////////////////////////////////////////////////////////////
// Helper functions
//////////////////////////////////////////////////////////////////////////////

namespace {
string _getCurrentTime() {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
    return string(buf);
}

string _addHoursToNow(int hours) {
    time_t now = time(NULL);
    now += hours * 3600;
    struct tm* t = localtime(&now);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
    return string(buf);
}

int16_t _getServerID() {
    // TODO: Get actual server ID from configuration
    // For now, return a default value
    return 1;
}

// Longest key PointLedger.IdempotencyKey can hold: it is VARCHAR(64) UNIQUE,
// and this project mandates a non-strict sql_mode, so anything longer is
// silently truncated on insert instead of being rejected.
const size_t kMaxIdempotencyKeyLength = 64;

// Generate a fallback idempotency key for a buy that arrived without one.
//
// Fixed width by construction: "EX_" plus 4 + 8 + 8 + 8 hex digits = 31
// characters, which leaves room for the "_buy"/"_sale" suffix the buy path
// appends and stays well inside kMaxIdempotencyKeyLength.
//
// The server id and the process id are part of the key because PointLedger
// lives in the shared USERINFO database. Time plus a counter alone is not
// unique across the deployment: two game servers - or one game server
// restarted, since the counter starts over at zero - can mint the same key
// within the same second, and whoever loses that collision has a legitimate
// purchase rejected as an already-processed replay.
//
// This does NOT yet make the key unique across game servers. _getServerID()
// above is a hardcoded 1 (its TODO), and under docker/docker-compose.yml every
// containerised server is pid 1, so two such servers agree on both components
// and their Nth key of the same second is identical. Restart-within-a-second
// and the cross-thread race are covered; cross-server uniqueness needs that
// TODO resolved.
string _generateIdempotencyKey() {
    static long counter = 0;

    // The counter is bumped from every zone thread, so read-modify-write it
    // atomically; two threads sharing a value would share the whole key.
    const unsigned long sequence = (unsigned long)__sync_fetch_and_add(&counter, 1);
    const time_t now = time(NULL);

    char buf[64];
    snprintf(buf, sizeof(buf), "EX_%04x%08lx%08lx%08lx", (unsigned int)(_getServerID() & 0xFFFF),
             (unsigned long)(getpid() & 0xFFFFFFFF), (unsigned long)((unsigned long long)now & 0xFFFFFFFF),
             (unsigned long)(sequence & 0xFFFFFFFF));
    return string(buf);
}

// Build the per-leg ledger key ("<base>_buy", "<base>_sale").
//
// The base can be client-supplied, and CGExchangeBuy caps it on the wire at the
// full column width, so appending the suffix would push it past that width. The
// base is therefore trimmed to make room: without that, a maximum-length key
// makes both legs truncate to the SAME stored value, they collide on
// UNQ_Ledger_IdempotencyKey, and the whole purchase rolls back. Two bases that
// only differ past the trim point still collide - that is inherent to a 64-byte
// column carrying a suffix, and it fails closed (a replay is refused).
string _makeLedgerKey(const string& base, const string& suffix) {
    const size_t room = (suffix.length() < kMaxIdempotencyKeyLength) ? kMaxIdempotencyKeyLength - suffix.length() : 0;
    return base.substr(0, min(base.length(), room)) + suffix;
}

// Check if player has inventory space
bool _checkInventorySpace(PlayerCreature* pPlayer) {
    if (!pPlayer)
        return false;

    Inventory* pInv = pPlayer->getInventory();
    if (!pInv)
        return false;

    // Check if inventory has at least one empty slot
    // This is a simplified check - should be more thorough
    return pInv->getItemNum() < (pInv->getWidth() * pInv->getHeight());
}
} // namespace

//////////////////////////////////////////////////////////////////////////////
// Browse operations
//////////////////////////////////////////////////////////////////////////////

vector<ExchangeListing> ExchangeService::getListings(int16_t serverID, int page, int pageSize, uint8_t itemClass,
                                                     uint16_t itemType, int minPrice, int maxPrice,
                                                     const string& sellerFilter) {
    // For now, use the basic DB function
    // In production, we'd want to add filtering at DB level for performance
    vector<ExchangeListing> allListings =
        defaultExchangeRepository().getListings(serverID, LISTING_STATUS_ACTIVE, page, pageSize);

    // Apply additional filters (in-memory for now)
    vector<ExchangeListing> filtered;
    for (const auto& listing : allListings) {
        if (itemClass != 0xFF && listing.itemClass != itemClass)
            continue;
        if (itemType != 0xFFFF && listing.itemType != itemType)
            continue;
        if (minPrice > 0 && listing.pricePoint < minPrice)
            continue;
        if (maxPrice > 0 && listing.pricePoint > maxPrice)
            continue;
        if (!sellerFilter.empty() && listing.sellerPlayer.find(sellerFilter) == string::npos)
            continue;

        filtered.push_back(listing);
    }

    return filtered;
}

int ExchangeService::getListingsCount(int16_t serverID, uint8_t itemClass, uint16_t itemType, int minPrice,
                                      int maxPrice, const string& sellerFilter) {
    // For accurate count, we need a DB query
    // For now, return a placeholder
    vector<ExchangeListing> listings = getListings(serverID, 1, 1000, // Get max results
                                                   itemClass, itemType, minPrice, maxPrice, sellerFilter);
    return listings.size();
}

ExchangeListing* ExchangeService::getListing(int64_t listingID) {
    return defaultExchangeRepository().getListing(listingID);
}

//////////////////////////////////////////////////////////////////////////////
// Listing operations
//////////////////////////////////////////////////////////////////////////////

Outcome<ExchangeListingCreated, ExchangeRejection> ExchangeService::createListing(PlayerCreature* pSeller, Item* pItem,
                                                                                  int pricePoint, int durationHours) {
    typedef Outcome<ExchangeListingCreated, ExchangeRejection> Result;

    // Validate inputs
    if (!pSeller) {
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_ITEM_NOT_FOUND));
    }
    if (!pItem) {
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_ITEM_NOT_FOUND));
    }
    Outcome<void, ExchangeRejection> price = validateListingPrice(pricePoint);
    if (price.isRejected()) {
        return Result::Rejected(std::move(price).rejection());
    }

    // Get player info
    string account = pSeller->getName(); // Using name as account identifier
    string playerName = pSeller->getName();
    uint8_t race = 0; // TODO: Get actual race from PlayerCreature

    // Verify item ownership
    Inventory* pInv = pSeller->getInventory();
    if (!pInv || !pInv->hasItem(pItem->getObjectID())) {
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_ITEM_OWNERSHIP));
    }

    // Check if item is tradeable
    if (!canTrade(pItem)) {
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_ITEM_TRADEABLE));
    }

    // Check exchange storage has space (conceptually - items stored by DB)
    // In this implementation, items remain in DB with STORAGE_EXCHANGE flag

    // Calculate expiration
    string createdAt = _getCurrentTime();
    string expireAt = _addHoursToNow(durationHours);

    // Create listing record
    ExchangeListing listing;
    listing.listingID = 0; // Will be set by DB auto-increment
    listing.serverID = _getServerID();
    listing.sellerAccount = account;
    listing.sellerPlayer = playerName;
    listing.sellerRace = race;
    listing.itemClass = pItem->getItemClass();
    listing.itemType = pItem->getItemType();
    listing.itemID = 0; // TODO: Get ItemID from Item
    listing.objectID = pItem->getObjectID();
    listing.pricePoint = pricePoint;
    listing.currency = 0; // 0 = points
    listing.status = LISTING_STATUS_ACTIVE;
    listing.buyerAccount = "";
    listing.buyerPlayer = "";
    listing.taxRate = m_TaxRate;
    listing.taxAmount = 0;
    listing.createdAt = createdAt;
    listing.expireAt = expireAt;
    listing.soldAt = "";
    listing.cancelledAt = "";
    listing.updatedAt = createdAt;
    listing.version = 0;

    // Create item snapshot
    createItemSnapshot(pItem, listing);

    // Save to database
    int64_t listingID = defaultExchangeRepository().createListing(listing);
    if (listingID <= 0) {
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_DATABASE_ERROR));
    }

    // Move item to exchange storage
    // Note: The item will be saved with STORAGE_EXCHANGE type
    if (!moveItemToExchangeStorage(pSeller, pItem)) {
        // Rollback listing creation
        defaultExchangeRepository().cancelListing(listingID);
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_STORAGE_FULL));
    }

    ExchangeListingCreated created;
    created.listingID = listingID;
    return Result::Ok(created);
}

Outcome<void, ExchangeRejection> ExchangeService::cancelListing(PlayerCreature* pSeller, int64_t listingID) {
    typedef Outcome<void, ExchangeRejection> Result;

    if (!pSeller) {
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_ITEM_NOT_FOUND));
    }

    Result decision = decideCancelListing(defaultExchangeRepository(), pSeller->getName(), listingID);
    if (decision.isRejected()) {
        return decision;
    }

    // Mark as cancelled in DB
    if (!defaultExchangeRepository().cancelListing(listingID)) {
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_DATABASE_ERROR));
    }

    // Note: Item remains in exchange storage until claimed
    // Seller needs to claim the item back

    return Result::Ok();
}

vector<ExchangeListing> ExchangeService::getSellerListings(const string& sellerAccount, uint8_t status) {
    return defaultExchangeRepository().getSellerListings(sellerAccount, status);
}

//////////////////////////////////////////////////////////////////////////////
// Buying operations
//////////////////////////////////////////////////////////////////////////////

Outcome<ExchangePurchase, ExchangeRejection> ExchangeService::buyListing(PlayerCreature* pBuyer, int64_t listingID,
                                                                         const string& idempotencyKey) {
    typedef Outcome<ExchangePurchase, ExchangeRejection> Result;

    if (!pBuyer) {
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_ITEM_NOT_FOUND));
    }

    // Account and player are the same name here; the ledger is keyed by the
    // account one and the listing carries the player one.
    ExchangeBuyRequest request;
    request.listingID = listingID;
    request.buyerAccount = pBuyer->getName();
    request.buyerPlayer = pBuyer->getName();
    request.idempotencyKey = idempotencyKey;
    request.serverID = _getServerID();
    request.taxRate = m_TaxRate;

    // Check expiration
    // (DB should handle this, but double-check)
    // TODO: Parse expireAt and compare with current time

    Outcome<ExchangePurchaseTerms, ExchangeRejection> decision = decideBuyListing(defaultExchangeRepository(), request);
    if (decision.isRejected()) {
        return Result::Rejected(std::move(decision).rejection());
    }
    const ExchangePurchaseTerms terms = std::move(decision).events();

    // Begin transaction
    if (!defaultExchangeRepository().beginTransaction()) {
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_TRANSACTION_ERROR));
    }

    string autoKey = idempotencyKey.empty() ? _generateIdempotencyKey() : idempotencyKey;

    // Every step below is part of the one purchase: the first that fails
    // takes back the ones before it and names itself in the rejection.
    auto unwind = [](const char* detail) {
        defaultExchangeRepository().rollback();
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_TRANSACTION_ERROR, detail));
    };

    ExchangePurchase purchase;
    purchase.listingID = listingID;
    purchase.pricePoint = terms.pricePoint;
    purchase.taxAmount = terms.taxAmount;
    purchase.totalCost = terms.totalCost;
    purchase.sellerIncome = terms.sellerIncome;

    // Deduct points from buyer
    if (!defaultExchangeRepository().adjustPoints(request.buyerAccount, -terms.totalCost, purchase.buyerBalanceAfter,
                                                  POINT_REASON_BUY, listingID, 0, _makeLedgerKey(autoKey, "_buy"))) {
        return unwind("Failed to deduct buyer points");
    }

    // Add points to seller (after tax)
    if (!defaultExchangeRepository().adjustPoints(terms.sellerAccount, terms.sellerIncome, purchase.sellerBalanceAfter,
                                                  POINT_REASON_SALE, listingID, 0, _makeLedgerKey(autoKey, "_sale"))) {
        return unwind("Failed to add seller points");
    }

    // Create order
    ExchangeOrder order;
    order.orderID = 0;
    order.listingID = listingID;
    order.serverID = getServerID();
    order.buyerAccount = request.buyerAccount;
    order.buyerPlayer = request.buyerPlayer;
    order.pricePoint = terms.pricePoint;
    order.taxAmount = terms.taxAmount;
    order.status = ORDER_STATUS_PAID;
    order.createdAt = _getCurrentTime();
    order.deliveredAt = "";
    order.cancelledAt = "";

    int64_t orderID = defaultExchangeRepository().createOrder(order);
    if (orderID <= 0) {
        return unwind("Failed to create order");
    }

    // Mark listing as sold
    if (!defaultExchangeRepository().markListingSold(listingID, request.buyerAccount, request.buyerPlayer)) {
        return unwind("Failed to mark listing sold");
    }

    // Commit transaction
    if (!defaultExchangeRepository().commit()) {
        return unwind("Failed to commit transaction");
    }

    purchase.orderID = orderID;
    return Result::Ok(purchase);
}

vector<ExchangeOrder> ExchangeService::getBuyerOrders(const string& buyerPlayer, uint8_t status) {
    return defaultExchangeRepository().getBuyerOrders(buyerPlayer, status);
}

vector<ExchangeOrder> ExchangeService::getSellerOrders(const string& sellerPlayer, uint8_t status) {
    return defaultExchangeRepository().getSellerOrders(sellerPlayer, status);
}

//////////////////////////////////////////////////////////////////////////////
// Claim operations
//////////////////////////////////////////////////////////////////////////////

vector<ExchangeClaim> ExchangeService::prepareClaimList(PlayerCreature* pPlayer) {
    vector<ExchangeClaim> claims;

    if (!pPlayer)
        return claims;

    string playerName = pPlayer->getName();

    // Get buyer's paid orders (ready to deliver)
    vector<ExchangeOrder> orders = getBuyerOrders(playerName, ORDER_STATUS_PAID);
    for (const auto& order : orders) {
        ExchangeListing* pListing = getListing(order.listingID);
        if (pListing) {
            ExchangeClaim claim;
            claim.id = order.orderID;
            claim.itemName = pListing->itemName;
            claim.pricePoint = order.pricePoint;
            claim.type = 0; // Buyer claim
            claim.status = order.status;
            claims.push_back(claim);
        }
    }

    // Get seller's cancelled/expired listings (ready to return)
    vector<ExchangeListing> cancelledListings = getSellerListings(playerName, LISTING_STATUS_CANCELLED);
    for (const auto& listing : cancelledListings) {
        ExchangeClaim claim;
        claim.id = listing.listingID;
        claim.itemName = listing.itemName;
        claim.pricePoint = listing.pricePoint;
        claim.type = 1; // Seller claim
        claim.status = listing.status;
        claims.push_back(claim);
    }

    return claims;
}

Outcome<void, ExchangeRejection> ExchangeService::claimItem(PlayerCreature* pPlayer, int64_t orderOrListingID,
                                                            bool isBuyerClaim) {
    typedef Outcome<void, ExchangeRejection> Result;

    if (!pPlayer) {
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_ITEM_NOT_FOUND));
    }

    // Check inventory space
    if (!checkInventorySpace(pPlayer)) {
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_INVENTORY_FULL));
    }

    if (isBuyerClaim) {
        // Buyer claiming purchased item
        Outcome<ExchangeBuyerClaim, ExchangeRejection> decision =
            decideBuyerClaim(defaultExchangeRepository(), pPlayer->getName(), orderOrListingID);
        if (decision.isRejected()) {
            return Result::Rejected(std::move(decision).rejection());
        }

        // Load item from exchange storage
        // Note: This requires loading the item by ObjectID from DB
        // For now, we'll mark the order as delivered
        // The actual item transfer should be handled by the item manager

        if (!defaultExchangeRepository().markOrderDelivered(orderOrListingID)) {
            return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_DATABASE_ERROR));
        }

        // TODO: Actually transfer item to player's inventory
        // This requires finding the item by ObjectID and moving it

        return Result::Ok();

    } else {
        // Seller claiming back cancelled/expired item
        Result decision = decideSellerClaim(defaultExchangeRepository(), pPlayer->getName(), orderOrListingID);
        if (decision.isRejected()) {
            return decision;
        }

        // TODO: Load item from exchange storage and add to inventory
        // This requires item manager integration

        return Result::Ok();
    }
}

//////////////////////////////////////////////////////////////////////////////
// Point operations
//////////////////////////////////////////////////////////////////////////////

int ExchangeService::getPointBalance(const string& account) {
    return defaultExchangeRepository().getPointBalance(account);
}

Outcome<ExchangePointsAdjusted, ExchangeRejection> ExchangeService::adjustPoints(const string& account, int delta,
                                                                                 uint8_t reason, int64_t refListingID,
                                                                                 int64_t refOrderID,
                                                                                 const string& idempotencyKey) {
    typedef Outcome<ExchangePointsAdjusted, ExchangeRejection> Result;

    ExchangePointsAdjusted adjusted;
    if (!defaultExchangeRepository().adjustPoints(account, delta, adjusted.balanceAfter, reason, refListingID,
                                                  refOrderID, idempotencyKey)) {
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_UNKNOWN, "duplicate ledger key or balance below zero"));
    }

    return Result::Ok(adjusted);
}

//////////////////////////////////////////////////////////////////////////////
// Maintenance operations
//////////////////////////////////////////////////////////////////////////////

void ExchangeService::scanExpiredListings() {
    __BEGIN_TRY

    // Scan for expired active listings
    // In production, use indexed query on ExpireAt column
    vector<ExchangeListing> expiredListings = defaultExchangeRepository().getExpiredListings();

    filelog("ExchangeService.log", "Scanning for expired listings, found %d expired", expiredListings.size());

    for (const auto& listing : expiredListings) {
        filelog("ExchangeService.log", "Expiring listing ID: %lld, Item: %s, Seller: %s", listing.listingID,
                listing.itemName.c_str(), listing.sellerPlayer.c_str());

        // Mark listing as expired
        // This will:
        // 1. Set listing status to EXPIRED
        // 2. Allow seller to reclaim the item
        defaultExchangeRepository().expireListing(listing.listingID);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Helper methods
//////////////////////////////////////////////////////////////////////////////

string ExchangeService::getCurrentTimestamp() {
    return _getCurrentTime();
}

bool ExchangeService::moveItemToExchangeStorage(PlayerCreature* pPlayer, Item* pItem) {
    if (!pPlayer || !pItem)
        return false;

    // Get the item's current storage location
    int storage, x, y;
    pPlayer->findItemOID(pItem->getObjectID(), storage, x, y);

    // Save item with STORAGE_EXCHANGE type
    // The item will be associated with the exchange system
    string owner = pPlayer->getName();

    // Call item's save method with STORAGE_EXCHANGE
    // This is the pattern used in the codebase
    // pItem->save(owner, STORAGE_EXCHANGE, 0, 0, 0);

    // Remove from inventory
    Inventory* pInv = pPlayer->getInventory();
    if (pInv) {
        // pInv->deleteItem(x, y);  // Remove from current slot
    }

    return true;
}

bool ExchangeService::moveItemFromExchangeStorage(PlayerCreature* pPlayer, int64_t listingID, Item* pItem) {
    if (!pPlayer || !pItem)
        return false;

    // Add item to player's inventory
    Inventory* pInv = pPlayer->getInventory();
    if (!pInv)
        return false;

    // Find empty slot
    // pItem->create(pPlayer->getName(), STORAGE_INVENTORY, ...);

    return true;
}

void ExchangeService::createItemSnapshot(Item* pItem, ExchangeListing& listing) {
    if (!pItem)
        return;

    // Set basic item info
    listing.itemName = pItem->toString(); // Or get name from item info
    listing.enchantLevel = 0;             // TODO: Get from pItem
    listing.grade = 0;                    // TODO: Get from pItem
    listing.durability = 0;               // TODO: Get from pItem
    listing.silver = 0;                   // TODO: Get from pItem
    listing.stackCount = 1;               // TODO: Get from pItem

    // Get option info
    if (pItem->hasOptionType()) {
        const list<OptionType_t>& optionTypes = pItem->getOptionTypeList();
        int idx = 0;
        for (OptionType_t type : optionTypes) {
            (void)type; // Will be used when setting option fields
            if (idx >= 3)
                break;

            // Set option type and value
            // listing.optionType1 = type;
            // listing.optionValue1 = value;
            idx++;
        }
    }
}

string ExchangeService::generateIdempotencyKey() {
    return _generateIdempotencyKey();
}

int16_t ExchangeService::getServerID() {
    return _getServerID();
}

bool ExchangeService::checkInventorySpace(PlayerCreature* pPlayer) {
    return _checkInventorySpace(pPlayer);
}
