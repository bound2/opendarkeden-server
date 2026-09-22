//////////////////////////////////////////////////////////////////////////////
// Filename    : SkillUtil.h
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILLUTIL_H__
#define __SKILLUTIL_H__

#include "ModifyInfo.h"
#include "Ousters.h"
#include "Slayer.h"
#include "Vampire.h"

//////////////////////////////////////////////////////////////////////////////
// Macros and constants
//////////////////////////////////////////////////////////////////////////////

// decreaseMana returns CONSUME_MP when only MP was spent,
// CONSUME_HP when only HP was spent,
// and CONSUME_BOTH when both were consumed.
#define CONSUME_MP 0
#define CONSUME_HP 1
#define CONSUME_BOTH 2

#define MAKEWORD(U, D) (WORD)((WORD)(U) << 8 | (WORD)(D))
#define MAKEDWORD(U, D) (DWORD)((DWORD)(U) << 16 | (DWORD)(D))


//////////////////////////////////////////////////////////////////////////////
// forward declaration
//////////////////////////////////////////////////////////////////////////////
class Creature;
class Monster;
class SkillInfo;
class SkillSlot;
class VampireSkillSlot;
class Item;
class Zone;

//////////////////////////////////////////////////////////////////////////////
// Damage computation functions
//////////////////////////////////////////////////////////////////////////////

// Computes the final damage from the attacker's and the defender's parameters.
Damage_t computeDamage(Creature* pCreature, Creature* pTargetCreature);

// Computes the attacker's pure damage.
Damage_t computePureDamage(Creature* pCreature);

// Computes the final damage from the attacker's and the defender's parameters.
// Same as the function above, but this one also handles critical hits internally.
Damage_t computeDamage(Creature* pCreature, Creature* pTargetCreature, int CriticalBonus, bool& bCritical);

// Returns the final damage, with protection subtracted from the raw damage.
double computeFinalDamage(Damage_t minDamage, Damage_t maxDamage, Damage_t realDamage, Protection_t Protection,
                          bool bCritical);

// Computes the damage between a Slayer attacker and the defender.
Damage_t computeSlayerDamage(Slayer* pSlayer, Creature* pTargetCreature, bool bCritical);

// Computes the damage between a Vampire attacker and the defender.
Damage_t computeVampireDamage(Vampire* pVampire, Creature* pTargetCreature, bool bCritical);

// Computes the damage between an Ousters attacker and the defender.
Damage_t computeOustersDamage(Ousters* pOusters, Creature* pTargetCreature, bool bCritical);

// Computes the damage between a monster attacker and the defender.
Damage_t computeMonsterDamage(Monster* pMonster, Creature* pTargetCreature, bool bCritical);

// Computes the pure damage of a Slayer attacker.
Damage_t computePureSlayerDamage(Slayer* pSlayer);

// Computes the pure damage of a Vampire attacker.
Damage_t computePureVampireDamage(Vampire* pVampire);

// Computes the pure damage of an Ousters attacker.
Damage_t computePureOustersDamage(Ousters* pOusters);

// Computes the damage between a monster attacker and the defender.
Damage_t computePureMonsterDamage(Monster* pMonster);

// Computes magic damage with resistance taken into account.
Damage_t computeMagicDamage(Creature* pTargetCreature, int Damage, SkillType_t SkillType, bool bVampire = false,
                            Creature* pAttacker = NULL);

// Computes Ousters magic damage with the wristlet taken into account.
Damage_t computeOustersMagicDamage(Ousters* pOusters, Creature* pTargetCreature, int Damage, SkillType_t SkillType);

// Computes the silver damage dealt to the target.
Damage_t computeSlayerSilverDamage(Creature* pCreature, int Damage, ModifyInfo* pMI);

// Computes the damage and critical changes from passive skills such as Critical Magic.
void computeCriticalBonus(Ousters* pOusters, SkillType_t skillType, Damage_t& Damage, bool& bCriticalHit);

//////////////////////////////////////////////////////////////////////////////
// Applying damage, reducing durability and so on
//////////////////////////////////////////////////////////////////////////////

// Applies damage directly.
HP_t setDamage(Creature* pTargetCreature, Damage_t Damage, Creature* pAttacker, SkillType_t SkillType = 0,
               ModifyInfo* pMI = NULL, ModifyInfo* pAttackerMI = NULL, bool canKillTarget = true, bool canSteal = true);

