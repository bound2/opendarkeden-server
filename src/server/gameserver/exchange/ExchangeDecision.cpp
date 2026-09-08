//////////////////////////////////////////////////////////////////////////////
// Filename    : ExchangeDecision.cpp
// Description : the Exchange system's rejection reasons and the decisions
//               that need nothing but a repository and plain values.
//////////////////////////////////////////////////////////////////////////////

#include "ExchangeDecision.h"

#include <memory>
#include <vector>

#include "GCExchangeList.h" // For ExchangeListing definition

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

Outcome<ExchangePurchaseTerms, ExchangeRejection> decideBuyListing(ExchangeRepository& repository,
                                                                   const ExchangeBuyRequest& request) {
    typedef Outcome<ExchangePurchaseTerms, ExchangeRejection> Result;

    // A client-supplied key that the ledger has already seen is a replay of
    // a purchase that went through; refuse it before reading anything else.
    if (!request.idempotencyKey.empty() && repository.hasIdempotencyKey(request.idempotencyKey))
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
