// The bound the connection managers walk their descriptor-indexed player
// tables with (src/server/DescriptorTable.h). The managers seed the range
// from their listening socket and grow it with every accepted connection, so
// the clamp is what keeps a sweep from reading past the table.

#include <vector>

#include <gtest/gtest.h>

#include "DescriptorTable.h"

namespace {

// Runs a walk the way the managers write it and reports every index it
// touches, so an out-of-table index shows up as a value, not as a crash.
std::vector<int> visited(int minFD, int maxFD, int tableSize) {
    const de::DescriptorRange walk = de::descriptorRange(minFD, maxFD, tableSize);
    std::vector<int> indices;
    for (int i = walk.first; i <= walk.last; i++)
        indices.push_back(i);
    return indices;
}

} // namespace

TEST(DescriptorRange, AWholeRangeInsideTheTableIsWalkedUnchanged) {
    const de::DescriptorRange walk = de::descriptorRange(3, 7, 100);
    EXPECT_FALSE(walk.empty());
    EXPECT_EQ(3, walk.first);
    EXPECT_EQ(7, walk.last);
    EXPECT_EQ(std::vector<int>({3, 4, 5, 6, 7}), visited(3, 7, 100));
}

TEST(DescriptorRange, TheLastIndexIsTheLastSlotTheTableHas) {
    const de::DescriptorRange walk = de::descriptorRange(0, 99, 100);
    EXPECT_EQ(99, walk.last);
}

TEST(DescriptorRange, AMaximumBeyondTheTableStopsAtTheLastSlot) {
    const de::DescriptorRange walk = de::descriptorRange(2, 4096, 100);
    EXPECT_EQ(2, walk.first);
    EXPECT_EQ(99, walk.last);
    EXPECT_EQ(100u - 2u, visited(2, 4096, 100).size());
    EXPECT_EQ(99, visited(2, 4096, 100).back());
}

// The defect this bound was written for: a listening socket at or past the
// table seeds both ends of the range, and every sweep then indexed the table
// with it.
TEST(DescriptorRange, AListenerPastTheTableLeavesNothingToWalk) {
    EXPECT_TRUE(de::descriptorRange(100, 100, 100).empty());
    EXPECT_TRUE(visited(100, 100, 100).empty());
    EXPECT_TRUE(de::descriptorRange(4096, 4096, 100).empty());
    EXPECT_TRUE(visited(4096, 4096, 100).empty());
}

TEST(DescriptorRange, TheNoConnectionMarkerWalksNothing) {
    EXPECT_TRUE(de::descriptorRange(-1, -1, 100).empty());
    EXPECT_TRUE(visited(-1, -1, 100).empty());
}

TEST(DescriptorRange, ANegativeMinimumNeverBecomesAnIndex) {
    const de::DescriptorRange walk = de::descriptorRange(-1, 2, 100);
    EXPECT_EQ(0, walk.first);
    EXPECT_EQ(std::vector<int>({0, 1, 2}), visited(-1, 2, 100));
}

TEST(DescriptorRange, AnInvertedRangeWalksNothing) {
    EXPECT_TRUE(de::descriptorRange(7, 3, 100).empty());
    EXPECT_TRUE(visited(7, 3, 100).empty());
}

TEST(DescriptorRange, ATableWithNoSlotsWalksNothing) {
    EXPECT_TRUE(de::descriptorRange(0, 10, 0).empty());
    EXPECT_TRUE(visited(0, 10, 0).empty());
}

TEST(DescriptorRange, TheClampIsAConstantExpression) {
    static_assert(de::descriptorRange(2, 4096, 100).last == 99);
    static_assert(de::descriptorRange(100, 100, 100).empty());
    static_assert(!de::descriptorRange(0, 0, 100).empty());
    SUCCEED();
}

TEST(FitsDescriptorTable, OnlyTheSlotsTheTableHasFit) {
    EXPECT_TRUE(de::fitsDescriptorTable(0, 100));
    EXPECT_TRUE(de::fitsDescriptorTable(99, 100));
    EXPECT_FALSE(de::fitsDescriptorTable(100, 100));
    EXPECT_FALSE(de::fitsDescriptorTable(-1, 100));
    EXPECT_FALSE(de::fitsDescriptorTable(0, 0));
}
