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
#include "repository/BalanceInfoRepository.h"
#include "repository/BloodBibleSignRepository.h"
#include "repository/BulletinBoardRepository.h"
#include "repository/CharacterRepository.h"
#include "repository/ComebackEventRepository.h"
#include "repository/ContentInfoRepository.h"
#include "repository/EffectSaveRepository.h"
#include "repository/FlagSetRepository.h"
#include "repository/GameInfoRepository.h"
#include "repository/GoldRepository.h"
#include "repository/GoodsRepository.h"
#include "repository/GuildRepository.h"
#include "repository/ItemRepository.h"
#include "repository/MessageRepository.h"
#include "repository/NicknameRepository.h"
#include "repository/PlayRecordRepository.h"
#include "repository/QuestItemRepository.h"
#include "repository/RankBonusRepository.h"
#include "repository/RegenZoneRepository.h"
#include "repository/SMSAddressRepository.h"
#include "repository/SessionRepository.h"
#include "repository/SkillSaveRepository.h"
#include "repository/StashRepository.h"
#include "repository/WarInfoRepository.h"
#include "repository/ZoneInfoRepository.h"

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

// --- the persisted-effect tables against real MySQL -----------------------
// EffectSaveRepository has no fake tier; the format strings are per-table
// data, so every table gets exercised, and the two structural quirks —
// the one keyed table among eight keyless ones, and EnemyErase's
// owner-wide UPDATE — are pinned against the real server.

const char* const DEADLINE_TABLE_NAMES[DEADLINE_EFFECT_TABLE_MAX] = {"EffectAftermath", "EffectKillAftermath",
                                                                     "EffectMute", "CanEnterGDRLair"};
const char* const REMAIN_TABLE_NAMES[REMAIN_EFFECT_TABLE_MAX] = {"EffectSafeForceScroll", "EffectBehemothForceScroll",
                                                                 "EffectCarnelianForceScroll"};

class EffectSaveMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
        for (int t = 0; t < DEADLINE_EFFECT_TABLE_MAX; t++)
            execSQL(std::string("DELETE FROM ") + DEADLINE_TABLE_NAMES[t] + " WHERE OwnerID IN " +
                    PlayerFixtures::nameList());
        for (int t = 0; t < REMAIN_EFFECT_TABLE_MAX; t++)
            execSQL(std::string("DELETE FROM ") + REMAIN_TABLE_NAMES[t] + " WHERE OwnerID IN " +
                    PlayerFixtures::nameList());
        execSQL(std::string("DELETE FROM EnemyErase WHERE OwnerID IN ") + PlayerFixtures::nameList());
    }

    static std::string yearTimeOf(const char* table, const std::string& owner) {
        return queryScalar(std::string("SELECT YearTime FROM ") + table + " WHERE OwnerID='" + owner + "'");
    }
};

TEST_F(EffectSaveMySQL, EveryDeadlineTableRoundTripsInsertUpdateLoadDelete) {
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    slayer.persist();
    EffectSaveRepository& repository = defaultEffectSaveRepository();

    for (int t = 0; t < DEADLINE_EFFECT_TABLE_MAX; t++) {
        DeadlineEffectTable table = (DeadlineEffectTable)t;
        SCOPED_TRACE(DEADLINE_TABLE_NAMES[t]);

        repository.insertDeadline(table, slayer.name, 1111, 1700000001);
        std::vector<DWORD> dayTimes = repository.loadDeadlines(table, slayer.name);
        ASSERT_EQ(1u, dayTimes.size());
        EXPECT_EQ(1700000001u, dayTimes[0]);
        EXPECT_EQ("1111", yearTimeOf(DEADLINE_TABLE_NAMES[t], slayer.name));

        repository.updateDeadline(table, slayer.name, 2222, 1700000002);
        dayTimes = repository.loadDeadlines(table, slayer.name);
        ASSERT_EQ(1u, dayTimes.size());
        EXPECT_EQ(1700000002u, dayTimes[0]);
        EXPECT_EQ("2222", yearTimeOf(DEADLINE_TABLE_NAMES[t], slayer.name));

        repository.deleteDeadline(table, slayer.name);
        EXPECT_TRUE(repository.loadDeadlines(table, slayer.name).empty());
    }
}

TEST_F(EffectSaveMySQL, KeylessTablesAccumulateDuplicatesButKillAftermathRefusesThem) {
    // Seven of the eight tables have only an OwnerID index; a second
    // create() for the same owner just adds a row (and the loader then
    // attaches the effect twice). EffectKillAftermath is the one with
    // OwnerID as PRIMARY KEY: its second insert raises ER_DUP_ENTRY.
    PlayerFixture vampire = PlayerFixtures::midLevelVampire();
    vampire.persist();
    EffectSaveRepository& repository = defaultEffectSaveRepository();

    repository.insertDeadline(EFFECT_TABLE_AFTERMATH, vampire.name, 1, 1700000001);
    repository.insertDeadline(EFFECT_TABLE_AFTERMATH, vampire.name, 1, 1700000002);
    EXPECT_EQ(2u, repository.loadDeadlines(EFFECT_TABLE_AFTERMATH, vampire.name).size());

    repository.insertDeadline(EFFECT_TABLE_KILL_AFTERMATH, vampire.name, 1, 1700000001);
    EXPECT_ANY_THROW(repository.insertDeadline(EFFECT_TABLE_KILL_AFTERMATH, vampire.name, 1, 1700000002));
    std::vector<DWORD> dayTimes = repository.loadDeadlines(EFFECT_TABLE_KILL_AFTERMATH, vampire.name);
    ASSERT_EQ(1u, dayTimes.size());
    EXPECT_EQ(1700000001u, dayTimes[0]);
}

TEST_F(EffectSaveMySQL, EveryRemainTableRoundTripsInsertUpdateLoadDelete) {
    PlayerFixture ousters = PlayerFixtures::midLevelOusters();
    ousters.persist();
    EffectSaveRepository& repository = defaultEffectSaveRepository();

    for (int t = 0; t < REMAIN_EFFECT_TABLE_MAX; t++) {
        RemainEffectTable table = (RemainEffectTable)t;
        SCOPED_TRACE(REMAIN_TABLE_NAMES[t]);
        DWORD remainTurn = 0;

        EXPECT_FALSE(repository.loadRemain(table, ousters.name, remainTurn));

        repository.insertRemain(table, ousters.name, 500);
        ASSERT_TRUE(repository.loadRemain(table, ousters.name, remainTurn));
        EXPECT_EQ(500u, remainTurn);

        repository.updateRemain(table, ousters.name, 600);
        ASSERT_TRUE(repository.loadRemain(table, ousters.name, remainTurn));
        EXPECT_EQ(600u, remainTurn);

        repository.deleteRemain(table, ousters.name);
        EXPECT_FALSE(repository.loadRemain(table, ousters.name, remainTurn));
    }
}

TEST_F(EffectSaveMySQL, EnemyEraseDeletesByEnemyButUpdatesEveryRowOfTheOwner) {
    // One row per enemy. The DELETE keys on (OwnerID, EnemyName); the
    // UPDATE keys on OwnerID alone, so saving one enemy-erase effect
    // rewrites every EnemyErase row the owner has to that enemy.
    PlayerFixture slayer = PlayerFixtures::highLevelSlayer();
    slayer.persist();
    EffectSaveRepository& repository = defaultEffectSaveRepository();

    repository.insertEnemyErase(slayer.name, 1, 1700000001, "foeone");
    repository.insertEnemyErase(slayer.name, 2, 1700000002, "foetwo");

    std::vector<EnemyEraseRow> rows = repository.loadEnemyErases(slayer.name);
    ASSERT_EQ(2u, rows.size());
    for (size_t r = 0; r < rows.size(); r++) {
        if (rows[r].enemyName == "foeone")
            EXPECT_EQ(1700000001u, rows[r].dayTime);
        else if (rows[r].enemyName == "foetwo")
            EXPECT_EQ(1700000002u, rows[r].dayTime);
        else
            ADD_FAILURE() << "unexpected enemy " << rows[r].enemyName;
    }

    repository.updateEnemyErase(slayer.name, 3, 1700000003, "foeone");
    rows = repository.loadEnemyErases(slayer.name);
    ASSERT_EQ(2u, rows.size());
    EXPECT_EQ("foeone", rows[0].enemyName);
    EXPECT_EQ("foeone", rows[1].enemyName);
    EXPECT_EQ(1700000003u, rows[0].dayTime);
    EXPECT_EQ(1700000003u, rows[1].dayTime);

    repository.deleteEnemyErase(slayer.name, "foeone");
    EXPECT_TRUE(repository.loadEnemyErases(slayer.name).empty());
}

TEST_F(EffectSaveMySQL, WritesAgainstMissingRowsAreSilentNoOps) {
    PlayerFixture ghost = PlayerFixtures::lowLevelVampire(); // never persisted
    EffectSaveRepository& repository = defaultEffectSaveRepository();

    repository.updateDeadline(EFFECT_TABLE_MUTE, ghost.name, 1, 1);
    repository.deleteDeadline(EFFECT_TABLE_MUTE, ghost.name);
    repository.updateRemain(EFFECT_TABLE_CARNELIAN_FORCE_SCROLL, ghost.name, 1);
    repository.deleteRemain(EFFECT_TABLE_CARNELIAN_FORCE_SCROLL, ghost.name);
    repository.updateEnemyErase(ghost.name, 1, 1, "nobody");
    repository.deleteEnemyErase(ghost.name, "nobody");

    DWORD remainTurn = 0;
    EXPECT_TRUE(repository.loadDeadlines(EFFECT_TABLE_MUTE, ghost.name).empty());
    EXPECT_FALSE(repository.loadRemain(EFFECT_TABLE_CARNELIAN_FORCE_SCROLL, ghost.name, remainTurn));
    EXPECT_TRUE(repository.loadEnemyErases(ghost.name).empty());
}

// --- FlagSet against real MySQL -------------------------------------------

class FlagSetMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
        execSQL(std::string("DELETE FROM FlagSet WHERE OwnerID IN ") + PlayerFixtures::nameList());
    }
};

TEST_F(FlagSetMySQL, InsertLoadUpdateRemoveRoundTrip) {
    PlayerFixture slayer = PlayerFixtures::lowLevelSlayer();
    slayer.persist();
    FlagSetRepository& repository = defaultFlagSetRepository();
    std::string text;

    EXPECT_FALSE(repository.load(slayer.name, text));

    repository.insert(slayer.name, "101010101010101010101010");
    ASSERT_TRUE(repository.load(slayer.name, text));
    EXPECT_EQ("101010101010101010101010", text);

    repository.update(slayer.name, "000000000000000000000001");
    ASSERT_TRUE(repository.load(slayer.name, text));
    EXPECT_EQ("000000000000000000000001", text);

    repository.remove(slayer.name);
    EXPECT_FALSE(repository.load(slayer.name, text));
}

TEST_F(FlagSetMySQL, PrimaryKeyRefusesASecondInsertWhileInsertEmptyIfMissingIsANoOp) {
    PlayerFixture vampire = PlayerFixtures::lowLevelVampire();
    PlayerFixture ghost = PlayerFixtures::highLevelVampire();
    vampire.persist();
    FlagSetRepository& repository = defaultFlagSetRepository();
    std::string text;

    repository.insert(vampire.name, "111100000000000000000000");
    EXPECT_ANY_THROW(repository.insert(vampire.name, "000000000000000000000000"));
    repository.insertEmptyIfMissing(vampire.name); // INSERT IGNORE: keeps the row that is there
    ASSERT_TRUE(repository.load(vampire.name, text));
    EXPECT_EQ("111100000000000000000000", text);

    repository.insertEmptyIfMissing(ghost.name); // no row yet: an empty one appears
    ASSERT_TRUE(repository.load(ghost.name, text));
    EXPECT_EQ("", text);
}

// --- SMSAddressBook against real MySQL ------------------------------------

class SMSAddressMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
        execSQL(std::string("DELETE FROM SMSAddressBook WHERE OwnerID IN ") + PlayerFixtures::nameList());
    }

    static const SMSAddressRow* find(const std::vector<SMSAddressRow>& rows, int eID) {
        for (size_t r = 0; r < rows.size(); r++)
            if (rows[r].eID == eID)
                return &rows[r];
        return NULL;
    }
};

TEST_F(SMSAddressMySQL, InsertLoadRemoveRoundTripScopedToTheOwner) {
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    PlayerFixture other = PlayerFixtures::midLevelOusters();
    slayer.persist();
    other.persist();
    SMSAddressRepository& repository = defaultSMSAddressRepository();

    repository.insert(slayer.name, 1, "friendone", "Custom One", "01011112222");
    repository.insert(slayer.name, 2, "friendtwo", "Custom Two", "01033334444");
    repository.insert(other.name, 1, "someone", "Elsewhere", "01055556666");

    std::vector<SMSAddressRow> rows = repository.load(slayer.name);
    ASSERT_EQ(2u, rows.size());
    const SMSAddressRow* one = find(rows, 1);
    ASSERT_TRUE(one != NULL);
    EXPECT_EQ("friendone", one->characterName);
    EXPECT_EQ("Custom One", one->customName);
    EXPECT_EQ("01011112222", one->number);
    const SMSAddressRow* two = find(rows, 2);
    ASSERT_TRUE(two != NULL);
    EXPECT_EQ("friendtwo", two->characterName);
    EXPECT_EQ("Custom Two", two->customName);
    EXPECT_EQ("01033334444", two->number);

    repository.remove(slayer.name, 1);
    rows = repository.load(slayer.name);
    ASSERT_EQ(1u, rows.size());
    EXPECT_EQ(2, rows[0].eID);
    EXPECT_EQ(1u, repository.load(other.name).size());
}

TEST_F(SMSAddressMySQL, CompositePrimaryKeyRefusesARepeatedIdForTheSameOwnerOnly) {
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    PlayerFixture other = PlayerFixtures::midLevelOusters();
    slayer.persist();
    other.persist();
    SMSAddressRepository& repository = defaultSMSAddressRepository();

    repository.insert(slayer.name, 5, "a", "b", "c");
    EXPECT_ANY_THROW(repository.insert(slayer.name, 5, "d", "e", "f"));
    repository.insert(other.name, 5, "g", "h", "i"); // same id, other owner: fine

    EXPECT_EQ(1u, repository.load(slayer.name).size());
    EXPECT_EQ(1u, repository.load(other.name).size());
}

// --- GQuestItemObject against real MySQL ----------------------------------

class QuestItemMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
        execSQL(std::string("DELETE FROM GQuestItemObject WHERE OwnerID IN ") + PlayerFixtures::nameList());
    }

    static int count(const std::vector<int>& itemTypes, int itemType) {
        int n = 0;
        for (size_t i = 0; i < itemTypes.size(); i++)
            if (itemTypes[i] == itemType)
                n++;
        return n;
    }
};

