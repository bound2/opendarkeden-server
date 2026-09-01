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
#include "domain/SkillOutputFormulas.h"

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

//////////////////////////////////////////////////////////////////////////
// Hit-roll success ratios (skill/HitRoll.cpp adapters). These are all int
// math — the narrow-width accumulation (Attr_t sums, ToHit_t) happens in
// the adapters before the values arrive here — so the pinned oddities are
// truncations, asymmetric caps/floors, and missing floors.
//////////////////////////////////////////////////////////////////////////

TEST(MeleeHitRatio, EqualStatsLandAtFifty) {
    EXPECT_EQ(50, decore::meleeHitRatio(100, 100, 0, false));
    EXPECT_EQ(56, decore::meleeHitRatio(120, 100, 0, false)); // 20/3 truncates to 6
}

TEST(MeleeHitRatio, MonsterFightsWidenTheBand) {
    // Cap 90 vs 95, floor 10 vs 5: monsters are easier to hit AND miss.
    EXPECT_EQ(90, decore::meleeHitRatio(300, 100, 0, false));
    EXPECT_EQ(95, decore::meleeHitRatio(300, 100, 0, true));
    EXPECT_EQ(10, decore::meleeHitRatio(100, 300, 0, false));
    EXPECT_EQ(5, decore::meleeHitRatio(100, 300, 0, true));
}

TEST(MeleeHitRatio, BonusIsHalvedWithTruncation) {
    EXPECT_EQ(52, decore::meleeHitRatio(100, 100, 5, false)); // 5/2 -> 2
    EXPECT_EQ(51, decore::meleeHitRatio(100, 103, 5, false)); // 50 - 1 + 2
    // Production passes NEGATIVE bonuses (e.g. DoubleShot's ToHitPenalty):
    // integer division truncates toward zero, so -5/2 is -2, not -3.
    EXPECT_EQ(48, decore::meleeHitRatio(100, 100, -5, false));
}

TEST(BloodDrainHitRatio, SeventyBaseWithHalfSlope) {
    EXPECT_EQ(70, decore::bloodDrainHitRatio(100, 100));
    EXPECT_EQ(72, decore::bloodDrainHitRatio(105, 100)); // 5/2 truncates
    EXPECT_EQ(90, decore::bloodDrainHitRatio(300, 100)); // cap
    EXPECT_EQ(10, decore::bloodDrainHitRatio(100, 300)); // floor
}

TEST(BloodDrainHPGate, ThirdForNormalHalfForMasters) {
    EXPECT_TRUE(decore::bloodDrainHPGate(33, 100, 3));
    EXPECT_FALSE(decore::bloodDrainHPGate(34, 100, 3));
    EXPECT_TRUE(decore::bloodDrainHPGate(50, 100, 2));
    EXPECT_FALSE(decore::bloodDrainHPGate(51, 100, 2));
}

TEST(SlayerMagicRatio, IntAndExpDividedByTwoPointFive) {
    EXPECT_EQ(72, decore::slayerMagicRatio(30, 50, 5, false)); // 55/2.5 = 22 exactly
    EXPECT_EQ(71, decore::slayerMagicRatio(30, 50, 4, false)); // 54/2.5 = 21.6 -> 21
    EXPECT_EQ(71, decore::slayerMagicRatio(33, 50, 5, false)); // 33/3 = 11
}

TEST(SlayerMagicRatio, SelfSkillsFloorAtFifty) {
    EXPECT_EQ(50, decore::slayerMagicRatio(90, 0, 0, true));
    EXPECT_EQ(30, decore::slayerMagicRatio(90, 0, 0, false));
    EXPECT_EQ(160, decore::slayerMagicRatio(0, 250, 0, true)); // floor never lowers
}

