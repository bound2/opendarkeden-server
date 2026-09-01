// MySQL-backed integration tier for the task 3.2 repositories
// (docs/RESTRUCTURING.md): runs the REAL MySQL implementations against a
// throwaway MySQL 5.7 loaded with the initdb/ schema and the production
// sql_mode. This is the authority the fakes in tests/support/ are
// corrected against — every quirk a fake pins is pinned HERE first,
// against the actual server (the 2026-09-01 adversarial review of PR #31
// falsified three fake-pinned claims exactly because no tier like this
// existed).
//
// Not part of the default ctest suite: it needs a database, which
// tests/integration/mysql_test.sh provides (a handrolled container
// fixture — start MySQL, wait for the schema import, run this binary on
// the same docker network, tear down). Run `make integration-test` from
// the host.
//
// Characters come from PlayerFixtures.h: ready-made low/mid/high level
// profiles per race. Each test persists exactly the profiles it wants —
// an unpersisted profile doubles as the "character with no rows" case.
//
// Wiring mirrors a zone thread: one world connection and one "dist"
// connection registered for this thread — two sockets, same server, same
// DARKEDEN schema, exactly like production (getDistConnection ignores its
// name argument; see MySQLGoodsRepository.cpp).

#include <cstdlib>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "PlayerFixtures.h"
#include "Thread.h"
#include "repository/BloodBibleSignRepository.h"
#include "repository/CharacterRepository.h"
#include "repository/GoldRepository.h"
#include "repository/GoodsRepository.h"
#include "repository/NicknameRepository.h"
#include "repository/RankBonusRepository.h"
#include "repository/SkillSaveRepository.h"
#include "repository/StashRepository.h"

namespace {

std::string env(const char* name, const char* fallback) {
    const char* value = getenv(name);
    return value != NULL && *value != '\0' ? value : fallback;
}

// --- RankBonusData against real MySQL -------------------------------------

class RankBonusMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
        execSQL("DELETE FROM RankBonusData WHERE OwnerID IN ('itvampmid', 'itslaymid')");
    }
};

TEST_F(RankBonusMySQL, LoadTypesComeBackTypeAscendingNotInsertionOrder) {
    // No ORDER BY on the query, but the covering index (OwnerID, Type)
    // fully serves it: InnoDB's index scan returns Type order.
    PlayerFixture vampire = PlayerFixtures::midLevelVampire();
    vampire.persist();

    RankBonusRepository& repository = defaultRankBonusRepository();
    repository.insert(vampire.name, 7);
    repository.insert(vampire.name, 3);

    std::vector<DWORD> types = repository.loadTypes(vampire.name);
    ASSERT_EQ(2u, types.size());
    EXPECT_EQ(3u, types[0]);
    EXPECT_EQ(7u, types[1]);
}

TEST_F(RankBonusMySQL, KeylessTableStoresDuplicatesAndDeleteOneRemovesThemAll) {
    PlayerFixture vampire = PlayerFixtures::midLevelVampire();
    vampire.persist();

    RankBonusRepository& repository = defaultRankBonusRepository();
    repository.insert(vampire.name, 7);
    repository.insert(vampire.name, 7); // no PK, no unique key: second row lands
    repository.insert(vampire.name, 3);
    ASSERT_EQ(3u, repository.loadTypes(vampire.name).size());

    repository.deleteOne(vampire.name, 7);

    std::vector<DWORD> types = repository.loadTypes(vampire.name);
    ASSERT_EQ(1u, types.size());
    EXPECT_EQ(3u, types[0]);
}

TEST_F(RankBonusMySQL, DeleteAllClearsOnlyThatOwner) {
    PlayerFixture vampire = PlayerFixtures::midLevelVampire();
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    vampire.persist();
    slayer.persist();

    RankBonusRepository& repository = defaultRankBonusRepository();
    repository.insert(vampire.name, 7);
    repository.insert(slayer.name, 7);

    repository.deleteAll(vampire.name);

    EXPECT_TRUE(repository.loadTypes(vampire.name).empty());
    EXPECT_EQ(1u, repository.loadTypes(slayer.name).size());
}

// --- stash columns against real MySQL -------------------------------------

class StashMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
    }
};

TEST_F(StashMySQL, NonOustersSaveWritesSlayerAndVampireTables) {
    // Character creation gives every non-Ousters character a Slayer AND a
    // Vampire row; the save hits both — the Slayer-unconditional quirk.
    PlayerFixture slayer = PlayerFixtures::lowLevelSlayer();
    slayer.persist();

    defaultStashRepository().saveStashGold(slayer.name, false, 1234);

    int gold = -1;
    ASSERT_TRUE(defaultStashRepository().loadStashGold(slayer.name, CHARACTER_RACE_SLAYER, gold));
    EXPECT_EQ(1234, gold);
    ASSERT_TRUE(defaultStashRepository().loadStashGold(slayer.name, CHARACTER_RACE_VAMPIRE, gold));
    EXPECT_EQ(1234, gold);
}

TEST_F(StashMySQL, OustersSaveWritesSlayerAndOustersTables) {
    PlayerFixture ousters = PlayerFixtures::midLevelOusters();
    ousters.persist();

    defaultStashRepository().saveStashNum(ousters.name, true, 3);

    EXPECT_EQ("3", queryScalar("SELECT StashNum FROM Slayer WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("3", queryScalar("SELECT StashNum FROM Ousters WHERE Name='" + ousters.name + "'"));
}

TEST_F(StashMySQL, SaveAgainstMissingRowsIsASilentNoOp) {
    // Deliberately NOT persisted: the profile stands in for a character
    // with no rows. The UPDATEs match zero rows — no error, no warning.
    PlayerFixture ghost = PlayerFixtures::highLevelVampire();

    defaultStashRepository().saveStashGold(ghost.name, false, 500);

    int gold = -1;
    EXPECT_FALSE(defaultStashRepository().loadStashGold(ghost.name, CHARACTER_RACE_VAMPIRE, gold));
}

TEST_F(StashMySQL, GoldAboveIntMaxClampsToZeroDestroyingTheBalance) {
    // The (int) cast emits a NEGATIVE literal; the UNSIGNED StashGold
    // column clamps it to 0 (warning 1264) under the non-strict sql_mode.
    // NOT stored negative — the balance is destroyed. Unreachable with
    // the MAX_MONEY cap, pinned anyway.
    PlayerFixture vampire = PlayerFixtures::highLevelVampire();
    vampire.persist();
    defaultStashRepository().saveStashGold(vampire.name, false, 5000); // a real balance first

    defaultStashRepository().saveStashGold(vampire.name, false, 4000000000u);

    int gold = -1;
    ASSERT_TRUE(defaultStashRepository().loadStashGold(vampire.name, CHARACTER_RACE_VAMPIRE, gold));
    EXPECT_EQ(0, gold);
}

// --- carried gold against real MySQL --------------------------------------

class GoldMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
    }
};

