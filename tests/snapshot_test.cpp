//////////////////////////////////////////////////////////////////////
//
// Filename    : snapshot_test.cpp
// Description : Pins de::Snapshot (src/server/Snapshot.h): the
//               copy-on-write table a ZoneGroup publishes its zone map
//               through, so a zone thread creating a dynamic zone can
//               insert into another group's map while that group, and
//               every transport on every other thread, keeps reading it.
//
//////////////////////////////////////////////////////////////////////

#include <atomic>
#include <cstddef>
#include <latch>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "Snapshot.h"

namespace {

using Table = std::map<int, std::string>;

TEST(Snapshot, StartsEmptyAndNeverNull) {
    de::Snapshot<Table> table;
    ASSERT_NE(table.load(), nullptr);
    EXPECT_TRUE(table.load()->empty());

    de::Snapshot<Table> seeded(Table{{1, "one"}});
    ASSERT_NE(seeded.load(), nullptr);
    EXPECT_EQ(seeded.load()->at(1), "one");
}

TEST(Snapshot, UpdatePublishesACopyAndLeavesHeldSnapshotsAlone) {
    de::Snapshot<Table> table(Table{{1, "one"}});
    de::Snapshot<Table>::Value before = table.load();

    table.update([](Table& t) { t[2] = "two"; });

    EXPECT_EQ(before->size(), 1u) << "a reader keeps the table it loaded";
    EXPECT_EQ(table.load()->size(), 2u);
    EXPECT_NE(before.get(), table.load().get()) << "the update is a new object, not an in-place change";
}

TEST(Snapshot, UpdateReturnsWhatTheChangeReturnsAndChainsOnThePublishedValue) {
    de::Snapshot<Table> table;
    bool inserted = table.update([](Table& t) { return t.emplace(1, "one").second; });
    EXPECT_TRUE(inserted);
    inserted = table.update([](Table& t) { return t.emplace(1, "again").second; });
    EXPECT_FALSE(inserted) << "the second writer saw the first writer's result";
    EXPECT_EQ(table.load()->at(1), "one");
}

TEST(Snapshot, AThrowingChangePublishesNothing) {
    de::Snapshot<Table> table(Table{{1, "one"}});
    EXPECT_THROW(table.update([](Table& t) {
        t[2] = "two";
        throw std::runtime_error("no");
    }),
                 std::runtime_error);
    EXPECT_EQ(table.load()->size(), 1u);
}

// Readers iterate whole tables while a writer keeps inserting; every table a
// reader sees must be internally consistent (keys 0..n-1, contiguous), and
// the readers must never block or crash. This is the zone-map scenario.
TEST(Snapshot, ReadersSeeConsistentTablesWhileAWriterInserts) {
    de::Snapshot<std::map<int, int>> table;
    constexpr int kInserts = 2000;
    constexpr int kReaders = 4;
    std::atomic<bool> done{false};
    std::atomic<int> inconsistent{0};
    std::atomic<long> tablesRead{0};
    std::latch start(kReaders + 1);

    std::vector<std::thread> readers;
    for (int r = 0; r < kReaders; ++r) {
        readers.emplace_back([&] {
            start.arrive_and_wait();
            while (!done.load(std::memory_order_relaxed)) {
                de::Snapshot<std::map<int, int>>::Value snap = table.load();
                int expected = 0;
                for (const auto& [key, value] : *snap) {
                    if (key != expected || value != key * 2)
                        inconsistent.fetch_add(1);
                    ++expected;
                }
                tablesRead.fetch_add(1);
            }
        });
    }

    std::thread writer([&] {
        start.arrive_and_wait();
        for (int i = 0; i < kInserts; ++i)
            table.update([i](std::map<int, int>& t) { t[i] = i * 2; });
        done.store(true);
    });

    writer.join();
    for (std::thread& t : readers)
        t.join();

    EXPECT_EQ(inconsistent.load(), 0);
    EXPECT_GT(tablesRead.load(), 0);
    EXPECT_EQ(table.load()->size(), static_cast<std::size_t>(kInserts));
}

} // namespace