TEST(VampireMagicRatio, LevelPenaltyAndBonusPercent) {
    EXPECT_EQ(45, decore::vampireMagicRatio(30, 40, 20, 0));
    EXPECT_EQ(46, decore::vampireMagicRatio(29, 40, 20, 0));  // 29/2 -> 14
    EXPECT_EQ(45, decore::vampireMagicRatio(30, 41, 20, 0));  // 61/4 -> 15
    EXPECT_EQ(67, decore::vampireMagicRatio(30, 40, 20, 50)); // 45 * 150% -> 67.5 -> 67
}

TEST(OustersMagicRatio, SelfFloorAppliesBeforeBonus) {
    EXPECT_EQ(63, decore::oustersMagicRatio(40, 20, 9, false, 0));
    EXPECT_EQ(64, decore::oustersMagicRatio(40, 20, 12, false, 0)); // 12/3 -> 4
    EXPECT_EQ(60, decore::oustersMagicRatio(0, 0, 0, true, 0));
    EXPECT_EQ(66, decore::oustersMagicRatio(0, 0, 0, true, 10)); // bonus scales the floored value
}

TEST(MonsterMagicRatio, SameShapeAsVampire) {
    EXPECT_EQ(45, decore::monsterMagicRatio(30, 40, 20));
    EXPECT_EQ(44, decore::monsterMagicRatio(33, 40, 20)); // 33/2 -> 16
}

TEST(CurseRatio, MagicLevelScaledByTwoOverOnePointFive) {
    EXPECT_EQ(95, decore::curseRatio(30, 20)); // 60/1.5 = 40, -20, +75
    EXPECT_EQ(96, decore::curseRatio(31, 20)); // 62/1.5 = 41.33 -> 21 after subtract+cast
    EXPECT_EQ(15, decore::curseRatio(30, 100));
    EXPECT_EQ(5, decore::curseRatio(30, 200)); // floor
}

TEST(VampireCurseRatio, MagicLevelDividedByOnePointFiveTruncated) {
    EXPECT_EQ(75, decore::vampireCurseRatio(30, 20));
    EXPECT_EQ(75, decore::vampireCurseRatio(31, 20)); // (int)(31/1.5) = 20, same as 30
    EXPECT_EQ(5, decore::vampireCurseRatio(30, 100)); // floor
}

TEST(DispelRatio, SharedByCurePoisonAndRemoveCurse) {
    EXPECT_EQ(50, decore::dispelRatio(50, 30, 20, 10, 0));
    EXPECT_EQ(15, decore::dispelRatio(50, 0, 100, 10, 15)); // MinRatio floor
    EXPECT_EQ(0, decore::dispelRatio(10, 0, 50, 10, 0));    // floor of 0, not negative
}

TEST(FlareRatio, HasNoFloorAndGoesNegative) {
    EXPECT_EQ(85, decore::flareRatio(30, 20));
    // A high-level target drives the ratio negative: rand()%100 < -15 never
    // succeeds. Shipped behavior — there is no floor here.
    EXPECT_EQ(-15, decore::flareRatio(10, 100));
}

TEST(RebukeRatio, TenthIntPlusHalfSkill) {
    EXPECT_EQ(45, decore::rebukeRatio(100, 30));
    EXPECT_EQ(45, decore::rebukeRatio(109, 31)); // both divisions truncate
    EXPECT_EQ(46, decore::rebukeRatio(110, 31));
}

TEST(TotalAttrDefenseRatio, SharedByMagicElusionAndIllusionOfAvenge) {
    EXPECT_EQ(50, decore::totalAttrDefenseRatio(0));
    EXPECT_EQ(59, decore::totalAttrDefenseRatio(149));
    EXPECT_EQ(60, decore::totalAttrDefenseRatio(150));
}

TEST(LevelSelfRatios, PoisonMeshAndWillOfLife) {
    EXPECT_EQ(50, decore::poisonMeshRatio(100));
    EXPECT_EQ(49, decore::poisonMeshRatio(99));
    EXPECT_EQ(70, decore::willOfLifeRatio(100));
    EXPECT_EQ(50, decore::willOfLifeRatio(4));
}