TEST_F(QuestItemMySQL, RemoveOneTakesASingleInstanceAndLeavesTheRest) {
    // One row per item instance; the LIMIT 1 on the DELETE is what keeps
    // the second copy of a duplicated item.
    PlayerFixture ousters = PlayerFixtures::lowLevelOusters();
    ousters.persist();
    QuestItemRepository& repository = defaultQuestItemRepository();

    repository.insert(ousters.name, 7);
    repository.insert(ousters.name, 7);
    repository.insert(ousters.name, 9);

    std::vector<int> itemTypes = repository.loadItemTypes(ousters.name);
    ASSERT_EQ(3u, itemTypes.size());
    EXPECT_EQ(2, count(itemTypes, 7));
    EXPECT_EQ(1, count(itemTypes, 9));

    repository.removeOne(ousters.name, 7);
    itemTypes = repository.loadItemTypes(ousters.name);
    ASSERT_EQ(2u, itemTypes.size());
    EXPECT_EQ(1, count(itemTypes, 7));
    EXPECT_EQ(1, count(itemTypes, 9));

    repository.removeOne(ousters.name, 7);
    repository.removeOne(ousters.name, 7); // none left: silent no-op
    itemTypes = repository.loadItemTypes(ousters.name);
    ASSERT_EQ(1u, itemTypes.size());
    EXPECT_EQ(9, itemTypes[0]);
}

TEST_F(QuestItemMySQL, RowsAreScopedToTheOwner) {
    PlayerFixture slayer = PlayerFixtures::lowLevelSlayer();
    PlayerFixture vampire = PlayerFixtures::lowLevelVampire();
    slayer.persist();
    vampire.persist();
    QuestItemRepository& repository = defaultQuestItemRepository();

    repository.insert(slayer.name, 4);
    repository.insert(vampire.name, 5);

    std::vector<int> itemTypes = repository.loadItemTypes(slayer.name);
    ASSERT_EQ(1u, itemTypes.size());
    EXPECT_EQ(4, itemTypes[0]);
    itemTypes = repository.loadItemTypes(vampire.name);
    ASSERT_EQ(1u, itemTypes.size());
    EXPECT_EQ(5, itemTypes[0]);
}

// --- zone configuration against real MySQL --------------------------------
// The config tables ship seeded in initdb/DARKEDEN.sql (zone groups 1
// and 2, the real zones), so these tests add rows with ids far above the
// shipped range and assert on those, never on the seeded set's size.

const int IT_ZONE_GROUP = 31000;
const int IT_ZONE_GROUP_2 = 31001;
const int IT_ZONE = 31000;
const int IT_ZONE_2 = 31001;
const int IT_ZONE_3 = 31003;
const int IT_SERVER = 250; // ServerID is tinyint unsigned

class ZoneInfoMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        execSQL("DELETE FROM ZoneGroupInfo WHERE ZoneGroupID >= 31000");
        execSQL("DELETE FROM ZoneInfo WHERE ZoneID >= 31000");
        execSQL("DELETE FROM ZoneTriggers WHERE ZoneID >= 31000");
        execSQL("DELETE FROM EffectPKZoneRegen WHERE ID >= 31000");
        execSQL("DELETE FROM WayPointInfo WHERE ZoneID >= 31000");
    }

    static bool contains(const std::vector<int>& ids, int id) {
        for (size_t i = 0; i < ids.size(); i++)
            if (ids[i] == id)
                return true;
        return false;
    }
};

TEST_F(ZoneInfoMySQL, ZoneGroupIDsComeBackOrderedOnlyWhenAsked) {
    execSQL("INSERT INTO ZoneGroupInfo (ZoneGroupID, ServerID) VALUES (31001, 1)");
    execSQL("INSERT INTO ZoneGroupInfo (ZoneGroupID, ServerID) VALUES (31000, 1)");

    std::vector<int> ordered = defaultZoneInfoRepository().loadZoneGroupIDs(true);
    ASSERT_GE(ordered.size(), 2u);
    EXPECT_EQ(IT_ZONE_GROUP, ordered[ordered.size() - 2]);
    EXPECT_EQ(IT_ZONE_GROUP_2, ordered[ordered.size() - 1]);

    std::vector<int> unordered = defaultZoneInfoRepository().loadZoneGroupIDs(false);
    EXPECT_EQ(ordered.size(), unordered.size());
    EXPECT_TRUE(contains(unordered, IT_ZONE_GROUP));
    EXPECT_TRUE(contains(unordered, IT_ZONE_GROUP_2));
}

TEST_F(ZoneInfoMySQL, ZoneIDsOfAGroupComeBackOrderedOnlyWhenAsked) {
    execSQL("INSERT INTO ZoneInfo (ZoneID, ZoneGroupID) VALUES (31003, 31000)");
    execSQL("INSERT INTO ZoneInfo (ZoneID, ZoneGroupID) VALUES (31001, 31000)");
    execSQL("INSERT INTO ZoneInfo (ZoneID, ZoneGroupID) VALUES (31000, 31001)"); // another group

    std::vector<int> ordered = defaultZoneInfoRepository().loadZoneIDsOfGroup(IT_ZONE_GROUP, true);
    ASSERT_EQ(2u, ordered.size());
    EXPECT_EQ(IT_ZONE_2, ordered[0]);
    EXPECT_EQ(IT_ZONE_3, ordered[1]);

    std::vector<int> unordered = defaultZoneInfoRepository().loadZoneIDsOfGroup(IT_ZONE_GROUP, false);
    ASSERT_EQ(2u, unordered.size());
    EXPECT_TRUE(contains(unordered, IT_ZONE_2));
    EXPECT_TRUE(contains(unordered, IT_ZONE_3));
}

TEST_F(ZoneInfoMySQL, LoadZoneInfosReturnsEveryColumnInSelectPosition) {
    execSQL("INSERT INTO ZoneInfo (ZoneID, ZoneGroupID, Type, Level, AccessMode, OwnerId, PayPlayZone, PremiumZone, "
            "PKZone, NoPortalZone, HolyLand, Available, OpenLevel, SmpFileName, SsiFileName, FullName, ShortName) "
            "VALUES (31000, 31001, 'NPC_SHOP', 7, 'PRIVATE', 'itowner', 1, 0, 1, 0, 1, 0, 13, 'it.smp', 'it.ssi', "
            "'It Full', 'ItShort')");

    std::vector<ZoneInfoRow> rows = defaultZoneInfoRepository().loadZoneInfos();
    const ZoneInfoRow* found = NULL;
    for (size_t r = 0; r < rows.size(); r++)
        if (rows[r].zoneID == IT_ZONE)
            found = &rows[r];
    ASSERT_TRUE(found != NULL);
    EXPECT_EQ(31001, found->zoneGroupID);
    EXPECT_EQ("NPC_SHOP", found->type);
    EXPECT_EQ(7, found->level);
    EXPECT_EQ("PRIVATE", found->accessMode);
    EXPECT_EQ("itowner", found->ownerID);
    EXPECT_EQ(1, found->payPlayZone);
    EXPECT_EQ(0, found->premiumZone);
    EXPECT_EQ(1, found->pkZone);
    EXPECT_EQ(0, found->noPortalZone);
    EXPECT_EQ(1, found->holyLand);
    EXPECT_EQ(0, found->available);
    EXPECT_EQ(13, found->openLevel);
    EXPECT_EQ("it.smp", found->smpFilename);
    EXPECT_EQ("it.ssi", found->ssiFilename);
    EXPECT_EQ("It Full", found->fullName);
    EXPECT_EQ("ItShort", found->shortName);
}

TEST_F(ZoneInfoMySQL, LoadResurrectLocationsReturnsEveryColumnInSelectPosition) {
    execSQL("INSERT INTO ZoneInfo (ZoneID, SResurrectZoneID, SResurrectX, SResurrectY, VResurrectZoneID, "
            "VResurrectX, VResurrectY, OResurrectZoneID, OResurrectX, OResurrectY) "
            "VALUES (31000, 2, 3, 4, 5, 6, 7, 8, 9, 10)");

    std::vector<ResurrectLocationRow> rows = defaultZoneInfoRepository().loadResurrectLocations();
    const ResurrectLocationRow* found = NULL;
    for (size_t r = 0; r < rows.size(); r++)
        if (rows[r].zoneID == IT_ZONE)
            found = &rows[r];
    ASSERT_TRUE(found != NULL);
    EXPECT_EQ(2, found->slayerZoneID);
    EXPECT_EQ(3, found->slayerX);
    EXPECT_EQ(4, found->slayerY);
    EXPECT_EQ(5, found->vampireZoneID);
    EXPECT_EQ(6, found->vampireX);
    EXPECT_EQ(7, found->vampireY);
    EXPECT_EQ(8, found->oustersZoneID);
    EXPECT_EQ(9, found->oustersX);
    EXPECT_EQ(10, found->oustersY);
}

TEST_F(ZoneInfoMySQL, TriggersRegenRectsAndWayPointsAreScopedToTheZoneAndRace) {
    execSQL("INSERT INTO ZoneTriggers (TriggerID, ZoneID, X1, Y1, X2, Y2, Conditions, Actions, CounterActions) "
            "VALUES (31000, 31000, 1, 2, 3, 4, '', '', '')");
    execSQL("INSERT INTO ZoneTriggers (TriggerID, ZoneID, X1, Y1, X2, Y2, Conditions, Actions, CounterActions) "
            "VALUES (31001, 31001, 9, 9, 9, 9, '', '', '')");
    execSQL(
        "INSERT INTO EffectPKZoneRegen (ID, ZoneID, LeftX, TopY, RightX, BottomY) VALUES (31000, 31000, 5, 6, 7, 8)");
    execSQL(
        "INSERT INTO EffectPKZoneRegen (ID, ZoneID, LeftX, TopY, RightX, BottomY) VALUES (31001, 31001, 9, 9, 9, 9)");
    execSQL("INSERT INTO WayPointInfo (ZoneID, X, Y, Race) VALUES (31000, 10, 11, 2)");
    execSQL("INSERT INTO WayPointInfo (ZoneID, X, Y, Race) VALUES (31000, 12, 13, 0)"); // other race
    execSQL("INSERT INTO WayPointInfo (ZoneID, X, Y, Race) VALUES (31001, 14, 15, 2)"); // other zone

    std::vector<ZoneRectRow> triggers = defaultZoneInfoRepository().loadTriggerRects(IT_ZONE);
    ASSERT_EQ(1u, triggers.size());
    EXPECT_EQ(1, triggers[0].left);
    EXPECT_EQ(2, triggers[0].top);
    EXPECT_EQ(3, triggers[0].right);
    EXPECT_EQ(4, triggers[0].bottom);

    std::vector<ZoneRectRow> regens = defaultZoneInfoRepository().loadPKZoneRegenRects(IT_ZONE);
    ASSERT_EQ(1u, regens.size());
    EXPECT_EQ(5, regens[0].left);
    EXPECT_EQ(6, regens[0].top);
    EXPECT_EQ(7, regens[0].right);
    EXPECT_EQ(8, regens[0].bottom);

    std::vector<ZonePointRow> points = defaultZoneInfoRepository().loadWayPoints(IT_ZONE, 2);
    ASSERT_EQ(1u, points.size());
    EXPECT_EQ(10, points[0].x);
    EXPECT_EQ(11, points[0].y);

    std::vector<WayPointRow> all = defaultZoneInfoRepository().loadAllWayPoints();
    int seen = 0;
    for (size_t w = 0; w < all.size(); w++) {
        if (all[w].zoneID == IT_ZONE && all[w].x == 10 && all[w].y == 11 && all[w].race == 2)
            seen++;
        if (all[w].zoneID == IT_ZONE && all[w].x == 12 && all[w].y == 13 && all[w].race == 0)
            seen++;
        if (all[w].zoneID == IT_ZONE_2 && all[w].x == 14 && all[w].y == 15 && all[w].race == 2)
            seen++;
    }
    EXPECT_EQ(3, seen);
}

// --- Messages against real MySQL ------------------------------------------

class MessageMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        PlayerFixtures::removeAll();
        execSQL(std::string("DELETE FROM Messages WHERE Receiver IN ") + PlayerFixtures::nameList());
    }
};

TEST_F(MessageMySQL, InsertLoadAndDeleteAreScopedToTheReceiver) {
    PlayerFixture slayer = PlayerFixtures::midLevelSlayer();
    PlayerFixture other = PlayerFixtures::midLevelOusters();
    slayer.persist();
    other.persist();
    MessageRepository& repository = defaultMessageRepository();

    repository.insertMessage(slayer.name, "first");
    repository.insertMessage(slayer.name, "first"); // keyless: a repeat is a second row
    repository.insertMessage(other.name, "elsewhere");

    std::vector<std::string> messages = repository.loadMessages(slayer.name);
    ASSERT_EQ(2u, messages.size());
    EXPECT_EQ("first", messages[0]);
    EXPECT_EQ("first", messages[1]);

    repository.deleteMessages(slayer.name);
    EXPECT_TRUE(repository.loadMessages(slayer.name).empty());
    EXPECT_EQ(1u, repository.loadMessages(other.name).size());
}

// --- Event200501 against real MySQL (the dist connection) -----------------

class ComebackEventMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        execSQL("DELETE FROM Event200501Main WHERE PlayerID = 'itaccount'");
        execSQL("DELETE FROM Event200501Recommend WHERE PlayerID = 'itaccount'");
    }
};

TEST_F(ComebackEventMySQL, ThePredicatesFollowTheZeroDateColumns) {
    ComebackEventRepository& repository = defaultComebackEventRepository();

    EXPECT_FALSE(repository.hasUnclaimedItem("itaccount"));
    EXPECT_FALSE(repository.hasUnclaimedPremiumItem("itaccount"));
    EXPECT_FALSE(repository.hasUnclaimedRecommendItem("itaccount"));

    execSQL("INSERT INTO Event200501Main (PlayerID) VALUES ('itaccount')"); // every date at the zero default
    EXPECT_TRUE(repository.hasUnclaimedItem("itaccount"));
    EXPECT_FALSE(repository.hasUnclaimedPremiumItem("itaccount")); // never paid

    execSQL("UPDATE Event200501Main SET PayPremiumDate = '2005-01-02' WHERE PlayerID = 'itaccount'");
    EXPECT_TRUE(repository.hasUnclaimedPremiumItem("itaccount"));

    execSQL("UPDATE Event200501Main SET RecvItemDate = '2005-01-03', RecvPremiumItemDate = '2005-01-03' "
            "WHERE PlayerID = 'itaccount'");
    EXPECT_FALSE(repository.hasUnclaimedItem("itaccount"));
    EXPECT_FALSE(repository.hasUnclaimedPremiumItem("itaccount"));

    execSQL("INSERT INTO Event200501Recommend (PlayerID, Recommender) VALUES ('itaccount', 'friend')");
    EXPECT_TRUE(repository.hasUnclaimedRecommendItem("itaccount"));
    execSQL("UPDATE Event200501Recommend SET RecvItemDate = '2005-01-04' WHERE PlayerID = 'itaccount'");
    EXPECT_FALSE(repository.hasUnclaimedRecommendItem("itaccount"));
}

// --- BulletinBoardObject against real MySQL -------------------------------

class BulletinBoardMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        execSQL("DELETE FROM BulletinBoardObject WHERE ServerID = 250");
    }
};

TEST_F(BulletinBoardMySQL, InsertLoadForZoneAndRemove) {
    BulletinBoardRepository& repository = defaultBulletinBoardRepository();

    EXPECT_EQ(1u, repository.insert(IT_SERVER, IT_ZONE, 40, 50, "hello board", 673, "2030-01-02 03:04:05"));
    EXPECT_EQ(1u, repository.insert(IT_SERVER, IT_ZONE_2, 1, 1, "other zone", 673, "2030-01-02 03:04:05"));

    std::vector<BulletinBoardRow> rows = repository.loadForZone(IT_SERVER, IT_ZONE);
    ASSERT_EQ(1u, rows.size());
    EXPECT_EQ(40, rows[0].x);
    EXPECT_EQ(50, rows[0].y);
    EXPECT_EQ("hello board", rows[0].message);
    EXPECT_EQ(673, rows[0].type);
    EXPECT_EQ("2030-01-02 03:04:05", rows[0].timeLimit);
    EXPECT_TRUE(repository.loadForZone(IT_SERVER + 1, IT_ZONE).empty()); // other server

    repository.remove(rows[0].id);
    EXPECT_TRUE(repository.loadForZone(IT_SERVER, IT_ZONE).empty());
    EXPECT_EQ(1u, repository.loadForZone(IT_SERVER, IT_ZONE_2).size());
}

