//////////////////////////////////////////////////////////////////////////////
// Filename    : PacketUtil.h
// Written by  : excel96
// Description :
// Packets that are sent often and are complex to build are all constructed here,
// which makes maintenance easier.
//////////////////////////////////////////////////////////////////////////////

#ifndef __PACKETUTIL_H__
#define __PACKETUTIL_H__

#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// forward declaration
//////////////////////////////////////////////////////////////////////////////

class Creature;
class Slayer;
class Vampire;
class Ousters;
class Monster;
class NPC;
class Item;
class SlayerCorpse;
class VampireCorpse;
class OustersCorpse;
class MonsterCorpse;
class SLAYER_RECORD;
class VAMPIRE_RECORD;
class OUSTERS_RECORD;
class GamePlayer;
class PlayerCreature;

class GCUpdateInfo;
class GCAddSlayer;
class GCAddVampire;
class GCAddOusters;
class GCAddMonster;
class GCAddNPC;
class GCAddNewItemToZone;
class GCDropItemToZone;
class GCAddSlayerCorpse;
class GCAddVampireCorpse;
class GCAddMonsterCorpse;
class GCAddOustersCorpse;
class GCOtherModifyInfo;
class GCCreateItem;
class GCWarScheduleList;
class GCMiniGameScores;
class GCPetStashList;
class GCModifyInformation;

// class GCItemNameInfoList;

//////////////////////////////////////////////////////////////////////////////
// function headers
//////////////////////////////////////////////////////////////////////////////

// Build the guild union information into ModifyInformation.
void makeGCModifyInfoGuildUnion(GCModifyInformation* pModifyInformation, Creature* pCreature);
void makeGCOtherModifyInfoGuildUnion(GCOtherModifyInfo* pModifyInformation, Creature* pCreature);
void sendGCOtherModifyInfoGuildUnion(Creature* pTargetCreature);
void sendGCOtherModifyInfoGuildUnionByGuildID(uint gID);


// Build the GCUpdateInfo used when moving between maps through a portal or on death.
void makeGCUpdateInfo(GCUpdateInfo* pUpdateInfo, Creature* pCreature);

// Build the packet that adds a Slayer.
void makeGCAddSlayer(GCAddSlayer* pAddSlayer, Slayer* pSlayer);

// Build the packet that adds a Vampire.
void makeGCAddVampire(GCAddVampire* pAddVampire, Vampire* pVampire);

// Build the packet that adds an Ousters.
void makeGCAddOusters(GCAddOusters* pAddOusters, Ousters* pOusters);

// Build the packet that adds a Monster.
void makeGCAddMonster(GCAddMonster* pAddMonster, Monster* pMonster);

// Build the packet that adds an NPC.
void makeGCAddNPC(GCAddNPC* pAddNPC, NPC* pNPC);

// Build the GCAddNewItemToZone sent when a new item is added to the zone.
void makeGCAddNewItemToZone(GCAddNewItemToZone* pAddItem, Item* pItem, int X, int Y);

// Build the GCDropItemToZone sent when an item is dropped into the zone.
void makeGCDropItemToZone(GCDropItemToZone* pAddItem, Item* pItem, int X, int Y);

// Build the packet sent when a Slayer corpse is added to the zone.
void makeGCAddSlayerCorpse(GCAddSlayerCorpse* pAddSlayerCorpse, SlayerCorpse* pSlayerCorpse);

// Build the packet sent when a Vampire corpse is added to the zone.
void makeGCAddVampireCorpse(GCAddVampireCorpse* pAddVampireCorpse, VampireCorpse* pVampireCorpse);

// Build the packet sent when a Monster corpse is added to the zone.
void makeGCAddMonsterCorpse(GCAddMonsterCorpse* pAddMonsterCorpse, MonsterCorpse* pMonsterCorpse, int X, int Y);

// Build the packet sent when an Ousters corpse is added to the zone.
void makeGCAddOustersCorpse(GCAddOustersCorpse* pAddOustersCorpse, OustersCorpse* pOustersCorpse);

// Build the GCOtherModifyInfo sent when something such as another player's maximum HP changes.
void makeGCOtherModifyInfo(GCOtherModifyInfo* pInfo, Slayer* pSlayer, const SLAYER_RECORD* prev);
void makeGCOtherModifyInfo(GCOtherModifyInfo* pInfo, Vampire* pVampire, const VAMPIRE_RECORD* prev);
void makeGCOtherModifyInfo(GCOtherModifyInfo* pInfo, Ousters* pOusters, const OUSTERS_RECORD* prev);

// Show the payment information.
void sendPayInfo(GamePlayer* pGamePlayer);

// Create an item.
void makeGCCreateItem(GCCreateItem* pGCCreateItem, Item* pItem, CoordInven_t x, CoordInven_t y);

// Show the level up effect.
void sendEffectLevelUp(Creature* pCreature);

// Send GCSystemMessage(msg) to the GamePlayer. While still incoming, it is shown on entering the zone.
void sendSystemMessage(GamePlayer* pGamePlayer, const string& msg);

bool makeGCWarScheduleList(GCWarScheduleList* pGCWarScheduleList, ZoneID_t zoneID);

// Packet that sends the information of named items.
// void makeGCItemNameInfoList(GCItemNameInfoList* pInfo, PlayerCreature* pPC) ;

void sendGCMiniGameScores(PlayerCreature* pPC, BYTE gameType, BYTE Level);

void makeGCPetStashList(GCPetStashList* pPacket, PlayerCreature* pPC);

#endif
