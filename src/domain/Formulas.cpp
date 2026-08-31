//////////////////////////////////////////////////////////////////////////////
// Filename    : Formulas.cpp
// Description :
// Pure balance formulas, transplanted verbatim from AbilityBalance.cpp and
// skill/SkillUtil.cpp (docs/RESTRUCTURING.md task 3.3). See Formulas.h.
//
// The originals accumulate into the narrow wire typedefs (HP_t/Damage_t are
// WORD, Speed_t/Range_t/Steal_t are BYTE), so extreme inputs wrap. That
// wrap-around is observed behavior — the transplants keep the same widths
// on purpose, and tests/formula_test.cpp pins representative wrap cases.
//////////////////////////////////////////////////////////////////////////////

#include "Formulas.h"

#include <algorithm>
#include <cmath>

using std::max;
using std::min;
// <cmath> is only guaranteed to provide std::pow/std::sqrt; the global
// names are an implementation courtesy this freestanding library must not
// lean on.
using std::pow;
using std::sqrt;

namespace decore {

namespace {
// Width-mirrors of the Core typedefs (Core's WORD/BYTE/DWORD). de-core is
// freestanding, so it carries its own copies.
typedef unsigned short Word;
typedef unsigned char Byte;
typedef unsigned int DWord;

// Caps from AbilityBalance.h.
const int kSlayerMaxHP = 10000;
const int kSlayerMaxMP = 10000;
const int kSlayerMaxToHit = 10000;
const int kSlayerMaxDefense = 10000;
const int kSlayerMaxProtection = 10000;
const int kSlayerMaxDamage = 10000;

const int kVampireMaxHP = 10000;
const int kVampireMaxToHit = 10000;
const int kVampireMaxDefense = 10000;
const int kVampireMaxProtection = 10000;
const int kVampireMaxDamage = 10000;

const int kOustersMaxHP = 10000;
const int kOustersMaxMP = 10000;
const int kOustersMaxToHit = 10000;
const int kOustersMaxDefense = 10000;
const int kOustersMaxProtection = 10000;
const int kOustersMaxDamage = 10000;

const int kMonsterMaxHP = 60000;
const int kMonsterMaxToHit = 1000;
const int kMonsterMaxDefense = 1000;
const int kMonsterMaxProtection = 1000;
const int kMonsterMaxDamage = 1000;

const Byte kSlayerMaxAttackSpeed = 35;
const Byte kVampireMaxAttackSpeed = 30;
const Byte kOustersMaxAttackSpeed = 35;
} // namespace

int percentValue(int value, int percent) {
    return (int)((long long)(value) * (long long)(percent) / 100);
}

//////////////////////////////////////////////////////////////////////////////
// Max HP
//////////////////////////////////////////////////////////////////////////////

int slayerMaxHP(const StatAttr& a, int hpRatioPercent, bool hardcoreTriple) {
    Word maxHP = 0;
    double CSTR = a.str;

    maxHP = (int)(CSTR * 3.00);

    // The weapon family swaps the whole base formula, not just the bonus:
    // priest/soldier weapons drop the base to STR*2.
    switch (a.weapon) {
    case WeaponFamily::Sword:
    case WeaponFamily::Blade:
        maxHP += (a.weaponDomainLevel * 5);
        break;
    case WeaponFamily::Cross:
    case WeaponFamily::Mace:
        maxHP = (int)(CSTR * 2.00);
        maxHP += (a.weaponDomainLevel * 4);
        break;
    case WeaponFamily::Arms:
        maxHP = (int)(CSTR * 2.00);
        maxHP += (a.weaponDomainLevel * 2);
        break;
    default:
        break;
    }

    maxHP = percentValue(maxHP, hpRatioPercent);
    maxHP = min((int)maxHP, kSlayerMaxHP);

    if (hardcoreTriple)
        maxHP *= 3;

    return maxHP;
}

int vampireMaxHP(const StatAttr& a, int hpRatioPercent, bool hardcoreTriple) {
    Word maxHP = 0;
    double CSTR = a.str;
    double CDEX = a.dex;
    double CINT = a.inte;
    double CLEVEL = a.level;

    maxHP = (int)((CSTR * 4.00 + CINT + CDEX + CLEVEL));
    maxHP = percentValue(maxHP, hpRatioPercent);
    maxHP = min((int)maxHP, kVampireMaxHP);

    if (hardcoreTriple)
        maxHP *= 3;

    return maxHP;
}

int oustersMaxHP(const StatAttr& a, int hpRatioPercent, bool hardcoreTriple) {
    Word maxHP = 0;
    double CSTR = a.str;
    double CDEX = a.dex;
    double CINT = a.inte;
    double CLEVEL = a.level;

    maxHP = (int)(CSTR * 3.00 + CINT / 2.00 + CDEX + CLEVEL);
    maxHP = percentValue(maxHP, hpRatioPercent);
    maxHP = min((int)maxHP, kOustersMaxHP);

    if (hardcoreTriple)
        maxHP *= 3;

    return maxHP;
}

int monsterMaxHP(const StatAttr& a, int enhancePercent, int hpRatioPercent, bool hardcoreTriple) {
    Word maxHP = 0;
    double CSTR = a.str;
    double CLEVEL = a.level;

    maxHP = (int)(CSTR * (2.00 + CLEVEL / 100.0));
    maxHP += percentValue(maxHP, enhancePercent);
    maxHP = percentValue(maxHP, hpRatioPercent);
    maxHP = min((int)maxHP, kMonsterMaxHP);

    if (hardcoreTriple)
        maxHP *= 3;

    return maxHP;
}

//////////////////////////////////////////////////////////////////////////////
// Max MP
//////////////////////////////////////////////////////////////////////////////

int slayerMaxMP(const StatAttr& a) {
    Word maxMP = 0;
    double CINTE = a.inte;

    maxMP = (int)(CINTE * 2.0);
    maxMP = min((int)maxMP, kSlayerMaxMP);

    return maxMP;
}

int oustersMaxMP(const StatAttr& a) {
    Word maxMP = 0;
    double CINTE = a.inte;
    double CLEVEL = a.level;

    maxMP = (int)((CINTE + CLEVEL) * 0.7);
    maxMP = min((int)maxMP, kOustersMaxMP);

    return maxMP;
}

//////////////////////////////////////////////////////////////////////////////
// To-hit
//////////////////////////////////////////////////////////////////////////////

int slayerToHit(const StatAttr& a) {
    Word toHit = 0;
    double CDEX = a.dex;

    toHit = (int)(CDEX / 2);

    // Any recognized weapon family pays a domain-level bonus.
    switch (a.weapon) {
    case WeaponFamily::Sword:
    case WeaponFamily::Blade:
    case WeaponFamily::Cross:
    case WeaponFamily::Mace:
    case WeaponFamily::Arms:
        toHit += (int)(a.weaponDomainLevel * 1.5);
        break;
    default:
        break;
    }

    toHit = min((int)toHit, kSlayerMaxToHit);
    return toHit;
}

int vampireToHit(const StatAttr& a) {
    Word toHit = 0;
    double CDEX = a.dex;
    double CLEVEL = a.level;

    toHit = (int)(CDEX + CLEVEL / 2.5);
    toHit = min((int)toHit, kVampireMaxToHit);
    return toHit;
}

int oustersToHit(const StatAttr& a) {
    Word toHit = 0;
    double CDEX = a.dex;
    double CLEVEL = a.level;

    toHit = (int)(CDEX / 2.0 + CLEVEL);
    toHit = min((int)toHit, kOustersMaxToHit);
    return toHit;
}

int monsterToHit(const StatAttr& a, int enhancePercent) {
    Word toHit = 0;
    double CDEX = a.dex;
    double CLEVEL = a.level;

    toHit = (int)((CDEX / 2.0) * (1.0 + CLEVEL / 100.0));
    toHit += percentValue(toHit, enhancePercent);
    toHit = min((int)toHit, kMonsterMaxToHit);
    return toHit;
}

//////////////////////////////////////////////////////////////////////////////
// Defense
//////////////////////////////////////////////////////////////////////////////

int slayerDefense(const StatAttr& a) {
    Word Defense = 0;
    double CDEX = a.dex;

    Defense = (int)(CDEX / 2.0);
    Defense = min((int)Defense, kSlayerMaxDefense);
    return Defense;
}

int vampireDefense(const StatAttr& a) {
    Word Defense = 0;
    double CDEX = a.dex;
    double CLEVEL = a.level;

    Defense = (int)(CDEX / 2.0 + CLEVEL / 5.0);
    Defense = min((int)Defense, kVampireMaxDefense);
    return Defense;
}

int oustersDefense(const StatAttr& a) {
    Word Defense = 0;
    double CDEX = a.dex;
    double CLEVEL = a.level;

    Defense = (int)(CDEX / 2.0 + CLEVEL / 5.0);
    Defense = min((int)Defense, kOustersMaxDefense);
    return Defense;
}

int monsterDefense(const StatAttr& a, int enhancePercent) {
    Word Defense = 0;
    double CDEX = a.dex;
    double CLEVEL = a.level;

    Defense = (int)((CDEX / 2.0) * (1.0 + CLEVEL / 100.0));
    Defense += percentValue(Defense, enhancePercent);
    Defense = min((int)Defense, kMonsterMaxDefense);
    return Defense;
}

//////////////////////////////////////////////////////////////////////////////
// Protection
//////////////////////////////////////////////////////////////////////////////

int slayerProtection(const StatAttr& a) {
    Word Protection = 0;
    double CSTR = a.str;

    Protection = (int)(CSTR);
    Protection = min((int)Protection, kSlayerMaxProtection);
    return Protection;
}

int vampireProtection(const StatAttr& a) {
    Word Protection = 0;
    double CSTR = a.str;
    double CLEVEL = a.level;

    Protection = (int)(CSTR + CLEVEL / 5.0);
    Protection = min((int)Protection, kVampireMaxProtection);
    return Protection;
}

int oustersProtection(const StatAttr& a) {
    Word Protection = 0;
    double CSTR = a.str;
    double CLEVEL = a.level;

    Protection = (int)(CSTR + CLEVEL / 10.0);
    Protection = min((int)Protection, kOustersMaxProtection);
    return Protection;
}

int monsterProtection(const StatAttr& a, int enhancePercent) {
    Word Protection = 0;
    double CSTR = a.str;
    double CLEVEL = a.level;

    Protection = (int)(CSTR / (5.0 - CLEVEL / 100.0));
    Protection += percentValue(Protection, enhancePercent);
    Protection = min((int)Protection, kMonsterMaxProtection);
    return Protection;
}

//////////////////////////////////////////////////////////////////////////////
// Melee damage range
//////////////////////////////////////////////////////////////////////////////

int slayerMinDamage(const StatAttr& a, int combatDamageBonus) {
    Word minDamage = 0;
    double CSTR = a.str;

    minDamage = (int)(CSTR / 15.0);

    // Soldier (gun) weapons get no strength bonus at all.
    if (a.weapon == WeaponFamily::Arms)
        minDamage = 1;

    minDamage += combatDamageBonus;
    minDamage = min((int)minDamage, kSlayerMaxDamage);
    return minDamage;
}

int vampireMinDamage(const StatAttr& a, int combatDamageBonus) {
    Word minDamage = 0;
    double CSTR = a.str;
    double CLEVEL = a.level;

    minDamage = (int)(CSTR / 6.0 + CLEVEL / 5.0);
    minDamage += combatDamageBonus;
    minDamage = min((int)minDamage, kVampireMaxDamage);
    return minDamage;
}

int oustersMinDamage(const StatAttr& a) {
    Word minDamage = 0;
    double CSTR = a.str;
    double CLEVEL = a.level;

    minDamage = (int)(CSTR / 10.0 + CLEVEL / 10.0);
    minDamage = min((int)minDamage, kOustersMaxDamage);
    return minDamage;
}

int monsterMinDamage(const StatAttr& a, int enhancePercent, int damageRatioPercent) {
    Word minDamage = 0;
    double CSTR = a.str;
    double CLEVEL = a.level;

    minDamage = (int)(CSTR / (6.0 - CLEVEL / 100.0));
    minDamage += percentValue(minDamage, enhancePercent);
    minDamage = percentValue(minDamage, damageRatioPercent);
    minDamage = min((int)minDamage, kMonsterMaxDamage);
    return minDamage;
}

int slayerMaxDamage(const StatAttr& a, int combatDamageBonus) {
    Word maxDamage = 0;
    double CSTR = a.str;

    maxDamage = (int)(CSTR / 10.0);

    // Soldier (gun) weapons get no strength bonus at all.
    if (a.weapon == WeaponFamily::Arms)
        maxDamage = 2;

    maxDamage += combatDamageBonus;
    maxDamage = min((int)maxDamage, kSlayerMaxDamage);
    return maxDamage;
}

int vampireMaxDamage(const StatAttr& a, int combatDamageBonus) {
    Word maxDamage = 0;
    double CSTR = a.str;
    double CLEVEL = a.level;

    maxDamage = (int)(CSTR / 4.0 + CLEVEL / 5.0);
    maxDamage += combatDamageBonus;
    maxDamage = min((int)maxDamage, kVampireMaxDamage);
    return maxDamage;
}

int oustersMaxDamage(const StatAttr& a) {
    Word maxDamage = 0;
    double CSTR = a.str;
    double CLEVEL = a.level;

    maxDamage = (int)(CSTR / 6.0 + CLEVEL / 6.0);
    maxDamage = min((int)maxDamage, kOustersMaxDamage);
    return maxDamage;
}

int monsterMaxDamage(const StatAttr& a, int enhancePercent, int damageRatioPercent) {
    Word maxDamage = 0;
    double CSTR = a.str;
    double CLEVEL = a.level;

    maxDamage = (int)(CSTR / (4.0 - CLEVEL / 100.0));
    maxDamage += percentValue(maxDamage, enhancePercent);
    maxDamage = percentValue(maxDamage, damageRatioPercent);
    maxDamage = min((int)maxDamage, kMonsterMaxDamage);
    return maxDamage;
}

//////////////////////////////////////////////////////////////////////////////
// Attack speed
//////////////////////////////////////////////////////////////////////////////

int slayerAttackSpeed(const StatAttr& a) {
    Byte AttackSpeed = 0;
    double CSTR = a.str;
    double CDEX = a.dex;

    // Strength drives attack speed by default...
    AttackSpeed = (int)(CSTR / 10.0);

    switch (a.weapon) {
    case WeaponFamily::Sword:
    case WeaponFamily::Blade:
        AttackSpeed += (int)(a.weaponDomainLevel / 5.0);
        break;
    case WeaponFamily::Arms:
        // ...but a gun's speed comes from dexterity instead.
        AttackSpeed = (int)(CDEX / 10.0);
        AttackSpeed += (int)(a.weaponDomainLevel / 5.0);
        break;
    default:
        break;
    }

    AttackSpeed = min(kSlayerMaxAttackSpeed, AttackSpeed);
    return AttackSpeed;
}

int vampireAttackSpeed(const StatAttr& a) {
    Byte AttackSpeed = 0;
    double CDEX = a.dex;

    AttackSpeed = (int)(CDEX / 10.0 + 10.0);
    AttackSpeed = min(kVampireMaxAttackSpeed, AttackSpeed);
    return AttackSpeed;
}

int oustersAttackSpeed(const StatAttr& a) {
    Byte AttackSpeed = 0;
    double CDEX = a.dex;
    double CLEVEL = a.level;

    AttackSpeed = (int)(CDEX / 10.0 + CLEVEL / 10.0);
    AttackSpeed = min(kOustersMaxAttackSpeed, AttackSpeed);
    return AttackSpeed;
}

//////////////////////////////////////////////////////////////////////////////
// Critical ratio (uncapped, and negative DEX deltas truncate toward zero)
//////////////////////////////////////////////////////////////////////////////

int slayerCriticalRatio(const StatAttr& a) {
    int CriticalRatio = 0;

    // Only sword, blade and gun domains grant criticals; priest weapons
    // (Cross/Mace) grant none.
    switch (a.weapon) {
    case WeaponFamily::Sword:
    case WeaponFamily::Blade:
    case WeaponFamily::Arms:
        CriticalRatio = (int)(a.weaponDomainLevel / 5.0);
        break;
    default:
        break;
    }

    return CriticalRatio;
}

int vampireCriticalRatio(const StatAttr& a) {
    double CDEX = a.dex;
    return (int)((CDEX - 20.0) / 30.0);
}

int oustersCriticalRatio(const StatAttr& a) {
    double CDEX = a.dex;
    return (int)((CDEX - 20.0) / 30.0);
}

int monsterCriticalRatio(const StatAttr& a, int enhancePercent) {
    double CDEX = a.dex;
    int CriticalRatio = (int)((CDEX - 20.0) / 30.0);
    CriticalRatio += percentValue(CriticalRatio, enhancePercent);
    return CriticalRatio;
}

//////////////////////////////////////////////////////////////////////////////
// Steal ratio
//////////////////////////////////////////////////////////////////////////////

int slayerStealRatio(const StatAttr& a, int amount) {
    // Stealing nothing succeeds never.
    if (amount == 0)
        return 0;

    Byte result = 0;

    // Bare hands count as the melee (90%) base; an unrecognized weapon
    // falls to the 65% base — the one place None and Other diverge.
    switch (a.weapon) {
    case WeaponFamily::None:
    case WeaponFamily::Sword:
    case WeaponFamily::Blade:
        result = (Byte)(90.0 - (float)amount * 1.4);
        break;
    default:
        result = (Byte)(65.0 - (float)amount * 1.4);
        break;
    }

    return result;
}

int vampireStealRatio(int amount) {
    if (amount == 0)
        return 0;
    return (Byte)(90.0 - (float)amount * 1.4);
}

int oustersStealRatio(int amount) {
    if (amount == 0)
        return 0;
    return (Byte)(90.0 - (float)amount * 1.4);
}

//////////////////////////////////////////////////////////////////////////////
// Combat / progression
//////////////////////////////////////////////////////////////////////////////

double finalDamage(int realDamage, int protection, bool critical) {
    // A critical hit ignores protection and lands the damage as-is.
    if (critical)
        return realDamage;

    if (protection > 640)
        protection = 640;

    Word FinalDamage;
    FinalDamage = realDamage - (realDamage * (protection / 8)) / 100;

    return max(1, (int)FinalDamage);
}

int tileDistance(int ox, int oy, int tx, int ty) {
    double OriginX = ox;
    double OriginY = oy;
    double TargetX = tx;
    double TargetY = ty;

    double XOffset = pow(OriginX - TargetX, 2.0);
    double YOffset = pow(OriginY - TargetY, 2.0);
    Byte range = (Byte)(sqrt(XOffset + YOffset));

    return range;
}

int rankExp(int myLevel, int otherLevel, int gainPercent, int premiumPercent) {
    DWord rankExp = 0;

    if (myLevel != 0 && otherLevel != 0) {
        int checkValue = otherLevel * 100 / myLevel;

        if (checkValue > 120) {
            rankExp = (otherLevel - myLevel) * (otherLevel - myLevel) / 10 + otherLevel * 100 / myLevel;
        } else {
            rankExp = otherLevel * 100 / myLevel - 10;
        }

        rankExp = max(0, (int)rankExp);
        rankExp = percentValue(rankExp, gainPercent);
    }

    rankExp = percentValue(rankExp, premiumPercent);

    return rankExp;
}

int vampireSkillConsumeMP(int originalMP, int magicLevel, int intStat) {
    int OriginalMP = originalMP;
    int MagicLevel = magicLevel;
    int INTE = max(0, intStat - 20);
    int DecreaseAmount = 0;

    if (INTE <= MagicLevel) {
    } else if (MagicLevel < INTE && INTE <= (MagicLevel * 1.5)) {
        DecreaseAmount = percentValue(OriginalMP, 10);
    } else if ((MagicLevel * 1.5) < INTE && INTE <= (MagicLevel * 2.0)) {
        DecreaseAmount = percentValue(OriginalMP, 25);
    } else if ((MagicLevel * 2.0) < INTE && INTE <= (MagicLevel * 3.0)) {
        DecreaseAmount = percentValue(OriginalMP, 50);
    } else if ((MagicLevel * 3.0) < INTE && INTE <= (MagicLevel * 4.0)) {
        DecreaseAmount = percentValue(OriginalMP, 60);
    } else if ((MagicLevel * 4.0) < INTE && INTE <= (MagicLevel * 5.0)) {
        DecreaseAmount = percentValue(OriginalMP, 75);
    } else if ((MagicLevel * 5.0) < INTE && INTE <= (MagicLevel * 6.0)) {
        DecreaseAmount = percentValue(OriginalMP, 85);
    } else if ((MagicLevel * 6.0) < INTE && INTE <= (MagicLevel * 7.0)) {
        DecreaseAmount = percentValue(OriginalMP, 90);
    } else if ((MagicLevel * 7.0) < INTE) {
        DecreaseAmount = percentValue(OriginalMP, 95);
    }

    return (OriginalMP - DecreaseAmount);
}

} // namespace decore
