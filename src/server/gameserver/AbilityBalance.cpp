//////////////////////////////////////////////////////////////////////////////
// Filename    : AbilityBalance.cpp
// Written By  : 김성민
// Description :
// Per-creature stat derivation. Since task 3.3 the actual formulas live as
// pure functions in de-core (src/domain/Formulas.cpp), where they are unit
// tested; these entry points only gather the inputs (weapon family, domain
// level, balance ratios from VariableManager, the Hardcore config flag) and
// delegate. Balance changes belong in de-core, next to their tests.
//////////////////////////////////////////////////////////////////////////////

#include "AbilityBalance.h"

#include "ItemUtil.h"
#include "Monster.h"
#include "Properties.h"
#include "Slayer.h"
#include "Vampire.h"
#include "VariableManager.h"
#include "domain/Formulas.h"

namespace {

decore::WeaponFamily weaponFamilyOf(Item* pWeapon) {
    if (pWeapon == NULL)
        return decore::WeaponFamily::None;

    switch (pWeapon->getItemClass()) {
    case Item::ITEM_CLASS_SWORD:
        return decore::WeaponFamily::Sword;
    case Item::ITEM_CLASS_BLADE:
        return decore::WeaponFamily::Blade;
    case Item::ITEM_CLASS_CROSS:
        return decore::WeaponFamily::Cross;
    case Item::ITEM_CLASS_MACE:
        return decore::WeaponFamily::Mace;
    default:
        return isArmsWeapon(pWeapon) ? decore::WeaponFamily::Arms : decore::WeaponFamily::Other;
    }
}

int domainLevelOf(decore::WeaponFamily family, BASIC_ATTR* pAttr) {
    switch (family) {
    case decore::WeaponFamily::Sword:
        return pAttr->pDomainLevel[SKILL_DOMAIN_SWORD];
    case decore::WeaponFamily::Blade:
        return pAttr->pDomainLevel[SKILL_DOMAIN_BLADE];
    case decore::WeaponFamily::Cross:
        return pAttr->pDomainLevel[SKILL_DOMAIN_HEAL];
    case decore::WeaponFamily::Mace:
        return pAttr->pDomainLevel[SKILL_DOMAIN_ENCHANT];
    case decore::WeaponFamily::Arms:
        return pAttr->pDomainLevel[SKILL_DOMAIN_GUN];
    default:
        return 0;
    }
}

// Only the Slayer formulas read the weapon fields, and the original code
// only ever touched pAttr->pWeapon inside the CREATURE_CLASS_SLAYER branch.
// That is load-bearing: BASIC_ATTR is a bare POD, and several call sites
// (Monster's constructor, Monster::initAllStat, Vampire/Ousters
// computeStatOffset) fill in only nSTR/nDEX/nINT/nLevel — their pWeapon and
// pDomainLevel[] are uninitialized stack. Reading them off the Slayer path
// is a wild-pointer virtual call, so the weapon data is gathered only when
// the caller's class actually consumes it.
decore::StatAttr toStatAttr(BASIC_ATTR* pAttr, bool withWeapon) {
    decore::StatAttr a;
    a.str = pAttr->nSTR;
    a.dex = pAttr->nDEX;
    a.inte = pAttr->nINT;
    a.level = pAttr->nLevel;
    a.weapon = withWeapon ? weaponFamilyOf(pAttr->pWeapon) : decore::WeaponFamily::None;
    a.weaponDomainLevel = withWeapon ? domainLevelOf(a.weapon, pAttr) : 0;
    return a;
}

bool isHardcore() {
    return g_pConfig->hasKey("Hardcore") && g_pConfig->getPropertyInt("Hardcore") != 0;
}

} // namespace