TEST_F(GoldMySQL, IncreaseAndDecreaseAreRelativeOnTheRowBalance) {
    PlayerFixture vampire = PlayerFixtures::midLevelVampire();
    vampire.persist(); // Vampire.Gold column default is 0

    defaultGoldRepository().increaseGold(vampire.name, CHARACTER_RACE_VAMPIRE, 500);
    defaultGoldRepository().decreaseGold(vampire.name, CHARACTER_RACE_VAMPIRE, 200);

    int gold = -1;
    ASSERT_TRUE(defaultGoldRepository().loadGold(vampire.name, CHARACTER_RACE_VAMPIRE, gold));
    EXPECT_EQ(300, gold);
}

TEST_F(GoldMySQL, OperationsTargetOnlyTheOwnRaceTableAndSlayerRowsStartAt2000) {
    // Unlike the stash writes, gold has NO Slayer fan-out — and the
    // Slayer table's Gold column DEFAULTS to 2000 while Vampire/Ousters
    // default to 0, so a fresh slayer's twin Vampire row sits at 0.
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    slayer.persist();

    defaultGoldRepository().increaseGold(slayer.name, CHARACTER_RACE_SLAYER, 500);

    int gold = -1;
    ASSERT_TRUE(defaultGoldRepository().loadGold(slayer.name, CHARACTER_RACE_SLAYER, gold));
    EXPECT_EQ(2500, gold);
    ASSERT_TRUE(defaultGoldRepository().loadGold(slayer.name, CHARACTER_RACE_VAMPIRE, gold));
    EXPECT_EQ(0, gold);
}

TEST_F(GoldMySQL, DecreaseBelowTheRowBalanceRaisesOutOfRangeAndLeavesTheRowUntouched) {
    // The caller clamps the delta against its IN-MEMORY balance; when the
    // row holds less (integrity drift), Gold - delta on the UNSIGNED
    // column raises ER_DATA_OUT_OF_RANGE (1690) — same failure shape as
    // taking a Num=0 goods row — and the row keeps its balance.
    PlayerFixture ousters = PlayerFixtures::lowLevelOusters();
    ousters.persist();
    defaultGoldRepository().increaseGold(ousters.name, CHARACTER_RACE_OUSTERS, 10);

    EXPECT_ANY_THROW(defaultGoldRepository().decreaseGold(ousters.name, CHARACTER_RACE_OUSTERS, 50));

    int gold = -1;
    ASSERT_TRUE(defaultGoldRepository().loadGold(ousters.name, CHARACTER_RACE_OUSTERS, gold));
    EXPECT_EQ(10, gold);
}

TEST_F(GoldMySQL, OperationsAgainstMissingRowsAreSilentNoOps) {
    PlayerFixture ghost = PlayerFixtures::highLevelSlayer(); // never persisted

    defaultGoldRepository().increaseGold(ghost.name, CHARACTER_RACE_SLAYER, 500);
    defaultGoldRepository().decreaseGold(ghost.name, CHARACTER_RACE_SLAYER, 500);

    int gold = -1;
    EXPECT_FALSE(defaultGoldRepository().loadGold(ghost.name, CHARACTER_RACE_SLAYER, gold));
}

// --- character-row saves against real MySQL -------------------------------
// CharacterRepository is a write-only seam with NO fake tier (maintainer's
// call — integration over fakes), so these tests carry the load: every
// written column is asserted with a distinct sentinel so argument
// transpositions cannot survive, and every dispatch branch is exercised.
// Known holes, stated honestly: the `Rank` backticks (load-bearing only
// on MySQL 8, where RANK is reserved — this tier runs 5.7) and the
// tinysave WHERE-casing byte-fidelity (immaterial to MySQL) are not
// testable here; a signature break surfaces in the gameserver build, not
// in the default ctest suite (nothing there compiles this seam).

class CharacterMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
    }
};