TEST(BackStabRatio, FifthOfIntPlusDexCappedAtFifty) {
    EXPECT_EQ(40, decore::backStabRatio(100, 100));
    EXPECT_EQ(39, decore::backStabRatio(101, 99)); // per-stat truncation
    EXPECT_EQ(50, decore::backStabRatio(150, 150));
}

TEST(HallucinationRatio, AttrGapClampedIntoPerRaceBand) {
    EXPECT_EQ(45, decore::hallucinationRatio(140, 95, 30, 60));
    EXPECT_EQ(30, decore::hallucinationRatio(100, 90, 30, 60)); // slayer floor
    EXPECT_EQ(60, decore::hallucinationRatio(200, 50, 30, 60)); // slayer cap
    EXPECT_EQ(40, decore::hallucinationRatio(200, 50, 10, 40)); // vampire band
}

//////////////////////////////////////////////////////////////////////////////
// Per-skill computeOutput formulas (src/domain/SkillOutputFormulas.cpp,
// adapters: skill/SkillFormula.cpp). 293 formulas moved verbatim (11
// dice-roll ones stayed in the adapter); this suite pins every gun-class
// branch (MultiShot, HeadShot, MoleShot), every grade switch including
// the unset-grade default, and the HeadShot fallthrough, plus a spread
// of representative shapes: party boosts, clamps, negative outputs,
// Delay=Duration couplings, the boost-after-Delay ordering quirk, and
// the empty formulas.
//////////////////////////////////////////////////////////////////////////////

using SFIn = decore::skillformula::SkillInput;
using SFOut = decore::skillformula::SkillOutput;
using decore::skillformula::GunClass;

SFIn sfin() {
    SFIn in;
    in.SkillLevel = 0;
    in.DomainLevel = 0;
    in.DomainGrade = -1;
    in.STR = 0;
    in.DEX = 0;
    in.INTE = 0;
    in.TargetType = SFIn::TARGET_SELF;
    in.Range = 0;
    in.Gun = GunClass::Other;
    in.PartySize = 0;
    return in;
}

TEST(SkillOutputFormula, DoubleImpactStrAndLevel) {
    SFIn in = sfin();
    in.STR = 100;
    in.SkillLevel = 60;
    SFOut out;
    decore::skillformula::DoubleImpact(in, out);
    EXPECT_EQ(9, out.Damage); // 1 + 100/20 + 60/20
    EXPECT_EQ(8, out.Delay);
    EXPECT_EQ(0, out.Duration);
    EXPECT_EQ(0, out.ToHit);
}

TEST(SkillOutputFormula, TripleShotNegativeOutputsPreserved) {
    SFIn in = sfin();
    in.SkillLevel = 30;
    SFOut out;
    decore::skillformula::TripleShot(in, out);
    EXPECT_EQ(-14, out.ToHit);  // -20 + 30/5
    EXPECT_EQ(-40, out.Damage); // -50 + 30/3
    EXPECT_EQ(2, out.Delay);
}

TEST(SkillOutputFormula, MultiShotBranchesPerGunClass) {
    SFIn in = sfin();
    in.SkillLevel = 50;
    SFOut out;
    in.Gun = GunClass::SG;
    decore::skillformula::MultiShot(in, out);
    EXPECT_EQ(13, out.Damage); // 8 + 50/10
    EXPECT_EQ(-10, out.ToHit);
    EXPECT_EQ(8, out.Delay);
    in.Gun = GunClass::AR;
    out = SFOut();
    decore::skillformula::MultiShot(in, out);
    EXPECT_EQ(8, out.Damage); // 5 + 50/15
    in.Gun = GunClass::SMG;
    out = SFOut();
    decore::skillformula::MultiShot(in, out);
    EXPECT_EQ(8, out.Damage); // AR and SMG share the branch
    in.Gun = GunClass::SR;
    out = SFOut();
    decore::skillformula::MultiShot(in, out);
    EXPECT_EQ(5, out.Damage); // 3 + 50/20
    in.Gun = GunClass::Other;
    out = SFOut();
    decore::skillformula::MultiShot(in, out);
    EXPECT_EQ(0, out.Damage); // no branch taken — damage stays zero
    EXPECT_EQ(-10, out.ToHit);
}