// --- RegenZonePosition against real MySQL ---------------------------------

class RegenZoneMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        execSQL("DELETE FROM RegenZonePosition WHERE ID >= 31000");
    }
};

TEST_F(RegenZoneMySQL, LoadPositionsReturnsEveryColumnInSelectPosition) {
    execSQL("INSERT INTO RegenZonePosition (ID, ZoneID, ZoneX, ZoneY, Owner) VALUES (31000, 31000, 20, 30, 2)");

    std::vector<RegenZoneRow> rows = defaultRegenZoneRepository().loadPositions();
    const RegenZoneRow* found = NULL;
    for (size_t r = 0; r < rows.size(); r++)
        if (rows[r].id == 31000)
            found = &rows[r];
    ASSERT_TRUE(found != NULL);
    EXPECT_EQ(IT_ZONE, found->zoneID);
    EXPECT_EQ(20, found->zoneX);
    EXPECT_EQ(30, found->zoneY);
    EXPECT_EQ(2, found->owner);
}

// --- the balance and game-info tables against real MySQL ------------------
// Every one of these tables ships seeded in initdb/DARKEDEN.sql and is
// read-only for the gameserver, so the tests assert on the shipped data's
// shape (a maximum exists, the rows stay within it, the lists are not
// empty — exactly what the boot-time loaders require) rather than on
// rows they insert. The one write-free quirk worth pinning is the MAX()
// probe over nothing: MySQL answers with one NULL row, which the inline
// code would have atoi(NULL)'d; the seam reports "no maximum".

TEST(BalanceInfoMySQL, EveryLadderHasAMaximumAndItsRowsStayWithinIt) {
    BalanceInfoRepository& repository = defaultBalanceInfoRepository();

    for (int t = 0; t < LEVEL_EXP_TABLE_MAX; t++) {
        LevelExpTable table = (LevelExpTable)t;
        SCOPED_TRACE(t);
        int maxLevel = -1;
        ASSERT_TRUE(repository.loadMaxLevel(table, maxLevel));
        EXPECT_GT(maxLevel, 0);

        std::vector<LevelExpRow> rows = repository.loadLevels(table);
        ASSERT_FALSE(rows.empty());
        for (size_t r = 0; r < rows.size(); r++) {
            EXPECT_LE(rows[r].level, maxLevel);
            if (table != LEVEL_EXP_TABLE_OUSTERS_EXP)
                EXPECT_EQ(0, rows[r].skillPointBonus); // not selected for the other four tables
        }
    }
}

TEST(BalanceInfoMySQL, RankLaddersExistPerRankTypeAndAMissingTypeHasNoMaximum) {
    BalanceInfoRepository& repository = defaultBalanceInfoRepository();

    for (int rankType = 0; rankType < 3; rankType++) {
        SCOPED_TRACE(rankType);
        int maxLevel = -1;
        ASSERT_TRUE(repository.loadMaxRankLevel(rankType, maxLevel));
        EXPECT_GT(maxLevel, 0);
        std::vector<LevelExpRow> rows = repository.loadRankLevels(rankType);
        ASSERT_FALSE(rows.empty());
        for (size_t r = 0; r < rows.size(); r++)
            EXPECT_LE(rows[r].level, maxLevel);
    }

    // MAX() over a WHERE that matches nothing is ONE row holding NULL —
    // reported as "no maximum", where the inline loader would have
    // called atoi(NULL) on it.
    int maxLevel = -1;
    EXPECT_FALSE(repository.loadMaxRankLevel(99, maxLevel));
    EXPECT_TRUE(repository.loadRankLevels(99).empty());
}

TEST(BalanceInfoMySQL, DomainLaddersExistAndAMissingDomainHasNoMaximum) {
    BalanceInfoRepository& repository = defaultBalanceInfoRepository();

    int maxLevel = -1;
    ASSERT_TRUE(repository.loadMaxDomainLevel(0, maxLevel));
    EXPECT_GT(maxLevel, 0);
    std::vector<DomainLevelRow> rows = repository.loadDomainLevels(0);
    ASSERT_FALSE(rows.empty());
    for (size_t r = 0; r < rows.size(); r++) {
        EXPECT_EQ(0, rows[r].domainType);
        EXPECT_LE(rows[r].level, maxLevel);
    }

    EXPECT_FALSE(repository.loadMaxDomainLevel(99, maxLevel));
    EXPECT_TRUE(repository.loadDomainLevels(99).empty());
}

TEST(BalanceInfoMySQL, TheFameLimitTableIsNotInTheShippedSchema) {
    // FameLimitInfoManager is never constructed by the gameserver, and
    // its table is absent from initdb/DARKEDEN.sql: the dead loader would
    // fail on its first query. Pinned so a schema that grows the table
    // (or a boot that revives the manager) shows up here.
    int maxLevel = -1;
    EXPECT_ANY_THROW(defaultBalanceInfoRepository().loadMaxFameLevel(0, maxLevel));
}

TEST(BalanceInfoMySQL, ThePetTablesLoad) {
    BalanceInfoRepository& repository = defaultBalanceInfoRepository();

    EXPECT_FALSE(repository.loadPetExp().empty());
    EXPECT_FALSE(repository.loadPetAttrBalance().empty());
    EXPECT_FALSE(repository.loadPetAttrRatios().empty());
}

TEST(GameInfoMySQL, EveryMaximumExistsAndEveryListIsNonEmpty) {
    GameInfoRepository& repository = defaultGameInfoRepository();
    int maximum = -1;

    ASSERT_TRUE(repository.loadMaxSkillType(maximum));
    EXPECT_GT(maximum, 0);
    std::vector<SkillParentRow> tree = repository.loadSkillTree();
    ASSERT_FALSE(tree.empty());
    for (size_t r = 0; r < tree.size(); r++)
        EXPECT_LE(tree[r].skillType, maximum);

    ASSERT_TRUE(repository.loadMaxRankBonusType(maximum));
    std::vector<RankBonusInfoRow> bonuses = repository.loadRankBonusInfos();
    ASSERT_FALSE(bonuses.empty());
    for (size_t r = 0; r < bonuses.size(); r++)
        EXPECT_LE(bonuses[r].type, maximum);

    ASSERT_TRUE(repository.loadMaxPetType(maximum));
    std::vector<PetTypeRow> pets = repository.loadPetTypes();
    ASSERT_FALSE(pets.empty());
    for (size_t r = 0; r < pets.size(); r++)
        EXPECT_LE(pets[r].petType, maximum);

    ASSERT_TRUE(repository.loadMaxWorldID(maximum));
    std::vector<GameServerGroupRow> groups = repository.loadGameServerGroups();
    ASSERT_FALSE(groups.empty());
    for (size_t r = 0; r < groups.size(); r++)
        EXPECT_LE(groups[r].worldID, maximum);

    ASSERT_TRUE(repository.loadMaxBloodBibleBonusType(maximum));
    std::vector<BloodBibleBonusRow> bibles = repository.loadBloodBibleBonuses();
    ASSERT_FALSE(bibles.empty());
    for (size_t r = 0; r < bibles.size(); r++)
        EXPECT_LE(bibles[r].type, maximum);
}

TEST(GameInfoMySQL, EveryMonsterNameListIsNonEmpty) {
    // MonsterNameManager::init throws on an empty list, so the shipped
    // data must carry all four — including the event monsters' last
    // names.
    for (int l = 0; l < MONSTER_NAME_LIST_MAX; l++) {
        SCOPED_TRACE(l);
        EXPECT_FALSE(defaultGameInfoRepository().loadMonsterNames((MonsterNameList)l).empty());
    }
}

// --- the config loaders of the second game-info round ---------------------
// All read-only, all shipped seeded: the tests assert the shape the boot
// requires (the weather and grade tables' exact row counts, the others
// non-empty and internally consistent) plus the two per-zone reads
// against rows they insert with ids from 31000 up.

TEST(ConfigLoadersMySQL, WeatherAndGradeTablesHaveTheirFixedRowCounts) {
    // WeatherInfoManager asserts 12 rows, ItemGradeManager 10 — the shipped
    // data must satisfy both or the boot would assert-fail.
    std::vector<WeatherRow> weather = defaultGameInfoRepository().loadWeather();
    ASSERT_EQ(12u, weather.size());
    for (size_t r = 0; r < weather.size(); r++)
        EXPECT_TRUE(weather[r].month >= 1 && weather[r].month <= 12);

    std::vector<ItemGradeRatioRow> grades = defaultGameInfoRepository().loadItemGradeRatios();
    ASSERT_EQ(10u, grades.size());
    for (size_t r = 0; r < grades.size(); r++)
        EXPECT_TRUE(grades[r].grade >= 1 && grades[r].grade <= 10);
}

TEST(ConfigLoadersMySQL, EveryWholeTableConfigListIsNonEmpty) {
    GameInfoRepository& repository = defaultGameInfoRepository();
    EXPECT_FALSE(repository.loadStrings().empty());
    EXPECT_FALSE(repository.loadShopTemplates().empty());
    EXPECT_FALSE(repository.loadLevelNicks().empty());
    EXPECT_FALSE(repository.loadItemMines().empty());
    EXPECT_FALSE(repository.loadDefaultOptionSets().empty());
    EXPECT_FALSE(repository.loadDarkLight().empty());
    EXPECT_FALSE(repository.loadCastleSkills().empty());
    EXPECT_FALSE(repository.loadCastleShrines().empty());
    EXPECT_FALSE(repository.loadLogUserNames().empty());

    std::vector<PKZoneRow> pkZones = defaultZoneInfoRepository().loadPKZones();
    EXPECT_FALSE(pkZones.empty());
    std::vector<EventZoneRow> eventZones = defaultZoneInfoRepository().loadEventZones();
    EXPECT_FALSE(eventZones.empty());
    std::vector<LevelWarZoneRow> levelWarZones = defaultZoneInfoRepository().loadLevelWarZones();
    EXPECT_FALSE(levelWarZones.empty());
}

class GoodsListMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        execSQL("DELETE FROM GoodsListInfo WHERE GoodsID >= 31000");
    }
    virtual void TearDown() {
        execSQL("DELETE FROM GoodsListInfo WHERE GoodsID >= 31000");
    }
};

TEST_F(GoodsListMySQL, GoodsComeFromTheDistConnectionWithoutTheSetKind) {
    // GoodsListInfo is read on the thread's dist connection (same schema
    // in this stack) and the SELECT excludes Kind 'SET'; Limited+0 is the
    // enum ordinal, 1..3. The shipped seed has neither a 'SET' nor a
    // 'FOREVER' row, so both are inserted here: without them the filter
    // and the ordinal's upper bound would pass unexercised.
    execSQL("INSERT INTO GoodsListInfo (GoodsID, Name, Description, Limited, Kind) "
            "VALUES (31000, 'it-set', '', 'LIMITED', 'SET')");
    execSQL("INSERT INTO GoodsListInfo (GoodsID, Name, Description, Limited, Kind) "
            "VALUES (31001, 'it-forever', '', 'FOREVER', 'ETC')");

    std::vector<GoodsInfoRow> goods = defaultGameInfoRepository().loadGoods();
    ASSERT_FALSE(goods.empty());
    bool sawSet = false, sawForever = false;
    for (size_t r = 0; r < goods.size(); r++) {
        EXPECT_TRUE(goods[r].limited >= 1 && goods[r].limited <= 3);
        EXPECT_NE("SET",
                  queryScalar("SELECT Kind FROM GoodsListInfo WHERE GoodsID=" + std::to_string(goods[r].goodsID)));
        if (goods[r].goodsID == 31000)
            sawSet = true;
        if (goods[r].goodsID == 31001) {
            sawForever = true;
            EXPECT_EQ(3, goods[r].limited);
        }
    }
    EXPECT_FALSE(sawSet);
    EXPECT_TRUE(sawForever);
}

class ZoneConfigMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        execSQL("DELETE FROM ZoneInfo WHERE ZoneID >= 31000");
        execSQL("DELETE FROM ZoneEffectInfo WHERE ZoneID >= 31000");
    }
    virtual void TearDown() {
        execSQL("DELETE FROM ZoneInfo WHERE ZoneID >= 31000");
        execSQL("DELETE FROM ZoneEffectInfo WHERE ZoneID >= 31000");
    }
};

TEST_F(ZoneConfigMySQL, ZoneEffectRectsAreScopedToZoneAndEffect) {
    execSQL("INSERT INTO ZoneEffectInfo (ZoneID, EffectID, LeftX, TopY, RightX, BottomY, Value1, Value2, Value3) "
            "VALUES (31000, 7, 1, 2, 3, 4, 5, 6, 7)");
    execSQL("INSERT INTO ZoneEffectInfo (ZoneID, EffectID, LeftX, TopY, RightX, BottomY, Value1, Value2, Value3) "
            "VALUES (31000, 8, 9, 9, 9, 9, 9, 9, 9)"); // other effect
    execSQL("INSERT INTO ZoneEffectInfo (ZoneID, EffectID, LeftX, TopY, RightX, BottomY, Value1, Value2, Value3) "
            "VALUES (31001, 7, 9, 9, 9, 9, 9, 9, 9)"); // other zone

    std::vector<ZoneEffectRow> rows = defaultZoneInfoRepository().loadZoneEffectRects(IT_ZONE, 7);
    ASSERT_EQ(1u, rows.size());
    EXPECT_EQ(1, rows[0].left);
    EXPECT_EQ(2, rows[0].top);
    EXPECT_EQ(3, rows[0].right);
    EXPECT_EQ(4, rows[0].bottom);
    EXPECT_EQ(5, rows[0].value1);
    EXPECT_EQ(6, rows[0].value2);
    EXPECT_EQ(7, rows[0].value3);
}

TEST_F(ZoneConfigMySQL, MonsterListsComeFromTheZoneRowOrReportNoRow) {
    execSQL("INSERT INTO ZoneInfo (ZoneID, MonsterList, EventMonsterList) VALUES (31000, 'a:1', 'b:2')");

    std::string monsters, eventMonsters;
    ASSERT_TRUE(defaultZoneInfoRepository().loadMonsterLists(IT_ZONE, monsters, eventMonsters));
    EXPECT_EQ("a:1", monsters);
    EXPECT_EQ("b:2", eventMonsters);

    EXPECT_FALSE(defaultZoneInfoRepository().loadMonsterLists(IT_ZONE_2, monsters, eventMonsters));
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

// --- the race-war cluster against real MySQL -------------------------------
// Seven war files, one seam mixing boot-time reads with runtime writes.
// Every table is seeded; the tests work on rows they insert (ids from
// 31000 up where the column allows it, 250 up for the BYTE-typed sweeper
// bonus Type) and clean them in SetUp/TearDown.

class WarInfoMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        clean();
    }
    virtual void TearDown() {
        clean();
    }
    static void clean() {
        execSQL("DELETE FROM ShrineInfo WHERE ID >= 31000");
        execSQL("DELETE FROM CastleInfo WHERE ServerID >= 31000");
        execSQL("DELETE FROM SweeperBonusInfo WHERE Type >= 250");
        execSQL("DELETE FROM SweeperSetInfo WHERE ID >= 31000");
        execSQL("DELETE FROM SweeperOwnerInfo WHERE SweeperType >= 31000");
        execSQL("DELETE FROM LevelWarHistory WHERE Level >= 31000");
        execSQL("DELETE FROM MasterLairInfo WHERE ZoneID >= 31000");
    }
};

