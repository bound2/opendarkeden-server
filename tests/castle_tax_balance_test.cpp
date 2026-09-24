// castle_tax_balance_test.cpp -- the compare-and-swap changes to a castle's
// tax balance (src/server/gameserver/CastleTaxBalance.h). Each reports the
// amount it actually moved the balance by, which is what the castle saves
// relatively, so the clamps at zero and at the maximum must show in it.

#include <atomic>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "CastleTaxBalance.h"

namespace {

TEST(CastleTaxBalance, ACreditBelowTheMaximumIsAppliedWhole) {
    std::atomic<Gold_t> balance(100);
    TaxBalanceChange change = creditTaxBalance(balance, 50);
    EXPECT_EQ(50u, change.applied);
    EXPECT_EQ(150u, change.balance);
    EXPECT_EQ(150u, balance.load());
}

TEST(CastleTaxBalance, ACreditStopsAtTheMaximumAndReportsOnlyWhatFit) {
    std::atomic<Gold_t> balance(GUILD_TAX_BALANCE_MAX - 30);
    TaxBalanceChange change = creditTaxBalance(balance, 100);
    EXPECT_EQ(30u, change.applied);
    EXPECT_EQ(GUILD_TAX_BALANCE_MAX, change.balance);
    EXPECT_EQ(GUILD_TAX_BALANCE_MAX, balance.load());

    change = creditTaxBalance(balance, 100);
    EXPECT_EQ(0u, change.applied);
    EXPECT_EQ(GUILD_TAX_BALANCE_MAX, balance.load());
}

TEST(CastleTaxBalance, ACreditNearTheTopOfTheTypeDoesNotWrap) {
    std::atomic<Gold_t> balance(10);
    TaxBalanceChange change = creditTaxBalance(balance, 0xFFFFFFFFu);
    EXPECT_EQ(GUILD_TAX_BALANCE_MAX - 10, change.applied);
    EXPECT_EQ(GUILD_TAX_BALANCE_MAX, balance.load());
}

TEST(CastleTaxBalance, ACreditUsesTheMaximumItIsGiven) {
    std::atomic<Gold_t> balance(90);
    TaxBalanceChange change = creditTaxBalance(balance, 50, 100);
    EXPECT_EQ(10u, change.applied);
    EXPECT_EQ(100u, change.balance);
}

TEST(CastleTaxBalance, ADebitStopsAtZeroAndReportsOnlyWhatWasThere) {
    std::atomic<Gold_t> balance(70);
    TaxBalanceChange change = debitTaxBalance(balance, 50);
    EXPECT_EQ(50u, change.applied);
    EXPECT_EQ(20u, change.balance);

    change = debitTaxBalance(balance, 50);
    EXPECT_EQ(20u, change.applied);
    EXPECT_EQ(0u, change.balance);
    EXPECT_EQ(0u, balance.load());

    change = debitTaxBalance(balance, 50);
    EXPECT_EQ(0u, change.applied);
    EXPECT_EQ(0u, balance.load());
}

TEST(CastleTaxBalance, TakingEmptiesTheBalanceAndReportsWhatItHeld) {
    std::atomic<Gold_t> balance(1234);
    TaxBalanceChange change = takeTaxBalance(balance);
    EXPECT_EQ(1234u, change.applied);
    EXPECT_EQ(0u, change.balance);
    EXPECT_EQ(0u, balance.load());
}

// What the castle saves is the sum of the applied amounts, so under
// contention, clamps included, that sum must be exactly where the balance
// ended.
TEST(CastleTaxBalance, TheAppliedAmountsOfConcurrentChangesSumToTheFinalBalance) {
    const Gold_t max = 5000;
    std::atomic<Gold_t> balance(2500);
    const int threads = 4;
    const int rounds = 20000;
    std::vector<long long> net(threads, 0);

    std::vector<std::thread> workers;
    for (int t = 0; t < threads; ++t) {
        workers.emplace_back([&, t] {
            for (int i = 0; i < rounds; ++i) {
                Gold_t amount = (Gold_t)(37 + (i * 7 + t * 13) % 400);
                if ((i + t) % 2 == 0)
                    net[t] += creditTaxBalance(balance, amount, max).applied;
                else
                    net[t] -= debitTaxBalance(balance, amount).applied;
                if (i % 5000 == 4999 && t == 0)
                    net[t] -= takeTaxBalance(balance).applied;
            }
        });
    }
    for (std::thread& worker : workers)
        worker.join();

    long long total = 2500;
    for (int t = 0; t < threads; ++t)
        total += net[t];

    EXPECT_EQ((long long)balance.load(), total);
    EXPECT_LE(balance.load(), max);
}

} // namespace