TEST(SkillOutputFormula, MoleShotBranchesPerGunClass) {
    SFIn in = sfin();
    in.SkillLevel = 50;
    SFOut out;
    in.Gun = GunClass::SG;
    decore::skillformula::MoleShot(in, out);
    EXPECT_EQ(8, out.Damage); // 3 + 50/10
    EXPECT_EQ(-10, out.ToHit);
    EXPECT_EQ(2, out.Delay);
    in.Gun = GunClass::AR;
    out = SFOut();
    decore::skillformula::MoleShot(in, out);
    EXPECT_EQ(2, out.Damage); // 1 + 50/30
    in.Gun = GunClass::SMG;
    out = SFOut();
    decore::skillformula::MoleShot(in, out);
    EXPECT_EQ(2, out.Damage); // shares the AR branch
    in.Gun = GunClass::SR;
    out = SFOut();
    decore::skillformula::MoleShot(in, out);
    EXPECT_EQ(1, out.Damage); // 50/50
    in.Gun = GunClass::Other;
    out = SFOut();
    decore::skillformula::MoleShot(in, out);
    EXPECT_EQ(0, out.Damage); // no branch taken
}

TEST(SkillOutputFormula, HeadShotCaseFallthroughIsTheBalance) {
    // The switch has no breaks: every in-range Range cascades to the
    // case-1 value. Shipped behavior — preserved on purpose.
    for (int range = 1; range <= 3; ++range) {
        SFIn in = sfin();
        in.Gun = GunClass::SG;
        in.Range = range;
        SFOut out;
        decore::skillformula::HeadShot(in, out);
        EXPECT_EQ(10, out.Damage) << "SG range " << range;
        EXPECT_EQ(8, out.Delay);
    }
    SFIn in = sfin();
    in.Gun = GunClass::AR;
    in.Range = 3;
    SFOut out;
    decore::skillformula::HeadShot(in, out);
    EXPECT_EQ(8, out.Damage); // 5 -> 6 -> 8
    in.Gun = GunClass::SR;
    in.Range = 2;
    out = SFOut();
    decore::skillformula::HeadShot(in, out);
    EXPECT_EQ(8, out.Damage); // 6 -> 8
    in.Gun = GunClass::SG;
    in.Range = 0;
    out = SFOut();
    decore::skillformula::HeadShot(in, out);
    EXPECT_EQ(0, out.Damage); // default: untouched
}

TEST(SkillOutputFormula, BlessSelfOtherAndPartyBoosts) {
    SFIn in = sfin();
    in.INTE = 80;
    in.SkillLevel = 40;
    in.PartySize = 4;
    SFOut out;
    decore::skillformula::Bless(in, out);
    EXPECT_EQ(11, out.Damage);     // (4 + 80/40 + 40/20) = 8, then 8*140/100
    EXPECT_EQ(1575, out.Duration); // (30 + 40*3/2)*10 = 900, then 900*175/100
    EXPECT_EQ(50, out.Delay);      // (7 - 40/20)*10
    in.TargetType = SFIn::TARGET_OTHER;
    out = SFOut();
    decore::skillformula::Bless(in, out);
    EXPECT_EQ(8, out.Damage); // (2 + 2 + 2) = 6, then 6*140/100
}

TEST(SkillOutputFormula, StrikingDurationBoostOnly) {
    SFIn in = sfin();
    in.TargetType = SFIn::TARGET_OTHER;
    in.INTE = 90;
    in.SkillLevel = 40;
    in.PartySize = 2;
    SFOut out;
    decore::skillformula::Striking(in, out);
    EXPECT_EQ(5, out.Damage);      // 90/30 + 40/20, no effect boost
    EXPECT_EQ(1170, out.Duration); // 900 * 130/100
    EXPECT_EQ(50, out.Delay);      // (6 - 40/33)*10
}

