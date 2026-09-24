//////////////////////////////////////////////////////////////////////////////
// Filename    : ExchangeDecision.h
// Description : the Exchange system's rejection reasons and the decisions
//               that need nothing but a repository and plain values, kept
//               apart from ExchangeService so they can be exercised without
//               a player, an item or a database.
//////////////////////////////////////////////////////////////////////////////

#ifndef __EXCHANGE_DECISION_H__
#define __EXCHANGE_DECISION_H__

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include "Outcome.h"
#include "repository/ExchangeRepository.h"

class CGExchangeList;

//////////////////////////////////////////////////////////////////////////////
// Exchange Result Codes
//////////////////////////////////////////////////////////////////////////////

enum ExchangeResult {
    EXCHANGE_SUCCESS,
    EXCHANGE_FAIL_ITEM_NOT_FOUND,
    EXCHANGE_FAIL_ITEM_OWNERSHIP,
    EXCHANGE_FAIL_ITEM_TRADEABLE,
    EXCHANGE_FAIL_INVALID_PRICE,
    EXCHANGE_FAIL_INSUFFICIENT_POINTS,
    EXCHANGE_FAIL_LISTING_NOT_FOUND,
    EXCHANGE_FAIL_LISTING_NOT_AVAILABLE,
    EXCHANGE_FAIL_INVENTORY_FULL,
    EXCHANGE_FAIL_STORAGE_FULL,
    EXCHANGE_FAIL_NOT_SELLER,
    EXCHANGE_FAIL_NOT_BUYER,
    EXCHANGE_FAIL_ALREADY_CLAIMED,
    EXCHANGE_FAIL_DATABASE_ERROR,
    EXCHANGE_FAIL_TRANSACTION_ERROR,
    EXCHANGE_FAIL_IDEMPOTENCY_CONFLICT,
    EXCHANGE_FAIL_UNKNOWN
};

// The English text one result code carries to the player, with the optional
// detail appended after a colon. EXCHANGE_SUCCESS is "Success" and ignores
// the detail. This is what GCExchangeBuy puts on the wire, so the strings are
// part of the client contract and may not be reworded on their own.
std::string formatExchangeError(ExchangeResult code, const std::string& detail = std::string());

// Why an Exchange mutation was refused: the typed code, plus the free text a
// transaction failure carries. The caller decides what to do with it - send
// message() to the client, log it, or both.
struct ExchangeRejection {
    explicit ExchangeRejection(ExchangeResult resultCode, std::string resultDetail = std::string())
        : code(resultCode), detail(std::move(resultDetail)) {}

    std::string message() const {
        return formatExchangeError(code, detail);
    }

    ExchangeResult code;
    std::string detail;
};

//////////////////////////////////////////////////////////////////////////////
// Decisions
//////////////////////////////////////////////////////////////////////////////

// The tax on one listing's price, truncated toward zero. It is charged to
// both sides: the buyer pays price + tax and the seller receives price - tax.
int calculateExchangeTax(int price, uint8_t taxRate);

// A listing may only be created at a price of at least one point.
[[nodiscard]] Outcome<void, ExchangeRejection> validateListingPrice(int pricePoint);

//////////////////////////////////////////////////////////////////////////////
// Browsing
//////////////////////////////////////////////////////////////////////////////

// What a browse narrows a page of active listings to. Every field left at its
// default matches every listing.
struct ExchangeListingFilter {
    // 0xFF matches every item class.
    uint8_t itemClass = 0xFF;
    // 0xFFFF matches every item type.
    uint16_t itemType = 0xFFFF;
    // A bound of 0 or less is no bound; both are inclusive.
    int minPrice = 0;
    int maxPrice = 0;
    // A substring of the seller's player name, compared case-sensitively.
    // Empty matches every seller.
    std::string sellerFilter;
};

// The filter a CGExchangeList asks for, every field of it.
ExchangeListingFilter exchangeListingFilterOf(const CGExchangeList& packet);

// Does this listing pass the filter?
bool matchesExchangeListingFilter(const ExchangeListing& listing, const ExchangeListingFilter& filter);

//////////////////////////////////////////////////////////////////////////////
// Idempotency keys
//
// A buy writes two PointLedger rows, the buyer's and the seller's, each keyed
// by the purchase's key plus a leg suffix. UNQ_Ledger_IdempotencyKey refuses a
// second row with the same key, and adjustPoints refuses a key it already
// holds, so a purchase whose key the ledger has seen cannot move points again.
//////////////////////////////////////////////////////////////////////////////

// The suffixes of the buyer's and the seller's ledger rows.
extern const char* const kExchangeBuyLedgerSuffix;  // "_buy"
extern const char* const kExchangeSaleLedgerSuffix; // "_sale"

// The prefix of every key the server derives itself; a client-supplied key
// is recorded under a prefix of its own (see resolveExchangeIdempotencyKey).
extern const char* const kExchangeServerKeyPrefix; // "EX_"

