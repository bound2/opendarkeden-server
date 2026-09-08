#ifndef __FAKE_EXCHANGE_REPOSITORY_H__
#define __FAKE_EXCHANGE_REPOSITORY_H__

#include <algorithm>
#include <string>
#include <vector>

#include "GCExchangeList.h" // For ExchangeListing definition
#include "repository/ExchangeRepository.h"

// In-memory ExchangeRepository for domain tests.
// Models the four Exchange tables as the MySQL implementation behaves
// (src/server/gameserver/repository/ExchangeRepository.h is the authority on
// that contract; the MySQL-backed integration tier is what pins it):
//  - getListing() hands out a NEW listing the caller owns, or NULL when the
//    id names no row.
//  - The status writes (cancelListing, expireListing, markListingSold,
//    markOrderDelivered) only match a row in the status they expect, and
//    answer true whether or not one matched.
//  - adjustPoints() answers false, writing nothing, for a ledger key that is
//    already stored AND for a balance that would go below zero; the caller
//    cannot tell the two apart. An account with no row has a balance of 0.
//  - The listing and order lists come back newest first, which here means
//    reverse insertion order.
//  - The transaction trio is counted, not honoured: these tests are about
//    the decisions taken before a transaction opens.
//  - NOT modeled: the UNIQUE keys that make a second listing of one object,
//    or a second order on one listing, fail with ER_DUP_ENTRY; the SQL
//    failures that surface as a raw const char*; and the datetime columns,
//    which are strings the caller supplies here rather than the database's
//    own NOW(). Because there is no clock, getExpiredListings() answers
//    every active listing instead of comparing ExpireAt.
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
        for (auto& listing : m_Listings) {
            if (listing.listingID == listingID && listing.status == LISTING_STATUS_ACTIVE) {
                listing.status = LISTING_STATUS_SOLD;
                listing.buyerAccount = buyerAccount;
                listing.buyerPlayer = buyerPlayer;
            }
        }
        return true;
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
        if (!idempotencyKey.empty() && hasIdempotencyKey(idempotencyKey))
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
        for (const auto& row : m_Ledger) {
            if (!row.idempotencyKey.empty() && row.idempotencyKey == idempotencyKey)
                return true;
        }
        return false;
    }

    bool beginTransaction() {
        ++m_Begun;
        return true;
    }
    bool commit() {
        ++m_Commits;
        return true;
    }
    bool rollback() {
        ++m_Rollbacks;
        return true;
    }

private:
    struct Account {
        std::string account;
        int balance;
    };

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
};

#endif // __FAKE_EXCHANGE_REPOSITORY_H__