TEST(SkillOutputFormula, RevealerBoostAppliesAfterDelayCopiesDuration) {
    // Delay is assigned from Duration BEFORE the party boost scales
    // Duration — so they intentionally end up different. Ordering quirk,
    // pinned.
    SFIn in = sfin();
    in.SkillLevel = 50;
    in.PartySize = 6;
    SFOut out;
    decore::skillformula::Revealer(in, out);
    EXPECT_EQ(400, out.Delay);    // (30 + 50/5)*10, pre-boost
    EXPECT_EQ(800, out.Duration); // 400 * 200/100
}

TEST(SkillOutputFormula, GradeSwitchesReadDomainGrade) {
    SFIn in = sfin();
    in.SkillLevel = 50;
    in.DomainGrade = decore::skillformula::SKILL_GRADE_EXPERT;
    SFOut out;
    decore::skillformula::ContinualLight(in, out);
    EXPECT_EQ(4, out.Range);
    EXPECT_EQ(40, out.Delay);     // (6 - 50/25)*10
    EXPECT_EQ(350, out.Duration); // (10 + 50/2)*10

    in = sfin();
    in.SkillLevel = 40;
    in.DomainGrade = decore::skillformula::SKILL_GRADE_GRAND_MASTER;
    out = SFOut();
    decore::skillformula::Purify(in, out);
    EXPECT_EQ(14, out.Damage); // 10 + 40/10
    EXPECT_EQ(40, out.Delay);  // (5 - 40/33)*10
    EXPECT_EQ(7, out.Range);

    in = sfin();
    in.SkillLevel = 20;
    in.PartySize = 3;
    in.DomainGrade = decore::skillformula::SKILL_GRADE_MASTER;
    out = SFOut();
    decore::skillformula::DetectInvisibility(in, out);
    EXPECT_EQ(5, out.Range);
    EXPECT_EQ(310, out.Duration); // (10 + 20/2)*10 = 200, then *155/100
    EXPECT_EQ(60, out.Delay);     // (6 - 20/33)*10

    // Unset grade (-1): the default case zeroes Range in all three grade
    // switches while the other fields still compute.
    in = sfin();
    in.SkillLevel = 50;
    out = SFOut();
    decore::skillformula::ContinualLight(in, out);
    EXPECT_EQ(0, out.Range);
    in = sfin();
    in.SkillLevel = 40;
    out = SFOut();
    decore::skillformula::Purify(in, out);
    EXPECT_EQ(0, out.Range);
    EXPECT_EQ(14, out.Damage);
    in = sfin();
    in.SkillLevel = 20;
    in.PartySize = 3;
    out = SFOut();
    decore::skillformula::DetectInvisibility(in, out);
    EXPECT_EQ(0, out.Range);
    EXPECT_EQ(310, out.Duration);
}

TEST(SkillOutputFormula, ExpansionDelayEqualsDuration) {
    SFIn in = sfin();
    in.STR = 100;
    in.SkillLevel = 60;
    SFOut out;
    decore::skillformula::Expansion(in, out);
    EXPECT_EQ(40, out.Damage);     // 10 + 60/2
    EXPECT_EQ(25, out.ToHit);      // 5 + 60/3
    EXPECT_EQ(1150, out.Duration); // (45 + 100/10 + 60)*10
    EXPECT_EQ(out.Duration, out.Delay);
}

TEST(SkillOutputFormula, DancingSwordDelayEqualsDuration) {
    SFIn in = sfin();
    in.DEX = 100;
    in.STR = 100;
    in.SkillLevel = 50;
    SFOut out;
    decore::skillformula::DancingSword(in, out);
    EXPECT_EQ(16, out.Damage);    // 1 + 100/10 + 50/10
    EXPECT_EQ(650, out.Duration); // (30 + 100/10 + 50/2)*10
    EXPECT_EQ(out.Duration, out.Delay);
}

