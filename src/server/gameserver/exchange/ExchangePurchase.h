//////////////////////////////////////////////////////////////////////////////
// Filename    : ExchangePurchase.h
// Description : the writes of an Exchange purchase, the transaction they run
//               in, and what each way of failing tells the buyer. Like
//               ExchangeDecision.h it needs nothing but a repository and
//               plain values, so the whole write path runs against the fake
//               repository in unit tests and the MySQL one in the
//               integration tier.
//////////////////////////////////////////////////////////////////////////////

#ifndef __EXCHANGE_PURCHASE_H__
#define __EXCHANGE_PURCHASE_H__

#include <cstdint>
#include <string>

#include "ExchangeDecision.h"
#include "Outcome.h"
#include "repository/ExchangeRepository.h"

// The order a buy wrote, and what it moved. The two balances are the ledger's
// answers after each leg, so a caller can report them without a second read.
struct ExchangePurchase {
    int64_t orderID = 0;
    int64_t listingID = 0;
    int pricePoint = 0;
    int taxAmount = 0;
    int totalCost = 0;
    int sellerIncome = 0;
    int buyerBalanceAfter = 0;
    int sellerBalanceAfter = 0;
};

//////////////////////////////////////////////////////////////////////////////
// The transaction guard
//
// Owns the repository's transaction pair for one scope. begin() opens it;
// commit() ends it by committing; every other way out of the scope - a
// return, a thrown DatabaseError or any other exception - ends it by
// rolling back. rollback() does that early, for a caller that must read
// the database again before it leaves the scope.
//
// The guard counts itself open from the moment begin() is called, not from
// the moment it returns, so a begin that fails part way (one connection
// started, the other refused) is still rolled back. A ROLLBACK on a
// connection with nothing begun is a no-op, and so is one on a connection
// whose COMMIT already went through, which is what makes the rollback safe
// after a commit that failed part way.
//
// The destructor swallows whatever its rollback throws: it may be running
// because of an exception already, and a rollback that fails on a lost
// connection has nothing left to undo - MySQL discards a transaction when
// its session ends.
//////////////////////////////////////////////////////////////////////////////

class ExchangeTransaction {
public:
    explicit ExchangeTransaction(ExchangeRepository& repository) : m_Repository(repository) {}
    ~ExchangeTransaction();

    ExchangeTransaction(const ExchangeTransaction&) = delete;
    ExchangeTransaction& operator=(const ExchangeTransaction&) = delete;

    // Open the pair. False, or a thrown DatabaseError, when it could not be
    // opened; the guard still rolls back whatever part of it began.
    bool begin();

    // Commit the pair. False, or a thrown DatabaseError, when the commit did
    // not go through; the guard then still rolls back whatever is left open.
    bool commit();

    // Roll the pair back now. Throws what the repository's rollback throws;
    // either way the guard counts the pair closed and will not roll it back
    // again.
    void rollback();

    bool isOpen() const {
        return m_Open;
    }

private:
    ExchangeRepository& m_Repository;
    bool m_Open = false;
};

//////////////////////////////////////////////////////////////////////////////
// Failures
//////////////////////////////////////////////////////////////////////////////

// The steps of a purchase, in the order they run.
enum class ExchangePurchaseStep {
    // The transaction pair's begin.
    Begin,
    // markListingSold: takes the listing out of ACTIVE.
    Claim,
    // createOrder.
    Order,
    // adjustPoints on the buyer, the "_buy" ledger row.
    BuyerDebit,
    // adjustPoints on the seller, the "_sale" ledger row.
    SellerCredit,
    // The transaction pair's commit.
    Commit
};

// How a step failed: it answered a refusal of its own (a false, or an id of
// 0), or it threw a DatabaseError.
enum class ExchangeStepFailure { Refused, Error };

// What a purchase that failed at this step tells the buyer, once both
// transactions are rolled back. legKeyRecorded says whether the ledger holds
// the failed leg's key after the rollback; it only matters for the two
// ledger steps.
//
//  - A claim that matched no ACTIVE row: the listing was sold, cancelled or
//    expired since the decision read it, EXCHANGE_FAIL_LISTING_NOT_AVAILABLE.
//  - A ledger step whose key the ledger now holds, refused or thrown: another
//    purchase recorded that key first, EXCHANGE_FAIL_IDEMPOTENCY_CONFLICT.
//  - The buyer's debit refused with the key absent: the balance does not
//    cover the cost, EXCHANGE_FAIL_INSUFFICIENT_POINTS.
//  - Anything else: EXCHANGE_FAIL_TRANSACTION_ERROR, with the step named in
//    the detail (a failed begin names none). The detail is the wire text
//    GCExchangeBuy carries; the database's own message stays in
//    DBError.log, where END_DB wrote it.
ExchangeRejection exchangePurchaseFailure(ExchangePurchaseStep step, ExchangeStepFailure failure, bool legKeyRecorded);

//////////////////////////////////////////////////////////////////////////////
// The purchase
//////////////////////////////////////////////////////////////////////////////

// Write an accepted purchase: claim the listing, write the order, debit the
// buyer and credit the seller, all inside one ExchangeTransaction, then
// commit. The order row carries orderServerID.
//
// Every failure rolls both transactions back before this returns, and comes
// back as the rejection exchangePurchaseFailure names for it: a DatabaseError
// from any step, the begin and the commit included, is caught for that and
// does not escape. Any other exception is not the database's answer and
// propagates, after the guard has rolled the pair back.
//
// The ledger-key probe that tells a collision from other failures reads the
// ledger after the rollback, so it sees only committed rows; a key is never
// removed from the ledger, so a key it finds stays found. A probe that fails
// itself counts as no key.
[[nodiscard]] Outcome<ExchangePurchase, ExchangeRejection> completeExchangePurchase(ExchangeRepository& repository,
                                                                                    const ExchangeBuyRequest& request,
                                                                                    const ExchangePurchaseTerms& terms,
                                                                                    int16_t orderServerID);

#endif // __EXCHANGE_PURCHASE_H__
