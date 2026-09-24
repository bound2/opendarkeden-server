// The Exchange system's decisions and its rejection texts
// (src/server/gameserver/exchange/ExchangeDecision.cpp): every rejection with
// the input that triggers it, the precedence between them, the terms an
// accepted buy computes, and the English text each result code puts on the
// wire in GCExchangeBuy; the ledger keys a buy is recorded under, and the
// browse filter a CGExchangeList asks for. The repository is a fake, so no
// database is involved; ExchangeService itself is not exercised here because
// it needs a PlayerCreature and an Item.

#include <cstdint>
#include <string>

#include <gtest/gtest.h>

#include "CGExchangeList.h"
#include "ExchangeDecision.h"
#include "FakeExchangeRepository.h"

namespace {

const int16_t kServerID = 1;
const uint8_t kTaxRate = 8;

// An active listing of this server, priced at 100 points, sold by "Seller".
ExchangeListing activeListing(int64_t listingID = 10) {
    ExchangeListing listing;
    listing.listingID = listingID;
    listing.serverID = kServerID;
    listing.sellerAccount = "seller-account";
    listing.sellerPlayer = "Seller";
    listing.pricePoint = 100;
    listing.status = LISTING_STATUS_ACTIVE;
    listing.itemName = "Sword";
    return listing;
}

// A buy of that listing by "Buyer", who has an account of the same name.
ExchangeBuyRequest buyRequest(int64_t listingID = 10) {
    ExchangeBuyRequest request;
    request.listingID = listingID;
    request.buyerAccount = "Buyer";
    request.buyerPlayer = "Buyer";
    request.serverID = kServerID;
    request.taxRate = kTaxRate;
    return request;
}

ExchangeOrder paidOrder(int64_t orderID, int64_t listingID, const std::string& buyerPlayer) {
    ExchangeOrder order;
    order.orderID = orderID;
    order.listingID = listingID;
    order.serverID = kServerID;
    order.buyerAccount = buyerPlayer;
    order.buyerPlayer = buyerPlayer;
    order.pricePoint = 100;
    order.taxAmount = 8;
    order.status = ORDER_STATUS_PAID;
    return order;
}

//////////////////////////////////////////////////////////////////////////////
// The rejection texts. These go on the wire in GCExchangeBuy's message field,
// so the client sees them: they are a contract, not diagnostics.
//////////////////////////////////////////////////////////////////////////////

TEST(ExchangeErrorText, EveryCodeHasItsOwnMessage) {
    EXPECT_EQ("Success", formatExchangeError(EXCHANGE_SUCCESS));
    EXPECT_EQ("Item not found", formatExchangeError(EXCHANGE_FAIL_ITEM_NOT_FOUND));
    EXPECT_EQ("You don't own this item", formatExchangeError(EXCHANGE_FAIL_ITEM_OWNERSHIP));
    EXPECT_EQ("This item cannot be traded", formatExchangeError(EXCHANGE_FAIL_ITEM_TRADEABLE));
    EXPECT_EQ("Invalid price", formatExchangeError(EXCHANGE_FAIL_INVALID_PRICE));
    EXPECT_EQ("Insufficient point balance", formatExchangeError(EXCHANGE_FAIL_INSUFFICIENT_POINTS));
    EXPECT_EQ("Listing not found", formatExchangeError(EXCHANGE_FAIL_LISTING_NOT_FOUND));
    EXPECT_EQ("Listing is no longer available", formatExchangeError(EXCHANGE_FAIL_LISTING_NOT_AVAILABLE));
    EXPECT_EQ("Inventory is full", formatExchangeError(EXCHANGE_FAIL_INVENTORY_FULL));
    EXPECT_EQ("Exchange storage is full", formatExchangeError(EXCHANGE_FAIL_STORAGE_FULL));
    EXPECT_EQ("You are not the seller of this item", formatExchangeError(EXCHANGE_FAIL_NOT_SELLER));
    EXPECT_EQ("You are not the buyer of this item", formatExchangeError(EXCHANGE_FAIL_NOT_BUYER));
    EXPECT_EQ("Item already claimed", formatExchangeError(EXCHANGE_FAIL_ALREADY_CLAIMED));
    EXPECT_EQ("Database error", formatExchangeError(EXCHANGE_FAIL_DATABASE_ERROR));
    EXPECT_EQ("Transaction error", formatExchangeError(EXCHANGE_FAIL_TRANSACTION_ERROR));
    EXPECT_EQ("Duplicate transaction", formatExchangeError(EXCHANGE_FAIL_IDEMPOTENCY_CONFLICT));
    EXPECT_EQ("Unknown error", formatExchangeError(EXCHANGE_FAIL_UNKNOWN));
}

TEST(ExchangeErrorText, DetailIsAppendedAfterAColon) {
    EXPECT_EQ("Transaction error: Failed to create order",
              formatExchangeError(EXCHANGE_FAIL_TRANSACTION_ERROR, "Failed to create order"));
    // An empty detail adds nothing, not even the separator.
    EXPECT_EQ("Transaction error", formatExchangeError(EXCHANGE_FAIL_TRANSACTION_ERROR, ""));
    // Success answers before the detail is looked at.
    EXPECT_EQ("Success", formatExchangeError(EXCHANGE_SUCCESS, "ignored"));
}

TEST(ExchangeRejectionTest, MessageIsTheCodesTextWithItsDetail) {
    EXPECT_EQ("Listing not found", ExchangeRejection(EXCHANGE_FAIL_LISTING_NOT_FOUND).message());
    EXPECT_EQ("Transaction error: Failed to commit transaction",
              ExchangeRejection(EXCHANGE_FAIL_TRANSACTION_ERROR, "Failed to commit transaction").message());
}

//////////////////////////////////////////////////////////////////////////////
// Tax and price
//////////////////////////////////////////////////////////////////////////////

TEST(ExchangeTaxTest, TruncatesTowardZero) {
    EXPECT_EQ(8, calculateExchangeTax(100, 8));
    EXPECT_EQ(0, calculateExchangeTax(12, 8));  // 0.96 truncates to 0
    EXPECT_EQ(1, calculateExchangeTax(13, 8));  // 1.04 truncates to 1
    EXPECT_EQ(0, calculateExchangeTax(100, 0)); // a rate of 0 is a free market
    EXPECT_EQ(0, calculateExchangeTax(0, 8));
}

TEST(ExchangePriceTest, AtLeastOnePoint) {
    EXPECT_TRUE(validateListingPrice(1).isOk());
    EXPECT_TRUE(validateListingPrice(1000).isOk());
    EXPECT_EQ(EXCHANGE_FAIL_INVALID_PRICE, validateListingPrice(0).rejection().code);
    EXPECT_EQ(EXCHANGE_FAIL_INVALID_PRICE, validateListingPrice(-1).rejection().code);
}

//////////////////////////////////////////////////////////////////////////////
// Buying
//////////////////////////////////////////////////////////////////////////////

TEST(ExchangeBuyDecision, AcceptedBuyPricesBothLegs) {
    FakeExchangeRepository repository;
    repository.addListing(activeListing());
    repository.setPointBalance("Buyer", 500);

    auto result = decideBuyListing(repository, buyRequest());

    ASSERT_TRUE(result.isOk());
    const ExchangePurchaseTerms& terms = result.events();
    EXPECT_EQ(10, terms.listingID);
    EXPECT_EQ("seller-account", terms.sellerAccount);
    EXPECT_EQ(100, terms.pricePoint);
    EXPECT_EQ(8, terms.taxAmount);
    // The tax is charged to both sides: the buyer pays it on top of the
    // price and the seller has it withheld.
    EXPECT_EQ(108, terms.totalCost);
    EXPECT_EQ(92, terms.sellerIncome);
    EXPECT_EQ(500, terms.buyerBalance);

    // Deciding writes nothing and opens no transaction.
    EXPECT_EQ(0, repository.transactionsBegun());
    EXPECT_TRUE(repository.ledger().empty());
    EXPECT_EQ(LISTING_STATUS_ACTIVE, repository.listings()[0].status);
}

TEST(ExchangeBuyDecision, BalanceMustCoverPriceAndTax) {
    FakeExchangeRepository repository;
    repository.addListing(activeListing());
    // The price alone is affordable; the price plus its tax is not.
    repository.setPointBalance("Buyer", 107);

    EXPECT_EQ(EXCHANGE_FAIL_INSUFFICIENT_POINTS, decideBuyListing(repository, buyRequest()).rejection().code);

    // Exactly enough is enough.
    repository.setPointBalance("Buyer", 108);
    EXPECT_TRUE(decideBuyListing(repository, buyRequest()).isOk());
}

TEST(ExchangeBuyDecision, AnAccountWithNoLedgerRowHasNoPoints) {
    FakeExchangeRepository repository;
    repository.addListing(activeListing());

    auto result = decideBuyListing(repository, buyRequest());
    ASSERT_TRUE(result.isRejected());
    EXPECT_EQ(EXCHANGE_FAIL_INSUFFICIENT_POINTS, result.rejection().code);
}

TEST(ExchangeBuyDecision, AFreeListingIsAffordableToAnyone) {
    FakeExchangeRepository repository;
    ExchangeListing listing = activeListing();
    listing.pricePoint = 0;
    repository.addListing(listing);

    auto result = decideBuyListing(repository, buyRequest());
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(0, result.events().totalCost);
}

TEST(ExchangeBuyDecision, UnknownListing) {
    FakeExchangeRepository repository;
    repository.setPointBalance("Buyer", 500);

    EXPECT_EQ(EXCHANGE_FAIL_LISTING_NOT_FOUND, decideBuyListing(repository, buyRequest(999)).rejection().code);
}

TEST(ExchangeBuyDecision, ListingMustBeActive) {
    const uint8_t inactive[] = {LISTING_STATUS_SOLD, LISTING_STATUS_CANCELLED, LISTING_STATUS_EXPIRED};
    for (uint8_t status : inactive) {
        FakeExchangeRepository repository;
        ExchangeListing listing = activeListing();
        listing.status = status;
        repository.addListing(listing);
        repository.setPointBalance("Buyer", 500);

        auto result = decideBuyListing(repository, buyRequest());
        ASSERT_TRUE(result.isRejected()) << "status " << (int)status;
        EXPECT_EQ(EXCHANGE_FAIL_LISTING_NOT_AVAILABLE, result.rejection().code) << "status " << (int)status;
    }
}

TEST(ExchangeBuyDecision, ListingOfAnotherServer) {
    FakeExchangeRepository repository;
    ExchangeListing listing = activeListing();
    listing.serverID = kServerID + 1;
    repository.addListing(listing);
    repository.setPointBalance("Buyer", 500);

    EXPECT_EQ(EXCHANGE_FAIL_LISTING_NOT_AVAILABLE, decideBuyListing(repository, buyRequest()).rejection().code);
}

TEST(ExchangeBuyDecision, SellerCannotBuyTheirOwnListing) {
    FakeExchangeRepository repository;
    repository.addListing(activeListing());
    repository.setPointBalance("Seller", 500);

    ExchangeBuyRequest request = buyRequest();
    request.buyerAccount = "seller-account";
    request.buyerPlayer = "Seller";

    EXPECT_EQ(EXCHANGE_FAIL_LISTING_NOT_AVAILABLE, decideBuyListing(repository, request).rejection().code);
}

TEST(ExchangeBuyDecision, AKeyTheLedgerHasSeenIsAReplay) {
    FakeExchangeRepository repository;
    repository.addListing(activeListing());
    repository.setPointBalance("Buyer", 500);

    // The buyer's row, under the key buyListing writes it with.
    int balanceAfter = 0;
    ASSERT_TRUE(repository.adjustPoints("Buyer", -1, balanceAfter, POINT_REASON_BUY, 10, 0,
                                        exchangeLedgerKey("key", kExchangeBuyLedgerSuffix)));

    ExchangeBuyRequest request = buyRequest();
    request.idempotencyKey = "key";

    auto result = decideBuyListing(repository, request);
    ASSERT_TRUE(result.isRejected());
    EXPECT_EQ(EXCHANGE_FAIL_IDEMPOTENCY_CONFLICT, result.rejection().code);
    // The replay is refused before the listing is even read.
    EXPECT_EQ(0, repository.listingLoads());
}

TEST(ExchangeBuyDecision, TheReplayCheckLooksUpTheBuyersRowNotTheBareKey) {
    FakeExchangeRepository repository;
    repository.addListing(activeListing());
    repository.setPointBalance("Buyer", 500);

    // No purchase writes a row under the bare key, so one there is not this
    // purchase's.
    int balanceAfter = 0;
    ASSERT_TRUE(repository.adjustPoints("Buyer", 0, balanceAfter, POINT_REASON_ADJUST, 0, 0, "key"));

    ExchangeBuyRequest request = buyRequest();
    request.idempotencyKey = "key";

    EXPECT_TRUE(decideBuyListing(repository, request).isOk());
}

// A double click sends the same buy twice with no key. Both requests derive
// the same key, so once the first has written its ledger rows the second is
// a replay.
TEST(ExchangeBuyDecision, ADoubleClickWithoutAKeyIsAReplay) {
    FakeExchangeRepository repository;
    repository.addListing(activeListing());
    repository.setPointBalance("Buyer", 500);
    repository.setPointBalance("seller-account", 0);

    ExchangeBuyRequest first = buyRequest();
    first.idempotencyKey = resolveExchangeIdempotencyKey("", 1, 0, first.listingID);
    ASSERT_TRUE(decideBuyListing(repository, first).isOk());

    // What buyListing writes for the first click.
    int balanceAfter = 0;
    ASSERT_TRUE(repository.adjustPoints("Buyer", -108, balanceAfter, POINT_REASON_BUY, 10, 0,
                                        exchangeLedgerKey(first.idempotencyKey, kExchangeBuyLedgerSuffix)));
    ASSERT_TRUE(repository.adjustPoints("seller-account", 92, balanceAfter, POINT_REASON_SALE, 10, 0,
                                        exchangeLedgerKey(first.idempotencyKey, kExchangeSaleLedgerSuffix)));

    ExchangeBuyRequest second = buyRequest();
    second.idempotencyKey = resolveExchangeIdempotencyKey("", 1, 0, second.listingID);
    EXPECT_EQ(first.idempotencyKey, second.idempotencyKey);
    EXPECT_EQ(EXCHANGE_FAIL_IDEMPOTENCY_CONFLICT, decideBuyListing(repository, second).rejection().code);
}

TEST(ExchangeBuyDecision, AnUnusedKeyDoesNotStandInTheWay) {
    FakeExchangeRepository repository;
    repository.addListing(activeListing());
    repository.setPointBalance("Buyer", 500);

    ExchangeBuyRequest request = buyRequest();
    request.idempotencyKey = "fresh_key";

    EXPECT_TRUE(decideBuyListing(repository, request).isOk());
}

TEST(ExchangeBuyDecision, TheReplayCheckComesBeforeEveryOtherRefusal) {
    FakeExchangeRepository repository;
    ExchangeListing listing = activeListing();
    listing.status = LISTING_STATUS_SOLD;
    repository.addListing(listing);

    int balanceAfter = 0;
    ASSERT_TRUE(repository.adjustPoints("Buyer", 0, balanceAfter, POINT_REASON_BUY, 10, 0,
                                        exchangeLedgerKey("used", kExchangeBuyLedgerSuffix)));

    ExchangeBuyRequest request = buyRequest();
    request.idempotencyKey = "used";

    // A sold listing and an empty wallet are both true here; the key wins.
    EXPECT_EQ(EXCHANGE_FAIL_IDEMPOTENCY_CONFLICT, decideBuyListing(repository, request).rejection().code);
}

TEST(ExchangeBuyDecision, ListingStateOutranksThePurse) {
    FakeExchangeRepository repository;
    ExchangeListing listing = activeListing();
    listing.status = LISTING_STATUS_CANCELLED;
    repository.addListing(listing);
    repository.setPointBalance("Buyer", 0);

    EXPECT_EQ(EXCHANGE_FAIL_LISTING_NOT_AVAILABLE, decideBuyListing(repository, buyRequest()).rejection().code);
}

//////////////////////////////////////////////////////////////////////////////
// Idempotency keys
//////////////////////////////////////////////////////////////////////////////

TEST(ExchangeIdempotencyKey, EachLegAppendsItsSuffix) {
    EXPECT_EQ("key_buy", exchangeLedgerKey("key", kExchangeBuyLedgerSuffix));
    EXPECT_EQ("key_sale", exchangeLedgerKey("key", kExchangeSaleLedgerSuffix));
}

TEST(ExchangeIdempotencyKey, AFullWidthBaseIsTrimmedSoTheLegsStayDistinct) {
    const std::string base(kMaxExchangeIdempotencyKeyLength, 'k');

    const std::string buy = exchangeLedgerKey(base, kExchangeBuyLedgerSuffix);
    const std::string sale = exchangeLedgerKey(base, kExchangeSaleLedgerSuffix);

    EXPECT_EQ(kMaxExchangeIdempotencyKeyLength, buy.size());
    EXPECT_EQ(kMaxExchangeIdempotencyKeyLength, sale.size());
    EXPECT_EQ(std::string(kMaxExchangeIdempotencyKeyLength - 4, 'k') + "_buy", buy);
    EXPECT_EQ(std::string(kMaxExchangeIdempotencyKeyLength - 5, 'k') + "_sale", sale);
    EXPECT_NE(buy, sale);
}

TEST(ExchangeIdempotencyKey, TheServerKeyNamesTheWorldTheServerAndTheListing) {
    EXPECT_EQ("EX_0100000000000000000a", exchangeServerIdempotencyKey(1, 0, 10));
    EXPECT_EQ("EX_0203000000000000000a", exchangeServerIdempotencyKey(2, 3, 10));
    EXPECT_EQ("EX_ffff7fffffffffffffff", exchangeServerIdempotencyKey(0xFF, 0xFF, INT64_MAX));

    // Fixed width, with room for either suffix inside the column.
    EXPECT_EQ(23u, exchangeServerIdempotencyKey(1, 0, 10).size());
    EXPECT_EQ(23u, exchangeServerIdempotencyKey(0xFF, 0xFF, INT64_MAX).size());

    // Every component tells two purchases apart.
    EXPECT_NE(exchangeServerIdempotencyKey(1, 0, 10), exchangeServerIdempotencyKey(2, 0, 10));
    EXPECT_NE(exchangeServerIdempotencyKey(1, 0, 10), exchangeServerIdempotencyKey(1, 1, 10));
    EXPECT_NE(exchangeServerIdempotencyKey(1, 0, 10), exchangeServerIdempotencyKey(1, 0, 11));
}

TEST(ExchangeIdempotencyKey, AClientKeyIsRecordedUnderItsOwnPrefix) {
    EXPECT_EQ("C_client-key", resolveExchangeIdempotencyKey("client-key", 1, 0, 10));
}

TEST(ExchangeIdempotencyKey, NoClientKeyMeansTheServerKey) {
    EXPECT_EQ(exchangeServerIdempotencyKey(1, 0, 10), resolveExchangeIdempotencyKey("", 1, 0, 10));
}

// Otherwise a buyer could spend a purchase of one listing on planting the key
// another listing's keyless buy derives, and that buy would be refused. The
// ledger compares keys without regard to case, so the client's keys are kept
// apart by a namespace, not by refusing one spelling of the server's prefix.
TEST(ExchangeIdempotencyKey, AClientKeyCannotReachTheServersNamespace) {
    const std::string planted = exchangeServerIdempotencyKey(1, 0, 11);
    EXPECT_EQ("C_" + planted, resolveExchangeIdempotencyKey(planted, 1, 0, 10));
    EXPECT_EQ("C_EX_", resolveExchangeIdempotencyKey("EX_", 1, 0, 10));
    EXPECT_EQ("C_ex_lower", resolveExchangeIdempotencyKey("ex_lower", 1, 0, 10));
    EXPECT_EQ("C_EX", resolveExchangeIdempotencyKey("EX", 1, 0, 10));
}

//////////////////////////////////////////////////////////////////////////////
// Browsing
//////////////////////////////////////////////////////////////////////////////

TEST(ExchangeListingFilterTest, TheDefaultFilterMatchesEveryListing) {
    ExchangeListing listing = activeListing();
    listing.itemClass = 7;
    listing.itemType = 3;
    EXPECT_TRUE(matchesExchangeListingFilter(listing, ExchangeListingFilter()));
}

TEST(ExchangeListingFilterTest, TheFilterCarriesEveryFieldOfTheRequest) {
    CGExchangeList packet;
    packet.setItemClass(7);
    packet.setItemType(3);
    packet.setMinPrice(50);
    packet.setMaxPrice(150);
    packet.setSellerFilter("Sell");

    const ExchangeListingFilter filter = exchangeListingFilterOf(packet);
    EXPECT_EQ(7, (int)filter.itemClass);
    EXPECT_EQ(3, (int)filter.itemType);
    EXPECT_EQ(50, filter.minPrice);
    EXPECT_EQ(150, filter.maxPrice);
    EXPECT_EQ("Sell", filter.sellerFilter);
}

// The client's browse sends no seller filter; that request narrows nothing.
TEST(ExchangeListingFilterTest, ARequestWithoutASellerFilterMatchesEverySeller) {
    ExchangeListing listing = activeListing();
    listing.sellerPlayer = "Anybody";
    EXPECT_TRUE(matchesExchangeListingFilter(listing, exchangeListingFilterOf(CGExchangeList())));
}

TEST(ExchangeListingFilterTest, TheSellerFilterIsACaseSensitiveSubstringOfTheSellersName) {
    CGExchangeList packet;
    packet.setSellerFilter("ell");
    const ExchangeListingFilter filter = exchangeListingFilterOf(packet);

    ExchangeListing listing = activeListing();
    listing.sellerPlayer = "Seller";
    EXPECT_TRUE(matchesExchangeListingFilter(listing, filter));
    listing.sellerPlayer = "ell";
    EXPECT_TRUE(matchesExchangeListingFilter(listing, filter));
    listing.sellerPlayer = "Buyer";
    EXPECT_FALSE(matchesExchangeListingFilter(listing, filter));
    listing.sellerPlayer = "SELLER";
    EXPECT_FALSE(matchesExchangeListingFilter(listing, filter));
}

TEST(ExchangeListingFilterTest, ItemAndPriceBounds) {
    ExchangeListing listing = activeListing(); // priced at 100
    listing.itemClass = 7;
    listing.itemType = 3;

    ExchangeListingFilter filter;
    filter.itemClass = 7;
    EXPECT_TRUE(matchesExchangeListingFilter(listing, filter));
    filter.itemClass = 8;
    EXPECT_FALSE(matchesExchangeListingFilter(listing, filter));

    filter = ExchangeListingFilter();
    filter.itemType = 3;
    EXPECT_TRUE(matchesExchangeListingFilter(listing, filter));
    filter.itemType = 4;
    EXPECT_FALSE(matchesExchangeListingFilter(listing, filter));

    // Both bounds are inclusive.
    filter = ExchangeListingFilter();
    filter.minPrice = 100;
    filter.maxPrice = 100;
    EXPECT_TRUE(matchesExchangeListingFilter(listing, filter));
    filter.minPrice = 101;
    EXPECT_FALSE(matchesExchangeListingFilter(listing, filter));
    filter.minPrice = 0;
    filter.maxPrice = 99;
    EXPECT_FALSE(matchesExchangeListingFilter(listing, filter));
}

//////////////////////////////////////////////////////////////////////////////
// Cancelling
//////////////////////////////////////////////////////////////////////////////

TEST(ExchangeCancelDecision, TheSellerMayWithdrawAnActiveListing) {
    FakeExchangeRepository repository;
    repository.addListing(activeListing());

    EXPECT_TRUE(decideCancelListing(repository, "Seller", 10).isOk());
    // Deciding does not write: the listing is still active.
    EXPECT_EQ(LISTING_STATUS_ACTIVE, repository.listings()[0].status);
}

TEST(ExchangeCancelDecision, UnknownListing) {
    FakeExchangeRepository repository;
    EXPECT_EQ(EXCHANGE_FAIL_LISTING_NOT_FOUND, decideCancelListing(repository, "Seller", 10).rejection().code);
}

TEST(ExchangeCancelDecision, OnlyTheSeller) {
    FakeExchangeRepository repository;
    repository.addListing(activeListing());

    EXPECT_EQ(EXCHANGE_FAIL_NOT_SELLER, decideCancelListing(repository, "Somebody", 10).rejection().code);
}

TEST(ExchangeCancelDecision, OwnershipOutranksStatus) {
    FakeExchangeRepository repository;
    ExchangeListing listing = activeListing();
    listing.status = LISTING_STATUS_SOLD;
    repository.addListing(listing);

    EXPECT_EQ(EXCHANGE_FAIL_NOT_SELLER, decideCancelListing(repository, "Somebody", 10).rejection().code);
}

TEST(ExchangeCancelDecision, OnlyWhileActive) {
    const uint8_t inactive[] = {LISTING_STATUS_SOLD, LISTING_STATUS_CANCELLED, LISTING_STATUS_EXPIRED};
    for (uint8_t status : inactive) {
        FakeExchangeRepository repository;
        ExchangeListing listing = activeListing();
        listing.status = status;
        repository.addListing(listing);

        auto result = decideCancelListing(repository, "Seller", 10);
        ASSERT_TRUE(result.isRejected()) << "status " << (int)status;
        EXPECT_EQ(EXCHANGE_FAIL_LISTING_NOT_AVAILABLE, result.rejection().code) << "status " << (int)status;
    }
}

//////////////////////////////////////////////////////////////////////////////
// Claiming
//////////////////////////////////////////////////////////////////////////////

TEST(ExchangeBuyerClaimDecision, APaidOrderNamesItsListing) {
    FakeExchangeRepository repository;
    repository.addListing(activeListing());
    repository.addOrder(paidOrder(7, 10, "Buyer"));

    auto result = decideBuyerClaim(repository, "Buyer", 7);
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(7, result.events().orderID);
    EXPECT_EQ(10, result.events().listingID);
}

TEST(ExchangeBuyerClaimDecision, NoSuchOrder) {
    FakeExchangeRepository repository;
    repository.addListing(activeListing());

    EXPECT_EQ(EXCHANGE_FAIL_LISTING_NOT_FOUND, decideBuyerClaim(repository, "Buyer", 7).rejection().code);
}

TEST(ExchangeBuyerClaimDecision, AnotherBuyersOrder) {
    FakeExchangeRepository repository;
    repository.addListing(activeListing());
    repository.addOrder(paidOrder(7, 10, "Someone"));

    EXPECT_EQ(EXCHANGE_FAIL_LISTING_NOT_FOUND, decideBuyerClaim(repository, "Buyer", 7).rejection().code);
}

TEST(ExchangeBuyerClaimDecision, AnOrderAlreadyDelivered) {
    FakeExchangeRepository repository;
    repository.addListing(activeListing());
    ExchangeOrder order = paidOrder(7, 10, "Buyer");
    order.status = ORDER_STATUS_DELIVERED;
    repository.addOrder(order);

    // Only paid orders are claimable; a delivered one reads as "no order".
    EXPECT_EQ(EXCHANGE_FAIL_LISTING_NOT_FOUND, decideBuyerClaim(repository, "Buyer", 7).rejection().code);
}

TEST(ExchangeBuyerClaimDecision, AnOrderWhoseListingIsGone) {
    FakeExchangeRepository repository;
    repository.addOrder(paidOrder(7, 10, "Buyer"));

    EXPECT_EQ(EXCHANGE_FAIL_LISTING_NOT_FOUND, decideBuyerClaim(repository, "Buyer", 7).rejection().code);
}

TEST(ExchangeSellerClaimDecision, CancelledAndExpiredListingsComeBack) {
    const uint8_t claimable[] = {LISTING_STATUS_CANCELLED, LISTING_STATUS_EXPIRED};
    for (uint8_t status : claimable) {
        FakeExchangeRepository repository;
        ExchangeListing listing = activeListing();
        listing.status = status;
        repository.addListing(listing);

        EXPECT_TRUE(decideSellerClaim(repository, "Seller", 10).isOk()) << "status " << (int)status;
    }
}

TEST(ExchangeSellerClaimDecision, ActiveAndSoldListingsDoNot) {
    const uint8_t held[] = {LISTING_STATUS_ACTIVE, LISTING_STATUS_SOLD};
    for (uint8_t status : held) {
        FakeExchangeRepository repository;
        ExchangeListing listing = activeListing();
        listing.status = status;
        repository.addListing(listing);

        auto result = decideSellerClaim(repository, "Seller", 10);
        ASSERT_TRUE(result.isRejected()) << "status " << (int)status;
        EXPECT_EQ(EXCHANGE_FAIL_LISTING_NOT_AVAILABLE, result.rejection().code) << "status " << (int)status;
    }
}

TEST(ExchangeSellerClaimDecision, UnknownListing) {
    FakeExchangeRepository repository;
    EXPECT_EQ(EXCHANGE_FAIL_LISTING_NOT_FOUND, decideSellerClaim(repository, "Seller", 10).rejection().code);
}

TEST(ExchangeSellerClaimDecision, OnlyTheSeller) {
    FakeExchangeRepository repository;
    ExchangeListing listing = activeListing();
    listing.status = LISTING_STATUS_CANCELLED;
    repository.addListing(listing);

    EXPECT_EQ(EXCHANGE_FAIL_NOT_SELLER, decideSellerClaim(repository, "Somebody", 10).rejection().code);
}

} // namespace