// If this changes, the maxHP computation in Slayer::load(), Vampire::load()
// and Ousters::load() must change with it. by sigi.
HP_t computeHP(Creature::CreatureClass CClass, BASIC_ATTR* pAttr, int enhance) {
    Assert(pAttr != NULL);

    decore::StatAttr a = toStatAttr(pAttr, CClass == Creature::CREATURE_CLASS_SLAYER);

    if (CClass == Creature::CREATURE_CLASS_SLAYER) {
        return decore::slayerMaxHP(a, g_pVariableManager->getVariable(SLAYER_HP_RATIO), isHardcore());
    } else if (CClass == Creature::CREATURE_CLASS_VAMPIRE) {
        return decore::vampireMaxHP(a, g_pVariableManager->getVariable(VAMPIRE_HP_RATIO), isHardcore());
    } else if (CClass == Creature::CREATURE_CLASS_OUSTERS) {
        return decore::oustersMaxHP(a, g_pVariableManager->getVariable(OUSTERS_HP_RATIO), isHardcore());
    } else if (CClass == Creature::CREATURE_CLASS_MONSTER) {
        return decore::monsterMaxHP(a, enhance, g_pVariableManager->getVariable(MONSTER_HP_RATIO), isHardcore());
    }

    return 0;
}

MP_t computeMP(Creature::CreatureClass CClass, BASIC_ATTR* pAttr) {
    Assert(pAttr != NULL);

    decore::StatAttr a = toStatAttr(pAttr, CClass == Creature::CREATURE_CLASS_SLAYER);

    if (CClass == Creature::CREATURE_CLASS_SLAYER) {
        return decore::slayerMaxMP(a);
    } else if (CClass == Creature::CREATURE_CLASS_OUSTERS) {
        return decore::oustersMaxMP(a);
    }

    return 0;
}

ToHit_t computeToHit(Creature::CreatureClass CClass, BASIC_ATTR* pAttr, int enhance) {
    Assert(pAttr != NULL);

    decore::StatAttr a = toStatAttr(pAttr, CClass == Creature::CREATURE_CLASS_SLAYER);

    if (CClass == Creature::CREATURE_CLASS_SLAYER) {
        return decore::slayerToHit(a);
    } else if (CClass == Creature::CREATURE_CLASS_VAMPIRE) {
        return decore::vampireToHit(a);
    } else if (CClass == Creature::CREATURE_CLASS_OUSTERS) {
        return decore::oustersToHit(a);
    } else if (CClass == Creature::CREATURE_CLASS_MONSTER) {
        return decore::monsterToHit(a, enhance);
    }

    return 0;
}

Defense_t computeDefense(Creature::CreatureClass CClass, BASIC_ATTR* pAttr, int enhance) {
    Assert(pAttr != NULL);

    decore::StatAttr a = toStatAttr(pAttr, CClass == Creature::CREATURE_CLASS_SLAYER);

    if (CClass == Creature::CREATURE_CLASS_SLAYER) {
        return decore::slayerDefense(a);
    } else if (CClass == Creature::CREATURE_CLASS_VAMPIRE) {
        return decore::vampireDefense(a);
    } else if (CClass == Creature::CREATURE_CLASS_OUSTERS) {
        return decore::oustersDefense(a);
    } else if (CClass == Creature::CREATURE_CLASS_MONSTER) {
        return decore::monsterDefense(a, enhance);
    }

    return 0;
}

Protection_t computeProtection(Creature::CreatureClass CClass, BASIC_ATTR* pAttr, int enhance) {
    Assert(pAttr != NULL);

    decore::StatAttr a = toStatAttr(pAttr, CClass == Creature::CREATURE_CLASS_SLAYER);

    if (CClass == Creature::CREATURE_CLASS_SLAYER) {
        return decore::slayerProtection(a);
    } else if (CClass == Creature::CREATURE_CLASS_VAMPIRE) {
        return decore::vampireProtection(a);
    } else if (CClass == Creature::CREATURE_CLASS_OUSTERS) {
        return decore::oustersProtection(a);
    } else if (CClass == Creature::CREATURE_CLASS_MONSTER) {
        return decore::monsterProtection(a, enhance);
    }

    return 0;
}

