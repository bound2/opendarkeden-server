#ifndef __RELIC_UTIL_H__
#define __RELIC_UTIL_H__

#include "Effect.h"
#include "Exception.h"
#include "Item.h"

class Object;
class Corpse;
class Creature;
class Zone;
class PlayerCreature;

bool isRelicItem(const Item* pItem);
bool isRelicItem(Item::ItemClass IClass);

// A relic's row is about to name storage. Every gateway into a storage
// other than the ground, a corpse, the inventory or the mouse refuses a
// relic (de::war::relicMayLieIn), and a war returns a relic only from those;
// a row naming another one is logged to WarError.log. The row is written
// all the same.
void logRelicStorage(const Item* pItem, Storage storage, const string& ownerID);

void saveItemInCorpse(Item* pItem, Corpse* pCorpse);

bool addRelicEffect(Creature* pCreature, Item* pItem);
bool addHasRelicEffect(Zone* pZone, Corpse* pCorpse, Item* pItem);

bool addEffectRelicPosition(Item* pItem, ZoneID_t zoneID, TPOINT tp);
bool deleteEffectRelicPosition(Item* pItem);
bool deleteRelicEffect(Corpse* pCorpse, Item* pItem);
bool deleteRelicEffect(Creature* pCreature, Item* pItem);

//////////////////////////////////////////////////////////////////////////////
// When a relic is dropped into a zone
//////////////////////////////////////////////////////////////////////////////
bool dropRelicToZone(PlayerCreature* pPC, Item* pItem);
bool dropRelicToZone(Creature* pCreature, bool bSendPacket = true);

bool dissectionRelicItem(Corpse* pCorpse, Item* pItem, const TPOINT& pt);

void sendBloodBibleEffect(Object* pObject, Effect::EffectClass EClass);
void sendHolyLandWarpEffect(Creature* pCreature);
void sendRelicWarpEffect(Corpse* pCorpse);


#endif