TEST(SkillOutputFormula, FlashSlidingClampAndNegativeDuration) {
    SFIn in = sfin();
    in.SkillLevel = 200;
    SFOut out;
    decore::skillformula::FlashSliding(in, out);
    EXPECT_EQ(10, out.Delay);    // max(3 - 200/50, 1)*10
    EXPECT_EQ(-1, out.Duration); // 3 - 200/50: unclamped, preserved
    in.SkillLevel = 0;
    out = SFOut();
    decore::skillformula::FlashSliding(in, out);
    EXPECT_EQ(30, out.Delay);
    EXPECT_EQ(3, out.Duration);
}

TEST(SkillOutputFormula, AuraRingDelayFloor) {
    SFIn in = sfin();
    in.INTE = 90;
    in.SkillLevel = 60;
    SFOut out;
    decore::skillformula::AuraRing(in, out);
    EXPECT_EQ(44, out.Damage); // 15 + 90/10 + 60/3
    EXPECT_EQ(10, out.Delay);  // max(1, 2 - 60/50)*10
    in.SkillLevel = 20;
    out = SFOut();
    decore::skillformula::AuraRing(in, out);
    EXPECT_EQ(20, out.Delay); // max(1, 2 - 0)*10
}

TEST(SkillOutputFormula, AcidBoltMinMaxClamps) {
    SFIn in = sfin();
    in.STR = 100;
    in.INTE = 100;
    in.DEX = 100;
    SFOut out;
    decore::skillformula::AcidBolt(in, out);
    EXPECT_EQ(25, out.Damage); // min(40, 100/20 + 100/5)
    EXPECT_EQ(10, out.Delay);  // max(1, 3 - 2 - 2)*10
    in.STR = 400;
    in.INTE = 200;
    out = SFOut();
    decore::skillformula::AcidBolt(in, out);
    EXPECT_EQ(40, out.Damage); // clamped
}

TEST(SkillOutputFormula, GreenStalkerTickAndDelayFloor) {
    SFIn in = sfin();
    in.STR = 100;
    in.INTE = 50;
    in.DEX = 200;
    SFOut out;
    decore::skillformula::GreenStalker(in, out);
    EXPECT_EQ(10, out.Damage); // 100/20 + 50/10
    EXPECT_EQ(40, out.Tick);
    EXPECT_EQ(500, out.Duration); // (200/5 + 50/5)*10
    EXPECT_EQ(10, out.Delay);     // max(1, 5 - 200/40)*10
}

TEST(SkillOutputFormula, BloodySnakeInteMinusTwentyBase) {
    SFIn in = sfin();
    in.INTE = 290;
    SFOut out;
    decore::skillformula::BloodySnake(in, out);
    EXPECT_EQ(35, out.Damage);   // min(35, 10 + 270/9)
    EXPECT_EQ(40, out.Duration); // (1 + 270/80)*10
    EXPECT_EQ(30, out.Delay);    // max(3, 6 - 270/50)*10
    EXPECT_EQ(3, out.Tick);
    in.INTE = 20;
    out = SFOut();
    decore::skillformula::BloodySnake(in, out);
    EXPECT_EQ(10, out.Damage);
    EXPECT_EQ(10, out.Duration);
    EXPECT_EQ(60, out.Delay);
}

TEST(SkillOutputFormula, EmptyFormulasLeaveEverythingZero) {
    SFIn in = sfin();
    in.STR = 999;
    in.SkillLevel = 999;
    SFOut out;
    decore::skillformula::RegenerationSkill(in, out);
    EXPECT_EQ(0, out.Damage);
    EXPECT_EQ(0, out.Duration);
    EXPECT_EQ(0, out.Tick);
    EXPECT_EQ(0, out.ToHit);
    EXPECT_EQ(0, out.Range);
    EXPECT_EQ(0, out.Delay);
}