// Wears down item durability.
void decreaseDurability(Creature* pCreature, Creature* pTargetCreature, SkillInfo* pSkillInfo, ModifyInfo*,
                        ModifyInfo*);

// Can the target be hit?
bool canHit(Creature* pAttacker, Creature* pDefender, SkillType_t SkillType, SkillLevel_t SkillLevel = 0);


//////////////////////////////////////////////////////////////////////////////
// Mana related functions
//////////////////////////////////////////////////////////////////////////////

// Computes the mana cost of a Vampire spell, which varies with INT.
MP_t decreaseConsumeMP(Vampire* pVampire, SkillInfo* pSkillInfo);

// Does the caster have enough mana to use the skill?
bool hasEnoughMana(Creature* pCaster, int RequiredMP);

// Reduces mana.
int decreaseMana(Creature* pCaster, int MP, ModifyInfo& modifyinfo);


//////////////////////////////////////////////////////////////////////////////
// Validation functions for range, duration, timing and so on
//////////////////////////////////////////////////////////////////////////////

// Computes the range of a Slayer skill.
Range_t computeSkillRange(SkillSlot* pSkillSlot, SkillInfo* pSkillInfo);

// Computes the distance between (OX,OY) and (TX,TY).
Range_t getDistance(ZoneCoord_t OX, ZoneCoord_t OY, ZoneCoord_t TX, ZoneCoord_t TY);

// Verifies that the distance allows the skill to be used.
bool verifyDistance(Creature* pCreature, ZoneCoord_t X, ZoneCoord_t Y, Range_t Distance);

// Verifies that the distance allows the skill to be used.
bool verifyDistance(Creature* pCreature, Creature* pTargetCreature, Range_t Distance);

// Verifies the run time of a Slayer skill.
bool verifyRunTime(SkillSlot* pSkillSlot);

// Verifies the run time of a Vampire skill.
bool verifyRunTime(VampireSkillSlot* pSkillSlot);

// Verifies the run time of an Ousters skill.
bool verifyRunTime(OustersSkillSlot* pSkillSlot);

// Decides whether PK is allowed, following each zone's PK policy.
bool verifyPK(Creature* pAttacker, Creature* pDefender);

// Is this a zone where the skill may be used?
// (For a self skill, this checks the zone level.)
bool checkZoneLevelToUseSkill(Creature* pCaster);

// Checks whether the creature standing at X, Y can be affected by a skill.
bool checkZoneLevelToHitTarget(Creature* pTargetCreature);

// Is the skill a melee attack?

// Is it a magic skill?
// Is it a physical skill?

//////////////////////////////////////////////////////////////////////////////
// Alignment related functions
//////////////////////////////////////////////////////////////////////////////

// Changes the alignment.
// Computes the alignment change caused by using a skill or by PK.
void computeAlignmentChange(Creature* pTargetCreature, Damage_t Damage, Creature* pAttacker, ModifyInfo* pMI = NULL,
                            ModifyInfo* pAttackerMI = NULL);

// Restores a little alignment when a Slayer or Vampire kills a monster.
// Independently of the self-defense system, simply attacking another race
// restores alignment a little at a time.
void increaseAlignment(Creature* pCreature, Creature* pEnemy, ModifyInfo& mi);

//////////////////////////////////////////////////////////////////////////////
// Experience related functions
//////////////////////////////////////////////////////////////////////////////

// Computes Slayer experience for a party.
void shareAttrExp(Slayer* pSlayer, Damage_t Damage, BYTE STRMultiplier, BYTE DEXMultiplier, BYTE INTMultiplier,
                  ModifyInfo&);

// Computes Vampire experience for a party.
void shareVampExp(Vampire*, Exp_t, ModifyInfo&);

// Computes Ousters experience for a party.
void shareOustersExp(Ousters*, Exp_t, ModifyInfo&);

// Computes Slayer stat (STR, DEX, INT) experience.
void divideAttrExp(Slayer* pSlayer, Damage_t Damage, BYTE STRMultiplier, BYTE DEXMultiplier, BYTE INTMultiplier,
                   ModifyInfo&, int numPartyMember = -1);

// Computes Slayer skill experience.
void increaseSkillExp(Slayer* pSlayer, SkillDomainType_t DomainType, SkillSlot* pSkillSlot, SkillInfo* pSkillInfo,
                      ModifyInfo&);

