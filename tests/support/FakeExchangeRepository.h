#ifndef __FAKE_EXCHANGE_REPOSITORY_H__
#define __FAKE_EXCHANGE_REPOSITORY_H__

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "DatabaseError.h"
#include "GCExchangeList.h" // For ExchangeListing definition
#include "repository/ExchangeRepository.h"

// In-memory ExchangeRepository for domain tests.
// Models the four Exchange tables as the MySQL implementation behaves
// (src/server/gameserver/repository/ExchangeRepository.h is the authority on
// that contract; the MySQL-backed integration tier is what pins it):
//  - getListing() hands out a NEW listing the caller owns, or NULL when the
//    id names no row.
//  - The status writes (cancelListing, expireListing, markListingSold,
//    markOrderDelivered) only match a row in the status they expect.
//    markListingSold answers whether it matched one; the others answer true
//    whether or not one matched.
//  - adjustPoints() answers false, writing nothing, for a ledger key that is
//    already stored AND for a balance that would go below zero; the caller
//    cannot tell the two apart. An account with no row has a balance of 0.
//  - The listing and order lists come back newest first, which here means
//    reverse insertion order.
//  - The transaction trio is counted and honoured as one transaction:
//    beginTransaction takes a copy of every table, rollback puts the copy
//    back and commit drops it. Nested transactions are not modeled.
//  - A statement fails on request: failOn() makes the nth call of
//    markListingSold, createOrder, adjustPoints, hasIdempotencyKey or one of
//    the transaction trio throw a DatabaseError, as END_DB does, before it
//    writes anything.
//  - NOT modeled: the UNIQUE keys that make a second listing of one object,
//    or a second order on one listing, fail with ER_DUP_ENTRY; row locks and
//    the waits they cause; and the datetime columns, which are strings the
//    caller supplies here rather than the database's own NOW(). Because
//    there is no clock, getExpiredListings() answers every active listing
//    instead of comparing ExpireAt.
class FakeExchangeRepository : public ExchangeRepository {
public:
    // --- test seeding -------------------------------------------------------
    // Store a listing under the id it carries, or under the next free id when
    // it carries none. Returns the id used.
    int64_t addListing(ExchangeListing listing) {
        if (listing.listingID == 0)
            listing.listingID = m_NextListingID++;
        else
            m_NextListingID = std::max(m_NextListingID, listing.listingID + 1);
        m_Listings.push_back(listing);
        return listing.listingID;
    }

    int64_t addOrder(ExchangeOrder order) {
        if (order.orderID == 0)
            order.orderID = m_NextOrderID++;
        else
            m_NextOrderID = std::max(m_NextOrderID, order.orderID + 1);
        m_Orders.push_back(order);
        return order.orderID;
    }

    void setPointBalance(const std::string& account, int balance) {
        for (auto& point : m_Points) {
            if (point.account == account) {
                point.balance = balance;
                return;
            }
        }
        Account added;
        added.account = account;
        added.balance = balance;
        m_Points.push_back(added);
    }

    // --- test inspection ----------------------------------------------------
    struct LedgerRow {
        std::string account;
        int delta;
        uint8_t reason;
        int64_t refListingID;
        int64_t refOrderID;
        std::string idempotencyKey;
    };

    const std::vector<LedgerRow>& ledger() const {
        return m_Ledger;
    }
    const std::vector<ExchangeListing>& listings() const {
        return m_Listings;
    }
    const std::vector<ExchangeOrder>& orders() const {
        return m_Orders;
    }
    int transactionsBegun() const {
        return m_Begun;
    }
    int commits() const {
        return m_Commits;
    }
    int rollbacks() const {
        return m_Rollbacks;
    }
    // Whether a transaction is begun and not yet committed or rolled back.
    bool inTransaction() const {
        return m_InTransaction;
    }

    // --- test failure injection ---------------------------------------------
    // The nth call (1-based, counted from now) of the named method throws a
    // DatabaseError instead of running; see the list above for the methods
    // that honour it.
    void failOn(const std::string& method, int nth = 1) {
        m_FailAt[method] = m_Calls[method] + nth;
    }

