//////////////////////////////////////////////////////////////////////////////
// Filename    : CastleTaxBalance.h
// Description : the arithmetic of a castle's tax balance. Shops, fees and
//               withdrawals move the balance from any zone thread, so each
//               change is a compare-and-swap that clamps the request to the
//               balance's range and reports what it actually moved. That
//               amount is what CastleInfoManager saves, relatively, so the
//               row stays the sum of the same changes the memory is, in
//               whatever order the saves land. Kept apart from the castle so
//               the clamps can be exercised without one.
//////////////////////////////////////////////////////////////////////////////

#ifndef __CASTLE_TAX_BALANCE_H__
#define __CASTLE_TAX_BALANCE_H__

#include <algorithm>
#include <atomic>

#include "Types.h"

struct TaxBalanceChange {
    Gold_t applied; // what the balance moved by: the request, clamped
    Gold_t balance; // the balance this change left
};

// Adds up to `tax`, stopping at `max`. A balance already at or above `max`
// takes nothing.
inline TaxBalanceChange creditTaxBalance(std::atomic<Gold_t>& balance, Gold_t tax, Gold_t max = GUILD_TAX_BALANCE_MAX) {
    Gold_t current = balance.load();
    Gold_t credit;
    do {
        credit = (current >= max) ? 0 : std::min(tax, max - current);
    } while (!balance.compare_exchange_weak(current, current + credit));

    return TaxBalanceChange{credit, current + credit};
}

// Takes up to `tax`, stopping at zero.
inline TaxBalanceChange debitTaxBalance(std::atomic<Gold_t>& balance, Gold_t tax) {
    Gold_t current = balance.load();
    Gold_t debit;
    do {
        debit = std::min(tax, current);
    } while (!balance.compare_exchange_weak(current, current - debit));

    return TaxBalanceChange{debit, current - debit};
}

// Empties the balance and reports what it held, which is the debit a reset
// saves: a credit made just before the reset is part of it, one made just
// after is not, and the row agrees either way.
inline TaxBalanceChange takeTaxBalance(std::atomic<Gold_t>& balance) {
    return TaxBalanceChange{balance.exchange(0), 0};
}

#endif // __CASTLE_TAX_BALANCE_H__