// The longest key PointLedger.IdempotencyKey can hold: it is VARCHAR(64)
// UNIQUE, and this project mandates a non-strict sql_mode, so anything longer
// is silently truncated on insert instead of being rejected.
const size_t kMaxExchangeIdempotencyKeyLength = 64;

// The ledger key of one leg of a purchase: the base, trimmed to leave room
// for the suffix within kMaxExchangeIdempotencyKeyLength, then the suffix.
// Without the trim a maximum-length base would make both legs truncate to the
// same stored value, collide, and roll the purchase back. Two bases that
// differ only past the trim point still collide; that is inherent to a 64-byte
// column carrying a suffix, and it fails closed (a replay is refused).
std::string exchangeLedgerKey(const std::string& base, const std::string& suffix);

// The key the server derives for a buy of this listing: the prefix, the world
// and server ids as two hex digits each, and the listing id as sixteen, 23
// characters in all. A listing is sold at most once (ExchangeOrder.ListingID
// is UNIQUE and no status write returns a listing to active), so the key names
// exactly one purchase, and a repeat of the same buy - a double click, a
// resend - derives the same key and is refused as a replay. The world and
// server ids keep the key unique where listing ids are not shared: the listing
// ids come from each world's own ExchangeListing table.
std::string exchangeServerIdempotencyKey(int worldID, int serverID, int64_t listingID);

// The key a buy is recorded under: the server-derived key when the client
// sent none, and the client's key under a "C_" prefix of its own when it sent
// one, so that no purchase can plant the key another listing's keyless buy
// will derive -- in any letter case, since the ledger's collation folds it --
// and turn that buy into a refused replay.
std::string resolveExchangeIdempotencyKey(const std::string& clientKey, int worldID, int serverID, int64_t listingID);

//////////////////////////////////////////////////////////////////////////////
// Buying
//////////////////////////////////////////////////////////////////////////////

// What a buy asks for: the listing, who is buying, the key that makes the
// purchase replay-proof, and the two values the terms are computed from.
struct ExchangeBuyRequest {
    int64_t listingID = 0;
    std::string buyerAccount;
    std::string buyerPlayer;
    // The key the purchase's ledger rows are built from (see
    // resolveExchangeIdempotencyKey). Empty skips the replay check.
    std::string idempotencyKey;
    // The server the listing must belong to for this buyer to see it.
    int16_t serverID = 0;
    uint8_t taxRate = 0;
};

// What an accepted buy costs and pays out. The listing itself is not carried:
// the repository hands out a copy per lookup, and only these fields are used
// past the decision.
struct ExchangePurchaseTerms {
    int64_t listingID = 0;
    std::string sellerAccount = std::string();
    int pricePoint = 0;
    int taxAmount = 0;
    // What the buyer is charged, price + tax.
    int totalCost = 0;
    // What the seller is credited, price - tax.
    int sellerIncome = 0;
    // The buyer's balance as it was read for the affordability check.
    int buyerBalance = 0;
};

// May this buyer take this listing, and on what terms?
//
// Refused, in this order, when the ledger already holds the buyer's row of
// this key (exchangeLedgerKey with kExchangeBuyLedgerSuffix), when the id
// names no listing, when the listing is not active, when it belongs to
// another server, when the buyer is its seller, and when the buyer cannot
// afford price + tax.
//
// The repository is passed in because every one of those but the last is a
// database read; the writes stay with the caller, so this is a pure decision
// over whatever the repository answers and needs no database in a test. A
// repository that fails its query throws (the DB layer's own const char*);
// that is a server fault, not a player-facing rejection, and is left to the
// caller.
[[nodiscard]] Outcome<ExchangePurchaseTerms, ExchangeRejection> decideBuyListing(ExchangeRepository& repository,
                                                                                 const ExchangeBuyRequest& request);

// May this seller withdraw this listing? Refused when the id names no
// listing, when the seller does not own it, and when it is not active.
[[nodiscard]] Outcome<void, ExchangeRejection> decideCancelListing(ExchangeRepository& repository,
                                                                   const std::string& sellerPlayer, int64_t listingID);

// The order a buyer claim delivers, and the listing it was placed on.
struct ExchangeBuyerClaim {
    int64_t orderID = 0;
    int64_t listingID = 0;
};

// May this buyer claim this order? Refused when the buyer has no paid order
// of that id, and when the order's listing has gone missing.
[[nodiscard]] Outcome<ExchangeBuyerClaim, ExchangeRejection>
decideBuyerClaim(ExchangeRepository& repository, const std::string& buyerPlayer, int64_t orderID);

// May this seller take back the item behind this listing? Refused when the
// id names no listing, when the seller does not own it, and when it is
// neither cancelled nor expired - an active or sold listing still owes its
// item to the exchange or to a buyer.
[[nodiscard]] Outcome<void, ExchangeRejection> decideSellerClaim(ExchangeRepository& repository,
                                                                 const std::string& sellerPlayer, int64_t listingID);

#endif // __EXCHANGE_DECISION_H__
