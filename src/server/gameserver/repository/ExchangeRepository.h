#ifndef __EXCHANGE_REPOSITORY_H__
#define __EXCHANGE_REPOSITORY_H__

#include <cstdint>
#include <string>
#include <vector>

#include "Types.h"

// The Exchange feature's tables: the listings and the orders placed on
// them (ExchangeListing, ExchangeOrder) and the account point ledger
// (AccountPoint, PointLedger), plus the transaction pair the buy path
// wraps around a purchase.
//
// Connections. The listing and order statements run on the thread's
// DARKEDEN connection. The point statements ask
// g_pDatabaseManager->getConnection("USERINFO") — but the string overload
// ignores its argument and returns that same DARKEDEN connection (the
// USERINFO socket is getUserInfoConnection(), which nothing here calls).
// initdb/DARKEDEN.sql does not create AccountPoint or PointLedger — both
// are in initdb/USERINFO.sql — so against the shipped schema every point
// statement fails with ER_NO_SUCH_TABLE, is logged to DBError.log and
// thrown as END_DB's DatabaseError. ExchangeService::buyListing reads the
// buyer's balance before it begins, so a buy always throws out of
// CGExchangeBuyHandler, where GamePlayer::processCommand's catch (...)
// disconnects the buyer.
//
// The transaction pair. beginTransaction, commit and rollback issue their
// statement twice, once through each name, and both names resolve to the
// one DARKEDEN connection: the second START TRANSACTION commits whatever
// the first left pending (MySQL commits a pending transaction when a new
// one begins), and the pair gives no cross-database atomicity. The
// DARKEDEN half does behave: a listing created after beginTransaction is
// discarded by rollback and kept by commit. A DatabaseError escaping from a
// statement inside the pair leaves ExchangeService::buyListing without a
// rollback (it catches only std::string), so the connection stays in the
// transaction until the next START TRANSACTION.
//
// Text arguments (accounts, players, item names, idempotency keys) are
// escaped with mysql_real_escape_string on the DARKEDEN connection, or by
// a manual quote/backslash pass when no connection is up; the datetime
// text expireAt is interpolated raw. CreatedAt, UpdatedAt, SoldAt,
// CancelledAt and DeliveredAt are the server process's local time,
// formatted here; a caller's createdAt/updatedAt/deliveredAt fields are
// ignored on insert.
//
// ExchangeListing's UNIQUE KEY (ItemClass, ItemID, ObjectID), and the
// fact that no statement deletes a listing row, mean createListing for an
// object that was ever listed — whatever status its old row is in now —
// fails with ER_DUP_ENTRY (thrown as a DatabaseError). ExchangeOrder's
// UNIQUE ListingID does the same to a second order on one listing.
//
// The methods keep the __BEGIN_TRY / __END_CATCH frames and the bool
// results the Exchange service was written against: a false from a
// listing or order write is unreachable (the SQL failure throws first),
// and only adjustPoints answers false on its own.
//
// ExchangeListing is declared in GCExchangeList.h (a wire struct the
// client mirrors); ExchangeOrder and the status and reason codes are
// declared here. The listing loads read SELECT * positionally, in the
// CREATE TABLE column order.
//
// Not enclosed: no other file in the tree names these four tables in SQL.

// Listing status constants
const uint8_t LISTING_STATUS_ACTIVE = 0;
const uint8_t LISTING_STATUS_SOLD = 1;
const uint8_t LISTING_STATUS_CANCELLED = 2;
const uint8_t LISTING_STATUS_EXPIRED = 3;

// Order status constants
const uint8_t ORDER_STATUS_PAID = 0;
const uint8_t ORDER_STATUS_DELIVERED = 1;
const uint8_t ORDER_STATUS_CANCELLED = 2;

// Point transaction reason codes
const uint8_t POINT_REASON_BUY = 0;
const uint8_t POINT_REASON_SALE = 1;
const uint8_t POINT_REASON_TAX = 2;
const uint8_t POINT_REASON_REFUND = 3;
const uint8_t POINT_REASON_ADJUST = 4;

// Forward declaration (defined in GCExchangeList.h)
struct ExchangeListing;

// One ExchangeOrder row in column order. The two ids come back through
// getString and strtoll, serverID through getInt, status through getBYTE,
// the three datetimes as text ("" for NULL).
struct ExchangeOrder {
    int64_t orderID;
    int64_t listingID;
    int16_t serverID;
    std::string buyerAccount;
    std::string buyerPlayer;
    int pricePoint;
    int taxAmount;
    uint8_t status;
    std::string createdAt;
    std::string deliveredAt;
    std::string cancelledAt;
};