TEST_F(WarInfoMySQL, SeededWholeTableReadsAreNonEmptyAndTheMaxProbeMatchesTheTable) {
    WarInfoRepository& repository = defaultWarInfoRepository();
    EXPECT_FALSE(repository.loadShrines().empty());
    EXPECT_FALSE(repository.loadShrineOwners().empty());
    EXPECT_FALSE(repository.loadSweeperBonuses().empty());
    EXPECT_FALSE(repository.loadMasterLairs().empty());

    int maxType = -1;
    ASSERT_TRUE(repository.loadMaxSweeperBonusType(maxType));
    EXPECT_EQ(atoi(queryScalar("SELECT MAX(Type) FROM SweeperBonusInfo").c_str()), maxType);
}

TEST_F(WarInfoMySQL, ShrineOwnerSaveRewritesOnlyThatShrine) {
    execSQL("INSERT INTO ShrineInfo (ID, Name, OwnerRace) VALUES (31000, 'it-shrine-a', 0)");
    execSQL("INSERT INTO ShrineInfo (ID, Name, OwnerRace) VALUES (31001, 'it-shrine-b', 0)");

    defaultWarInfoRepository().saveShrineOwner(2, 31000);

    EXPECT_EQ("2", queryScalar("SELECT OwnerRace FROM ShrineInfo WHERE ID=31000"));
    EXPECT_EQ("0", queryScalar("SELECT OwnerRace FROM ShrineInfo WHERE ID=31001"));

    std::vector<ShrineOwnerRow> owners = defaultWarInfoRepository().loadShrineOwners();
    bool seen = false;
    for (size_t r = 0; r < owners.size(); r++) {
        if (owners[r].id == 31000) {
            seen = true;
            EXPECT_EQ(2, owners[r].ownerRace);
        }
    }
    EXPECT_TRUE(seen);
}

TEST_F(WarInfoMySQL, CastlesAreScopedToTheServerAndTheSavesKeyOnServerAndZone) {
    execSQL("INSERT INTO CastleInfo (ServerID, ZoneID, ShrineID, Name, GuildID, Race, ItemTaxRatio, EntranceFee, "
            "TaxBalance, BonusOptionType, FirstResurrectZoneID, FirstResurrectX, FirstResurrectY, "
            "SecondResurrectZoneID, SecondResurrectX, SecondResurrectY, ThirdResurrectZoneID, ThirdResurrectX, "
            "ThirdResurrectY, ZoneIDList) VALUES (31000, 31000, 5, 'it-castle', 7, 1, 10, 100, 1000, '1,2', "
            "31000, 3, 4, 31001, 5, 6, 31002, 7, 8, '31000,31001')");
    execSQL("INSERT INTO CastleInfo (ServerID, ZoneID, Name, BonusOptionType, ZoneIDList) "
            "VALUES (31001, 31000, 'other-server', '', '')");

    std::vector<CastleRow> rows = defaultWarInfoRepository().loadCastles(31000);
    ASSERT_EQ(1u, rows.size());
    EXPECT_EQ(31000, rows[0].zoneID);
    EXPECT_EQ(5, rows[0].shrineID);
    EXPECT_EQ(7, rows[0].guildID);
    EXPECT_EQ("it-castle", rows[0].name);
    EXPECT_EQ(1, rows[0].race);
    EXPECT_EQ(10, rows[0].itemTaxRatio);
    EXPECT_EQ(100, rows[0].entranceFee);
    EXPECT_EQ(1000, rows[0].taxBalance);
    EXPECT_EQ("1,2", rows[0].bonusOptionType);
    EXPECT_EQ(31000, rows[0].firstResurrectZoneID);
    EXPECT_EQ(3, rows[0].firstResurrectX);
    EXPECT_EQ(4, rows[0].firstResurrectY);
    EXPECT_EQ(31001, rows[0].secondResurrectZoneID);
    EXPECT_EQ(5, rows[0].secondResurrectX);
    EXPECT_EQ(6, rows[0].secondResurrectY);
    EXPECT_EQ(31002, rows[0].thirdResurrectZoneID);
    EXPECT_EQ(7, rows[0].thirdResurrectX);
    EXPECT_EQ(8, rows[0].thirdResurrectY);
    EXPECT_EQ("31000,31001", rows[0].zoneIDList);

    CastleStateRecord record;
    record.guildID = 8;
    record.name = "renamed";
    record.race = 2;
    record.itemTaxRatio = 20;
    record.entranceFee = 200;
    record.taxBalance = 2000;
    defaultWarInfoRepository().saveCastle(31000, 31000, record);

    EXPECT_EQ("8", queryScalar("SELECT GuildID FROM CastleInfo WHERE ServerID=31000 AND ZoneID=31000"));
    EXPECT_EQ("renamed", queryScalar("SELECT Name FROM CastleInfo WHERE ServerID=31000 AND ZoneID=31000"));
    EXPECT_EQ("2", queryScalar("SELECT Race FROM CastleInfo WHERE ServerID=31000 AND ZoneID=31000"));
    EXPECT_EQ("20", queryScalar("SELECT ItemTaxRatio FROM CastleInfo WHERE ServerID=31000 AND ZoneID=31000"));
    EXPECT_EQ("200", queryScalar("SELECT EntranceFee FROM CastleInfo WHERE ServerID=31000 AND ZoneID=31000"));
    EXPECT_EQ("2000", queryScalar("SELECT TaxBalance FROM CastleInfo WHERE ServerID=31000 AND ZoneID=31000"));
    EXPECT_EQ("other-server", queryScalar("SELECT Name FROM CastleInfo WHERE ServerID=31001 AND ZoneID=31000"));

    // tinysave applies the caller's SET fragment verbatim and reports
    // whether a row changed.
    EXPECT_TRUE(defaultWarInfoRepository().tinysaveCastle("TaxBalance=1", 31000, 31000));
    EXPECT_EQ("1", queryScalar("SELECT TaxBalance FROM CastleInfo WHERE ServerID=31000 AND ZoneID=31000"));
    EXPECT_FALSE(defaultWarInfoRepository().tinysaveCastle("TaxBalance=1", 31002, 31000)); // no such zone
}

TEST_F(WarInfoMySQL, SweeperBonusOwnersAreFilteredByLevelAndSavedByType) {
    // Type is a BYTE at the callers (SweeperBonusType_t), so the rows sit
    // at 250/251; the seed uses 0..11 with levels 0..3.
    execSQL("INSERT INTO SweeperBonusInfo (Type, Name, OptionList, OwnerRace, Level) "
            "VALUES (250, 'it-a', 'ATTR+2', 0, 200)");
    execSQL("INSERT INTO SweeperBonusInfo (Type, Name, OptionList, OwnerRace, Level) "
            "VALUES (251, 'it-b', 'DAM+3', 1, 201)");

    int maxType = 0;
    ASSERT_TRUE(defaultWarInfoRepository().loadMaxSweeperBonusType(maxType));
    EXPECT_EQ(251, maxType);

    std::vector<SweeperBonusRow> all = defaultWarInfoRepository().loadSweeperBonuses();
    bool seen = false;
    for (size_t r = 0; r < all.size(); r++) {
        if (all[r].type == 250) {
            seen = true;
            EXPECT_EQ("it-a", all[r].name);
            EXPECT_EQ("ATTR+2", all[r].optionList);
            EXPECT_EQ(0, all[r].ownerRace);
            EXPECT_EQ(200, all[r].level);
        }
    }
    EXPECT_TRUE(seen);

    std::vector<SweeperBonusOwnerRow> owners = defaultWarInfoRepository().loadSweeperBonusOwners(200);
    ASSERT_EQ(1u, owners.size());
    EXPECT_EQ(250, owners[0].type);
    EXPECT_EQ(0, owners[0].ownerRace);

    defaultWarInfoRepository().saveSweeperBonusOwner((Race_t)2, (SweeperBonusType_t)250);
    EXPECT_EQ("2", queryScalar("SELECT OwnerRace FROM SweeperBonusInfo WHERE Type=250"));
    EXPECT_EQ("1", queryScalar("SELECT OwnerRace FROM SweeperBonusInfo WHERE Type=251"));
}

TEST_F(WarInfoMySQL, SweeperSetsAndOwnersAreScopedToTheZoneAndTheOwnerSaveKeysOnTypeAlone) {
    execSQL("INSERT INTO SweeperSetInfo (ID, Name, ZoneID, ItemType, SlayerX, SlayerY, SlayerMType, VampireX, "
            "VampireY, VampireMType, OustersX, OustersY, OustersMType, DefaultX, DefaultY, DefaultMType) "
            "VALUES (31000, 'it-set', 31000, 3, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12)");
    execSQL("INSERT INTO SweeperSetInfo (ID, Name, ZoneID, ItemType) VALUES (31001, 'other-zone', 31001, 4)");
    execSQL("INSERT INTO SweeperOwnerInfo (SweeperType, ZoneID, OwnerRace, SweeperSafeType) "
            "VALUES (31000, 31000, 1, 3)");
    execSQL("INSERT INTO SweeperOwnerInfo (SweeperType, ZoneID, OwnerRace, SweeperSafeType) "
            "VALUES (31001, 31001, 0, 4)");

    std::vector<SweeperSetRow> sets = defaultWarInfoRepository().loadSweeperSets(IT_ZONE);
    ASSERT_EQ(1u, sets.size());
    EXPECT_EQ(3, sets[0].itemType);
    EXPECT_EQ(1, sets[0].slayerX);
    EXPECT_EQ(2, sets[0].slayerY);
    EXPECT_EQ(3, sets[0].slayerMonsterType);
    EXPECT_EQ(4, sets[0].vampireX);
    EXPECT_EQ(5, sets[0].vampireY);
    EXPECT_EQ(6, sets[0].vampireMonsterType);
    EXPECT_EQ(7, sets[0].oustersX);
    EXPECT_EQ(8, sets[0].oustersY);
    EXPECT_EQ(9, sets[0].oustersMonsterType);
    EXPECT_EQ(10, sets[0].defaultX);
    EXPECT_EQ(11, sets[0].defaultY);
    EXPECT_EQ(12, sets[0].defaultMonsterType);
    EXPECT_EQ("it-set", sets[0].name);

    std::vector<SweeperOwnerRow> owners = defaultWarInfoRepository().loadSweeperOwners(IT_ZONE);
    ASSERT_EQ(1u, owners.size());
    EXPECT_EQ(31000, owners[0].sweeperType);
    EXPECT_EQ(1, owners[0].ownerRace);
    EXPECT_EQ(3, owners[0].sweeperSafeType);

    std::vector<SweeperBonusOwnerRow> races = defaultWarInfoRepository().loadSweeperOwnerRaces(IT_ZONE);
    ASSERT_EQ(1u, races.size());
    EXPECT_EQ(31000, races[0].type);
    EXPECT_EQ(1, races[0].ownerRace);

    // The UPDATE keys on SweeperType alone (the table's PK), not on ZoneID.
    defaultWarInfoRepository().saveSweeperOwner(2, 5, 31000u);
    EXPECT_EQ("2", queryScalar("SELECT OwnerRace FROM SweeperOwnerInfo WHERE SweeperType=31000"));
    EXPECT_EQ("5", queryScalar("SELECT SweeperSafeType FROM SweeperOwnerInfo WHERE SweeperType=31000"));
    EXPECT_EQ("0", queryScalar("SELECT OwnerRace FROM SweeperOwnerInfo WHERE SweeperType=31001"));
}

TEST_F(WarInfoMySQL, LevelWarHistoryInsertThenUpdateFillsOnlyThatWarsNewColumns) {
    WarInfoRepository& repository = defaultWarInfoRepository();
    repository.insertLevelWarHistory(31000, "it-a", "1|", "2|", "3|", "4|");
    repository.insertLevelWarHistory(31000, "it-b", "", "", "", "");

    EXPECT_EQ("2", queryScalar("SELECT COUNT(*) FROM LevelWarHistory WHERE Level=31000"));
    EXPECT_EQ("1|",
              queryScalar("SELECT SlayerOldSweeper FROM LevelWarHistory WHERE Level=31000 AND LevelWarID='it-a'"));
    EXPECT_EQ("4|",
              queryScalar("SELECT DefaultOldSweeper FROM LevelWarHistory WHERE Level=31000 AND LevelWarID='it-a'"));
    EXPECT_EQ("", queryScalar("SELECT SlayerSweeper FROM LevelWarHistory WHERE Level=31000 AND LevelWarID='it-a'"));

    repository.updateLevelWarHistory("5|", "6|", "7|", "8|", 31000, "it-a");

    EXPECT_EQ("5|", queryScalar("SELECT SlayerSweeper FROM LevelWarHistory WHERE Level=31000 AND LevelWarID='it-a'"));
    EXPECT_EQ("6|", queryScalar("SELECT VampireSweeper FROM LevelWarHistory WHERE Level=31000 AND LevelWarID='it-a'"));
    EXPECT_EQ("7|", queryScalar("SELECT OustersSweeper FROM LevelWarHistory WHERE Level=31000 AND LevelWarID='it-a'"));
    EXPECT_EQ("8|", queryScalar("SELECT DefaultSweeper FROM LevelWarHistory WHERE Level=31000 AND LevelWarID='it-a'"));
    EXPECT_EQ("1|",
              queryScalar("SELECT SlayerOldSweeper FROM LevelWarHistory WHERE Level=31000 AND LevelWarID='it-a'"));
    EXPECT_EQ("", queryScalar("SELECT SlayerSweeper FROM LevelWarHistory WHERE Level=31000 AND LevelWarID='it-b'"));
}

TEST_F(WarInfoMySQL, MasterLairRowsCarryAllTwentyFiveColumns) {
    execSQL("INSERT INTO MasterLairInfo (ZoneID, MasterNotReadyMonsterType, MasterMonsterType, MasterRemainNotReady, "
            "MasterX, MasterY, MasterDir, MaxPassPlayer, SummonX, SummonY, FirstRegenDelay, RegenDelay, StartDelay, "
            "EndDelay, KickOutDelay, KickZoneID, KickZoneX, KickZoneY, LairAttackTick, LairAttackMinNumber, "
            "LairAttackMaxNumber, MasterSummonSay, MasterDeadSlayerSay, MasterDeadVampireSay, MasterNotDeadSay) "
            "VALUES (31000, 1, 2, 1, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, "
            "'\"s\"', '\"ds\"', '\"dv\"', '\"nd\"')");

    std::vector<MasterLairRow> rows = defaultWarInfoRepository().loadMasterLairs();
    const MasterLairRow* mine = NULL;
    for (size_t r = 0; r < rows.size(); r++)
        if (rows[r].zoneID == 31000)
            mine = &rows[r];
    ASSERT_TRUE(mine != NULL);
    EXPECT_EQ(1, mine->masterNotReadyMonsterType);
    EXPECT_EQ(2, mine->masterMonsterType);
    EXPECT_EQ(1, mine->masterRemainNotReady);
    EXPECT_EQ(4, mine->masterX);
    EXPECT_EQ(5, mine->masterY);
    EXPECT_EQ(6, mine->masterDir);
    EXPECT_EQ(7, mine->maxPassPlayer);
    EXPECT_EQ(8, mine->summonX);
    EXPECT_EQ(9, mine->summonY);
    EXPECT_EQ(10, mine->firstRegenDelay);
    EXPECT_EQ(11, mine->regenDelay);
    EXPECT_EQ(12, mine->startDelay);
    EXPECT_EQ(13, mine->endDelay);
    EXPECT_EQ(14, mine->kickOutDelay);
    EXPECT_EQ(15, mine->kickZoneID);
    EXPECT_EQ(16, mine->kickZoneX);
    EXPECT_EQ(17, mine->kickZoneY);
    EXPECT_EQ(18, mine->lairAttackTick);
    EXPECT_EQ(19, mine->lairAttackMinNumber);
    EXPECT_EQ(20, mine->lairAttackMaxNumber);
    EXPECT_EQ("\"s\"", mine->masterSummonSay);
    EXPECT_EQ("\"ds\"", mine->masterDeadSlayerSay);
    EXPECT_EQ("\"dv\"", mine->masterDeadVampireSay);
    EXPECT_EQ("\"nd\"", mine->masterNotDeadSay);
}

