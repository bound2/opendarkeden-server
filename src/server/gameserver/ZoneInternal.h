//////////////////////////////////////////////////////////////////////////////
// FileName 	: ZoneInternal.h
// Description	: Zone helpers shared between the Zone translation units.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ZONE_INTERNAL_H__
#define __ZONE_INTERNAL_H__

#include "types/ZoneTypes.h"

class Creature;
class Monster;
class MonsterCorpse;
class Player;
class Zone;

// Whether an ordinary monster treats the creature as an enemy.
bool isPotentialEnemy(Monster* pMonster, Creature* pCreature);

// Sends the effects attached to the corpse to one player, or to everyone who
// can see (x,y) in the zone.
void sendRelicEffect(MonsterCorpse* pMonsterCorpse, Player* pPlayer);
void sendRelicEffect(MonsterCorpse* pMonsterCorpse, Zone* pZone, ZoneCoord_t x, ZoneCoord_t y);

#endif
