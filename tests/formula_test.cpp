// formula_test.cpp — pins the pure balance formulas in de-core
// (src/domain/Formulas.cpp, docs/RESTRUCTURING.md task 3.3).
//
// These values ARE the game balance: every expectation below was computed
// from the formulas as they shipped, including their narrow-integer
// wrap-around and truncation quirks. A failing test here means the balance
// changed — that must be a deliberate act, not a refactoring side effect.
//
// The suite links ONLY de-core and gtest: it is also the proof that de-core
// stays freestanding (no Core, no gameserver, no transport).

#include <gtest/gtest.h>

#include "domain/Formulas.h"

using decore::StatAttr;
using decore::WeaponFamily;

namespace {

StatAttr attr(int str, int dex, int inte, int level, WeaponFamily weapon = WeaponFamily::None, int domainLevel = 0) {
    StatAttr a;
    a.str = str;
    a.dex = dex;
    a.inte = inte;
    a.level = level;
    a.weapon = weapon;
    a.weaponDomainLevel = domainLevel;
    return a;
}

TEST(PercentValue, TruncatesTowardZeroWithWideIntermediate) {
    EXPECT_EQ(0, decore::percentValue(0, 100));
    EXPECT_EQ(45, decore::percentValue(90, 50));
    EXPECT_EQ(24, decore::percentValue(33, 75)); // 24.75 truncates
    EXPECT_EQ(135, decore::percentValue(90, 150));
    // The (long long) widening keeps big products from overflowing int.
    EXPECT_EQ(1500000000, decore::percentValue(1000000000, 150));
}

//////////////////////////////////////////////////////////////////////////
// Max HP
//////////////////////////////////////////////////////////////////////////

TEST(SlayerMaxHP, BareHandsIsStrTimesThree) {
    EXPECT_EQ(300, decore::slayerMaxHP(attr(100, 0, 0, 0), 100, false));
}

TEST(SlayerMaxHP, WeaponFamilySwapsBaseAndBonus) {
    // Sword/Blade: STR*3 + domain*5.
    EXPECT_EQ(350, decore::slayerMaxHP(attr(100, 0, 0, 0, WeaponFamily::Sword, 10), 100, false));
    EXPECT_EQ(350, decore::slayerMaxHP(attr(100, 0, 0, 0, WeaponFamily::Blade, 10), 100, false));
    // Cross/Mace drop the base to STR*2, bonus domain*4.
    EXPECT_EQ(240, decore::slayerMaxHP(attr(100, 0, 0, 0, WeaponFamily::Cross, 10), 100, false));
    EXPECT_EQ(240, decore::slayerMaxHP(attr(100, 0, 0, 0, WeaponFamily::Mace, 10), 100, false));
    // Guns: STR*2 + domain*2.
    EXPECT_EQ(220, decore::slayerMaxHP(attr(100, 0, 0, 0, WeaponFamily::Arms, 10), 100, false));
    // An unrecognized weapon behaves like bare hands.
    EXPECT_EQ(300, decore::slayerMaxHP(attr(100, 0, 0, 0, WeaponFamily::Other, 10), 100, false));
}

TEST(SlayerMaxHP, RatioCapAndHardcore) {
    EXPECT_EQ(450, decore::slayerMaxHP(attr(100, 0, 0, 0), 150, false));
    EXPECT_EQ(10000, decore::slayerMaxHP(attr(5000, 0, 0, 0), 100, false));
    // Hardcore triples after the cap.
    EXPECT_EQ(30000, decore::slayerMaxHP(attr(5000, 0, 0, 0), 100, true));
}

TEST(SlayerMaxHP, WordWidthWrapIsPreserved) {
    // STR 25000 -> 75000 wraps into the WORD-wide accumulator (75000 -
    // 65536 = 9464) BEFORE the cap can bite. Shipped behavior.
    EXPECT_EQ(9464, decore::slayerMaxHP(attr(25000, 0, 0, 0), 100, false));
}

TEST(VampireMaxHP, StrWeightedSum) {
    // STR*4 + INT + DEX + LEVEL.
    EXPECT_EQ(500, decore::vampireMaxHP(attr(100, 50, 30, 20), 100, false));
    EXPECT_EQ(10000, decore::vampireMaxHP(attr(4000, 0, 0, 0), 100, false));
}

TEST(OustersMaxHP, HalfIntWeight) {
    // STR*3 + INT/2 + DEX + LEVEL.
    EXPECT_EQ(370, decore::oustersMaxHP(attr(90, 50, 40, 30), 100, false));
}

TEST(MonsterMaxHP, LevelScalesStrMultiplier) {
    // STR * (2 + level/100), then enhance% is added on top.
    EXPECT_EQ(250, decore::monsterMaxHP(attr(100, 0, 0, 50), 0, 100, false));
    EXPECT_EQ(275, decore::monsterMaxHP(attr(100, 0, 0, 50), 10, 100, false));
    // Monster cap is 60000, not 10000.
    EXPECT_EQ(60000, decore::monsterMaxHP(attr(31000, 0, 0, 0), 0, 100, false));
}

//////////////////////////////////////////////////////////////////////////
// Max MP
//////////////////////////////////////////////////////////////////////////

TEST(MaxMP, SlayerIsIntDoubled) {
    EXPECT_EQ(80, decore::slayerMaxMP(attr(0, 0, 40, 0)));
    EXPECT_EQ(10000, decore::slayerMaxMP(attr(0, 0, 6000, 0)));
}

TEST(MaxMP, OustersIsSeventyPercentOfIntPlusLevel) {
    EXPECT_EQ(49, decore::oustersMaxMP(attr(0, 0, 30, 40)));
    EXPECT_EQ(0, decore::oustersMaxMP(attr(0, 0, 0, 0)));
}

//////////////////////////////////////////////////////////////////////////
// To-hit / defense / protection
//////////////////////////////////////////////////////////////////////////

TEST(ToHit, PerRaceFormulas) {
    EXPECT_EQ(50, decore::slayerToHit(attr(0, 100, 0, 0)));
    // Any recognized weapon family adds domain*1.5.
    EXPECT_EQ(65, decore::slayerToHit(attr(0, 100, 0, 0, WeaponFamily::Sword, 10)));
    EXPECT_EQ(65, decore::slayerToHit(attr(0, 100, 0, 0, WeaponFamily::Cross, 10)));
    EXPECT_EQ(50, decore::slayerToHit(attr(0, 100, 0, 0, WeaponFamily::Other, 10)));
    // Vampire: DEX + level/2.5.
    EXPECT_EQ(60, decore::vampireToHit(attr(0, 50, 0, 25)));
    // Ousters: DEX/2 + level.
    EXPECT_EQ(45, decore::oustersToHit(attr(0, 60, 0, 15)));
    // Monster: (DEX/2) * (1 + level/100), then enhance%.
    EXPECT_EQ(75, decore::monsterToHit(attr(0, 100, 0, 50), 0));
    EXPECT_EQ(82, decore::monsterToHit(attr(0, 100, 0, 50), 10)); // 75 + 7.5 truncated
}

TEST(Defense, PerRaceFormulas) {
    EXPECT_EQ(50, decore::slayerDefense(attr(0, 101, 0, 0))); // 50.5 truncates
    EXPECT_EQ(60, decore::vampireDefense(attr(0, 100, 0, 50)));
    EXPECT_EQ(60, decore::oustersDefense(attr(0, 100, 0, 50)));
    EXPECT_EQ(110, decore::monsterDefense(attr(0, 100, 0, 100), 10));
}

TEST(Protection, PerRaceFormulas) {
    EXPECT_EQ(123, decore::slayerProtection(attr(123, 0, 0, 0)));
    EXPECT_EQ(110, decore::vampireProtection(attr(100, 0, 0, 50)));
    EXPECT_EQ(105, decore::oustersProtection(attr(100, 0, 0, 50)));
    // Monster: STR / (5 - level/100) — higher level divides by less.
    EXPECT_EQ(25, decore::monsterProtection(attr(100, 0, 0, 100), 0));
    EXPECT_EQ(30, decore::monsterProtection(attr(100, 0, 0, 100), 20));
}

//////////////////////////////////////////////////////////////////////////
// Damage range
//////////////////////////////////////////////////////////////////////////

TEST(MinDamage, SlayerGunsIgnoreStrength) {
    EXPECT_EQ(10, decore::slayerMinDamage(attr(150, 0, 0, 0), 0));
    EXPECT_EQ(1, decore::slayerMinDamage(attr(150, 0, 0, 0, WeaponFamily::Arms, 30), 0));
    EXPECT_EQ(6, decore::slayerMinDamage(attr(150, 0, 0, 0, WeaponFamily::Arms, 30), 5));
    // A sword keeps the strength-derived minimum.
    EXPECT_EQ(10, decore::slayerMinDamage(attr(150, 0, 0, 0, WeaponFamily::Sword, 30), 0));
}

TEST(MinDamage, OtherRaces) {
    EXPECT_EQ(30, decore::vampireMinDamage(attr(120, 0, 0, 50), 0));
    EXPECT_EQ(35, decore::vampireMinDamage(attr(120, 0, 0, 50), 5));
    EXPECT_EQ(15, decore::oustersMinDamage(attr(100, 0, 0, 50)));
    EXPECT_EQ(24, decore::monsterMinDamage(attr(120, 0, 0, 100), 0, 100));
}

TEST(MaxDamage, SlayerGunsIgnoreStrength) {
    EXPECT_EQ(15, decore::slayerMaxDamage(attr(150, 0, 0, 0), 0));
    EXPECT_EQ(2, decore::slayerMaxDamage(attr(150, 0, 0, 0, WeaponFamily::Arms, 30), 0));
}

TEST(MaxDamage, OtherRaces) {
    EXPECT_EQ(40, decore::vampireMaxDamage(attr(120, 0, 0, 50), 0));
    EXPECT_EQ(30, decore::oustersMaxDamage(attr(120, 0, 0, 60)));
    EXPECT_EQ(40, decore::monsterMaxDamage(attr(120, 0, 0, 100), 0, 100));
}

//////////////////////////////////////////////////////////////////////////
// Attack speed / criticals / steal
//////////////////////////////////////////////////////////////////////////

TEST(AttackSpeed, SlayerWeaponBranches) {
    EXPECT_EQ(10, decore::slayerAttackSpeed(attr(100, 0, 0, 0)));
    EXPECT_EQ(15, decore::slayerAttackSpeed(attr(100, 0, 0, 0, WeaponFamily::Sword, 25)));
    // Guns swap the driving stat from STR to DEX.
    EXPECT_EQ(17, decore::slayerAttackSpeed(attr(100, 120, 0, 0, WeaponFamily::Arms, 25)));
    // Cross/Mace grant no speed bonus at all.
    EXPECT_EQ(10, decore::slayerAttackSpeed(attr(100, 0, 0, 0, WeaponFamily::Cross, 25)));
    EXPECT_EQ(35, decore::slayerAttackSpeed(attr(1000, 0, 0, 0)));
}

TEST(AttackSpeed, VampireAndOusters) {
    EXPECT_EQ(20, decore::vampireAttackSpeed(attr(0, 100, 0, 0)));
    EXPECT_EQ(30, decore::vampireAttackSpeed(attr(0, 900, 0, 0)));
    EXPECT_EQ(15, decore::oustersAttackSpeed(attr(0, 100, 0, 50)));
}

TEST(CriticalRatio, OnlyCombatDomainsGrantSlayerCriticals) {
    EXPECT_EQ(5, decore::slayerCriticalRatio(attr(0, 0, 0, 0, WeaponFamily::Sword, 25)));
    EXPECT_EQ(5, decore::slayerCriticalRatio(attr(0, 0, 0, 0, WeaponFamily::Arms, 25)));
    EXPECT_EQ(0, decore::slayerCriticalRatio(attr(0, 0, 0, 0, WeaponFamily::Cross, 25)));
    EXPECT_EQ(0, decore::slayerCriticalRatio(attr(0, 0, 0, 0)));
}

TEST(CriticalRatio, DexDrivenRacesTruncateTowardZero) {
    EXPECT_EQ(2, decore::vampireCriticalRatio(attr(0, 80, 0, 0)));
    // (DEX-20)/30 truncates toward zero, so DEX below 20 yields 0, not -1.
    EXPECT_EQ(0, decore::vampireCriticalRatio(attr(0, 5, 0, 0)));
    EXPECT_EQ(2, decore::oustersCriticalRatio(attr(0, 80, 0, 0)));
    EXPECT_EQ(4, decore::monsterCriticalRatio(attr(0, 80, 0, 0), 100));
}

TEST(StealRatio, WeaponFamilyPicksTheBase) {
    // Bare hands and melee weapons: 90 - amount*1.4.
    EXPECT_EQ(76, decore::slayerStealRatio(attr(0, 0, 0, 0), 10));
    EXPECT_EQ(76, decore::slayerStealRatio(attr(0, 0, 0, 0, WeaponFamily::Sword, 0), 10));
    // Anything else: 65 - amount*1.4 — the one place None and Other differ.
    EXPECT_EQ(51, decore::slayerStealRatio(attr(0, 0, 0, 0, WeaponFamily::Cross, 0), 10));
    EXPECT_EQ(51, decore::slayerStealRatio(attr(0, 0, 0, 0, WeaponFamily::Other, 0), 10));
    EXPECT_EQ(0, decore::slayerStealRatio(attr(0, 0, 0, 0), 0));
    EXPECT_EQ(62, decore::vampireStealRatio(20));
    EXPECT_EQ(20, decore::oustersStealRatio(50));
    EXPECT_EQ(0, decore::vampireStealRatio(0));
}

//////////////////////////////////////////////////////////////////////////
// Combat / progression
//////////////////////////////////////////////////////////////////////////

TEST(FinalDamage, CriticalHitsBypassProtection) {
    EXPECT_EQ(500.0, decore::finalDamage(500, 640, true));
    EXPECT_EQ(0.0, decore::finalDamage(0, 0, true));
}

TEST(FinalDamage, EveryEightProtectionShavesOnePercent) {
    EXPECT_EQ(100.0, decore::finalDamage(100, 0, false));
    EXPECT_EQ(100.0, decore::finalDamage(100, 7, false)); // 7/8 == 0: no effect
    EXPECT_EQ(99.0, decore::finalDamage(100, 15, false));
    EXPECT_EQ(90.0, decore::finalDamage(100, 80, false));
    // Clamped at 640 protection = 80% reduction, the formula's floor.
    EXPECT_EQ(20.0, decore::finalDamage(100, 640, false));
    EXPECT_EQ(20.0, decore::finalDamage(100, 9999, false));
}

TEST(FinalDamage, NeverBelowOne) {
    EXPECT_EQ(1.0, decore::finalDamage(1, 640, false));
    EXPECT_EQ(1.0, decore::finalDamage(0, 0, false));
}

TEST(TileDistance, EuclideanTruncated) {
    EXPECT_EQ(5, decore::tileDistance(0, 0, 3, 4));
    EXPECT_EQ(0, decore::tileDistance(7, 7, 7, 7));
    EXPECT_EQ(5, decore::tileDistance(10, 10, 13, 14));
    EXPECT_EQ(1, decore::tileDistance(0, 0, 1, 1)); // sqrt(2) truncates
}

TEST(TileDistance, ByteWidthWrapIsPreserved) {
    // Range_t is BYTE-wide: a 300-tile distance wraps to 44. Shipped
    // behavior — real zones keep coordinates within range in practice.
    EXPECT_EQ(44, decore::tileDistance(0, 0, 300, 0));
}

TEST(RankExp, ZeroLevelPaysNothing) {
    EXPECT_EQ(0, decore::rankExp(0, 50, 100, 100));
    EXPECT_EQ(0, decore::rankExp(50, 0, 100, 100));
}

TEST(RankExp, LinearBelowQuadraticAbove120Percent) {
    // Equal levels: 100% ratio - 10.
    EXPECT_EQ(90, decore::rankExp(10, 10, 100, 100));
    // Exactly 120% stays on the linear branch.
    EXPECT_EQ(110, decore::rankExp(10, 12, 100, 100));
    // Above 120%: (diff^2)/10 + ratio.
    EXPECT_EQ(130, decore::rankExp(10, 13, 100, 100));
    EXPECT_EQ(210, decore::rankExp(10, 20, 100, 100));
    // Far below: floored at 0.
    EXPECT_EQ(40, decore::rankExp(100, 50, 100, 100));
    EXPECT_EQ(0, decore::rankExp(100, 5, 100, 100));
}

TEST(RankExp, PercentagesScaleTheBase) {
    EXPECT_EQ(45, decore::rankExp(10, 10, 50, 100));
    EXPECT_EQ(135, decore::rankExp(10, 10, 100, 150));
}

TEST(VampireSkillConsumeMP, IntDiscountBrackets) {
    // INT has a 20-point deadband; below MagicLevel there is no discount.
    EXPECT_EQ(100, decore::vampireSkillConsumeMP(100, 10, 0));
    EXPECT_EQ(100, decore::vampireSkillConsumeMP(100, 10, 25));
    EXPECT_EQ(100, decore::vampireSkillConsumeMP(100, 10, 30)); // INTE == level: no discount
    EXPECT_EQ(90, decore::vampireSkillConsumeMP(100, 10, 35));  // <=1.5x: 10%
    EXPECT_EQ(75, decore::vampireSkillConsumeMP(100, 10, 40));  // <=2x: 25%
    EXPECT_EQ(50, decore::vampireSkillConsumeMP(100, 10, 50));  // <=3x: 50%
    EXPECT_EQ(40, decore::vampireSkillConsumeMP(100, 10, 60));  // <=4x: 60%
    EXPECT_EQ(25, decore::vampireSkillConsumeMP(100, 10, 70));  // <=5x: 75%
    EXPECT_EQ(15, decore::vampireSkillConsumeMP(100, 10, 80));  // <=6x: 85%
    EXPECT_EQ(10, decore::vampireSkillConsumeMP(100, 10, 90));  // <=7x: 90%
    EXPECT_EQ(5, decore::vampireSkillConsumeMP(100, 10, 91));   // >7x: 95%
}

TEST(VampireSkillConsumeMP, DiscountTruncatesViaPercentValue) {
    // 75% of 33 is 24.75 -> 24, leaving 9.
    EXPECT_EQ(9, decore::vampireSkillConsumeMP(33, 1, 25));
}

} // namespace
