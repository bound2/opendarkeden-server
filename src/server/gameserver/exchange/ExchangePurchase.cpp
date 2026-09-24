//////////////////////////////////////////////////////////////////////////////
// Filename    : ExchangePurchase.cpp
// Description : the writes of an Exchange purchase, the transaction they run
//               in, and what each way of failing tells the buyer.
//////////////////////////////////////////////////////////////////////////////

#include "ExchangePurchase.h"

#include "DatabaseError.h"

//////////////////////////////////////////////////////////////////////////////
// The transaction guard
//////////////////////////////////////////////////////////////////////////////

ExchangeTransaction::~ExchangeTransaction() {
    if (!m_Open)
        return;

    try {
        m_Repository.rollback();
    } catch (...) {
        // See the class comment: nothing is left open when a rollback fails.
    }
}

bool ExchangeTransaction::begin() {
    // Open before the call: a begin that throws after starting anything
    // still leaves that to be rolled back.
    m_Open = true;
    return m_Repository.beginTransaction();
}

bool ExchangeTransaction::commit() {
    if (!m_Repository.commit())
        return false;

    m_Open = false;
    return true;
}

void ExchangeTransaction::rollback() {
    if (!m_Open)
        return;

    m_Open = false;
    m_Repository.rollback();
}

//////////////////////////////////////////////////////////////////////////////
// Failures
//////////////////////////////////////////////////////////////////////////////

namespace {

// The detail a transaction error names its step with. These are wire text
// (GCExchangeBuy's message), so they keep the wording the client has seen.
const char* stepDetail(ExchangePurchaseStep step) {
    switch (step) {
    case ExchangePurchaseStep::Begin:
        return "";
    case ExchangePurchaseStep::Claim:
        return "Failed to mark listing sold";
    case ExchangePurchaseStep::Order:
        return "Failed to create order";
    case ExchangePurchaseStep::BuyerDebit:
        return "Failed to deduct buyer points";
    case ExchangePurchaseStep::SellerCredit:
        return "Failed to add seller points";
    case ExchangePurchaseStep::Commit:
        return "Failed to commit transaction";
    }
    return "";
}

// Whether the ledger holds this key. A probe the database fails counts as
// no: the purchase is refused either way, and only the reason differs.
bool ledgerHolds(ExchangeRepository& repository, const std::string& key) {
    try {
        return repository.hasIdempotencyKey(key);
    } catch (const DatabaseError&) {
        return false;
    }
}

} // namespace

ExchangeRejection exchangePurchaseFailure(ExchangePurchaseStep step, ExchangeStepFailure failure, bool legKeyRecorded) {
    const bool ledgerStep = step == ExchangePurchaseStep::BuyerDebit || step == ExchangePurchaseStep::SellerCredit;

    if (step == ExchangePurchaseStep::Claim && failure == ExchangeStepFailure::Refused)
        return ExchangeRejection(EXCHANGE_FAIL_LISTING_NOT_AVAILABLE);

    if (ledgerStep && legKeyRecorded)
        return ExchangeRejection(EXCHANGE_FAIL_IDEMPOTENCY_CONFLICT);

    if (step == ExchangePurchaseStep::BuyerDebit && failure == ExchangeStepFailure::Refused)
        return ExchangeRejection(EXCHANGE_FAIL_INSUFFICIENT_POINTS);

    return ExchangeRejection(EXCHANGE_FAIL_TRANSACTION_ERROR, stepDetail(step));
}

//////////////////////////////////////////////////////////////////////////////
// The purchase
//////////////////////////////////////////////////////////////////////////////

Outcome<ExchangePurchase, ExchangeRejection> completeExchangePurchase(ExchangeRepository& repository,
                                                                      const ExchangeBuyRequest& request,
                                                                      const ExchangePurchaseTerms& terms,
                                                                      int16_t orderServerID) {
    typedef Outcome<ExchangePurchase, ExchangeRejection> Result;

    const std::string buyKey = exchangeLedgerKey(request.idempotencyKey, kExchangeBuyLedgerSuffix);
    const std::string saleKey = exchangeLedgerKey(request.idempotencyKey, kExchangeSaleLedgerSuffix);

    ExchangePurchase purchase;
    purchase.listingID = request.listingID;
    purchase.pricePoint = terms.pricePoint;
    purchase.taxAmount = terms.taxAmount;
    purchase.totalCost = terms.totalCost;
    purchase.sellerIncome = terms.sellerIncome;

    // The repository stamps CreatedAt itself; the order's datetimes are not
    // read on insert.
    ExchangeOrder order;
    order.orderID = 0;
    order.listingID = request.listingID;
    order.serverID = orderServerID;
    order.buyerAccount = request.buyerAccount;
    order.buyerPlayer = request.buyerPlayer;
    order.pricePoint = terms.pricePoint;
    order.taxAmount = terms.taxAmount;
    order.status = ORDER_STATUS_PAID;

    ExchangeTransaction transaction(repository);
    ExchangePurchaseStep step = ExchangePurchaseStep::Begin;

    // The steps in order; false is the refusal of the step named in `step`.
    auto write = [&]() -> bool {
        if (!transaction.begin())
            return false;

        // The claim comes first. Its UPDATE matches only an ACTIVE row and
        // holds that row's lock until the pair ends, so a second buyer of
        // the listing - on this server or another of the world - waits for
        // this purchase and then matches nothing.
        step = ExchangePurchaseStep::Claim;
        if (!repository.markListingSold(request.listingID, request.buyerAccount, request.buyerPlayer))
            return false;

        step = ExchangePurchaseStep::Order;
        purchase.orderID = repository.createOrder(order);
        if (purchase.orderID <= 0)
            return false;

        step = ExchangePurchaseStep::BuyerDebit;
        if (!repository.adjustPoints(request.buyerAccount, -terms.totalCost, purchase.buyerBalanceAfter,
                                     POINT_REASON_BUY, request.listingID, 0, buyKey))
            return false;

        step = ExchangePurchaseStep::SellerCredit;
        if (!repository.adjustPoints(terms.sellerAccount, terms.sellerIncome, purchase.sellerBalanceAfter,
                                     POINT_REASON_SALE, request.listingID, 0, saleKey))
            return false;

        step = ExchangePurchaseStep::Commit;
        return transaction.commit();
    };

    ExchangeStepFailure failure = ExchangeStepFailure::Refused;
    try {
        if (write())
            return Result::Ok(purchase);
    } catch (const DatabaseError&) {
        failure = ExchangeStepFailure::Error;
    }

    // Roll back before probing, so the probe sees only what other purchases
    // committed and never this one's own rows.
    try {
        transaction.rollback();
    } catch (const DatabaseError&) {
        // A ROLLBACK fails only with its session, which takes the
        // transaction with it.
    }

    bool legKeyRecorded = false;
    if (step == ExchangePurchaseStep::BuyerDebit)
        legKeyRecorded = ledgerHolds(repository, buyKey);
    else if (step == ExchangePurchaseStep::SellerCredit)
        legKeyRecorded = ledgerHolds(repository, saleKey);

    return Result::Rejected(exchangePurchaseFailure(step, failure, legKeyRecorded));
}
