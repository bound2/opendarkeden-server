//////////////////////////////////////////////////////////////////////////////
// Filename    : HitRoll.h
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __HITROLL_H__
#define __HITROLL_H__

#include "ModifyInfo.h"

//////////////////////////////////////////////////////////////////////////////
// forward declaration
//////////////////////////////////////////////////////////////////////////////
class Creature;
class Slayer;
class Vampire;
class Ousters;
class Monster;
class SkillInfo;
class SkillSlot;
class VampireSkillSlot;
class OustersSkillSlot;

//////////////////////////////////////////////////////////////////////////////
// class HitRoll
// A class that groups the various hit roll functions into one namespace.
//////////////////////////////////////////////////////////////////////////////

class HitRoll {
public:
    // Hit roll for a normal attack
    static bool isSuccess(Creature* pAttacker, Creature* pDefender, int ToHitBonus = 0);

    // Magic hit roll for a Slayer
    static bool isSuccessMagic(Slayer* pSlayer, SkillInfo* pSkillInfo, SkillSlot* pSkillSlot);

    // Magic hit roll for a Vampire
    static bool isSuccessMagic(Vampire* pVampire, SkillInfo* pSkillInfo, VampireSkillSlot* pVampireSkillSlot,
                               int BonusPoint = 0);

    // Magic hit roll for an Ousters
    static bool isSuccessMagic(Ousters* pOusters, SkillInfo* pSkillInfo, OustersSkillSlot* pOustersSkillSlot,
                               int BonusPoint = 0);

    // Magic hit roll for a monster
    static bool isSuccessMagic(Monster* pMonster, SkillInfo* pSkillInfo);

    // Blood drain hit roll for a Vampire or a monster
    static bool isSuccessBloodDrain(Creature* pAttacker, Creature* pDefender, int multiplier = 3);

    // Hit roll for curse magic
    static bool isSuccessCurse(int MagicLevel, Resist_t resist);
    static bool isSuccessVampireCurse(int MagicLevel, Resist_t resist);

    // Hit roll for CurePoison
    static bool isSuccessCurePoison(int Base, int SkillLevel, int Difficulty, int MagicLevel, int MinRatio = 0);

    // Hit roll for Flare
    static bool isSuccessFlare(Creature* pTargetCreature, int SkillLevel);

    // Hit roll for RemoveCurse
    static bool isSuccessRemoveCurse(int Base, int SkillLevel, int Difficulty, int MagicLevel, int MinRatio = 0);

    // Hit roll for Rebuke
    static bool isSuccessRebuke(Slayer* pSlayer, SkillSlot* pSkillSlot, Creature* pDefender);

    // Hit roll for Magic Elusion
    static bool isSuccessMagicElusion(Slayer* pSlayer);

    // Hit roll for Poison Mesh
    static bool isSuccessPoisonMesh(Vampire* pVampire);

    // Hit roll for Illusion Of Avenge
    static bool isSuccessIllusionOfAvenge(Slayer* pSlayer);

    // Hit roll for Will Of Life
    static bool isSuccessWillOfLife(Vampire* pVampire);

    // Performs the critical hit roll.
    static bool isCriticalHit(Creature* pCreature, int CriticalBonus = 0);

    // Checks whether the skill is a Slayer or an Ousters self skill.
    static bool isSlayerSelfSkill(SkillType_t skillType);
    static bool isOustersSelfSkill(SkillType_t skillType);

    // Hallucination success rate
    static bool isSuccessHallucination(Vampire* pAttacker, Creature* pTarget);

    // Backstab success rate
    static bool isSuccessBackStab(Ousters* pAttacker);
};

#endif
