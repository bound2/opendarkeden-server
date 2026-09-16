// characterRaceOf() decides, for every persistent write a player creature
// makes, which of the three race tables the row is addressed to. Before the
// races shared one implementation each class spelled its own constant, so a
// wrong answer here is a whole race writing into another race's table.

#include <gtest/gtest.h>

#include "PlayerRace.h"

TEST(PlayerRaceTableTest, EachPlayerRaceMapsToItsOwnTable) {
    EXPECT_EQ(CHARACTER_RACE_SLAYER, characterRaceOf(RACE_SLAYER));
    EXPECT_EQ(CHARACTER_RACE_VAMPIRE, characterRaceOf(RACE_VAMPIRE));
    EXPECT_EQ(CHARACTER_RACE_OUSTERS, characterRaceOf(RACE_OUSTERS));
}

TEST(PlayerRaceTableTest, TablesAreDistinct) {
    EXPECT_NE(characterRaceOf(RACE_SLAYER), characterRaceOf(RACE_VAMPIRE));
    EXPECT_NE(characterRaceOf(RACE_SLAYER), characterRaceOf(RACE_OUSTERS));
    EXPECT_NE(characterRaceOf(RACE_VAMPIRE), characterRaceOf(RACE_OUSTERS));
}

TEST(PlayerRaceTableTest, TableNamesMatchTheSchema) {
    EXPECT_STREQ("Slayer", characterRaceTable(characterRaceOf(RACE_SLAYER)));
    EXPECT_STREQ("Vampire", characterRaceTable(characterRaceOf(RACE_VAMPIRE)));
    EXPECT_STREQ("Ousters", characterRaceTable(characterRaceOf(RACE_OUSTERS)));
}

TEST(PlayerRaceTableTest, AnUnknownRaceIsAnsweredAsSlayer) {
    // Race_t is a BYTE, so values outside RaceType are representable; the
    // mapping stays total rather than falling off the end of the switch.
    EXPECT_EQ(CHARACTER_RACE_SLAYER, characterRaceOf((Race_t)200));
}