Damage_t computeMinDamage(Creature::CreatureClass CClass, BASIC_ATTR* pAttr, int enhance) {
    Assert(pAttr != NULL);

    decore::StatAttr a = toStatAttr(pAttr, CClass == Creature::CREATURE_CLASS_SLAYER);

    if (CClass == Creature::CREATURE_CLASS_SLAYER) {
        return decore::slayerMinDamage(a, g_pVariableManager->getCombatSlayerDamageBonus());
    } else if (CClass == Creature::CREATURE_CLASS_VAMPIRE) {
        return decore::vampireMinDamage(a, g_pVariableManager->getCombatVampireDamageBonus());
    } else if (CClass == Creature::CREATURE_CLASS_OUSTERS) {
        return decore::oustersMinDamage(a);
    } else if (CClass == Creature::CREATURE_CLASS_MONSTER) {
        return decore::monsterMinDamage(a, enhance, g_pVariableManager->getVariable(MONSTER_DAMAGE_RATIO));
    }

    return 0;
}

Damage_t computeMaxDamage(Creature::CreatureClass CClass, BASIC_ATTR* pAttr, int enhance) {
    Assert(pAttr != NULL);

    decore::StatAttr a = toStatAttr(pAttr, CClass == Creature::CREATURE_CLASS_SLAYER);

    if (CClass == Creature::CREATURE_CLASS_SLAYER) {
        return decore::slayerMaxDamage(a, g_pVariableManager->getCombatSlayerDamageBonus());
    } else if (CClass == Creature::CREATURE_CLASS_VAMPIRE) {
        return decore::vampireMaxDamage(a, g_pVariableManager->getCombatVampireDamageBonus());
    } else if (CClass == Creature::CREATURE_CLASS_OUSTERS) {
        return decore::oustersMaxDamage(a);
    } else if (CClass == Creature::CREATURE_CLASS_MONSTER) {
        return decore::monsterMaxDamage(a, enhance, g_pVariableManager->getVariable(MONSTER_DAMAGE_RATIO));
    }

    return 0;
}

Speed_t computeAttackSpeed(Creature::CreatureClass CClass, BASIC_ATTR* pAttr, int enhance) {
    Assert(pAttr != NULL);

    decore::StatAttr a = toStatAttr(pAttr, CClass == Creature::CREATURE_CLASS_SLAYER);

    if (CClass == Creature::CREATURE_CLASS_SLAYER) {
        return decore::slayerAttackSpeed(a);
    } else if (CClass == Creature::CREATURE_CLASS_VAMPIRE) {
        return decore::vampireAttackSpeed(a);
    } else if (CClass == Creature::CREATURE_CLASS_OUSTERS) {
        return decore::oustersAttackSpeed(a);
    }

    return 0;
}

int computeCriticalRatio(Creature::CreatureClass CClass, BASIC_ATTR* pAttr, int enhance) {
    Assert(pAttr != NULL);

    decore::StatAttr a = toStatAttr(pAttr, CClass == Creature::CREATURE_CLASS_SLAYER);

    if (CClass == Creature::CREATURE_CLASS_SLAYER) {
        return decore::slayerCriticalRatio(a);
    } else if (CClass == Creature::CREATURE_CLASS_VAMPIRE) {
        return decore::vampireCriticalRatio(a);
    } else if (CClass == Creature::CREATURE_CLASS_OUSTERS) {
        return decore::oustersCriticalRatio(a);
    } else if (CClass == Creature::CREATURE_CLASS_MONSTER) {
        return decore::monsterCriticalRatio(a, enhance);
    }

    return 0;
}

Steal_t computeStealRatio(Creature::CreatureClass CClass, Steal_t amount, BASIC_ATTR* pAttr) {
    // The original returned before touching pAttr at all when amount == 0;
    // keep that ordering so the guard runs ahead of any input gathering.
    if (amount == 0)
        return 0;

    decore::StatAttr a = toStatAttr(pAttr, CClass == Creature::CREATURE_CLASS_SLAYER);

    if (CClass == Creature::CREATURE_CLASS_SLAYER) {
        return decore::slayerStealRatio(a, amount);
    } else if (CClass == Creature::CREATURE_CLASS_VAMPIRE) {
        return decore::vampireStealRatio(amount);
    } else if (CClass == Creature::CREATURE_CLASS_OUSTERS) {
        return decore::oustersStealRatio(amount);
    }

    return 0;
}