// --- the item bookkeeping cluster against real MySQL --------------------
// Seven item files, one seam plus four option-table loads on the game-info
// seam. The tests work on rows they insert (ids from 31000 up; ItemClass
// 250 for UniqueItemInfo's tinyint key; 'it-' owners) and clean them in
// SetUp/TearDown. PotionObject stands in for the per-class item-object
// tables whose NAME the seam takes as data.

class ItemMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        clean();
    }
    virtual void TearDown() {
        clean();
    }
    static void clean() {
        execSQL("DELETE FROM ItemTraceLog WHERE OwnerID LIKE 'it-%'");
        execSQL("DELETE FROM MoneyTraceLog WHERE OwnerID LIKE 'it-%'");
        execSQL("DELETE FROM EventQuestRewardSchedule WHERE RewardID >= 31000");
        execSQL("DELETE FROM UniqueItemInfo WHERE ItemClass >= 250");
        execSQL("DELETE FROM TimeLimitItems WHERE OwnerID LIKE 'it-%'");
        execSQL("DELETE FROM CardCount WHERE CARDKIND >= 31000");
        execSQL("DELETE FROM LuckyBagCount WHERE BAGKIND >= 31000");
        execSQL("DELETE FROM GiftBoxCount WHERE BOXKIND >= 31000");
        execSQL("DELETE FROM EventItemCount WHERE ItemClass >= 31000");
        execSQL("DELETE FROM ResurrectItemCount WHERE Count >= 31000");
        execSQL("DELETE FROM PotionObject WHERE ItemID >= 31000");
    }
};

TEST_F(ItemMySQL, TraceLogsAreInsertedWithTheirEnumTextsAndAServerSideTime) {
    ItemTraceRecord record;
    record.itemID = 31000;
    record.itemClass = "it-class";
    record.itemType = 7;
    record.optionName = "1,2";
    record.preOwner = "it-pre";
    record.owner = "it-own";
    record.logType = "TRADE";
    record.detailType = "PICKUP";
    defaultItemRepository().insertItemTraceLog(record);

    EXPECT_EQ("1", queryScalar("SELECT COUNT(*) FROM ItemTraceLog WHERE ItemID=31000 AND OwnerID='it-own'"));
    EXPECT_EQ("it-class", queryScalar("SELECT ItemClass FROM ItemTraceLog WHERE ItemID=31000"));
    EXPECT_EQ("7", queryScalar("SELECT ItemType FROM ItemTraceLog WHERE ItemID=31000"));
    EXPECT_EQ("1,2", queryScalar("SELECT OptionType FROM ItemTraceLog WHERE ItemID=31000"));
    EXPECT_EQ("it-pre", queryScalar("SELECT PreOwnerID FROM ItemTraceLog WHERE ItemID=31000"));
    EXPECT_EQ("TRADE", queryScalar("SELECT LogType FROM ItemTraceLog WHERE ItemID=31000"));
    EXPECT_EQ("PICKUP", queryScalar("SELECT DetailType FROM ItemTraceLog WHERE ItemID=31000"));
    EXPECT_NE("0000-00-00 00:00:00", queryScalar("SELECT Time FROM ItemTraceLog WHERE ItemID=31000"));

    defaultItemRepository().insertMoneyTraceLog("it-pre", "it-own", "TRADE", "DROP", 12345);

    EXPECT_EQ("1", queryScalar("SELECT COUNT(*) FROM MoneyTraceLog WHERE OwnerID='it-own'"));
    EXPECT_EQ("it-pre", queryScalar("SELECT PreOwnerID FROM MoneyTraceLog WHERE OwnerID='it-own'"));
    EXPECT_EQ("TRADE", queryScalar("SELECT LogType FROM MoneyTraceLog WHERE OwnerID='it-own'"));
    EXPECT_EQ("DROP", queryScalar("SELECT DetailType FROM MoneyTraceLog WHERE OwnerID='it-own'"));
    EXPECT_EQ("12345", queryScalar("SELECT Amount FROM MoneyTraceLog WHERE OwnerID='it-own'"));
    EXPECT_NE("0000-00-00 00:00:00", queryScalar("SELECT Time FROM MoneyTraceLog WHERE OwnerID='it-own'"));
}

TEST_F(ItemMySQL, UniqueItemNumbersAreReadAndCountedPerClassAndType) {
    execSQL("INSERT INTO UniqueItemInfo (ItemClass, ItemType, LimitNumber, CurrentNumber, ItemClassName) "
            "VALUES (250, 1, 3, 2, 'it-a')");
    execSQL("INSERT INTO UniqueItemInfo (ItemClass, ItemType, LimitNumber, CurrentNumber, ItemClassName) "
            "VALUES (250, 2, 0, 9, 'it-b')");
    execSQL("INSERT INTO UniqueItemInfo (ItemClass, ItemType, LimitNumber, CurrentNumber, ItemClassName) "
            "VALUES (250, 3, 1, 0, 'it-c')");

    ItemRepository& repository = defaultItemRepository();

    std::vector<UniqueItemRow> rows = repository.loadUniqueItems();
    int seen = 0;
    for (size_t r = 0; r < rows.size(); r++)
        if (rows[r].itemClass == 250)
            seen++;
    EXPECT_EQ(3, seen);

    int limit = -1, current = -1;
    ASSERT_TRUE(repository.loadUniqueItemNumbers(250, 1, limit, current));
    EXPECT_EQ(3, limit);
    EXPECT_EQ(2, current);
    EXPECT_FALSE(repository.loadUniqueItemNumbers(250, 4, limit, current)); // no such row

    repository.incrementUniqueItemCount(250, 1);
    EXPECT_EQ("3", queryScalar("SELECT CurrentNumber FROM UniqueItemInfo WHERE ItemClass=250 AND ItemType=1"));
    repository.decrementUniqueItemCount(250, 1);
    repository.decrementUniqueItemCount(250, 1);
    EXPECT_EQ("1", queryScalar("SELECT CurrentNumber FROM UniqueItemInfo WHERE ItemClass=250 AND ItemType=1"));
    EXPECT_EQ("9", queryScalar("SELECT CurrentNumber FROM UniqueItemInfo WHERE ItemClass=250 AND ItemType=2"));

    // CurrentNumber is UNSIGNED: a decrement at 0 is an out-of-range error
    // (ER 1690), not a clamp — the statement fails, the row is untouched,
    // and the failure surfaces as END_DB's rethrow. Pinned as observed.
    EXPECT_ANY_THROW(repository.decrementUniqueItemCount(250, 3));
    EXPECT_EQ("0", queryScalar("SELECT CurrentNumber FROM UniqueItemInfo WHERE ItemClass=250 AND ItemType=3"));
}

TEST_F(ItemMySQL, TimeLimitItemsAreLoadedByOwnerAndStatusAndUpdatedByOwnerClassAndId) {
    ItemRepository& repository = defaultItemRepository();
    repository.insertTimeLimitItem("it-owner", 5, 31000u, "2030-01-02 03:04:05");
    repository.insertTimeLimitItem("it-owner", 6, 31001u, "2031-01-02 03:04:05");
    repository.insertTimeLimitItem("it-other", 5, 31002u, "2032-01-02 03:04:05");

    std::vector<TimeLimitItemRow> rows = repository.loadTimeLimitItems("it-owner", 0);
    ASSERT_EQ(2u, rows.size());
    bool seen31000 = false;
    for (size_t r = 0; r < rows.size(); r++) {
        if (rows[r].itemID == 31000) {
            seen31000 = true;
            EXPECT_EQ(5, rows[r].itemClass);
            EXPECT_EQ("2030-01-02 03:04:05", rows[r].limitDateTime);
        }
    }
    EXPECT_TRUE(seen31000);

    EXPECT_TRUE(repository.updateTimeLimitItemStatus(1, "it-owner", 5, 31000u));
    EXPECT_FALSE(repository.updateTimeLimitItemStatus(1, "it-owner", 5, 31009u)); // no such item
    EXPECT_FALSE(repository.updateTimeLimitItemStatus(1, "it-owner", 6, 31000u)); // wrong class

    EXPECT_EQ("1", queryScalar("SELECT Status FROM TimeLimitItems WHERE OwnerID='it-owner' AND ItemID=31000"));
    EXPECT_EQ("0", queryScalar("SELECT Status FROM TimeLimitItems WHERE OwnerID='it-owner' AND ItemID=31001"));

    rows = repository.loadTimeLimitItems("it-owner", 0);
    ASSERT_EQ(1u, rows.size());
    EXPECT_EQ(31001, rows[0].itemID);
    EXPECT_EQ(1u, repository.loadTimeLimitItems("it-other", 0).size());
}

TEST_F(ItemMySQL, EventCountersIncrementOnlyTheirRowExceptTheKeylessResurrectCount) {
    execSQL("INSERT INTO CardCount (CARDKIND, CARDCOUNT) VALUES (31000, 5)");
    execSQL("INSERT INTO CardCount (CARDKIND, CARDCOUNT) VALUES (31001, 5)");
    execSQL("INSERT INTO LuckyBagCount (BAGKIND, BAGCOUNT) VALUES (31000, 5)");
    execSQL("INSERT INTO LuckyBagCount (BAGKIND, BAGCOUNT) VALUES (31001, 5)");
    execSQL("INSERT INTO GiftBoxCount (BOXKIND, BOXCOUNT) VALUES (31000, 5)");
    execSQL("INSERT INTO GiftBoxCount (BOXKIND, BOXCOUNT) VALUES (31001, 5)");
    execSQL("INSERT INTO EventItemCount (ItemClass, ItemType, Count) VALUES (31000, 1, 5)");
    execSQL("INSERT INTO EventItemCount (ItemClass, ItemType, Count) VALUES (31000, 2, 5)");

    ItemRepository& repository = defaultItemRepository();
    repository.incrementCardCount(31000);
    repository.incrementLuckyBagCount(31000);
    repository.incrementGiftBoxCount(31000);
    repository.incrementEventItemCount(31000u, 1u);

    EXPECT_EQ("6", queryScalar("SELECT CARDCOUNT FROM CardCount WHERE CARDKIND=31000"));
    EXPECT_EQ("5", queryScalar("SELECT CARDCOUNT FROM CardCount WHERE CARDKIND=31001"));
    EXPECT_EQ("6", queryScalar("SELECT BAGCOUNT FROM LuckyBagCount WHERE BAGKIND=31000"));
    EXPECT_EQ("5", queryScalar("SELECT BAGCOUNT FROM LuckyBagCount WHERE BAGKIND=31001"));
    EXPECT_EQ("6", queryScalar("SELECT BOXCOUNT FROM GiftBoxCount WHERE BOXKIND=31000"));
    EXPECT_EQ("5", queryScalar("SELECT BOXCOUNT FROM GiftBoxCount WHERE BOXKIND=31001"));
    EXPECT_EQ("6", queryScalar("SELECT Count FROM EventItemCount WHERE ItemClass=31000 AND ItemType=1"));
    EXPECT_EQ("5", queryScalar("SELECT Count FROM EventItemCount WHERE ItemClass=31000 AND ItemType=2"));

    // ResurrectItemCount has no key column: the UPDATE touches every row.
    // The seed has one row, so a second is inserted (and cleaned by value)
    // to tell an every-row UPDATE from a one-row one.
    execSQL("INSERT INTO ResurrectItemCount (Count) VALUES (31000)");
    int before = atoi(queryScalar("SELECT SUM(Count) FROM ResurrectItemCount").c_str());
    int rowCount = atoi(queryScalar("SELECT COUNT(*) FROM ResurrectItemCount").c_str());
    ASSERT_GE(rowCount, 2);
    repository.incrementResurrectItemCount();
    EXPECT_EQ(before + rowCount, atoi(queryScalar("SELECT SUM(Count) FROM ResurrectItemCount").c_str()));
    EXPECT_EQ("1", queryScalar("SELECT COUNT(*) FROM ResurrectItemCount WHERE Count=31001"));
}

TEST_F(ItemMySQL, EventQuestRewardIsTakenOncePerCountAndOnlyWhenDue) {
    execSQL("INSERT INTO EventQuestRewardSchedule (RewardID, QuestLevel, Count, Time) "
            "VALUES (31000, 3, 1, '2000-01-01 00:00:00')");
    execSQL("INSERT INTO EventQuestRewardSchedule (RewardID, QuestLevel, Count, Time) "
            "VALUES (31001, 3, 5, '2099-01-01 00:00:00')"); // not due yet

    ItemRepository& repository = defaultItemRepository();
    EXPECT_TRUE(repository.takeEventQuestReward(31000, 3));
    EXPECT_EQ("0", queryScalar("SELECT Count FROM EventQuestRewardSchedule WHERE RewardID=31000"));
    EXPECT_FALSE(repository.takeEventQuestReward(31000, 3)); // exhausted
    EXPECT_FALSE(repository.takeEventQuestReward(31000, 4)); // other level
    EXPECT_FALSE(repository.takeEventQuestReward(31001, 3)); // not due
    EXPECT_EQ("5", queryScalar("SELECT Count FROM EventQuestRewardSchedule WHERE RewardID=31001"));
}

TEST_F(ItemMySQL, ItemRowPositionReadAndDeleteWorkOnTheNamedObjectTable) {
    execSQL("INSERT INTO PotionObject (ItemID, ObjectID, ItemType, OwnerID, Storage, StorageID, X, Y, Num) "
            "VALUES (31000, 77, 2, 'it-owner', 1, 5, 3, 4, 1)");

    ItemRepository& repository = defaultItemRepository();
    ItemPositionRow row;
    ASSERT_TRUE(repository.loadItemPosition("PotionObject", 31000, row));
    EXPECT_EQ("it-owner", row.ownerID);
    EXPECT_EQ(1, row.storage);
    EXPECT_EQ(5, row.storageID);
    EXPECT_EQ(3, row.x);
    EXPECT_EQ(4, row.y);
    EXPECT_EQ(77, row.objectID);
    EXPECT_FALSE(repository.loadItemPosition("PotionObject", 31001, row));

    EXPECT_TRUE(repository.deleteItemRow("PotionObject", 31000));
    EXPECT_FALSE(repository.deleteItemRow("PotionObject", 31000)); // already gone
    EXPECT_EQ("0", queryScalar("SELECT COUNT(*) FROM PotionObject WHERE ItemID=31000"));
}