    // A ledger row another purchase commits while this one is in flight: it
    // appears when the current transaction rolls back, as a concurrent
    // commit is visible to the reads that follow a rollback.
    void commitElsewhereOnRollback(const LedgerRow& row) {
        m_CommittedElsewhere.push_back(row);
    }
    // How many listing loads the caller asked for: the decisions are meant to
    // read a listing once.
    int listingLoads() const {
        return m_ListingLoads;
    }

    // --- ExchangeRepository -------------------------------------------------
    int64_t createListing(const ExchangeListing& listing) {
        return addListing(listing);
    }

    bool cancelListing(int64_t listingID) {
        setStatusIf(listingID, LISTING_STATUS_ACTIVE, LISTING_STATUS_CANCELLED);
        return true;
    }

    bool expireListing(int64_t listingID) {
        setStatusIf(listingID, LISTING_STATUS_ACTIVE, LISTING_STATUS_EXPIRED);
        return true;
    }

    bool markListingSold(int64_t listingID, const std::string& buyerAccount, const std::string& buyerPlayer) {
        call("markListingSold");
        bool marked = false;
        for (auto& listing : m_Listings) {
            if (listing.listingID == listingID && listing.status == LISTING_STATUS_ACTIVE) {
                listing.status = LISTING_STATUS_SOLD;
                listing.buyerAccount = buyerAccount;
                listing.buyerPlayer = buyerPlayer;
                marked = true;
            }
        }
        return marked;
    }

    std::vector<ExchangeListing> getListings(int16_t serverID, uint8_t status, int page, int pageSize) {
        std::vector<ExchangeListing> matched;
        for (auto itr = m_Listings.rbegin(); itr != m_Listings.rend(); ++itr) {
            if (itr->serverID == serverID && itr->status == status)
                matched.push_back(*itr);
        }
        return paged(matched, page, pageSize);
    }

    ExchangeListing* getListing(int64_t listingID) {
        ++m_ListingLoads;
        for (const auto& listing : m_Listings) {
            if (listing.listingID == listingID)
                return new ExchangeListing(listing);
        }
        return NULL;
    }

    std::vector<ExchangeListing> getSellerListings(const std::string& sellerAccount, uint8_t status) {
        std::vector<ExchangeListing> matched;
        for (auto itr = m_Listings.rbegin(); itr != m_Listings.rend(); ++itr) {
            if (itr->sellerAccount == sellerAccount && itr->status == status)
                matched.push_back(*itr);
        }
        return matched;
    }

    std::vector<ExchangeListing> getExpiredListings() {
        std::vector<ExchangeListing> matched;
        for (const auto& listing : m_Listings) {
            if (listing.status == LISTING_STATUS_ACTIVE)
                matched.push_back(listing);
        }
        return matched;
    }

    int64_t createOrder(const ExchangeOrder& order) {
        call("createOrder");
        return addOrder(order);
    }

    bool markOrderDelivered(int64_t orderID) {
        for (auto& order : m_Orders) {
            if (order.orderID == orderID && order.status == ORDER_STATUS_PAID)
                order.status = ORDER_STATUS_DELIVERED;
        }
        return true;
    }

    std::vector<ExchangeOrder> getBuyerOrders(const std::string& buyerPlayer, uint8_t status) {
        std::vector<ExchangeOrder> matched;
        for (auto itr = m_Orders.rbegin(); itr != m_Orders.rend(); ++itr) {
            if (itr->buyerPlayer == buyerPlayer && itr->status == status)
                matched.push_back(*itr);
        }
        return matched;
    }

    std::vector<ExchangeOrder> getSellerOrders(const std::string& sellerPlayer, uint8_t status) {
        std::vector<ExchangeOrder> matched;
        for (auto itr = m_Orders.rbegin(); itr != m_Orders.rend(); ++itr) {
            if (itr->status != status)
                continue;
            for (const auto& listing : m_Listings) {
                if (listing.listingID == itr->listingID && listing.sellerPlayer == sellerPlayer) {
                    matched.push_back(*itr);
                    break;
                }
            }
        }
        return matched;
    }

