//////////////////////////////////////////////////////////////////////////////
// Filename    : ExchangeDecision.cpp
// Description : the Exchange system's rejection reasons and the decisions
//               that need nothing but a repository and plain values.
//////////////////////////////////////////////////////////////////////////////

#include "ExchangeDecision.h"

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <memory>
#include <vector>

#include "CGExchangeList.h"
#include "GCExchangeList.h" // For ExchangeListing definition

const char* const kExchangeBuyLedgerSuffix = "_buy";
const char* const kExchangeSaleLedgerSuffix = "_sale";
const char* const kExchangeServerKeyPrefix = "EX_";

std::string formatExchangeError(ExchangeResult code, const std::string& detail) {
    std::string error;

    switch (code) {
    case EXCHANGE_SUCCESS:
        return "Success";

    case EXCHANGE_FAIL_ITEM_NOT_FOUND:
        error = "Item not found";
        break;
    case EXCHANGE_FAIL_ITEM_OWNERSHIP:
        error = "You don't own this item";
        break;
    case EXCHANGE_FAIL_ITEM_TRADEABLE:
        error = "This item cannot be traded";
        break;
    case EXCHANGE_FAIL_INVALID_PRICE:
        error = "Invalid price";
        break;
    case EXCHANGE_FAIL_INSUFFICIENT_POINTS:
        error = "Insufficient point balance";
        break;
    case EXCHANGE_FAIL_LISTING_NOT_FOUND:
        error = "Listing not found";
        break;
    case EXCHANGE_FAIL_LISTING_NOT_AVAILABLE:
        error = "Listing is no longer available";
        break;
    case EXCHANGE_FAIL_INVENTORY_FULL:
        error = "Inventory is full";
        break;
    case EXCHANGE_FAIL_STORAGE_FULL:
        error = "Exchange storage is full";
        break;
    case EXCHANGE_FAIL_NOT_SELLER:
        error = "You are not the seller of this item";
        break;
    case EXCHANGE_FAIL_NOT_BUYER:
        error = "You are not the buyer of this item";
        break;
    case EXCHANGE_FAIL_ALREADY_CLAIMED:
        error = "Item already claimed";
        break;
    case EXCHANGE_FAIL_DATABASE_ERROR:
        error = "Database error";
        break;
    case EXCHANGE_FAIL_TRANSACTION_ERROR:
        error = "Transaction error";
        break;
    case EXCHANGE_FAIL_IDEMPOTENCY_CONFLICT:
        error = "Duplicate transaction";
        break;
    default:
        error = "Unknown error";
        break;
    }

    if (!detail.empty()) {
        error += ": " + detail;
    }

    return error;
}

int calculateExchangeTax(int price, uint8_t taxRate) {
    return (price * taxRate) / 100;
}

Outcome<void, ExchangeRejection> validateListingPrice(int pricePoint) {
    if (pricePoint < 1)
        return Outcome<void, ExchangeRejection>::Rejected(ExchangeRejection(EXCHANGE_FAIL_INVALID_PRICE));

    return Outcome<void, ExchangeRejection>::Ok();
}

ExchangeListingFilter exchangeListingFilterOf(const CGExchangeList& packet) {
    ExchangeListingFilter filter;
    filter.itemClass = packet.getItemClass();
    filter.itemType = packet.getItemType();
    filter.minPrice = packet.getMinPrice();
    filter.maxPrice = packet.getMaxPrice();
    filter.sellerFilter = packet.getSellerFilter();
    return filter;
}

bool matchesExchangeListingFilter(const ExchangeListing& listing, const ExchangeListingFilter& filter) {
    if (filter.itemClass != 0xFF && listing.itemClass != filter.itemClass)
        return false;
    if (filter.itemType != 0xFFFF && listing.itemType != filter.itemType)
        return false;
    if (filter.minPrice > 0 && listing.pricePoint < filter.minPrice)
        return false;
    if (filter.maxPrice > 0 && listing.pricePoint > filter.maxPrice)
        return false;
    if (!filter.sellerFilter.empty() && listing.sellerPlayer.find(filter.sellerFilter) == std::string::npos)
        return false;
    return true;
}

std::string exchangeLedgerKey(const std::string& base, const std::string& suffix) {
    const size_t room =
        (suffix.length() < kMaxExchangeIdempotencyKeyLength) ? kMaxExchangeIdempotencyKeyLength - suffix.length() : 0;
    return base.substr(0, std::min(base.length(), room)) + suffix;
}

std::string exchangeServerIdempotencyKey(int worldID, int serverID, int64_t listingID) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%s%02x%02x%016llx", kExchangeServerKeyPrefix, (unsigned int)(worldID & 0xFF),
             (unsigned int)(serverID & 0xFF), (unsigned long long)listingID);
    return std::string(buf);
}