TEST(ConfigLoadersMySQL, OptionInfoIsReadInSelectOrderAndTheOtherOptionTablesAreSeeded) {
    GameInfoRepository& repository = defaultGameInfoRepository();

    std::vector<OptionInfoRow> options = repository.loadOptionInfos();
    ASSERT_FALSE(options.empty());
    // The schema orders UpgradeOptionType before PreviousOptionType; the
    // SELECT (and so the row) the other way round. Pin the mapping.
    std::string id = std::to_string(options[0].optionType);
    EXPECT_EQ(options[0].name, queryScalar("SELECT Name FROM OptionInfo WHERE OptionType=" + id));
    EXPECT_EQ(options[0].nickname, queryScalar("SELECT Nickname FROM OptionInfo WHERE OptionType=" + id));
    EXPECT_EQ(std::to_string(options[0].optionClass),
              queryScalar("SELECT Class FROM OptionInfo WHERE OptionType=" + id));
    EXPECT_EQ(std::to_string(options[0].previousOptionType),
              queryScalar("SELECT PreviousOptionType FROM OptionInfo WHERE OptionType=" + id));
    EXPECT_EQ(std::to_string(options[0].upgradeOptionType),
              queryScalar("SELECT UpgradeOptionType FROM OptionInfo WHERE OptionType=" + id));
    EXPECT_EQ(std::to_string(options[0].grade), queryScalar("SELECT Grade FROM OptionInfo WHERE OptionType=" + id));

    EXPECT_FALSE(repository.loadOptionClassInfos().empty());
    EXPECT_FALSE(repository.loadRareEnchantInfos().empty());
    EXPECT_FALSE(repository.loadPetEnchantOptionRatios().empty());
}

// --- the content-info cluster against real MySQL ---------------------------
// Monsters, skill balance, NPCs, scripts, directive sets and variables. The
// tables are seeded; the tests insert their own rows (ids from 31000 up)
// and clean them in SetUp/TearDown.

class ContentInfoMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        clean();
    }
    virtual void TearDown() {
        clean();
    }
    static void clean() {
        execSQL("DELETE FROM MonsterInfo WHERE MType >= 31000");
        execSQL("DELETE FROM SkillBalance WHERE Type >= 31000");
        execSQL("DELETE FROM NPC WHERE ZoneID >= 31000");
        execSQL("DELETE FROM Script WHERE ScriptID >= 31000");
        execSQL("DELETE FROM DirectiveSet WHERE ID >= 31000");
        execSQL("DELETE FROM AttrInfo WHERE attrID >= 31000");
    }
    static int scalar(const std::string& sql) {
        return atoi(queryScalar(sql).c_str());
    }
};

TEST_F(ContentInfoMySQL, SeededReadsAreNonEmptyAndEveryMaxProbeMatchesItsTable) {
    ContentInfoRepository& repository = defaultContentInfoRepository();
    int maximum = -1;

    ASSERT_TRUE(repository.loadMaxMonsterType(maximum));
    EXPECT_EQ(scalar("SELECT MAX(MType) FROM MonsterInfo"), maximum);
    std::vector<MonsterInfoRow> monsters = repository.loadMonsterInfos();
    EXPECT_FALSE(monsters.empty());
    EXPECT_EQ(monsters.size(), repository.loadMonsterSummonInfos().size());
    EXPECT_EQ(monsters.size(), repository.loadMonsterInfosForReload().size());

    ASSERT_TRUE(repository.loadMaxSkillBalanceType(maximum));
    EXPECT_EQ(scalar("SELECT MAX(Type) FROM SkillBalance"), maximum);
    EXPECT_FALSE(repository.loadSkillBalances().empty());

    EXPECT_FALSE(repository.loadScripts().empty());

    ASSERT_TRUE(repository.loadMaxDirectiveSetID(maximum));
    EXPECT_EQ(scalar("SELECT MAX(ID) FROM DirectiveSet"), maximum);
    EXPECT_FALSE(repository.loadDirectiveSets().empty());

    ASSERT_TRUE(repository.loadMaxAttrID(maximum));
    EXPECT_EQ(scalar("SELECT MAX(attrID) FROM AttrInfo"), maximum);
    EXPECT_FALSE(repository.loadVariables().empty());

    ASSERT_TRUE(defaultZoneInfoRepository().loadMaxZoneGroupID(maximum));
    EXPECT_EQ(scalar("SELECT MAX(ZoneGroupID) FROM ZoneGroupInfo"), maximum);
}

TEST_F(ContentInfoMySQL, MonsterRowsCarryEveryColumnAndReloadReadsOneTypeOrAll) {
    execSQL("INSERT INTO MonsterInfo (MType, SType, HName, EName, Level, STR, DEX, INTE, BSize, Exp, MColor, SColor, "
            "Align, AOrder, Moral, Delay, ADelay, Sight, MeleeRange, MissileRange, RegenPortal, RegenInvisible, "
            "RegenBat, UnburrowChance, MMode, AIType, Enhance, Master, ClanType, MonsterSummonInfo, DefaultEffects, "
            "Chief, NormalRegen, HasTreasure, MonsterClass, SkullType) VALUES (31000, 2, 'it-h', 'it-e', 5, 6, 7, 8, "
            "9, 10, 11, 12, 1, 2, 15, 16, 17, 18, 19, 20, 21, 22, 23, 27, 'FLY', 25, 'e26', 1, 29, 's30', 'd31', 1, "
            "0, 1, 35, 36)");

    ContentInfoRepository& repository = defaultContentInfoRepository();

    std::vector<MonsterInfoRow> all = repository.loadMonsterInfos();
    const MonsterInfoRow* mine = NULL;
    for (size_t r = 0; r < all.size(); r++)
        if (all[r].monsterType == 31000)
            mine = &all[r];
    ASSERT_TRUE(mine != NULL);
    EXPECT_EQ(2, mine->spriteType);
    EXPECT_EQ("it-h", mine->hName);
    EXPECT_EQ("it-e", mine->eName);
    EXPECT_EQ(5, mine->level);
    EXPECT_EQ(6, mine->str);
    EXPECT_EQ(7, mine->dex);
    EXPECT_EQ(8, mine->inte);
    EXPECT_EQ(9, mine->bodySize);
    EXPECT_EQ(10, mine->exp);
    EXPECT_EQ(11, mine->mainColor);
    EXPECT_EQ(12, mine->subColor);
    EXPECT_EQ(1, mine->alignment);
    EXPECT_EQ(2, mine->attackOrder);
    EXPECT_EQ(15, mine->moral);
    EXPECT_EQ(16, mine->delay);
    EXPECT_EQ(17, mine->attackDelay);
    EXPECT_EQ(18, mine->sight);
    EXPECT_EQ(19, mine->meleeRange);
    EXPECT_EQ(20, mine->missileRange);
    EXPECT_EQ(21, mine->regenPortal);
    EXPECT_EQ(22, mine->regenInvisible);
    EXPECT_EQ(23, mine->regenBat);
    EXPECT_EQ("FLY", mine->moveMode);
    EXPECT_EQ(25, mine->aiType);
    EXPECT_EQ("e26", mine->enhance);
    EXPECT_EQ(27, mine->unburrowChance);
    EXPECT_EQ(1, mine->master);
    EXPECT_EQ(29, mine->clanType);
    EXPECT_EQ("d31", mine->defaultEffects);
    EXPECT_EQ(1, mine->chief);
    EXPECT_EQ(0, mine->normalRegen);
    EXPECT_EQ(1, mine->hasTreasure);
    EXPECT_EQ(35, mine->monsterClass);
    EXPECT_EQ(36, mine->skullType);

    std::vector<MonsterSummonRow> summons = repository.loadMonsterSummonInfos();
    bool seenSummon = false;
    for (size_t r = 0; r < summons.size(); r++) {
        if (summons[r].monsterType == 31000) {
            seenSummon = true;
            EXPECT_EQ("s30", summons[r].summonInfo);
        }
    }
    EXPECT_TRUE(seenSummon);

    std::vector<MonsterReloadRow> one = repository.loadMonsterInfoForReload(31000);
    ASSERT_EQ(1u, one.size());
    EXPECT_EQ(31000, one[0].monsterType);
    EXPECT_EQ("it-e", one[0].eName);
    EXPECT_EQ(20, one[0].missileRange);
    EXPECT_EQ("FLY", one[0].moveMode);
    EXPECT_EQ("s30", one[0].monsterSummonInfo);
    EXPECT_EQ("d31", one[0].defaultEffects);
    EXPECT_EQ(0, one[0].normalRegen);
    EXPECT_TRUE(repository.loadMonsterInfoForReload(31001).empty());
    EXPECT_EQ(all.size(), repository.loadMonsterInfosForReload().size());
}

TEST_F(ContentInfoMySQL, SkillBalanceRowsCarryEveryColumnWithByteDomains) {
    execSQL("INSERT INTO SkillBalance (Type, Name, HName, Level, MinDam, MaxDam, MinDelay, MaxDelay, MinDur, MaxDur, "
            "Mana, MinRange, MaxRange, Target, SubExp, Point, Domain, MagicDomain, Melee, Magic, Physic, SkillPoint, "
            "LevelUpPoint, RequireSkill, `Condition`, ElementalDomain, CanDelete) VALUES (31000, 'it-skill', '', 3, "
            "4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 3, 4, 18, 19, 20, 21, 22, 'r23', 'c24', 25, 1)");

    std::vector<SkillBalanceRow> rows = defaultContentInfoRepository().loadSkillBalances();
    const SkillBalanceRow* mine = NULL;
    for (size_t r = 0; r < rows.size(); r++)
        if (rows[r].type == 31000)
            mine = &rows[r];
    ASSERT_TRUE(mine != NULL);
    EXPECT_EQ("it-skill", mine->name);
    EXPECT_EQ(3, mine->level);
    EXPECT_EQ(4, mine->minDamage);
    EXPECT_EQ(5, mine->maxDamage);
    EXPECT_EQ(6, mine->minDelay);
    EXPECT_EQ(7, mine->maxDelay);
    EXPECT_EQ(8, mine->minDuration);
    EXPECT_EQ(9, mine->maxDuration);
    EXPECT_EQ(10, mine->mana);
    EXPECT_EQ(11, mine->minRange);
    EXPECT_EQ(12, mine->maxRange);
    EXPECT_EQ(13, mine->target);
    EXPECT_EQ(14, mine->subExp);
    EXPECT_EQ(15, mine->point);
    EXPECT_EQ(3, (int)mine->domain);
    EXPECT_EQ(4, (int)mine->magicDomain);
    EXPECT_EQ(18, mine->melee);
    EXPECT_EQ(19, mine->magic);
    EXPECT_EQ(20, mine->physic);
    EXPECT_EQ(21, mine->skillPoint);
    EXPECT_EQ(22, mine->levelUpPoint);
    EXPECT_EQ("r23", mine->requireSkill);
    EXPECT_EQ("c24", mine->condition);
    EXPECT_EQ(25, mine->elementalDomain);
    EXPECT_EQ(1, mine->canDelete);
}

TEST_F(ContentInfoMySQL, NPCsAreScopedToTheZoneAndOptionallyTheRace) {
    execSQL("INSERT INTO NPC (Name, NPCID, SpriteType, Race, MainColor, SubColor, ZoneID, ClanType, ShowInMinimap, "
            "Description, TaxingCastleZoneID) VALUES ('it-npc-a', 1, 2, 1, 4, 5, 31000, 7, 1, '', 9)");
    execSQL("INSERT INTO NPC (Name, NPCID, SpriteType, Race, MainColor, SubColor, ZoneID, ClanType, ShowInMinimap, "
            "Description, TaxingCastleZoneID) VALUES ('it-npc-b', 11, 12, 2, 14, 15, 31000, 17, 0, '', 19)");
    execSQL("INSERT INTO NPC (Name, NPCID, SpriteType, Race, MainColor, SubColor, ZoneID, ClanType, ShowInMinimap, "
            "Description, TaxingCastleZoneID) VALUES ('it-npc-c', 21, 22, 2, 24, 25, 31001, 27, 1, '', 29)");

    ContentInfoRepository& repository = defaultContentInfoRepository();

    std::vector<NPCRow> zone = repository.loadNPCs(31000);
    ASSERT_EQ(2u, zone.size());

    std::vector<NPCRow> race = repository.loadNPCsOfRace(31000, 2);
    ASSERT_EQ(1u, race.size());
    EXPECT_EQ("it-npc-b", race[0].name);
    EXPECT_EQ(11, race[0].npcID);
    EXPECT_EQ(12, race[0].spriteType);
    EXPECT_EQ(2, race[0].race);
    EXPECT_EQ(14, race[0].mainColor);
    EXPECT_EQ(15, race[0].subColor);
    EXPECT_EQ(17, race[0].clanType);
    EXPECT_EQ(0, race[0].showInMinimap);
    EXPECT_EQ(19, race[0].taxingCastleZoneID);

    EXPECT_TRUE(repository.loadNPCsOfRace(31000, 3).empty());
    EXPECT_EQ(1u, repository.loadNPCs(31001).size());
}

TEST_F(ContentInfoMySQL, ScriptsComeBackOrderedByScriptID) {
    execSQL("INSERT INTO Script (ScriptID, OwnerID, Subject, Content) VALUES (31001, 'it-owner', 's1**s2', 'c1')");
    execSQL("INSERT INTO Script (ScriptID, OwnerID, Subject, Content) VALUES (31000, 'it-owner', 's0', 'c0**c1')");

    std::vector<ScriptRow> rows = defaultContentInfoRepository().loadScripts();
    int at31000 = -1, at31001 = -1;
    for (size_t r = 0; r < rows.size(); r++) {
        if (rows[r].scriptID == 31000) {
            at31000 = (int)r;
            EXPECT_EQ("it-owner", rows[r].ownerID);
            EXPECT_EQ("s0", rows[r].subject);
            EXPECT_EQ("c0**c1", rows[r].content);
        }
        if (rows[r].scriptID == 31001)
            at31001 = (int)r;
    }
    ASSERT_NE(-1, at31000);
    ASSERT_NE(-1, at31001);
    EXPECT_LT(at31000, at31001); // ORDER BY ScriptID, despite the reversed insert order
    for (size_t r = 1; r < rows.size(); r++)
        EXPECT_LT(rows[r - 1].scriptID, rows[r].scriptID);
}

TEST_F(ContentInfoMySQL, DirectiveSetsAndVariablesAreReadAndTheVariableSaved) {
    execSQL("INSERT INTO DirectiveSet (ID, Name, Content, DeadContent) VALUES (31000, 'it-set', 'live', 'dead')");
    execSQL("INSERT INTO AttrInfo (attrID, attr1, attr2, comm) VALUES (31000, 5, 6, 'it')");

    ContentInfoRepository& repository = defaultContentInfoRepository();

    int maximum = -1;
    ASSERT_TRUE(repository.loadMaxDirectiveSetID(maximum));
    EXPECT_EQ(31000, maximum);
    std::vector<DirectiveSetRow> sets = repository.loadDirectiveSets();
    bool seenSet = false;
    for (size_t r = 0; r < sets.size(); r++) {
        if (sets[r].id == 31000) {
            seenSet = true;
            EXPECT_EQ("it-set", sets[r].name);
            EXPECT_EQ("live", sets[r].content);
            EXPECT_EQ("dead", sets[r].deadContent);
        }
    }
    EXPECT_TRUE(seenSet);

    ASSERT_TRUE(repository.loadMaxAttrID(maximum));
    EXPECT_EQ(31000, maximum);
    std::vector<VariableRow> variables = repository.loadVariables();
    bool seenVariable = false;
    for (size_t r = 0; r < variables.size(); r++) {
        if (variables[r].attrID == 31000) {
            seenVariable = true;
            EXPECT_EQ(5, variables[r].attr1);
            EXPECT_EQ(6, variables[r].attr2);
        }
    }
    EXPECT_TRUE(seenVariable);

    repository.saveVariable(9, 31000);
    EXPECT_EQ("9", queryScalar("SELECT attr1 FROM AttrInfo WHERE attrID=31000"));
    EXPECT_EQ("6", queryScalar("SELECT attr2 FROM AttrInfo WHERE attrID=31000"));
}