TEST_F(CharacterMySQL, SlayerVitalsLandInTheSlayerRowInFull) {
    // Every written column is asserted with a distinct sentinel: an
    // argument transposition in the impl cannot survive this test.
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    slayer.persist();

    SlayerVitalsRecord record;
    record.currentHP = 111;
    record.maxHP = 222;
    record.currentMP = 33;
    record.maxMP = 44;
    record.zoneID = 21;
    record.x = 100;
    record.y = 200;
    defaultCharacterRepository().saveSlayerVitals(slayer.name, record);

    EXPECT_EQ("111", queryScalar("SELECT CurrentHP FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("222", queryScalar("SELECT HP FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("33", queryScalar("SELECT CurrentMP FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("44", queryScalar("SELECT MP FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("21", queryScalar("SELECT ZoneID FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("100", queryScalar("SELECT XCoord FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("200", queryScalar("SELECT YCoord FROM Slayer WHERE Name='" + slayer.name + "'"));
}

TEST_F(CharacterMySQL, VampireVitalsCarrySilverDamageInsteadOfMPInFull) {
    PlayerFixture vampire = PlayerFixtures::midLevelVampire();
    vampire.persist();

    VampireVitalsRecord record;
    record.currentHP = 150;
    record.maxHP = 300;
    record.silverDamage = 12;
    record.zoneID = 23;
    record.x = 50;
    record.y = 60;
    defaultCharacterRepository().saveVampireVitals(vampire.name, record);

    EXPECT_EQ("150", queryScalar("SELECT CurrentHP FROM Vampire WHERE Name='" + vampire.name + "'"));
    EXPECT_EQ("300", queryScalar("SELECT HP FROM Vampire WHERE Name='" + vampire.name + "'"));
    EXPECT_EQ("12", queryScalar("SELECT SilverDamage FROM Vampire WHERE Name='" + vampire.name + "'"));
    EXPECT_EQ("23", queryScalar("SELECT ZoneID FROM Vampire WHERE Name='" + vampire.name + "'"));
    EXPECT_EQ("50", queryScalar("SELECT XCoord FROM Vampire WHERE Name='" + vampire.name + "'"));
    EXPECT_EQ("60", queryScalar("SELECT YCoord FROM Vampire WHERE Name='" + vampire.name + "'"));
}

TEST_F(CharacterMySQL, OustersVitalsLandInTheOustersRowInFull) {
    PlayerFixture ousters = PlayerFixtures::lowLevelOusters();
    ousters.persist();

    OustersVitalsRecord record;
    record.currentHP = 90;
    record.maxHP = 120;
    record.currentMP = 70;
    record.maxMP = 80;
    record.zoneID = 51;
    record.x = 10;
    record.y = 20;
    defaultCharacterRepository().saveOustersVitals(ousters.name, record);

    EXPECT_EQ("90", queryScalar("SELECT CurrentHP FROM Ousters WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("120", queryScalar("SELECT HP FROM Ousters WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("70", queryScalar("SELECT CurrentMP FROM Ousters WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("80", queryScalar("SELECT MP FROM Ousters WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("51", queryScalar("SELECT ZoneID FROM Ousters WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("10", queryScalar("SELECT XCoord FROM Ousters WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("20", queryScalar("SELECT YCoord FROM Ousters WHERE Name='" + ousters.name + "'"));
}

TEST_F(CharacterMySQL, SlayerExpsTailLandsInFull) {
    PlayerFixture slayer = PlayerFixtures::highLevelSlayer();
    slayer.persist();

    SlayerExpsRecord record;
    record.strGoalExp = 1001;
    record.dexGoalExp = 1002;
    record.intGoalExp = 1003;
    record.bladeGoalExp = 2001;
    record.swordGoalExp = 2002;
    record.gunGoalExp = 2003;
    record.enchantGoalExp = 2004;
    record.healGoalExp = 2005;
    record.etcGoalExp = 2006;
    record.alignment = -50;
    record.fame = 777;
    record.rank = 9;
    record.rankGoalExp = 3001;
    record.advancementClass = 4;
    record.advancementGoalExp = 4001;
    record.advancedSTR = 11;
    record.advancedDEX = 12;
    record.advancedINT = 13;
    record.advancedAttrBonus = 14;
    defaultCharacterRepository().saveSlayerExps(slayer.name, record);

    // all 19 written columns, distinct sentinels — transpositions die here
    EXPECT_EQ("1001", queryScalar("SELECT STRGoalExp FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("1002", queryScalar("SELECT DEXGoalExp FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("1003", queryScalar("SELECT INTGoalExp FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("2001", queryScalar("SELECT BladeGoalExp FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("2002", queryScalar("SELECT SwordGoalExp FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("2003", queryScalar("SELECT GunGoalExp FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("2004", queryScalar("SELECT EnchantGoalExp FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("2005", queryScalar("SELECT HealGoalExp FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("2006", queryScalar("SELECT ETCGoalExp FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("-50", queryScalar("SELECT Alignment FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("777", queryScalar("SELECT Fame FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("9", queryScalar("SELECT `Rank` FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("3001", queryScalar("SELECT RankGoalExp FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("4", queryScalar("SELECT AdvancementClass FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("4001", queryScalar("SELECT AdvancementGoalExp FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("11", queryScalar("SELECT AdvancedSTR FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("12", queryScalar("SELECT AdvancedDEX FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("13", queryScalar("SELECT AdvancedINT FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("14", queryScalar("SELECT Bonus FROM Slayer WHERE Name='" + slayer.name + "'"));
}

TEST_F(CharacterMySQL, VampireExpsSkipSilverDamageWhenZero) {
    // The original composed an optional ",SilverDamage = %d" fragment: a
    // zero value leaves the column UNTOUCHED — this save cannot reset a
    // vampire's silver damage to zero.
    PlayerFixture vampire = PlayerFixtures::highLevelVampire();
    vampire.persist();
    execSQL("UPDATE Vampire SET SilverDamage = 7 WHERE Name='" + vampire.name + "'");

    VampireExpsRecord record;
    record.alignment = 10;
    record.fame = 55;
    record.goalExp = 900;
    record.silverDamage = 0;
    record.rank = 3;
    record.rankGoalExp = 800;
    record.advancementClass = 1;
    record.advancementGoalExp = 700;
    defaultCharacterRepository().saveVampireExps(vampire.name, record);
    EXPECT_EQ("7", queryScalar("SELECT SilverDamage FROM Vampire WHERE Name='" + vampire.name + "'"));
    // the other seven written columns, distinct sentinels
    EXPECT_EQ("10", queryScalar("SELECT Alignment FROM Vampire WHERE Name='" + vampire.name + "'"));
    EXPECT_EQ("55", queryScalar("SELECT Fame FROM Vampire WHERE Name='" + vampire.name + "'"));
    EXPECT_EQ("900", queryScalar("SELECT GoalExp FROM Vampire WHERE Name='" + vampire.name + "'"));
    EXPECT_EQ("3", queryScalar("SELECT `Rank` FROM Vampire WHERE Name='" + vampire.name + "'"));
    EXPECT_EQ("800", queryScalar("SELECT RankGoalExp FROM Vampire WHERE Name='" + vampire.name + "'"));
    EXPECT_EQ("1", queryScalar("SELECT AdvancementClass FROM Vampire WHERE Name='" + vampire.name + "'"));
    EXPECT_EQ("700", queryScalar("SELECT AdvancementGoalExp FROM Vampire WHERE Name='" + vampire.name + "'"));

    record.silverDamage = 5;
    defaultCharacterRepository().saveVampireExps(vampire.name, record);
    EXPECT_EQ("5", queryScalar("SELECT SilverDamage FROM Vampire WHERE Name='" + vampire.name + "'"));
}

TEST_F(CharacterMySQL, OustersExpsAlwaysWriteSilverDamage) {
    // Unlike the vampire's conditional fragment, the ousters save writes
    // SilverDamage unconditionally — zero included.
    PlayerFixture ousters = PlayerFixtures::midLevelOusters();
    ousters.persist();
    execSQL("UPDATE Ousters SET SilverDamage = 7 WHERE Name='" + ousters.name + "'");

    OustersExpsRecord record;
    record.alignment = 10;
    record.fame = 55;
    record.goalExp = 900;
    record.silverDamage = 0;
    record.rank = 3;
    record.rankGoalExp = 800;
    record.advancementClass = 1;
    record.advancementGoalExp = 700;
    defaultCharacterRepository().saveOustersExps(ousters.name, record);

    EXPECT_EQ("0", queryScalar("SELECT SilverDamage FROM Ousters WHERE Name='" + ousters.name + "'"));
    // the other seven written columns, distinct sentinels
    EXPECT_EQ("10", queryScalar("SELECT Alignment FROM Ousters WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("55", queryScalar("SELECT Fame FROM Ousters WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("900", queryScalar("SELECT GoalExp FROM Ousters WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("3", queryScalar("SELECT `Rank` FROM Ousters WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("800", queryScalar("SELECT RankGoalExp FROM Ousters WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("1", queryScalar("SELECT AdvancementClass FROM Ousters WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("700", queryScalar("SELECT AdvancementGoalExp FROM Ousters WHERE Name='" + ousters.name + "'"));
}

TEST_F(CharacterMySQL, TinysaveSlayerBranchHitsOnlyTheSlayerTable) {
    // tinysave is the seam's only dispatching method (~400 call sites,
    // including the absolute setGoldEx writes): every branch gets its
    // own test, each asserting the target row changed AND the twin row
    // did not.
    PlayerFixture slayer = PlayerFixtures::lowLevelSlayer();
    slayer.persist(); // Slayer + twin Vampire row

    defaultCharacterRepository().tinysave(slayer.name, CHARACTER_RACE_SLAYER, "StashNum=9");

    EXPECT_EQ("9", queryScalar("SELECT StashNum FROM Slayer WHERE Name='" + slayer.name + "'"));
    EXPECT_EQ("0", queryScalar("SELECT StashNum FROM Vampire WHERE Name='" + slayer.name + "'"));
}

TEST_F(CharacterMySQL, TinysaveVampireBranchHitsOnlyTheVampireTable) {
    PlayerFixture vampire = PlayerFixtures::lowLevelVampire();
    vampire.persist(); // Slayer + Vampire rows

    defaultCharacterRepository().tinysave(vampire.name, CHARACTER_RACE_VAMPIRE, "StashNum=9");

    EXPECT_EQ("9", queryScalar("SELECT StashNum FROM Vampire WHERE Name='" + vampire.name + "'"));
    EXPECT_EQ("0", queryScalar("SELECT StashNum FROM Slayer WHERE Name='" + vampire.name + "'"));
}

TEST_F(CharacterMySQL, TinysaveOustersBranchHitsOnlyTheOustersTable) {
    PlayerFixture ousters = PlayerFixtures::lowLevelOusters();
    ousters.persist(); // Slayer + Ousters rows

    defaultCharacterRepository().tinysave(ousters.name, CHARACTER_RACE_OUSTERS, "StashNum=9");

    EXPECT_EQ("9", queryScalar("SELECT StashNum FROM Ousters WHERE Name='" + ousters.name + "'"));
    EXPECT_EQ("0", queryScalar("SELECT StashNum FROM Slayer WHERE Name='" + ousters.name + "'"));
}

// --- character-row loads against real MySQL -------------------------------
// The loads are POSITIONAL (column N of the SELECT lands in field N of
// the record), so each test sets every column to its own SELECT position
// as the sentinel: a transposed pair in the column list or the reader
// swaps two values and fails here. Name is the one column left as
// persisted (it is the key).

TEST_F(CharacterMySQL, LoadSlayerReturnsEveryColumnInSelectPosition) {
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    slayer.persist();
    execSQL("UPDATE Slayer SET AdvancementClass=2, AdvancementGoalExp=3, Competence=4, CompetenceShape=5, "
            "Sex='FEMALE', MasterEffectColor=7, HairStyle='HAIR_STYLE2', HairColor=9, SkinColor=10, "
            "Phone='1234567', STR=12, STRGoalExp=13, DEX=14, DEXGoalExp=15, INTE=16, INTGoalExp=17, "
            "AdvancedSTR=18, AdvancedDEX=19, AdvancedINT=20, Bonus=21, `Rank`=22, RankGoalExp=23, CurrentHP=24, "
            "HP=25, CurrentMP=26, MP=27, Fame=28, Gold=29, GuildID=30, BladeLevel=31, BladeGoalExp=32, "
            "SwordLevel=33, SwordGoalExp=34, GunLevel=35, GunGoalExp=36, EnchantLevel=37, EnchantGoalExp=38, "
            "HealLevel=39, HealGoalExp=40, ETCLevel=41, ETCGoalExp=42, ZoneID=43, XCoord=44, YCoord=45, Sight=46, "
            "GunBonusExp=47, RifleBonusExp=48, Alignment=-49, StashGold=50, StashNum=51, ResurrectZone=52, "
            "Reward=-53, SMSCharge=54 WHERE Name='" +
            slayer.name + "'");

    SlayerLoadRecord record;
    ASSERT_TRUE(defaultCharacterRepository().loadSlayer(slayer.name, record));
    EXPECT_EQ(slayer.name, record.name);
    EXPECT_EQ(2, record.advancementClass);
    EXPECT_EQ(3, record.advancementGoalExp);
    EXPECT_EQ(4, record.competence);
    EXPECT_EQ(5, record.competenceShape);
    EXPECT_EQ("FEMALE", record.sex);
    EXPECT_EQ(7, record.masterEffectColor);
    EXPECT_EQ("HAIR_STYLE2", record.hairStyle);
    EXPECT_EQ(9, record.hairColor);
    EXPECT_EQ(10, record.skinColor);
    EXPECT_EQ("1234567", record.phone);
    EXPECT_EQ(12, record.str);
    EXPECT_EQ(13, record.strGoalExp);
    EXPECT_EQ(14, record.dex);
    EXPECT_EQ(15, record.dexGoalExp);
    EXPECT_EQ(16, record.inte);
    EXPECT_EQ(17, record.intGoalExp);
    EXPECT_EQ(18, record.advancedSTR);
    EXPECT_EQ(19, record.advancedDEX);
    EXPECT_EQ(20, record.advancedINT);
    EXPECT_EQ(21, record.bonus);
    EXPECT_EQ(22, record.rank);
    EXPECT_EQ(23, record.rankGoalExp);
    EXPECT_EQ(24, record.currentHP);
    EXPECT_EQ(25, record.maxHP);
    EXPECT_EQ(26, record.currentMP);
    EXPECT_EQ(27, record.maxMP);
    EXPECT_EQ(28, record.fame);
    EXPECT_EQ(29, record.gold);
    EXPECT_EQ(30, record.guildID);
    EXPECT_EQ(31, record.bladeLevel);
    EXPECT_EQ(32, record.bladeGoalExp);
    EXPECT_EQ(33, record.swordLevel);
    EXPECT_EQ(34, record.swordGoalExp);
    EXPECT_EQ(35, record.gunLevel);
    EXPECT_EQ(36, record.gunGoalExp);
    EXPECT_EQ(37, record.enchantLevel);
    EXPECT_EQ(38, record.enchantGoalExp);
    EXPECT_EQ(39, record.healLevel);
    EXPECT_EQ(40, record.healGoalExp);
    EXPECT_EQ(41, record.etcLevel);
    EXPECT_EQ(42, record.etcGoalExp);
    EXPECT_EQ(43, record.zoneID);
    EXPECT_EQ(44, record.x);
    EXPECT_EQ(45, record.y);
    EXPECT_EQ(46, record.sight);
    EXPECT_EQ(47, record.gunBonusExp);
    EXPECT_EQ(48, record.rifleBonusExp);
    EXPECT_EQ(-49, record.alignment);
    EXPECT_EQ(50, record.stashGold);
    EXPECT_EQ(51, record.stashNum);
    EXPECT_EQ(52, record.resurrectZone);
    EXPECT_EQ(-53, record.reward);
    EXPECT_EQ(54, record.smsCharge);
}

TEST_F(CharacterMySQL, LoadVampireReturnsEveryColumnInSelectPosition) {
    PlayerFixture vampire = PlayerFixtures::midLevelVampire();
    vampire.persist();
    execSQL("UPDATE Vampire SET AdvancementClass=2, AdvancementGoalExp=3, Sex='FEMALE', MasterEffectColor=5, "
            "BatColor=6, SkinColor=7, STR=8, DEX=9, INTE=10, HP=11, CurrentHP=12, Fame=13, GoalExp=14, Level=15, "
            "Bonus=16, Gold=17, GuildID=18, ZoneID=19, XCoord=20, YCoord=21, Sight=22, Alignment=-23, "
            "StashGold=24, StashNum=25, Competence=26, CompetenceShape=27, ResurrectZone=28, SilverDamage=29, "
            "Reward=-30, SMSCharge=31, `Rank`=32, RankGoalExp=33 WHERE Name='" +
            vampire.name + "'");

    VampireLoadRecord record;
    ASSERT_TRUE(defaultCharacterRepository().loadVampire(vampire.name, record));
    EXPECT_EQ(vampire.name, record.name);
    EXPECT_EQ(2, record.advancementClass);
    EXPECT_EQ(3, record.advancementGoalExp);
    EXPECT_EQ("FEMALE", record.sex);
    EXPECT_EQ(5, record.masterEffectColor);
    EXPECT_EQ(6, record.batColor);
    EXPECT_EQ(7, record.skinColor);
    EXPECT_EQ(8, record.str);
    EXPECT_EQ(9, record.dex);
    EXPECT_EQ(10, record.inte);
    EXPECT_EQ(11, record.maxHP);
    EXPECT_EQ(12, record.currentHP);
    EXPECT_EQ(13, record.fame);
    EXPECT_EQ(14, record.goalExp);
    EXPECT_EQ(15, record.level);
    EXPECT_EQ(16, record.bonus);
    EXPECT_EQ(17, record.gold);
    EXPECT_EQ(18, record.guildID);
    EXPECT_EQ(19, record.zoneID);
    EXPECT_EQ(20, record.x);
    EXPECT_EQ(21, record.y);
    EXPECT_EQ(22, record.sight);
    EXPECT_EQ(-23, record.alignment);
    EXPECT_EQ(24, record.stashGold);
    EXPECT_EQ(25, record.stashNum);
    EXPECT_EQ(26, record.competence);
    EXPECT_EQ(27, record.competenceShape);
    EXPECT_EQ(28, record.resurrectZone);
    EXPECT_EQ(29, record.silverDamage);
    EXPECT_EQ(-30, record.reward);
    EXPECT_EQ(31, record.smsCharge);
    EXPECT_EQ(32, record.rank);
    EXPECT_EQ(33, record.rankGoalExp);
}

TEST_F(CharacterMySQL, LoadOustersReturnsEveryColumnInSelectPosition) {
    PlayerFixture ousters = PlayerFixtures::midLevelOusters();
    ousters.persist();
    execSQL("UPDATE Ousters SET AdvancementClass=2, AdvancementGoalExp=3, Sex='FEMALE', MasterEffectColor=5, "
            "STR=6, DEX=7, INTE=8, HP=9, CurrentHP=10, MP=11, CurrentMP=12, Fame=13, GoalExp=14, Level=15, "
            "Bonus=16, SkillBonus=17, Gold=18, GuildID=19, ZoneID=20, XCoord=21, YCoord=22, Sight=23, "
            "Alignment=-24, StashGold=25, StashNum=26, Competence=27, CompetenceShape=28, ResurrectZone=29, "
            "SilverDamage=30, SMSCharge=31, `Rank`=32, RankGoalExp=33, HairColor=34 WHERE Name='" +
            ousters.name + "'");

    OustersLoadRecord record;
    ASSERT_TRUE(defaultCharacterRepository().loadOusters(ousters.name, record));
    EXPECT_EQ(ousters.name, record.name);
    EXPECT_EQ(2, record.advancementClass);
    EXPECT_EQ(3, record.advancementGoalExp);
    EXPECT_EQ("FEMALE", record.sex);
    EXPECT_EQ(5, record.masterEffectColor);
    EXPECT_EQ(6, record.str);
    EXPECT_EQ(7, record.dex);
    EXPECT_EQ(8, record.inte);
    EXPECT_EQ(9, record.maxHP);
    EXPECT_EQ(10, record.currentHP);
    EXPECT_EQ(11, record.maxMP);
    EXPECT_EQ(12, record.currentMP);
    EXPECT_EQ(13, record.fame);
    EXPECT_EQ(14, record.goalExp);
    EXPECT_EQ(15, record.level);
    EXPECT_EQ(16, record.bonus);
    EXPECT_EQ(17, record.skillBonus);
    EXPECT_EQ(18, record.gold);
    EXPECT_EQ(19, record.guildID);
    EXPECT_EQ(20, record.zoneID);
    EXPECT_EQ(21, record.x);
    EXPECT_EQ(22, record.y);
    EXPECT_EQ(23, record.sight);
    EXPECT_EQ(-24, record.alignment);
    EXPECT_EQ(25, record.stashGold);
    EXPECT_EQ(26, record.stashNum);
    EXPECT_EQ(27, record.competence);
    EXPECT_EQ(28, record.competenceShape);
    EXPECT_EQ(29, record.resurrectZone);
    EXPECT_EQ(30, record.silverDamage);
    EXPECT_EQ(31, record.smsCharge);
    EXPECT_EQ(32, record.rank);
    EXPECT_EQ(33, record.rankGoalExp);
    EXPECT_EQ(34, record.hairColor);
}

TEST_F(CharacterMySQL, LoadsSkipInactiveRowsAndMissingRows) {
    // An INACTIVE row is what the login server leaves behind when a
    // character is deleted; a load must treat it exactly like no row.
    PlayerFixture slayer = PlayerFixtures::lowLevelSlayer();
    PlayerFixture vampire = PlayerFixtures::lowLevelVampire();
    PlayerFixture ousters = PlayerFixtures::lowLevelOusters();
    slayer.persist();
    vampire.persist();
    ousters.persist();
    execSQL("UPDATE Slayer SET Active='INACTIVE' WHERE Name='" + slayer.name + "'");
    execSQL("UPDATE Vampire SET Active='INACTIVE' WHERE Name='" + vampire.name + "'");
    execSQL("UPDATE Ousters SET Active='INACTIVE' WHERE Name='" + ousters.name + "'");

    SlayerLoadRecord slayerRecord;
    VampireLoadRecord vampireRecord;
    OustersLoadRecord oustersRecord;
    EXPECT_FALSE(defaultCharacterRepository().loadSlayer(slayer.name, slayerRecord));
    EXPECT_FALSE(defaultCharacterRepository().loadVampire(vampire.name, vampireRecord));
    EXPECT_FALSE(defaultCharacterRepository().loadOusters(ousters.name, oustersRecord));

    PlayerFixture ghost = PlayerFixtures::highLevelOusters(); // never persisted
    EXPECT_FALSE(defaultCharacterRepository().loadSlayer(ghost.name, slayerRecord));
    EXPECT_FALSE(defaultCharacterRepository().loadVampire(ghost.name, vampireRecord));
    EXPECT_FALSE(defaultCharacterRepository().loadOusters(ghost.name, oustersRecord));
}

TEST_F(CharacterMySQL, LoadSlayerFindsTheTwinRowEveryCharacterGetsAtCreation) {
    // Character creation writes a Slayer row for every race, so a
    // vampire's name loads as an ACTIVE slayer too (in production the
    // twin row carries the character's real stats — CLCreatePCHandler
    // writes STR/DEX/INTE/HP/Sight/Gold into it; only this fixture
    // leaves it at the column defaults). The race chosen at login
    // decides which loader runs — nothing in the rows does.
    PlayerFixture vampire = PlayerFixtures::midLevelVampire();
    vampire.persist();

    SlayerLoadRecord record;
    ASSERT_TRUE(defaultCharacterRepository().loadSlayer(vampire.name, record));
    EXPECT_EQ(2000, record.gold); // the Slayer table's Gold default
    EXPECT_EQ(0, record.swordLevel);
}

// --- SkillSave / VampireSkillSave / OustersSkillSave against real MySQL ---
// Like CharacterRepository, a seam with NO fake tier: these tests are the
// net. Every inserted column is read back through the load, every
// update asserts both the columns it writes and the ones it must leave
// alone, and the row order the ORDER-BY-less loads produce is pinned
// against the real server rather than assumed.

class SkillSaveMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
        execSQL(std::string("DELETE FROM SkillSave WHERE OwnerID IN ") + PlayerFixtures::nameList());
        execSQL(std::string("DELETE FROM VampireSkillSave WHERE OwnerID IN ") + PlayerFixtures::nameList());
        execSQL(std::string("DELETE FROM OustersSkillSave WHERE OwnerID IN ") + PlayerFixtures::nameList());
    }

    static SlayerSkillRecord slayerSkill(SkillType_t type, ExpLevel_t level, Exp_t exp, Turn_t delay,
                                         Turn_t castingTime, time_t nextTime) {
        SlayerSkillRecord record;
        record.skillType = type;
        record.skillLevel = level;
        record.skillExp = exp;
        record.delay = delay;
        record.castingTime = castingTime;
        record.nextTime = nextTime;
        return record;
    }

    static VampireSkillRecord vampireSkill(SkillType_t type, Turn_t delay, Turn_t castingTime, time_t nextTime) {
        VampireSkillRecord record;
        record.skillType = type;
        record.delay = delay;
        record.castingTime = castingTime;
        record.nextTime = nextTime;
        return record;
    }

    static OustersSkillRecord oustersSkill(SkillType_t type, ExpLevel_t level, Turn_t delay, Turn_t castingTime,
                                           time_t nextTime) {
        OustersSkillRecord record;
        record.skillType = type;
        record.skillLevel = level;
        record.delay = delay;
        record.castingTime = castingTime;
        record.nextTime = nextTime;
        return record;
    }
};

TEST_F(SkillSaveMySQL, SlayerInsertThenLoadRoundTripsEveryColumn) {
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    slayer.persist();

    defaultSkillSaveRepository().insertSlayerSkill(slayer.name, slayerSkill(101, 2, 3003, 44, 55, 1700000006));

    std::vector<SlayerSkillRow> rows = defaultSkillSaveRepository().loadSlayerSkills(slayer.name);
    ASSERT_EQ(1u, rows.size());
    EXPECT_EQ(101, rows[0].skillType);
    EXPECT_EQ(2, rows[0].skillLevel);
    EXPECT_EQ(3003, rows[0].skillExp);
    EXPECT_EQ(44, rows[0].delay);
    EXPECT_EQ(55, rows[0].castingTime);
    EXPECT_EQ(1700000006, rows[0].nextTime);
}

TEST_F(SkillSaveMySQL, LoadReturnsEveryRowIncludingDuplicateTypes) {
    // The contract: every row comes back, duplicates of a type included
    // (the keyless table cannot refuse them). Asserted as a multiset —
    // the order is deliberately NOT part of this test, see the next one.
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    slayer.persist();

    defaultSkillSaveRepository().insertSlayerSkill(slayer.name, slayerSkill(7, 1, 0, 11, 0, 0));
    defaultSkillSaveRepository().insertSlayerSkill(slayer.name, slayerSkill(3, 1, 0, 22, 0, 0));
    defaultSkillSaveRepository().insertSlayerSkill(slayer.name, slayerSkill(7, 1, 0, 33, 0, 0));

    std::vector<SlayerSkillRow> rows = defaultSkillSaveRepository().loadSlayerSkills(slayer.name);
    ASSERT_EQ(3u, rows.size());
    int seenType7Delay11 = 0, seenType3Delay22 = 0, seenType7Delay33 = 0;
    for (size_t r = 0; r < rows.size(); r++) {
        if (rows[r].skillType == 7 && rows[r].delay == 11)
            seenType7Delay11++;
        else if (rows[r].skillType == 3 && rows[r].delay == 22)
            seenType3Delay22++;
        else if (rows[r].skillType == 7 && rows[r].delay == 33)
            seenType7Delay33++;
        else
            ADD_FAILURE() << "unexpected row " << rows[r].skillType << "/" << rows[r].delay;
    }
    EXPECT_EQ(1, seenType7Delay11);
    EXPECT_EQ(1, seenType3Delay22);
    EXPECT_EQ(1, seenType7Delay33);
}

TEST_F(SkillSaveMySQL, LoadOrderObservedOnThe57TierIsInsertionOrderNotSkillTypeOrder) {
    // An OBSERVATION, not a contract: no ORDER BY and no primary key, so
    // the row order is whatever access path the optimizer picks. The
    // first draft asserted SkillType-ascending order (reasoning from the
    // (OwnerID, SkillType) secondary index) and the real MySQL 5.7
    // FALSIFIED it: on this tier's near-empty table — where the WHERE
    // matches essentially every row and the index does not cover the
    // SELECT — the rows come back in insertion order, a scan in
    // hidden-row-id order. A populated table, or MySQL 8 (supported, but
    // not what this tier runs), may reorder. If this test fails on such
    // a configuration, update the observation — do NOT "fix" a loader.
    // Why it matters at all: the vampire/ousters loaders keep whichever
    // duplicate of a type arrives FIRST, so that choice is plan-dependent.
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    slayer.persist();

    defaultSkillSaveRepository().insertSlayerSkill(slayer.name, slayerSkill(7, 1, 0, 11, 0, 0));
    defaultSkillSaveRepository().insertSlayerSkill(slayer.name, slayerSkill(3, 1, 0, 22, 0, 0));
    defaultSkillSaveRepository().insertSlayerSkill(slayer.name, slayerSkill(7, 1, 0, 33, 0, 0));

    std::vector<SlayerSkillRow> rows = defaultSkillSaveRepository().loadSlayerSkills(slayer.name);
    ASSERT_EQ(3u, rows.size());
    EXPECT_EQ(7, rows[0].skillType);
    EXPECT_EQ(11, rows[0].delay);
    EXPECT_EQ(3, rows[1].skillType);
    EXPECT_EQ(22, rows[1].delay);
    EXPECT_EQ(7, rows[2].skillType);
    EXPECT_EQ(33, rows[2].delay);
}

TEST_F(SkillSaveMySQL, SlayerUpdateWritesLevelExpAndDelayOnly) {
    PlayerFixture slayer = PlayerFixtures::highLevelSlayer();
    slayer.persist();
    defaultSkillSaveRepository().insertSlayerSkill(slayer.name, slayerSkill(101, 2, 3003, 44, 55, 1700000006));
    defaultSkillSaveRepository().insertSlayerSkill(slayer.name, slayerSkill(102, 1, 10, 20, 30, 40)); // untouched

    defaultSkillSaveRepository().updateSlayerSkill(slayer.name, 101, 9, 9999, 88);

    std::vector<SlayerSkillRow> rows = defaultSkillSaveRepository().loadSlayerSkills(slayer.name);
    ASSERT_EQ(2u, rows.size());
    // select each row by type — the load order is not a contract
    const SlayerSkillRow& updated = rows[0].skillType == 101 ? rows[0] : rows[1];
    const SlayerSkillRow& untouched = rows[0].skillType == 101 ? rows[1] : rows[0];
    ASSERT_EQ(101, updated.skillType);
    ASSERT_EQ(102, untouched.skillType);
    EXPECT_EQ(9, updated.skillLevel);
    EXPECT_EQ(9999, updated.skillExp);
    EXPECT_EQ(88, updated.delay);
    EXPECT_EQ(55, updated.castingTime);      // not part of the update
    EXPECT_EQ(1700000006, updated.nextTime); // not part of the update
    EXPECT_EQ(1, untouched.skillLevel);
    EXPECT_EQ(10, untouched.skillExp);
    EXPECT_EQ(20, untouched.delay);
}

TEST_F(SkillSaveMySQL, VampireInsertLoadAndUpdateDelayOnly) {
    PlayerFixture vampire = PlayerFixtures::midLevelVampire();
    vampire.persist();

    defaultSkillSaveRepository().insertVampireSkill(vampire.name, vampireSkill(201, 10, 20, 1700000030));

    std::vector<VampireSkillRow> rows = defaultSkillSaveRepository().loadVampireSkills(vampire.name);
    ASSERT_EQ(1u, rows.size());
    EXPECT_EQ(201, rows[0].skillType);
    EXPECT_EQ(10, rows[0].delay);
    EXPECT_EQ(20, rows[0].castingTime);
    EXPECT_EQ(1700000030, rows[0].nextTime);

    defaultSkillSaveRepository().updateVampireSkill(vampire.name, 201, 77);

    rows = defaultSkillSaveRepository().loadVampireSkills(vampire.name);
    ASSERT_EQ(1u, rows.size());
    EXPECT_EQ(77, rows[0].delay);
    EXPECT_EQ(20, rows[0].castingTime);
    EXPECT_EQ(1700000030, rows[0].nextTime);
}

TEST_F(SkillSaveMySQL, OustersInsertLandsSkillLevelDespiteItsTrailingPositionInTheInsert) {
    // The ousters INSERT names SkillLevel after NextTime — the column and
    // value lists agree, so the level must land in SkillLevel and not in
    // the neighbouring NextTime.
    PlayerFixture ousters = PlayerFixtures::midLevelOusters();
    ousters.persist();

    defaultSkillSaveRepository().insertOustersSkill(ousters.name, oustersSkill(301, 4, 10, 20, 1700000040));

    std::vector<OustersSkillRow> rows = defaultSkillSaveRepository().loadOustersSkills(ousters.name);
    ASSERT_EQ(1u, rows.size());
    EXPECT_EQ(301, rows[0].skillType);
    EXPECT_EQ(4, rows[0].skillLevel);
    EXPECT_EQ(10, rows[0].delay);
    EXPECT_EQ(20, rows[0].castingTime);
    EXPECT_EQ(1700000040, rows[0].nextTime);
}

TEST_F(SkillSaveMySQL, OustersUpdateWritesLevelAndDelayOnly) {
    PlayerFixture ousters = PlayerFixtures::highLevelOusters();
    ousters.persist();
    defaultSkillSaveRepository().insertOustersSkill(ousters.name, oustersSkill(301, 4, 10, 20, 1700000040));

    defaultSkillSaveRepository().updateOustersSkill(ousters.name, 301, 6, 66);

    std::vector<OustersSkillRow> rows = defaultSkillSaveRepository().loadOustersSkills(ousters.name);
    ASSERT_EQ(1u, rows.size());
    EXPECT_EQ(6, rows[0].skillLevel);
    EXPECT_EQ(66, rows[0].delay);
    EXPECT_EQ(20, rows[0].castingTime);
    EXPECT_EQ(1700000040, rows[0].nextTime);
}

TEST_F(SkillSaveMySQL, OustersDeleteRemovesOnlyThatTypeForThatOwner) {
    PlayerFixture ousters = PlayerFixtures::lowLevelOusters();
    PlayerFixture other = PlayerFixtures::midLevelOusters();
    ousters.persist();
    other.persist();
    defaultSkillSaveRepository().insertOustersSkill(ousters.name, oustersSkill(301, 1, 0, 0, 0));
    defaultSkillSaveRepository().insertOustersSkill(ousters.name, oustersSkill(301, 2, 0, 0, 0)); // duplicate
    defaultSkillSaveRepository().insertOustersSkill(ousters.name, oustersSkill(302, 1, 0, 0, 0));
    defaultSkillSaveRepository().insertOustersSkill(other.name, oustersSkill(301, 1, 0, 0, 0));

    defaultSkillSaveRepository().deleteOustersSkill(ousters.name, 301);

    std::vector<OustersSkillRow> rows = defaultSkillSaveRepository().loadOustersSkills(ousters.name);
    ASSERT_EQ(1u, rows.size()); // both 301 rows went, the keyless table has no way to pick one
    EXPECT_EQ(302, rows[0].skillType);
    EXPECT_EQ(1u, defaultSkillSaveRepository().loadOustersSkills(other.name).size());
}

TEST_F(SkillSaveMySQL, WritesAgainstMissingRowsAreSilentNoOps) {
    PlayerFixture ghost = PlayerFixtures::highLevelVampire(); // never persisted

    defaultSkillSaveRepository().updateSlayerSkill(ghost.name, 101, 1, 1, 1);
    defaultSkillSaveRepository().updateVampireSkill(ghost.name, 201, 1);
    defaultSkillSaveRepository().updateOustersSkill(ghost.name, 301, 1, 1);
    defaultSkillSaveRepository().deleteOustersSkill(ghost.name, 301);

    EXPECT_TRUE(defaultSkillSaveRepository().loadSlayerSkills(ghost.name).empty());
    EXPECT_TRUE(defaultSkillSaveRepository().loadVampireSkills(ghost.name).empty());
    EXPECT_TRUE(defaultSkillSaveRepository().loadOustersSkills(ghost.name).empty());
}

// --- BloodBibleSignObject against real MySQL ------------------------------

class BloodBibleSignMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
        execSQL("DELETE FROM BloodBibleSignObject WHERE OwnerID = 'itvamphi'");
    }
};

TEST_F(BloodBibleSignMySQL, LoadReturnsItemTypeAscending) {
    PlayerFixture vampire = PlayerFixtures::highLevelVampire();
    vampire.persist();
    // Rows arrive from OUTSIDE the server process (the interface is
    // read-only on purpose) — seeded here the way the web side would.
    execSQL("INSERT INTO BloodBibleSignObject (ItemType, OwnerID) VALUES (5, '" + vampire.name + "'), (2, '" +
            vampire.name + "'), (9, '" + vampire.name + "')");

    std::vector<ItemType_t> itemTypes = defaultBloodBibleSignRepository().loadItemTypes(vampire.name);
    ASSERT_EQ(3u, itemTypes.size());
    EXPECT_EQ(2, itemTypes[0]);
    EXPECT_EQ(5, itemTypes[1]);
    EXPECT_EQ(9, itemTypes[2]);
}

TEST_F(BloodBibleSignMySQL, DuplicateItemTypesSurvive) {
    PlayerFixture vampire = PlayerFixtures::highLevelVampire();
    vampire.persist();
    execSQL("INSERT INTO BloodBibleSignObject (ItemType, OwnerID) VALUES (5, '" + vampire.name + "'), (5, '" +
            vampire.name + "')");

    EXPECT_EQ(2u, defaultBloodBibleSignRepository().loadItemTypes(vampire.name).size());
}

// --- GoodsListObject against real MySQL -----------------------------------

class GoodsMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
        execSQL("DELETE FROM GoodsListObject WHERE PlayerID = 'itaccount'");
    }

    static void seed(const std::string& id, int world, const std::string& name, int num, const std::string& status) {
        char sql[240];
        sprintf(sql,
                "INSERT INTO GoodsListObject (BuyID, ID, World, PlayerID, Name, GoodsID, Num, Status) "
                "VALUES ('itbuy', %s, %d, 'itaccount', '%s', 5000, %d, '%s')",
                id.c_str(), world, name.c_str(), num, status.c_str());
        execSQL(sql);
    }
};

TEST_F(GoodsMySQL, PendingIsFilteredByWorldPlayerNameAndStatus) {
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    PlayerFixture ousters = PlayerFixtures::lowLevelOusters();
    slayer.persist();
    ousters.persist();

    seed("9101", 1, slayer.name, 2, "NOT");
    seed("9102", 2, slayer.name, 2, "NOT");  // other world
    seed("9103", 1, ousters.name, 2, "NOT"); // other character
    seed("9104", 1, slayer.name, 2, "GET");  // already taken

    std::vector<GoodsRecord> records = defaultGoodsRepository().loadPending(1, "itaccount", slayer.name);
    ASSERT_EQ(1u, records.size());
    EXPECT_EQ("9101", records[0].id);
    EXPECT_EQ(5000u, records[0].goodsID);
    EXPECT_EQ(2, records[0].num);
}

TEST_F(GoodsMySQL, TakeOneDecrementsAndFlipsStatusOnTheLastUnit) {
    // One UPDATE, left-to-right SET evaluation: the IF() reads the
    // already-decremented Num, so the last unit flips Status to 'GET' in
    // the statement that takes it.
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    slayer.persist();
    seed("9101", 1, slayer.name, 2, "NOT");

    EXPECT_TRUE(defaultGoodsRepository().takeOne("9101"));
    std::vector<GoodsRecord> records = defaultGoodsRepository().loadPending(1, "itaccount", slayer.name);
    ASSERT_EQ(1u, records.size());
    EXPECT_EQ(1, records[0].num);

    EXPECT_TRUE(defaultGoodsRepository().takeOne("9101"));
    EXPECT_TRUE(defaultGoodsRepository().loadPending(1, "itaccount", slayer.name).empty());
    EXPECT_EQ("GET", queryScalar("SELECT Status FROM GoodsListObject WHERE ID=9101"));
    EXPECT_EQ("0", queryScalar("SELECT Num FROM GoodsListObject WHERE ID=9101"));
}

TEST_F(GoodsMySQL, TakingAZeroCountRowFailsAndLeavesItUntouched) {
    // Num - 1 on the UNSIGNED column raises ER_DATA_OUT_OF_RANGE (1690)
    // regardless of strict mode; the row is untouched and the error
    // escapes as an exception (through END_DB, as a raw const char*) —
    // the stuck-item path documented on MySQLGoodsRepository.
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    slayer.persist();
    seed("9101", 1, slayer.name, 0, "NOT");

    EXPECT_ANY_THROW(defaultGoodsRepository().takeOne("9101"));

    EXPECT_EQ("0", queryScalar("SELECT Num FROM GoodsListObject WHERE ID=9101"));
    EXPECT_EQ("NOT", queryScalar("SELECT Status FROM GoodsListObject WHERE ID=9101"));
}

TEST_F(GoodsMySQL, TakeOneOfAnUnknownIdIsFalse) {
    EXPECT_FALSE(defaultGoodsRepository().takeOne("999999"));
}

// --- NicknameBook against real MySQL (the 3.2 pilot) ----------------------

class NicknameMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
        execSQL("DELETE FROM NicknameBook WHERE OwnerID = 'itoustmid'");
    }
};

TEST_F(NicknameMySQL, LoadReturnsNIDAscendingNotInsertionOrder) {
    // The pilot's fake documented "insertion order" for this ORDER-BY-less
    // SELECT; flagged in the PR #31 review as unverified. Reality: the
    // secondary index IDX_OwnerID carries the primary key (nID, OwnerID)
    // as its suffix, so the ref scan returns nID ascending.
    PlayerFixture ousters = PlayerFixtures::midLevelOusters();
    ousters.persist();

    NicknameRepository& repository = defaultNicknameRepository();
    repository.insert(ousters.name, 10001, NicknameInfo::NICK_CUSTOM, "second-id-first");
    repository.insert(ousters.name, 10000, NicknameInfo::NICK_CUSTOM, "first-id-second");

    std::vector<NicknameRecord> records = repository.load(ousters.name);
    ASSERT_EQ(2u, records.size());
    EXPECT_EQ(10000, records[0].id);
    EXPECT_EQ(10001, records[1].id);
}

} // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    std::string host = env("IT_DB_HOST", "127.0.0.1");
    std::string db = env("IT_DB_DB", "DARKEDEN");
    std::string user = env("IT_DB_USER", "elcastle");
    std::string password = env("IT_DB_PASSWORD", "elca110");
    uint port = (uint)atoi(env("IT_DB_PORT", "3306").c_str());

    g_pDatabaseManager = new DatabaseManager();
    g_pDatabaseManager->addConnection((int)(long)Thread::self(), new Connection(host, db, user, password, port));
    g_pDatabaseManager->addDistConnection((int)(long)Thread::self(), new Connection(host, db, user, password, port));

    return RUN_ALL_TESTS();
}
