//////////////////////////////////////////////////////////////////////////////
// Filename    : Formulas.h
// Description :
// de-core: pure balance formulas (docs/RESTRUCTURING.md task 3.3).
//
// Every function here is a pure function of its arguments: no globals, no
// game-object types, no server-type macros, no includes outside the C++
// standard library. The gameserver keeps its original entry points
// (AbilityBalance.cpp, SkillUtil.cpp) as thin adapters that gather the
// inputs from Creature/Item/VariableManager/Properties and delegate here.
//
// The math is transplanted verbatim from those files — these formulas ARE
// the game balance. Oddities (narrow-integer wrap-around, asymmetric
// weapon-family branches, magic constants) are preserved on purpose and
// pinned by tests/formula_test.cpp; fixing one is a balance change, not a
// refactor.
//////////////////////////////////////////////////////////////////////////////

#ifndef __DECORE_FORMULAS_H__
#define __DECORE_FORMULAS_H__

namespace decore {

// How the balance formulas see the equipped right-hand weapon. None (bare
// hands) and Other (a weapon class no formula branches on) are distinct
// because slayerStealRatio treats them differently.
enum class WeaponFamily { None, Sword, Blade, Cross, Mace, Arms, Other };

// Numeric inputs of the stat-derivation formulas. weaponDomainLevel is the
// skill-domain level matching `weapon` (sword domain for Sword, gun domain
// for Arms, ...); 0 for None/Other.
struct StatAttr {
    int str;
    int dex;
    int inte;
    int level;
    WeaponFamily weapon;
    int weaponDomainLevel;
};

// getPercentValue from src/Core/Utility.h, duplicated so de-core stays
// freestanding. The (long long) widening is part of the contract.
int percentValue(int value, int percent);

//////////////////////////////////////////////////////////////////////////////
// Stat derivation (adapters: AbilityBalance.cpp). hardcoreTriple is the
// "Hardcore" server config flag: max HP is tripled after the cap.
//////////////////////////////////////////////////////////////////////////////
int slayerMaxHP(const StatAttr& a, int hpRatioPercent, bool hardcoreTriple);
int vampireMaxHP(const StatAttr& a, int hpRatioPercent, bool hardcoreTriple);
int oustersMaxHP(const StatAttr& a, int hpRatioPercent, bool hardcoreTriple);
int monsterMaxHP(const StatAttr& a, int enhancePercent, int hpRatioPercent, bool hardcoreTriple);

int slayerMaxMP(const StatAttr& a);
int oustersMaxMP(const StatAttr& a);

int slayerToHit(const StatAttr& a);
int vampireToHit(const StatAttr& a);
int oustersToHit(const StatAttr& a);
int monsterToHit(const StatAttr& a, int enhancePercent);

int slayerDefense(const StatAttr& a);
int vampireDefense(const StatAttr& a);
int oustersDefense(const StatAttr& a);
int monsterDefense(const StatAttr& a, int enhancePercent);

int slayerProtection(const StatAttr& a);
int vampireProtection(const StatAttr& a);
int oustersProtection(const StatAttr& a);
int monsterProtection(const StatAttr& a, int enhancePercent);

int slayerMinDamage(const StatAttr& a, int combatDamageBonus);
int vampireMinDamage(const StatAttr& a, int combatDamageBonus);
int oustersMinDamage(const StatAttr& a);
int monsterMinDamage(const StatAttr& a, int enhancePercent, int damageRatioPercent);

int slayerMaxDamage(const StatAttr& a, int combatDamageBonus);
int vampireMaxDamage(const StatAttr& a, int combatDamageBonus);
int oustersMaxDamage(const StatAttr& a);
int monsterMaxDamage(const StatAttr& a, int enhancePercent, int damageRatioPercent);

int slayerAttackSpeed(const StatAttr& a);
int vampireAttackSpeed(const StatAttr& a);
int oustersAttackSpeed(const StatAttr& a);

int slayerCriticalRatio(const StatAttr& a);
int vampireCriticalRatio(const StatAttr& a);
int oustersCriticalRatio(const StatAttr& a);
int monsterCriticalRatio(const StatAttr& a, int enhancePercent);

// amount is the steal amount (BYTE-ranged); returns the success percentage.
int slayerStealRatio(const StatAttr& a, int amount);
int vampireStealRatio(int amount);
int oustersStealRatio(int amount);

//////////////////////////////////////////////////////////////////////////////
// Combat / progression (adapters: skill/SkillUtil.cpp).
//////////////////////////////////////////////////////////////////////////////

// Final damage after protection. Critical hits ignore protection entirely;
// protection is clamped to 640 and each 8 points shave 1% off the damage,
// floored at 1.
double finalDamage(int realDamage, int protection, bool critical);

// Euclidean tile distance, truncated into the BYTE-wide Range_t exactly as
// the original does (distances beyond 255 wrap — preserved behavior).
int tileDistance(int ox, int oy, int tx, int ty);

// Rank experience for a kill: killing 20%+ above your level pays a
// quadratic bonus; below that a linear ratio minus 10, floored at 0.
// gainPercent scales the base; premiumPercent applies even when a level is
// 0 (where the base is 0 anyway).
int rankExp(int myLevel, int otherLevel, int gainPercent, int premiumPercent);

// Vampire magic MP cost after the INT discount brackets. intStat is the raw
// INT stat; the 20-point deadband and the bracket table live here.
int vampireSkillConsumeMP(int originalMP, int magicLevel, int intStat);

} // namespace decore

#endif