// --- the play-record cluster against real MySQL -----------------------------
// Saved quest states, the head-count log and the minigame score board. All
// three tables are unseeded or log-only; the tests use 'it-' owners.

class PlayRecordMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        clean();
    }
    virtual void TearDown() {
        clean();
    }
    static void clean() {
        execSQL("DELETE FROM GQuestSave WHERE OwnerID LIKE 'it-%'");
        execSQL("DELETE FROM HeadCount WHERE Name LIKE 'it-%'");
        execSQL("DELETE FROM MiniGameScores WHERE Name LIKE 'it-%'");
    }
};

TEST_F(PlayRecordMySQL, SavedQuestsAreReplacedLoadedPerOwnerAndDeleted) {
    PlayRecordRepository& repository = defaultPlayRecordRepository();
    repository.replaceSavedQuest(31000, "it-quester", 3);
    repository.replaceSavedQuest(31001, "it-quester", 4);
    repository.replaceSavedQuest(31000, "it-other", 1);

    std::vector<SavedQuestRow> rows = repository.loadSavedQuests("it-quester");
    ASSERT_EQ(2u, rows.size());
    bool seen31000 = false;
    for (size_t r = 0; r < rows.size(); r++) {
        if (rows[r].questID == 31000) {
            seen31000 = true;
            EXPECT_EQ(3, rows[r].status);
        }
        EXPECT_TRUE(rows[r].secondsSinceSave >= 0 && rows[r].secondsSinceSave <= 5);
    }
    EXPECT_TRUE(seen31000);

    // REPLACE on the (QuestID, OwnerID) key rewrites in place.
    repository.replaceSavedQuest(31000, "it-quester", 5);
    EXPECT_EQ("2", queryScalar("SELECT COUNT(*) FROM GQuestSave WHERE OwnerID='it-quester'"));
    EXPECT_EQ("5", queryScalar("SELECT Status FROM GQuestSave WHERE OwnerID='it-quester' AND QuestID=31000"));

    repository.deleteSavedQuest("it-quester", 31000);
    rows = repository.loadSavedQuests("it-quester");
    ASSERT_EQ(1u, rows.size());
    EXPECT_EQ(31001, rows[0].questID);
    EXPECT_EQ("1", queryScalar("SELECT COUNT(*) FROM GQuestSave WHERE OwnerID='it-other'"));
}

TEST_F(PlayRecordMySQL, HeadCountRowsCarryTheLevelsCountAndAServerSideTime) {
    defaultPlayRecordRepository().insertHeadCount("it-head", 30, 35, 7);

    EXPECT_EQ("1", queryScalar("SELECT COUNT(*) FROM HeadCount WHERE Name='it-head'"));
    EXPECT_EQ("30", queryScalar("SELECT FirstLevel FROM HeadCount WHERE Name='it-head'"));
    EXPECT_EQ("35", queryScalar("SELECT LastLevel FROM HeadCount WHERE Name='it-head'"));
    EXPECT_EQ("7", queryScalar("SELECT HeadCount FROM HeadCount WHERE Name='it-head'"));
    EXPECT_NE("2003-01-01 00:00:00", queryScalar("SELECT Time FROM HeadCount WHERE Name='it-head'"));
}

TEST_F(PlayRecordMySQL, MiniGameScoreReadReportsTheRowOrNone) {
    PlayRecordRepository& repository = defaultPlayRecordRepository();
    std::string name;
    int score = -1;

    EXPECT_FALSE(repository.loadMiniGameScore(120, 5, name, score));

    execSQL("INSERT INTO MiniGameScores (Name, Type, Level, Score) VALUES ('it-scorer', 120, 5, 999)");
    ASSERT_TRUE(repository.loadMiniGameScore(120, 5, name, score));
    EXPECT_EQ("it-scorer", name);
    EXPECT_EQ(999, score);
    EXPECT_FALSE(repository.loadMiniGameScore(120, 6, name, score)); // other level
}

// --- the session cluster against real MySQL ----------------------------------
// Session end, the boot sweep, the PC-room lotto and the NetMarble user
// count. Player/PCRoom rows go through the dist connection (same schema),
// UserStatus through the USERINFO connection the tier now registers. All
// rows use 'it-' ids or ids from 31000 up and are cleaned in SetUp/TearDown.

class SessionMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        clean();
    }
    virtual void TearDown() {
        clean();
    }
    static void clean() {
        execSQL("DELETE FROM GuildMember WHERE Name LIKE 'it-%'");
        execSQL("DELETE FROM Player WHERE PlayerID LIKE 'it-%'");
        execSQL("DELETE FROM PCRoomUserInfo WHERE PlayerID LIKE 'it-%'");
        execSQL("DELETE FROM PCRoomLottoObject WHERE PlayerID LIKE 'it-%'");
        execSQL("DELETE FROM UserIPInfo WHERE Name LIKE 'it-%'");
        execSQL("DELETE FROM USERINFO.UserStatus WHERE ServerID >= 31000");
    }
};

TEST_F(SessionMySQL, GuildMemberLogOffFlagsOnlyThatMember) {
    execSQL("INSERT INTO GuildMember (GuildID, Name, Rank, Intro, LogOn) VALUES (5, 'it-gm-a', 1, '', 1)");
    execSQL("INSERT INTO GuildMember (GuildID, Name, Rank, Intro, LogOn) VALUES (5, 'it-gm-b', 1, '', 1)");

    defaultSessionRepository().markGuildMemberLoggedOff("it-gm-a");

    EXPECT_EQ("0", queryScalar("SELECT LogOn FROM GuildMember WHERE Name='it-gm-a'"));
    EXPECT_EQ("1", queryScalar("SELECT LogOn FROM GuildMember WHERE Name='it-gm-b'"));
}

TEST_F(SessionMySQL, SessionEndFlipsOnlyGameRowsAndTheBootSweepClearsThisServersAccounts) {
    execSQL("INSERT INTO Player (PlayerID, LogOn, CurrentWorldID, CurrentServerGroupID) VALUES ('it-p1', 'GAME', 250, "
            "251)");
    execSQL("INSERT INTO Player (PlayerID, LogOn, CurrentWorldID, CurrentServerGroupID) VALUES ('it-p2', 'LOGON', 250, "
            "251)");
    execSQL("INSERT INTO Player (PlayerID, LogOn, CurrentWorldID, CurrentServerGroupID) VALUES ('it-p3', 'GAME', 250, "
            "252)");
    execSQL("INSERT INTO Player (PlayerID, LogOn, CurrentWorldID, CurrentServerGroupID) VALUES ('it-p4', 'GAME', 250, "
            "251)");
    execSQL("INSERT INTO PCRoomUserInfo (ID, PlayerID) VALUES (31000, 'it-p4')");
    execSQL("INSERT INTO PCRoomUserInfo (ID, PlayerID) VALUES (31000, 'it-p3')");
    execSQL("INSERT INTO UserIPInfo (Name, IP, ServerID) VALUES ('it-ip1', 1, 31000)");
    execSQL("INSERT INTO UserIPInfo (Name, IP, ServerID) VALUES ('it-ip2', 2, 31001)");
    execSQL("INSERT INTO UserIPInfo (Name, IP, ServerID) VALUES ('it-ip3', 3, 31001)");

    SessionRepository& repository = defaultSessionRepository();

    // Session end: only a row still in GAME flips, and it gets a logout time.
    repository.markPlayerLoggedOff("it-p1");
    repository.markPlayerLoggedOff("it-p2");
    EXPECT_EQ("LOGOFF", queryScalar("SELECT LogOn FROM Player WHERE PlayerID='it-p1'"));
    EXPECT_NE("0000-00-00 00:00:00", queryScalar("SELECT LastLogoutDate FROM Player WHERE PlayerID='it-p1'"));
    EXPECT_EQ("LOGON", queryScalar("SELECT LogOn FROM Player WHERE PlayerID='it-p2'"));
    EXPECT_EQ("0000-00-00 00:00:00", queryScalar("SELECT LastLogoutDate FROM Player WHERE PlayerID='it-p2'"));

    // The boot sweep sees only this world/server group's GAME rows.
    std::vector<std::string> inGame = repository.loadPlayersInGame(250, 251);
    ASSERT_EQ(1u, inGame.size());
    EXPECT_EQ("it-p4", inGame[0]);

    repository.deletePCRoomUser("it-p4");
    EXPECT_EQ("0", queryScalar("SELECT COUNT(*) FROM PCRoomUserInfo WHERE PlayerID='it-p4'"));
    EXPECT_EQ("1", queryScalar("SELECT COUNT(*) FROM PCRoomUserInfo WHERE PlayerID='it-p3'"));

    repository.logOffPlayersOfServer(250, 251);
    EXPECT_EQ("LOGOFF", queryScalar("SELECT LogOn FROM Player WHERE PlayerID='it-p4'"));
    EXPECT_EQ("GAME", queryScalar("SELECT LogOn FROM Player WHERE PlayerID='it-p3'"));  // other server group
    EXPECT_EQ("LOGON", queryScalar("SELECT LogOn FROM Player WHERE PlayerID='it-p2'")); // not in GAME

    repository.deleteUserIP("it-ip1");
    EXPECT_EQ("0", queryScalar("SELECT COUNT(*) FROM UserIPInfo WHERE Name='it-ip1'"));
    repository.deleteUserIPsOfServer(31001);
    EXPECT_EQ("0", queryScalar("SELECT COUNT(*) FROM UserIPInfo WHERE Name LIKE 'it-%'"));
}

TEST_F(SessionMySQL, SpecialEventCountIsReadSavedAndFalseForAnUnknownAccount) {
    execSQL("INSERT INTO Player (PlayerID, SpecialEventCount) VALUES ('it-p1', 7)");

    SessionRepository& repository = defaultSessionRepository();
    DWORD count = 0;
    ASSERT_TRUE(repository.loadSpecialEventCount("it-p1", count));
    EXPECT_EQ(7u, count);

    repository.saveSpecialEventCount(9, "it-p1");
    EXPECT_EQ("9", queryScalar("SELECT SpecialEventCount FROM Player WHERE PlayerID='it-p1'"));

    EXPECT_FALSE(repository.loadSpecialEventCount("it-none", count));
}

TEST_F(SessionMySQL, PCRoomLottoRowIsInsertedWithItsPositionalColumnsThenCounted) {
    SessionRepository& repository = defaultSessionRepository();
    int amount = -1;

    EXPECT_FALSE(repository.loadPCRoomLottoAmount("it-p1", "it-name", 250, 251, amount));
    EXPECT_EQ(-1, amount); // untouched when there is no row

    repository.insertPCRoomLotto(31000, "it-p1", 250, 251, "it-name", 1);
    ASSERT_TRUE(repository.loadPCRoomLottoAmount("it-p1", "it-name", 250, 251, amount));
    EXPECT_EQ(1, amount);
    EXPECT_EQ("31000", queryScalar("SELECT PCRoomID FROM PCRoomLottoObject WHERE PlayerID='it-p1'"));
    EXPECT_EQ("1", queryScalar("SELECT Race FROM PCRoomLottoObject WHERE PlayerID='it-p1'"));
    EXPECT_EQ("250", queryScalar("SELECT DimensionID FROM PCRoomLottoObject WHERE PlayerID='it-p1'"));
    EXPECT_EQ("251", queryScalar("SELECT WorldID FROM PCRoomLottoObject WHERE PlayerID='it-p1'"));

    repository.updatePCRoomLottoAmount(2, "it-p1", "it-name", 250, 251);
    ASSERT_TRUE(repository.loadPCRoomLottoAmount("it-p1", "it-name", 250, 251, amount));
    EXPECT_EQ(2, amount);

    EXPECT_FALSE(repository.loadPCRoomLottoAmount("it-p1", "it-name", 252, 251, amount)); // other dimension
}

TEST_F(SessionMySQL, UserStatusIsUpdatedOrInsertedOnTheUserInfoDatabase) {
    SessionRepository& repository = defaultSessionRepository();

    EXPECT_FALSE(repository.updateUserStatus(5, 120, 31000)); // no row yet
    repository.insertUserStatus(120, 31000, 5);
    EXPECT_EQ("5", queryScalar("SELECT CurrentUser FROM USERINFO.UserStatus WHERE WorldID=120 AND ServerID=31000"));

    EXPECT_TRUE(repository.updateUserStatus(6, 120, 31000));
    EXPECT_EQ("6", queryScalar("SELECT CurrentUser FROM USERINFO.UserStatus WHERE WorldID=120 AND ServerID=31000"));
    EXPECT_EQ("1", queryScalar("SELECT COUNT(*) FROM USERINFO.UserStatus WHERE ServerID=31000"));

    // CurrentUser is a signed tinyint: a count above 127 is clamped by the
    // tier's non-strict sql_mode, not stored. Pinned as observed.
    EXPECT_TRUE(repository.updateUserStatus(300, 120, 31000));
    EXPECT_EQ("127", queryScalar("SELECT CurrentUser FROM USERINFO.UserStatus WHERE WorldID=120 AND ServerID=31000"));
}

// --- the ExpTable template's generic balance read ---------------------------
// SomethingGrowingUp.h's ExpTable::load names its table and columns; the seam
// formats "SELECT %s, %s, %s FROM %s %s" from them. The seeded RankEXPInfo,
// AdvancementClassEXPInfo and the attribute balance tables stand in.

TEST(ExpTableMySQL, ExpTablesLoadByNamedColumnsWithAndWithoutACondition) {
    BalanceInfoRepository& repository = defaultBalanceInfoRepository();

    std::vector<ExpTableRow> ranks =
        repository.loadExpTable("Level", "GoalExp", "AccumExp", "RankEXPInfo", "where RankType=0");
    ASSERT_FALSE(ranks.empty());
    EXPECT_EQ(atoi(queryScalar("SELECT COUNT(*) FROM RankEXPInfo where RankType=0").c_str()), (int)ranks.size());
    for (size_t r = 0; r < ranks.size(); r++) {
        EXPECT_TRUE(ranks[r].level >= 1 && ranks[r].level <= 50);
        EXPECT_EQ(atoi(queryScalar("SELECT GoalExp FROM RankEXPInfo where RankType=0 AND Level=" +
                                   std::to_string(ranks[r].level))
                           .c_str()),
                  ranks[r].goalExp);
    }

    // No condition: the whole table. The original's trailing space lives only
    // in the statement's bytes; the rows cannot show it.
    std::vector<ExpTableRow> all = repository.loadExpTable("Level", "GoalExp", "AccumExp", "RankEXPInfo", "");
    EXPECT_EQ(atoi(queryScalar("SELECT COUNT(*) FROM RankEXPInfo").c_str()), (int)all.size());

    EXPECT_FALSE(repository.loadExpTable("Level", "GoalExp", "AccumExp", "AdvancementClassEXPInfo", "").empty());
    // STRBalanceInfo.AccumExp exceeds INT_MAX in the top rows; the row's int
    // (getInt = atoi) truncates exactly as the original's getInt did.
    EXPECT_FALSE(repository.loadExpTable("Level", "GoalExp", "AccumExp", "STRBalanceInfo", "").empty());
}