std::string resolveExchangeIdempotencyKey(const std::string& clientKey, int worldID, int serverID, int64_t listingID) {
    if (clientKey.empty() || clientKey.compare(0, strlen(kExchangeServerKeyPrefix), kExchangeServerKeyPrefix) == 0)
        return exchangeServerIdempotencyKey(worldID, serverID, listingID);
    return clientKey;
}

Outcome<ExchangePurchaseTerms, ExchangeRejection> decideBuyListing(ExchangeRepository& repository,
                                                                   const ExchangeBuyRequest& request) {
    typedef Outcome<ExchangePurchaseTerms, ExchangeRejection> Result;

    // A key whose buyer row the ledger already holds is a replay of a
    // purchase that went through; refuse it before reading anything else.
    // The buyer row is looked up under the key buyListing writes it with.
    if (!request.idempotencyKey.empty() &&
        repository.hasIdempotencyKey(exchangeLedgerKey(request.idempotencyKey, kExchangeBuyLedgerSuffix)))
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_IDEMPOTENCY_CONFLICT));

    // getListing hands out a listing the caller owns.
    std::unique_ptr<ExchangeListing> pListing(repository.getListing(request.listingID));
    if (!pListing)
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_LISTING_NOT_FOUND));

    if (pListing->status != LISTING_STATUS_ACTIVE)
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_LISTING_NOT_AVAILABLE));

    // A listing of another server is not this buyer's to take.
    if (pListing->serverID != request.serverID)
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_LISTING_NOT_AVAILABLE));

    // Buying one's own listing would move points in a circle and pay tax
    // twice for nothing.
    if (pListing->sellerPlayer == request.buyerPlayer)
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_LISTING_NOT_AVAILABLE));

    ExchangePurchaseTerms terms;
    terms.listingID = request.listingID;
    terms.sellerAccount = pListing->sellerAccount;
    terms.pricePoint = pListing->pricePoint;
    terms.taxAmount = calculateExchangeTax(terms.pricePoint, request.taxRate);
    terms.totalCost = terms.pricePoint + terms.taxAmount;
    terms.sellerIncome = terms.pricePoint - terms.taxAmount;
    terms.buyerBalance = repository.getPointBalance(request.buyerAccount);

    if (terms.buyerBalance < terms.totalCost)
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_INSUFFICIENT_POINTS));

    return Result::Ok(terms);
}

Outcome<void, ExchangeRejection> decideCancelListing(ExchangeRepository& repository, const std::string& sellerPlayer,
                                                     int64_t listingID) {
    typedef Outcome<void, ExchangeRejection> Result;

    std::unique_ptr<ExchangeListing> pListing(repository.getListing(listingID));
    if (!pListing)
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_LISTING_NOT_FOUND));

    if (pListing->sellerPlayer != sellerPlayer)
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_NOT_SELLER));

    if (pListing->status != LISTING_STATUS_ACTIVE)
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_LISTING_NOT_AVAILABLE));

    return Result::Ok();
}

Outcome<ExchangeBuyerClaim, ExchangeRejection> decideBuyerClaim(ExchangeRepository& repository,
                                                                const std::string& buyerPlayer, int64_t orderID) {
    typedef Outcome<ExchangeBuyerClaim, ExchangeRejection> Result;

    // The order has to be one of this buyer's own paid orders: that is both
    // the ownership check and the status check.
    std::vector<ExchangeOrder> orders = repository.getBuyerOrders(buyerPlayer, ORDER_STATUS_PAID);
    const ExchangeOrder* pTargetOrder = NULL;
    for (const auto& order : orders) {
        if (order.orderID == orderID) {
            pTargetOrder = &order;
            break;
        }
    }

    if (!pTargetOrder)
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_LISTING_NOT_FOUND));

    std::unique_ptr<ExchangeListing> pListing(repository.getListing(pTargetOrder->listingID));
    if (!pListing)
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_LISTING_NOT_FOUND));

    ExchangeBuyerClaim claim;
    claim.orderID = orderID;
    claim.listingID = pTargetOrder->listingID;
    return Result::Ok(claim);
}

Outcome<void, ExchangeRejection> decideSellerClaim(ExchangeRepository& repository, const std::string& sellerPlayer,
                                                   int64_t listingID) {
    typedef Outcome<void, ExchangeRejection> Result;

    std::unique_ptr<ExchangeListing> pListing(repository.getListing(listingID));
    if (!pListing)
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_LISTING_NOT_FOUND));

    if (pListing->sellerPlayer != sellerPlayer)
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_NOT_SELLER));

    if (pListing->status != LISTING_STATUS_CANCELLED && pListing->status != LISTING_STATUS_EXPIRED)
        return Result::Rejected(ExchangeRejection(EXCHANGE_FAIL_LISTING_NOT_AVAILABLE));

    return Result::Ok();
}
