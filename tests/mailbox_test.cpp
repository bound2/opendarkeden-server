//////////////////////////////////////////////////////////////////////
//
// Filename    : mailbox_test.cpp
// Description : Pins de::Mailbox (src/server/Mailbox.h), the queue a
//               ZoneGroup drains at the top of its tick so work posted
//               from other threads runs under the group mutex.
//
//////////////////////////////////////////////////////////////////////

#include <atomic>
#include <cstddef>
#include <latch>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "Mailbox.h"

namespace {

TEST(Mailbox, DrainRunsCommandsInPostingOrderOnTheDrainingThread) {
    de::Mailbox box;
    std::vector<int> seen;
    std::thread::id ranOn;
    box.post([&] { seen.push_back(1); });
    box.post([&] {
        seen.push_back(2);
        ranOn = std::this_thread::get_id();
    });
    EXPECT_EQ(box.size(), 2u);

    EXPECT_EQ(box.drain(), 2u);
    EXPECT_EQ(seen, (std::vector<int>{1, 2}));
    EXPECT_EQ(ranOn, std::this_thread::get_id());
    EXPECT_EQ(box.size(), 0u);
    EXPECT_EQ(box.drain(), 0u);
}

TEST(Mailbox, CommandsPostedWhileDrainingWaitForTheNextDrain) {
    de::Mailbox box;
    int nested = 0;
    box.post([&] { box.post([&] { ++nested; }); });

    EXPECT_EQ(box.drain(), 1u);
    EXPECT_EQ(nested, 0) << "a command posted by a command must not run in the same batch";
    EXPECT_EQ(box.size(), 1u);
    EXPECT_EQ(box.drain(), 1u);
    EXPECT_EQ(nested, 1);
}

TEST(Mailbox, FailureHandlerSeesTheExceptionAndTheRestStillRun) {
    de::Mailbox box;
    std::vector<std::string> log;
    box.post([&] { log.push_back("first"); });
    box.post([] { throw std::runtime_error("boom"); });
    box.post([&] { log.push_back("third"); });

    std::size_t ran = box.drain([&] {
        try {
            throw;
        } catch (const std::runtime_error& e) {
            log.push_back(std::string("failed: ") + e.what());
        }
    });

    EXPECT_EQ(ran, 3u);
    EXPECT_EQ(log, (std::vector<std::string>{"first", "failed: boom", "third"}));
}

TEST(Mailbox, WithoutAHandlerTheExceptionPropagatesAndDropsTheBatch) {
    de::Mailbox box;
    bool third = false;
    box.post([] {});
    box.post([] { throw std::runtime_error("boom"); });
    box.post([&] { third = true; });

    EXPECT_THROW(box.drain(), std::runtime_error);
    EXPECT_FALSE(third) << "the batch was taken out of the box before running";
    EXPECT_EQ(box.size(), 0u);
}

TEST(Mailbox, ProducersOnOtherThreadsLoseNothing) {
    de::Mailbox box;
    constexpr int kProducers = 4;
    constexpr int kPerProducer = 500;
    std::atomic<int> sum{0};
    std::latch start(kProducers);

    std::vector<std::thread> producers;
    for (int p = 0; p < kProducers; ++p) {
        producers.emplace_back([&, p] {
            start.arrive_and_wait();
            for (int i = 0; i < kPerProducer; ++i)
                box.post([&sum, p] { sum += p + 1; });
        });
    }
    // Drain concurrently with the producers; whatever is left is drained
    // once they are done. Every posted command must run exactly once.
    std::size_t ran = 0;
    while (ran < kProducers * kPerProducer) {
        ran += box.drain();
        std::this_thread::yield();
    }
    for (std::thread& t : producers)
        t.join();

    EXPECT_EQ(ran, static_cast<std::size_t>(kProducers * kPerProducer));
    EXPECT_EQ(box.drain(), 0u);
    EXPECT_EQ(sum.load(), kPerProducer * (1 + 2 + 3 + 4));
}

} // namespace