// --- the guild cluster against real MySQL --------------------------------------
// Guilds, members, unions and union offers, plus the guild-scoped war reads.
// GuildInfo and GuildMember are unseeded; the union and war tables are
// seeded, so the tests use ids from 31000 up and clean them in SetUp/TearDown.

class GuildMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        clean();
    }
    virtual void TearDown() {
        clean();
    }
    static void clean() {
        execSQL("DELETE FROM GuildMember WHERE Name LIKE 'it-%'");
        execSQL("DELETE FROM GuildInfo WHERE GuildID >= 31000");
        execSQL("DELETE FROM GuildUnionMember WHERE OwnerGuildID >= 31000");
        execSQL("DELETE FROM GuildUnionInfo WHERE MasterGuildID >= 31000");
        execSQL("DELETE FROM GuildUnionOffer WHERE OwnerGuildID >= 31000");
        execSQL("DELETE FROM WarScheduleInfo WHERE WarID >= 31000");
        execSQL("DELETE FROM ReinforceRegisterInfo WHERE WarID >= 31000");
        execSQL("DELETE FROM CastleInfo WHERE ServerID >= 31000");
    }
};

TEST_F(GuildMySQL, MemberRowsAreCreatedRejoinedSavedExpiredAndDeleted) {
    GuildRepository& repository = defaultGuildRepository();

    EXPECT_FALSE(repository.memberExists("it-m1"));
    repository.insertMember(31000, "it-m1", 2);
    EXPECT_TRUE(repository.memberExists("it-m1"));

    GuildMemberRow row;
    ASSERT_TRUE(repository.loadMember("it-m1", row));
    EXPECT_EQ(31000, row.guildID);
    EXPECT_EQ("it-m1", row.name);
    EXPECT_EQ(2, row.rank);
    EXPECT_EQ(0, row.logOn);
    EXPECT_FALSE(repository.loadMember("it-none", row));

    // Expiring sets the rank and the expiry date.
    repository.setMemberRankAndExpireDate(5, "1260901", "it-m1");
    EXPECT_EQ("5", queryScalar("SELECT `Rank` FROM GuildMember WHERE Name='it-m1'"));
    EXPECT_EQ("1260901", queryScalar("SELECT ExpireDate FROM GuildMember WHERE Name='it-m1'"));
    // Re-joining rewrites the row and clears the expiry.
    repository.rejoinWaitingMember(31001, 1, "2026-01-02 03:04:05", "it-m1");
    EXPECT_EQ("31001", queryScalar("SELECT GuildID FROM GuildMember WHERE Name='it-m1'"));
    EXPECT_EQ("1", queryScalar("SELECT `Rank` FROM GuildMember WHERE Name='it-m1'"));
    EXPECT_EQ("", queryScalar("SELECT ExpireDate FROM GuildMember WHERE Name='it-m1'"));
    EXPECT_EQ("2026-01-02 03:04:05", queryScalar("SELECT RequestDateTime FROM GuildMember WHERE Name='it-m1'"));
    repository.rejoinMember(31000, 3, "it-m1");
    EXPECT_EQ("31000", queryScalar("SELECT GuildID FROM GuildMember WHERE Name='it-m1'"));
    EXPECT_EQ("3", queryScalar("SELECT `Rank` FROM GuildMember WHERE Name='it-m1'"));

    repository.saveMember(31002, 2, "it-m1");
    EXPECT_EQ("31002", queryScalar("SELECT GuildID FROM GuildMember WHERE Name='it-m1'"));

    repository.saveMemberIntro("hello", "it-m1");
    std::string intro;
    ASSERT_TRUE(repository.loadMemberIntro("it-m1", intro));
    EXPECT_EQ("hello", intro);
    EXPECT_FALSE(repository.loadMemberIntro("it-none", intro));

    // The boot-time member list takes ranks 0..3 only.
    repository.insertWaitingMember(31002, "it-m2", 0, "2026-02-03 04:05:06");
    repository.insertMember(31002, "it-m3", 7);
    std::vector<GuildMemberListRow> active = repository.loadActiveMembers();
    bool seenM1 = false, seenM2 = false, seenM3 = false;
    for (size_t r = 0; r < active.size(); r++) {
        if (active[r].name == "it-m1")
            seenM1 = true;
        if (active[r].name == "it-m2") {
            seenM2 = true;
            EXPECT_EQ(31002, active[r].guildID);
            EXPECT_EQ(0, active[r].rank);
            EXPECT_EQ("2026-02-03 04:05:06", active[r].requestDateTime);
        }
        if (active[r].name == "it-m3")
            seenM3 = true;
    }
    EXPECT_TRUE(seenM1);
    EXPECT_TRUE(seenM2);
    EXPECT_FALSE(seenM3);

    repository.deleteMember("it-m2");
    EXPECT_FALSE(repository.loadMember("it-m2", row));
}

TEST_F(GuildMySQL, GuildRowsAreCreatedLoadedSavedListedByStateAndDeletedWithTheirUnionRows) {
    GuildRepository& repository = defaultGuildRepository();

    GuildRecord record;
    record.id = 31000;
    record.name = "it-guild";
    record.type = 1;
    record.race = 2;
    record.state = 1;
    record.serverGroupID = 3;
    record.zoneID = 31000;
    record.master = "it-master";
    record.date = "2026-09-02";
    record.intro = "intro";
    repository.insertGuild(record);

    GuildRow row;
    ASSERT_TRUE(repository.loadGuild(31000, row));
    EXPECT_EQ("it-guild", row.name);
    EXPECT_EQ(1, row.type);
    EXPECT_EQ(2, row.race);
    EXPECT_EQ(1, row.state);
    EXPECT_EQ(3, row.serverGroupID);
    EXPECT_EQ(31000, row.zoneID);
    EXPECT_EQ("it-master", row.master);
    EXPECT_EQ("2026-09-02", row.date);
    EXPECT_EQ("intro", queryScalar("SELECT Intro FROM GuildInfo WHERE GuildID=31000"));
    EXPECT_FALSE(repository.loadGuild(31001, row));

    record.name = "it-renamed";
    record.state = 2;
    record.master = "it-master2";
    repository.saveGuild(record);
    ASSERT_TRUE(repository.loadGuild(31000, row));
    EXPECT_EQ("it-renamed", row.name);
    EXPECT_EQ(2, row.state);
    EXPECT_EQ("it-master2", row.master);
    EXPECT_EQ("intro", queryScalar("SELECT Intro FROM GuildInfo WHERE GuildID=31000")); // save leaves the intro alone

    record.id = 31001;
    record.state = 9;
    repository.insertGuild(record);
    std::vector<GuildListRow> listed = repository.loadGuildsInStates(1, 2);
    bool seen31000 = false, seen31001 = false;
    for (size_t r = 0; r < listed.size(); r++) {
        if (listed[r].id == 31000) {
            seen31000 = true;
            EXPECT_EQ("it-renamed", listed[r].name);
            EXPECT_EQ(2, listed[r].state);
            EXPECT_EQ("intro", listed[r].intro);
        }
        if (listed[r].id == 31001)
            seen31001 = true;
    }
    EXPECT_TRUE(seen31000);
    EXPECT_FALSE(seen31001);

    std::string name, master;
    ASSERT_TRUE(repository.loadGuildNameAndMaster(31000, name, master));
    EXPECT_EQ("it-renamed", name);
    EXPECT_EQ("it-master2", master);
    EXPECT_FALSE(repository.loadGuildNameAndMaster(31002, name, master));

    // Destroying a guild also drops its union memberships.
    execSQL("INSERT INTO GuildUnionMember (UnionID, OwnerGuildID) VALUES (31000, 31000)");
    repository.deleteGuild(31000);
    EXPECT_FALSE(repository.loadGuild(31000, row));
    EXPECT_EQ("0", queryScalar("SELECT COUNT(*) FROM GuildUnionMember WHERE OwnerGuildID=31000"));
    EXPECT_TRUE(repository.loadGuild(31001, row));
}

TEST_F(GuildMySQL, CastleAndWarReadsAreScopedToTheGuild) {
    execSQL("INSERT INTO CastleInfo (ServerID, ZoneID, GuildID, Name, BonusOptionType, ZoneIDList) "
            "VALUES (31000, 31000, 31000, 'it-castle', '', '')");
    execSQL("INSERT INTO WarScheduleInfo (WarID, ServerID, ZoneID, AttackGuildID3, Status) "
            "VALUES (31000, 31000, 31000, 31000, 'WAIT')");
    execSQL("INSERT INTO WarScheduleInfo (WarID, ServerID, ZoneID, AttackGuildID, Status) "
            "VALUES (31001, 31000, 31000, 31000, 'END')");
    execSQL("INSERT INTO WarScheduleInfo (WarID, ServerID, ZoneID, AttackGuildID, Status) "
            "VALUES (31002, 31000, 31000, 31002, 'START')");
    execSQL("INSERT INTO ReinforceRegisterInfo (WarID, ServerID, ReinforceGuildID, Status) "
            "VALUES (31000, 31000, 31005, 'WAIT')");
    execSQL("INSERT INTO ReinforceRegisterInfo (WarID, ServerID, ReinforceGuildID, Status) "
            "VALUES (31000, 31000, 31006, 'DENY')");

    GuildRepository& repository = defaultGuildRepository();

    EXPECT_EQ(1, repository.countCastlesOfGuild(31000));
    EXPECT_EQ(0, repository.countCastlesOfGuild(31001));
    int serverID = 0, zoneID = 0;
    ASSERT_TRUE(repository.loadCastleOfGuild(31000, serverID, zoneID));
    EXPECT_EQ(31000, serverID);
    EXPECT_EQ(31000, zoneID);
    EXPECT_FALSE(repository.loadCastleOfGuild(31001, serverID, zoneID));
    EXPECT_EQ(31000, serverID); // a miss leaves the out-params alone (hasActiveWar relies on it)
    EXPECT_EQ(31000, zoneID);

    // Two of the five attacker slots are seeded (AttackGuildID, AttackGuildID3);
    // either counts, but only WAIT/START wars.
    EXPECT_EQ(1, repository.countWarSchedulesOfAttacker(31000));
    EXPECT_EQ(1, repository.countWarSchedulesOfAttacker(31002));
    EXPECT_EQ(0, repository.countWarSchedulesOfAttacker(31003));

    EXPECT_EQ(1, repository.countReinforceRegistrations(31005));
    EXPECT_EQ(0, repository.countReinforceRegistrations(31006)); // DENY

    EXPECT_EQ(1, repository.countStartedWarsAtCastle(31000, 31000));
    EXPECT_EQ(0, repository.countStartedWarsAtCastle(31000, 31001));
    EXPECT_EQ(1, repository.countStartedWarsOfAttacker(31002));
    EXPECT_EQ(0, repository.countStartedWarsOfAttacker(31000)); // its war is WAIT
}

TEST_F(GuildMySQL, UnionsAreCreatedWithAnAutoIncrementIdAndTheirMembersReadAndRemoved) {
    GuildRepository& repository = defaultGuildRepository();

    uint unionID = repository.insertUnion(31000);
    ASSERT_GT(unionID, 0u);
    repository.insertUnionMember(unionID, 31000);
    repository.insertUnionMember(unionID, 31001);

    std::vector<UnionRow> unions = repository.loadUnions();
    bool seen = false;
    for (size_t r = 0; r < unions.size(); r++) {
        if (unions[r].unionID == (int)unionID) {
            seen = true;
            EXPECT_EQ(31000, unions[r].masterGuildID);
        }
    }
    EXPECT_TRUE(seen);

    std::vector<int> members = repository.loadUnionMemberGuilds(unionID);
    ASSERT_EQ(2u, members.size());
    EXPECT_EQ(2, repository.countUnionMembers(unionID));

    int foundUnion = 0, foundOwner = 0;
    ASSERT_TRUE(repository.loadUnionOfGuild(31001, foundUnion, foundOwner));
    EXPECT_EQ((int)unionID, foundUnion);
    EXPECT_EQ(31001, foundOwner);
    EXPECT_FALSE(repository.loadUnionOfGuild(31002, foundUnion, foundOwner));

    int master = 0;
    ASSERT_TRUE(repository.loadUnionMaster((int)unionID, master));
    EXPECT_EQ(31000, master);

    EXPECT_TRUE(repository.deleteUnionMember(unionID, 31001));
    EXPECT_FALSE(repository.deleteUnionMember(unionID, 31001)); // already gone
    EXPECT_EQ(1, repository.countUnionMembers(unionID));

    repository.deleteUnion(unionID);
    EXPECT_FALSE(repository.loadUnionMaster((int)unionID, master));
    EXPECT_EQ(0, repository.countUnionMembers(unionID));
}

TEST_F(GuildMySQL, UnionOffersAreInsertedReadByTypeAgedAndCleared) {
    GuildRepository& repository = defaultGuildRepository();

    EXPECT_EQ(0, repository.countOffers(31000));
    repository.insertJoinOffer(31000, 31000);
    repository.insertQuitOffer(31000, 31001);
    EXPECT_EQ(1, repository.countOffers(31000));

    int unionID = 0;
    ASSERT_TRUE(repository.loadJoinOfferUnion(31000, unionID));
    EXPECT_EQ(31000, unionID);
    EXPECT_FALSE(repository.loadQuitOfferUnion(31000, unionID));
    ASSERT_TRUE(repository.loadQuitOfferUnion(31001, unionID));
    EXPECT_EQ(31000, unionID);

    std::vector<UnionOfferRow> offers = repository.loadOffers(31000);
    ASSERT_EQ(2u, offers.size());
    int today = atoi(queryScalar("SELECT DATE_FORMAT(now(),'%y%m%d')").c_str());
    for (size_t r = 0; r < offers.size(); r++) {
        EXPECT_EQ(today, offers[r].date);
        if (offers[r].ownerGuildID == 31000)
            EXPECT_EQ(1, offers[r].offerType); // JOIN is the enum's first value
        if (offers[r].ownerGuildID == 31001)
            EXPECT_EQ(2, offers[r].offerType); // QUIT
    }

    // The join penalty looks ten days back for an ESCAPE.
    execSQL("INSERT INTO GuildUnionOffer (UnionID, OfferType, OwnerGuildID, OfferTime) "
            "VALUES (31000, 'ESCAPE', 31002, now())");
    execSQL("INSERT INTO GuildUnionOffer (UnionID, OfferType, OwnerGuildID, OfferTime) "
            "VALUES (31000, 'ESCAPE', 31003, now() - interval 20 day)");
    EXPECT_EQ(1, repository.countRecentEscapes(31002));
    EXPECT_EQ(0, repository.countRecentEscapes(31003));

    repository.deleteStaleOffers(31003);
    EXPECT_EQ(0, repository.countOffers(31003));
    repository.deleteStaleOffers(31002);
    EXPECT_EQ(1, repository.countOffers(31002)); // fresh, kept

    repository.deleteOffers(31000);
    EXPECT_EQ(0, repository.countOffers(31000));
    EXPECT_EQ(1, repository.countOffers(31001));
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
    // The USERINFO database the session seam writes UserStatus to; the
    // tier loads initdb/USERINFO.sql next to DARKEDEN.sql.
    std::string userInfoDb = env("IT_DB_USERINFO_DB", "USERINFO");
    g_pDatabaseManager->setUserInfoConnection(new Connection(host, userInfoDb, user, password, port));

    return RUN_ALL_TESTS();
}
