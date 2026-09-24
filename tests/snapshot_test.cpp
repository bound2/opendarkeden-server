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
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
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

// The string pool's shape (src/server/gameserver/StringPool.h): the whole
// table is replaced on a reload, and a reader is handed a const char* into
// the strings of the table it read. That pointer must survive the reload, so
// every table the pool has published is kept for the life of the pool. This
// pins that it is the retained table, not the snapshot slot, that keeps it
// valid: a pointer taken before the swap still reads the old text afterwards,
// while the pool itself reads the new one.
TEST(Snapshot, ARetainedTableKeepsAPointerIntoItValidAcrossAReplacement) {
    using Pool = std::map<int, std::string>;
    de::Snapshot<Pool> pool(Pool{{1, "the old text"}});
    std::vector<std::shared_ptr<const Pool>> retained;

    const char* before = pool.load()->at(1).c_str();
    ASSERT_STREQ(before, "the old text");

    // Replace the whole table, keeping the one it replaced.
    retained.push_back(pool.load());
    pool.update([](Pool& next) { next = Pool{{1, "the new text"}}; });

    EXPECT_STREQ(before, "the old text") << "the pointer a reader still holds reads what it always read";
    EXPECT_STREQ(pool.load()->at(1).c_str(), "the new text");
    EXPECT_NE(before, pool.load()->at(1).c_str()) << "the reload really did replace the string";
}

// Readers keep taking pointers into the pool and reading through them while
// it is replaced under them; with every replaced table retained, none of them
// reads freed memory or half a string.
TEST(Snapshot, ReadersDereferencePointersWhileTheTableIsReplaced) {
    using Pool = std::map<int, std::string>;
    de::Snapshot<Pool> pool(Pool{{1, "generation 0"}});
    std::mutex retainedMutex;
    std::vector<std::shared_ptr<const Pool>> retained;

    constexpr int kReloads = 200;
    constexpr int kReaders = 4;
    std::atomic<bool> done{false};
    std::atomic<int> wrong{0};
    std::atomic<long> readsDone{0};
    std::latch start(kReaders + 1);

    std::vector<std::thread> readers;
    for (int r = 0; r < kReaders; ++r) {
        readers.emplace_back([&] {
            start.arrive_and_wait();
            while (!done.load(std::memory_order_relaxed)) {
                // Take the pointer the way a c_str() caller does -- letting
                // the snapshot it came from go -- then read through it.
                const char* text = pool.load()->at(1).c_str();
                if (std::string(text).rfind("generation ", 0) != 0)
                    wrong.fetch_add(1);
                readsDone.fetch_add(1);
            }
        });
    }

    std::thread writer([&] {
        start.arrive_and_wait();
        for (int i = 1; i <= kReloads; ++i) {
            std::shared_ptr<const Pool> replaced = pool.load();
            pool.update([i](Pool& next) { next = Pool{{1, "generation " + std::to_string(i)}}; });
            std::lock_guard<std::mutex> lock(retainedMutex);
            retained.push_back(std::move(replaced));
        }
        done.store(true);
    });

    writer.join();
    for (std::thread& t : readers)
        t.join();

    EXPECT_EQ(wrong.load(), 0);
    EXPECT_GT(readsDone.load(), 0);
    EXPECT_EQ(retained.size(), static_cast<std::size_t>(kReloads));
}

} // namespace