    bool adjustPoints(const std::string& account, int delta, int& balanceAfter, uint8_t reason, int64_t refListingID,
                      int64_t refOrderID, const std::string& idempotencyKey) {
        call("adjustPoints");
        if (!idempotencyKey.empty() && holdsKey(idempotencyKey))
            return false;

        const int balance = getPointBalance(account);
        if (balance + delta < 0)
            return false;

        balanceAfter = balance + delta;
        setPointBalance(account, balanceAfter);

        LedgerRow row;
        row.account = account;
        row.delta = delta;
        row.reason = reason;
        row.refListingID = refListingID;
        row.refOrderID = refOrderID;
        row.idempotencyKey = idempotencyKey;
        m_Ledger.push_back(row);
        return true;
    }

    int getPointBalance(const std::string& account) {
        for (const auto& point : m_Points) {
            if (point.account == account)
                return point.balance;
        }
        return 0;
    }

    bool hasIdempotencyKey(const std::string& idempotencyKey) {
        call("hasIdempotencyKey");
        return holdsKey(idempotencyKey);
    }

    bool beginTransaction() {
        call("beginTransaction");
        ++m_Begun;
        m_Saved = Tables{m_Listings, m_Orders, m_Points, m_Ledger};
        m_InTransaction = true;
        return true;
    }
    bool commit() {
        call("commit");
        ++m_Commits;
        m_InTransaction = false;
        return true;
    }
    bool rollback() {
        call("rollback");
        ++m_Rollbacks;
        if (m_InTransaction) {
            m_Listings = m_Saved.listings;
            m_Orders = m_Saved.orders;
            m_Points = m_Saved.points;
            m_Ledger = m_Saved.ledger;
            m_InTransaction = false;
        }
        for (const auto& row : m_CommittedElsewhere)
            m_Ledger.push_back(row);
        m_CommittedElsewhere.clear();
        return true;
    }

private:
    struct Account {
        std::string account;
        int balance;
    };

    struct Tables {
        std::vector<ExchangeListing> listings;
        std::vector<ExchangeOrder> orders;
        std::vector<Account> points;
        std::vector<LedgerRow> ledger;
    };

    // Count a call of the named method, and throw where failOn() asked.
    void call(const std::string& method) {
        const int nth = ++m_Calls[method];
        auto itr = m_FailAt.find(method);
        if (itr != m_FailAt.end() && itr->second == nth) {
            m_FailAt.erase(itr);
            throw DatabaseError(method + " : injected failure");
        }
    }

    bool holdsKey(const std::string& idempotencyKey) const {
        for (const auto& row : m_Ledger) {
            if (!row.idempotencyKey.empty() && row.idempotencyKey == idempotencyKey)
                return true;
        }
        return false;
    }

    void setStatusIf(int64_t listingID, uint8_t expected, uint8_t next) {
        for (auto& listing : m_Listings) {
            if (listing.listingID == listingID && listing.status == expected)
                listing.status = next;
        }
    }

    static std::vector<ExchangeListing> paged(const std::vector<ExchangeListing>& rows, int page, int pageSize) {
        std::vector<ExchangeListing> out;
        if (pageSize < 1 || page < 1)
            return out;
        const size_t offset = (size_t)(page - 1) * (size_t)pageSize;
        for (size_t i = offset; i < rows.size() && out.size() < (size_t)pageSize; ++i)
            out.push_back(rows[i]);
        return out;
    }

    std::vector<ExchangeListing> m_Listings;
    std::vector<ExchangeOrder> m_Orders;
    std::vector<Account> m_Points;
    std::vector<LedgerRow> m_Ledger;
    int64_t m_NextListingID = 1;
    int64_t m_NextOrderID = 1;
    int m_ListingLoads = 0;
    int m_Begun = 0;
    int m_Commits = 0;
    int m_Rollbacks = 0;
    bool m_InTransaction = false;
    Tables m_Saved;
    std::vector<LedgerRow> m_CommittedElsewhere;
    std::map<std::string, int> m_Calls;
    std::map<std::string, int> m_FailAt;
};

#endif // __FAKE_EXCHANGE_REPOSITORY_H__