TEST(SkillOutputFormula, MagicElusionAttrSumIsWordWide) {
    SFIn in = sfin();
    in.STR = 100;
    in.DEX = 100;
    in.INTE = 100;
    SFOut out;
    decore::skillformula::MagicElusion(in, out);
    EXPECT_EQ(60, out.Damage);    // 300/5
    EXPECT_EQ(150, out.Duration); // 50 + 300/3
    EXPECT_EQ(50, out.Delay);
    // The sum funnels through Attr_t (WORD): 70000 wraps to 4464 before
    // the divisions. Shipped behavior, preserved.
    in.STR = 30000;
    in.DEX = 30000;
    in.INTE = 10000;
    out = SFOut();
    decore::skillformula::MagicElusion(in, out);
    EXPECT_EQ(892, out.Damage);    // 4464/5
    EXPECT_EQ(1538, out.Duration); // 50 + 4464/3
    out = SFOut();
    decore::skillformula::IllusionOfAvenge(in, out);
    EXPECT_EQ(1503, out.Damage); // 15 + 4464/3
}

TEST(SkillOutputFormula, MentalSwordRangeGrowsWithLevel) {
    SFIn in = sfin();
    in.SkillLevel = 50;
    SFOut out;
    decore::skillformula::MentalSword(in, out);
    EXPECT_EQ(45, out.Damage); // 30 + 15*50/50
    EXPECT_EQ(3, out.Range);   // 2 + 50/33
    EXPECT_EQ(60, out.Delay);  // (8 - 50/20)*10
}

//////////////////////////////////////////////////////////////////////////////
// initAllStat bonus formulas (src/domain/Formulas.cpp, adapters:
// InitAllStat.cpp).
//////////////////////////////////////////////////////////////////////////////

TEST(InitAllStatBonus, ConcealmentDividesBeforeScaling) {
    // The int division by 20/10 happens BEFORE the float scale — 85 DEX
    // gives 4*1.4 = 5.6 -> 5, not (85*1.4)/20 = 5.95 -> 5. Pinned via a
    // value where the orders differ: dex 99, level 25 -> 4*2.0 = 8
    // (naive order would give 9).
    EXPECT_EQ(10, decore::concealmentDefenseBonus(100, 25));    // 5 * 2.0
    EXPECT_EQ(5, decore::concealmentDefenseBonus(85, 10));      // 4 * 1.4
    EXPECT_EQ(8, decore::concealmentDefenseBonus(99, 25));      // 4 * 2.0
    EXPECT_EQ(20, decore::concealmentProtectionBonus(100, 25)); // 10 * 2.0
    EXPECT_EQ(12, decore::concealmentProtectionBonus(95, 10));  // 9 * 1.4
}

TEST(InitAllStatBonus, WillOfIronFifteenPercentTruncated) {
    EXPECT_EQ(150, decore::willOfIronHPBonus(1000));
    EXPECT_EQ(149, decore::willOfIronHPBonus(999)); // 149.85 truncates
    EXPECT_EQ(1, decore::willOfIronHPBonus(13));    // 1.95 truncates
    EXPECT_EQ(0, decore::willOfIronHPBonus(0));
}

TEST(InitAllStatBonus, LivenessTablesAndLevelOverride) {
    decore::LivenessBonus b = decore::livenessBonus(4, 100);
    EXPECT_EQ(40, b.hpPercent);
    EXPECT_EQ(100, b.defenseBonus);
    b = decore::livenessBonus(4, 125); // level >= 125 overrides hpPercent
    EXPECT_EQ(50, b.hpPercent);
    EXPECT_EQ(100, b.defenseBonus);
    b = decore::livenessBonus(1, 130); // override applies whatever the grade
    EXPECT_EQ(50, b.hpPercent);
    EXPECT_EQ(10, b.defenseBonus);
    b = decore::livenessBonus(2, 60);
    EXPECT_EQ(20, b.hpPercent);
    EXPECT_EQ(35, b.defenseBonus);
    b = decore::livenessBonus(-1, 0); // unknown grade: zeros
    EXPECT_EQ(0, b.hpPercent);
    EXPECT_EQ(0, b.defenseBonus);
    // China table: different steps, no level override.
    b = decore::livenessBonusChina(2);
    EXPECT_EQ(25, b.hpPercent);
    EXPECT_EQ(35, b.defenseBonus);
    b = decore::livenessBonusChina(3);
    EXPECT_EQ(40, b.hpPercent);
    b = decore::livenessBonusChina(4);
    EXPECT_EQ(100, b.hpPercent);
    EXPECT_EQ(100, b.defenseBonus);
}