// Computes Slayer domain experience.
bool increaseDomainExp(Slayer* pSlayer, SkillDomainType_t Domain, Exp_t Exp, ModifyInfo&, Level_t EnemyLevel = 0,
                       int TargetNum = -1);

// Computes Vampire experience.
void increaseVampExp(Vampire*, Exp_t, ModifyInfo&);

// Computes Ousters experience.
void increaseOustersExp(Ousters*, Exp_t, ModifyInfo&);

// Computes Slayer and Vampire fame.
void increaseFame(Creature* pAttackee, uint amount);

// Handling for a kill
RankExp_t computeRankExp(int myLevel, int otherLevel);
void affectKillCount(Creature* pAttacker, Creature* pDeadCreature);

//////////////////////////////////////////////////////////////////////////////
// Miscellaneous functions
//////////////////////////////////////////////////////////////////////////////

// Computes the SG and SR bonuses for the distance.
int computeArmsWeaponSplashSize(Item* pWeapon, int ox, int oy, int tx, int ty);
int computeArmsWeaponDamageBonus(Item* pWeapon, int ox, int oy, int tx, int ty);
int computeArmsWeaponToHitBonus(Item* pWeapon, int ox, int oy, int tx, int ty);

// Collects the creatures around the given coordinates that take splash damage.
int getSplashVictims(Zone* pZone, int cx, int cy, Creature::CreatureClass CClass, list<Creature*>& creatureList,
                     int splash);
int getSplashVictims(Zone* pZone, int cx, int cy, Creature::CreatureClass CClass, list<Creature*>& creatureList,
                     int splash, int range);
int getSplashVictims(Zone* pZone, int cx, int cy, list<Creature*>& creatureList, int splash);

// Fills HP and MP on level up.
void healCreatureForLevelUp(Slayer* pSlayer, ModifyInfo& _ModifyInfo, SLAYER_RECORD* prev);
void healCreatureForLevelUp(Vampire* pVampire, ModifyInfo& _ModifyInfo, VAMPIRE_RECORD* prev);
void healCreatureForLevelUp(Ousters* pOusters, ModifyInfo& _ModifyInfo, OUSTERS_RECORD* prev);

// Sends a packet when a skill fails.
void executeSkillFailNormal(Creature* pCreature, SkillType_t SkillType, Creature* pTargetCreature, BYTE Grade = 0);
void executeAbsorbSoulSkillFail(Creature* pCreature, SkillType_t SkillType, ObjectID_t TargetObjectID, bool bBroadcast,
                                bool bSendTwice);
void executeSkillFailNormalWithGun(Creature* pCreature, SkillType_t SkillType, Creature* pTargetCreature,
                                   BYTE RemainBullet);
void executeSkillFailException(Creature* pCreature, SkillType_t SkillType, BYTE Grade = 0);

void decreaseHP(Zone* pZone, Creature* pCreature, int Damage, ObjectID_t attackerObjectID = 0);

Dir_t getDirectionToPosition(int originX, int originY, int destX, int destY);

Exp_t computeSkillPointBonus(SkillDomainType_t Domain, SkillLevel_t DomainLevel, Item* pWeapon, Exp_t Point);


// Can one walk from point to point? (Creatures are not counted as blockers.)
bool isPassLine(Zone* pZone, ZoneCoord_t sX, ZoneCoord_t sY, ZoneCoord_t eX, ZoneCoord_t eY,
                bool blockByCreature = false);

// Collects the points forming the straight line between two points.
void getLinePoint(ZoneCoord_t sX, ZoneCoord_t sY, ZoneCoord_t eX, ZoneCoord_t eY, list<TPOINT>& tpList);

ElementalType getElementalTypeFromString(const string& type);

Damage_t computeElementalCombatSkill(Ousters* pOusters, Creature* pTargetCreature, ModifyInfo& AttackerMI);

//////////////////////////////////////////////////////////////////////////////
// Can the attack proceed?
// Checks invulnerability and the non-PK setting.
//////////////////////////////////////////////////////////////////////////////
bool canAttack(Creature* pAttacker, Creature* pDefender);

//////////////////////////////////////////////////////////////////////////
// add by Coffee 2007-6-9
// Consumes the caster's race-specific skill card, failing if none is held.
//////////////////////////////////////////////////////////////////////////
bool useSkillCrad(Creature* pCreature);


#endif