class ExchangeRepository {
public:
    virtual ~ExchangeRepository() {}

    // --- listings (DARKEDEN) ------------------------------------------------
    // INSERT of every column but ListingID, then SELECT LAST_INSERT_ID() on
    // the same Statement; the new id, or 0 when the id read answers no row
    // (it always answers one).
    virtual int64_t createListing(const ExchangeListing& listing) = 0;
    // Status = 2, CancelledAt and UpdatedAt = now, for an ACTIVE row only.
    // True whether or not a row matched.
    virtual bool cancelListing(int64_t listingID) = 0;
    // Status = 3, UpdatedAt = now, for an ACTIVE row only. True either way.
    virtual bool expireListing(int64_t listingID) = 0;
    // Status = 1, the buyer columns, SoldAt and UpdatedAt = now, for an
    // ACTIVE row only. True either way.
    virtual bool markListingSold(int64_t listingID, const std::string& buyerAccount,
                                 const std::string& buyerPlayer) = 0;
    // One server's listings in one status, newest CreatedAt first, the
    // 1-based page of pageSize rows: LIMIT pageSize OFFSET (page - 1) *
    // pageSize.
    virtual std::vector<ExchangeListing> getListings(int16_t serverID, uint8_t status, int page, int pageSize) = 0;
    // A new ExchangeListing the caller owns, NULL when the id has no row.
    // Every caller in ExchangeService leaks it.
    virtual ExchangeListing* getListing(int64_t listingID) = 0;
    // One seller account's listings in one status, newest first.
    virtual std::vector<ExchangeListing> getSellerListings(const std::string& sellerAccount, uint8_t status) = 0;
    // Up to 1000 ACTIVE listings whose ExpireAt is before the database's
    // NOW(), oldest ExpireAt first.
    virtual std::vector<ExchangeListing> getExpiredListings() = 0;

    // --- orders (DARKEDEN) --------------------------------------------------
    // INSERT of the eight non-id columns (CreatedAt = now), then SELECT
    // LAST_INSERT_ID() on the same Statement; the new id, or 0.
    virtual int64_t createOrder(const ExchangeOrder& order) = 0;
    // Status = 1, DeliveredAt = now, for a PAID row only. True either way.
    virtual bool markOrderDelivered(int64_t orderID) = 0;
    // One buyer's orders in one status, newest first.
    virtual std::vector<ExchangeOrder> getBuyerOrders(const std::string& buyerPlayer, uint8_t status) = 0;
    // The orders in one status whose listing's SellerPlayer is this player
    // (INNER JOIN on ListingID), newest first.
    virtual std::vector<ExchangeOrder> getSellerOrders(const std::string& sellerPlayer, uint8_t status) = 0;

    // --- the point ledger (asks for USERINFO, reaches DARKEDEN) -----------
    // Up to four statements on one connection, each on its own Statement:
    // when a key is given, a count of the PointLedger rows carrying it
    // (false, nothing written, when one exists); the account's
    // AccountPoint.PointBalance (0 when it has no row); false, nothing
    // written, when balance + delta is below 0; otherwise REPLACE INTO
    // AccountPoint with the new balance and UpdatedAt = now, then an
    // INSERT INTO PointLedger — with the IdempotencyKey column when a key
    // is given, without it otherwise — and balanceAfter is the new
    // balance. The caller cannot tell the two false results apart.
    virtual bool adjustPoints(const std::string& account, int delta, int& balanceAfter, uint8_t reason,
                              int64_t refListingID, int64_t refOrderID, const std::string& idempotencyKey) = 0;
    // AccountPoint.PointBalance through getInt; 0 when the account has no
    // row.
    virtual int getPointBalance(const std::string& account) = 0;
    // Whether any PointLedger row carries this IdempotencyKey.
    virtual bool hasIdempotencyKey(const std::string& idempotencyKey) = 0;

    // --- the transaction pair (see the note above) ------------------------
    // START TRANSACTION through the DARKEDEN name, then through the
    // USERINFO name. True unless a statement fails, and then the const
    // char* escapes and the false is never reached.
    virtual bool beginTransaction() = 0;
    // COMMIT, twice, the same way.
    virtual bool commit() = 0;
    // ROLLBACK, twice, the same way.
    virtual bool rollback() = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLExchangeRepository.cpp.
ExchangeRepository& defaultExchangeRepository();

#endif // __EXCHANGE_REPOSITORY_H__