TEST(InitAllStatBonus, SnipingPercentsEvaluateLeftToRight) {
    // percent = stat/div * level / 20, LEFT TO RIGHT: the stat division
    // truncates first, the /20 only after the multiply — so 95 DEX at
    // level 50 gives (95/10)*50/20 = 450/20 = 22%, not 9*(50/20) = 18%.
    EXPECT_EQ(20, decore::snipingDamageBonus(200, 100, 40)); // (5*40)/20=10% of 200
    EXPECT_EQ(0, decore::snipingDamageBonus(200, 19, 40));   // 19/20=0 -> 0%
    EXPECT_EQ(33, decore::snipingToHitBonus(150, 95, 50));   // 22% of 150
}

TEST(InitAllStatBonus, SlayerWeaponPassives) {
    EXPECT_EQ(7, decore::swordMasteryDamageBonus(60)); // 3 + 60/15
    EXPECT_EQ(8, decore::concentrationToHitBonus(50)); // 3 + 50/10
    EXPECT_EQ(19, decore::evasionDefenseBonus(100));   // 3 + 80/5
    // Below level 20 the (level-20)/5 term goes negative, toward zero.
    EXPECT_EQ(1, decore::evasionDefenseBonus(10));            // 3 + (-10)/5
    EXPECT_EQ(5, decore::shieldMasteryProtectionBonus(20));   // 5 + 0
    EXPECT_EQ(25, decore::shieldMasteryProtectionBonus(120)); // 5 + 20
}

TEST(InitAllStatBonus, VampireTransformAndExtreme) {
    EXPECT_EQ(13, decore::wolfDamageBonus(80, 90));     // 80/8 + 90/30
    EXPECT_EQ(18, decore::werwolfDamageBonus(90, 120)); // 90/6 + 120/40
    EXPECT_EQ(5, decore::extremeDamageBonus(50));       // 4 + 30/30
    EXPECT_EQ(15, decore::extremeDamageBonus(470));     // capped
    EXPECT_EQ(9, decore::extremeToHitBonus(100, 100));  // 4 + 200/40
    EXPECT_EQ(20, decore::extremeToHitBonus(500, 500)); // capped
}

TEST(InitAllStatBonus, OustersEffectsAndHideSight) {
    EXPECT_EQ(13, decore::intimateGrailPenaltyRatio(35)); // 10 + 35/10
    EXPECT_EQ(5, decore::summonSylphProtectionBonus(40)); // floor
    EXPECT_EQ(10, decore::summonSylphProtectionBonus(100));
    EXPECT_EQ(5, decore::summonSylphResistBonus(60)); // floor (60/15 = 4)
    EXPECT_EQ(6, decore::summonSylphResistBonus(90));
    // Hide Sight: low band 15 + level*8/9, high band 35 + level*4/9,
    // with a 10% truncated bump exactly at level 30.
    EXPECT_EQ(23, decore::hideSightToHitBonus(9));  // 15 + 8
    EXPECT_EQ(28, decore::hideSightToHitBonus(15)); // 15 + 13 (last low)
    EXPECT_EQ(42, decore::hideSightToHitBonus(16)); // 35 + 7 (first high)
    EXPECT_EQ(47, decore::hideSightToHitBonus(29)); // 35 + 12
    EXPECT_EQ(52, decore::hideSightToHitBonus(30)); // 48 * 1.1 = 52.8 -> 52
}

} // namespace
