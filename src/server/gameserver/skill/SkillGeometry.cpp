//////////////////////////////////////////////////////////////////////////////
// FileName 	: SkillGeometry.cpp
// Description	: Zone geometry for skills: distance checks, line of sight, splash areas and facing.
//////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <stdio.h>

#include "AlignmentManager.h"
#include "CreatureUtil.h"
#include "DynamicZone.h"
#include "EffectAirShield.h"
#include "EffectAlignmentRecovery.h"
#include "EffectArmageddon.h"
#include "EffectAuraShield.h"
#include "EffectBlindness.h"
#include "EffectBlockHead.h"
#include "EffectCanEnterGDRLair.h"
#include "EffectDivineSpirits.h"
#include "EffectEnemyErase.h"
#include "EffectExpansion.h"
#include "EffectExplosionWater.h"
#include "EffectFrozenArmor.h"
#include "EffectGrandMasterOusters.h"
#include "EffectGrandMasterSlayer.h"
#include "EffectGrandMasterVampire.h"
#include "EffectHandsOfFire.h"
#include "EffectHymn.h"
#include "EffectIceFieldToCreature.h"
#include "EffectIceOfSoulStone.h"
#include "EffectInstallTurret.h"
#include "EffectMephisto.h"
#include "EffectPrecedence.h"
#include "EffectReactiveArmor.h"
#include "EffectRediance.h"
#include "EffectRequital.h"
#include "EffectRevealer.h"
#include "EffectShareHP.h"
#include "EffectSharpShield.h"
#include "EffectSleep.h"
#include "EffectStoneSkin.h"
#include "EffectStriking.h"
#include "EffectSwordOfThor.h"
#include "EffectWaterBarrier.h"
#include "EventHeadCount.h"
#include "EventItemUtil.h"
#include "GCAddEffect.h"
#include "GCAddEffectToTile.h"
#include "GCAddInjuriousCreature.h"
#include "GCKickMessage.h"
#include "GCLearnSkillReady.h"
#include "GCOtherModifyInfo.h"
#include "GCRemoveEffect.h"
#include "GCRemoveFromGear.h"
#include "GCSkillFailed1.h"
#include "GCSkillFailed2.h"
#include "GCSkillToObjectOK4.h"
#include "GCSkillToObjectOK6.h"
#include "GCStatusCurrentHP.h"
#include "GCSystemMessage.h"
#include "GDRLairManager.h"
#include "GQuestManager.h"
#include "GamePlayer.h"
#include "GameServerInfoManager.h"
#include "HitRoll.h"
#include "ItemFactoryManager.h"
#include "ItemUtil.h"
#include "LogClient.h"
#include "MasterLairInfoManager.h"
#include "Monster.h"
#include "OustersEXPInfo.h"
#include "PKZoneInfoManager.h"
#include "PacketUtil.h"
#include "Party.h"
#include "Player.h"
#include "PrecedenceTable.h"
#include "Properties.h"
#include "SkillDomainInfoManager.h"
#include "SkillInfo.h"
#include "SkillPropertyManager.h"
#include "SkillUtil.h"
#include "SkillUtilInternal.h"
#include "SummonGroundElemental.h"
#include "Thread.h"
#include "VampEXPInfo.h"
#include "VariableManager.h"
#include "Zone.h"
#include "ZoneUtil.h"
#include "domain/Formulas.h"
#include "mission/EventQuestLootingManager.h"
#include "mission/MonsterKillQuestStatus.h"
#include "mission/QuestManager.h"

//////////////////////////////////////////////////////////////////////////////
// (OX,OY)¿Í (TX,TY) »çÀÌÀÇ °Å¸®¸¦ ±¸ÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
Range_t getDistance(ZoneCoord_t Ox, ZoneCoord_t Oy, ZoneCoord_t Tx, ZoneCoord_t Ty) {
    // Pure geometry — lives in de-core.
    return decore::tileDistance(Ox, Oy, Tx, Ty);
}

//////////////////////////////////////////////////////////////////////////////
// ½ºÅ³À» ¾µ ¼ö ÀÖ´Â Àû´çÇÑ °Å¸®ÀÎ°¡¸¦ °ËÁõÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
bool verifyDistance(Creature* pCreature, ZoneCoord_t X, ZoneCoord_t Y, Range_t Dist) {
    Assert(pCreature != NULL);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    ZoneCoord_t cx = pCreature->getX();
    ZoneCoord_t cy = pCreature->getY();

    ZoneLevel_t AttackerZoneLevel = pZone->getZoneLevel(cx, cy);

    // ¾Æ´ãÀÇ ¼ºÁö³ª PKÁ¸ ³»ÀÇ ¾ÈÀüÁö´ë¿¡¼­´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    if ((AttackerZoneLevel & SAFE_ZONE) && (g_pPKZoneInfoManager->isPKZone(pZone->getZoneID()) || pZone->isHolyLand()))
        return false;

    // °ø°ÝÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ½½·¹ÀÌ¾î ¾ÈÀüÁö´ë¶ó¸é,
    // ½½·¹ÀÌ¾î°¡ ¾Æ´Ñ ÀÚ´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    if ((AttackerZoneLevel & SLAYER_SAFE_ZONE) && !pCreature->isSlayer())
        return false;
    // °ø°ÝÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ¹ìÆÄÀÌ¾î ¾ÈÀüÁö´ë¶ó¸é,
    // ¹ìÆÄÀÌ¾î°¡ ¾Æ´Ñ ÀÚ´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    else if ((AttackerZoneLevel & VAMPIRE_SAFE_ZONE) && !pCreature->isVampire())
        return false;
    // °ø°ÝÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ¾Æ¿ì½ºÅÍ½º ¾ÈÀüÁö´ë¶ó¸é,
    // ¾Æ¿ì½ºÅÍ½º°¡ ¾Æ´Ñ ÀÚ´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    else if ((AttackerZoneLevel & OUSTERS_SAFE_ZONE) && !pCreature->isOusters())
        return false;
    // ¿ÏÀü ¾ÈÀüÁö´ë¶ó¸é ½½·¹ÀÌ¾îµç ¹ìÆÄÀÌ¾îµç ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    else if (AttackerZoneLevel & COMPLETE_SAFE_ZONE)
        return false;

    // ¹æ¾îÀÚ°¡ ¼­ ÀÖ´Â °÷ÀÌ ¿ÏÀüÁö´ë¶ó¸é ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.

    if ((abs(cx - X) <= Dist) && (abs(cy - Y) <= Dist))
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////////
// ½ºÅ³À» ¾µ ¼ö ÀÖ´Â Àû´çÇÑ °Å¸®ÀÎ°¡¸¦ °ËÁõÇÑ´Ù.
//////////////////////////////////////////////////////////////////////////////
bool verifyDistance(Creature* pCreature, Creature* pTargetCreature, Range_t Dist) {
    Assert(pCreature != NULL);
    Assert(pTargetCreature != NULL);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    int ox = pCreature->getX();
    int oy = pCreature->getY();
    int tx = pTargetCreature->getX();
    int ty = pTargetCreature->getY();

    ZoneLevel_t AttackerZoneLevel = pZone->getZoneLevel(ox, oy);
    ZoneLevel_t DefenderZoneLevel = pZone->getZoneLevel(tx, ty);

    // ¾Æ´ãÀÇ ¼ºÁö³ª PKÁ¸ ³»ÀÇ ¾ÈÀüÁö´ë¿¡¼­´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    if ((AttackerZoneLevel & SAFE_ZONE) && (g_pPKZoneInfoManager->isPKZone(pZone->getZoneID()) || pZone->isHolyLand()))
        return false;

    // °ø°ÝÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ½½·¹ÀÌ¾î ¾ÈÀüÁö´ë¶ó¸é,
    // ½½·¹ÀÌ¾î°¡ ¾Æ´Ñ ÀÚ´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    if ((AttackerZoneLevel & SLAYER_SAFE_ZONE) && !pCreature->isSlayer())
        return false;
    // °ø°ÝÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ¹ìÆÄÀÌ¾î ¾ÈÀüÁö´ë¶ó¸é,
    // ¹ìÆÄÀÌ¾î°¡ ¾Æ´Ñ ÀÚ´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    else if ((AttackerZoneLevel & VAMPIRE_SAFE_ZONE) && !pCreature->isVampire())
        return false;
    // °ø°ÝÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ¾Æ¿ì½ºÅÍ½º ¾ÈÀüÁö´ë¶ó¸é,
    // ¾Æ¿ì½ºÅÍ½º°¡ ¾Æ´Ñ ÀÚ´Â ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    else if ((AttackerZoneLevel & OUSTERS_SAFE_ZONE) && !pCreature->isOusters())
        return false;
    // ¿ÏÀü ¾ÈÀüÁö´ë¶ó¸é ½½·¹ÀÌ¾îµç ¹ìÆÄÀÌ¾îµç ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    else if (AttackerZoneLevel & COMPLETE_SAFE_ZONE)
        return false;

    // ¹æ¾îÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ½½·¹ÀÌ¾î ¾ÈÀüÁö´ëÀÌ°í,
    // ¹æ¾îÀÚ°¡ ½½·¹ÀÌ¾î¶ó¸é ±â¼úÀº ¸ÂÁö ¾Ê´Â´Ù.
    if ((DefenderZoneLevel & SLAYER_SAFE_ZONE) && pTargetCreature->isSlayer())
        return false;
    // ¹æ¾îÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ¹ìÆÄÀÌ¾î ¾ÈÀüÁö´ëÀÌ°í,
    // ¹æ¾îÀÚ°¡ ¹ìÆÄÀÌ¾î¶ó¸é ±â¼úÀº ¸ÂÁö ¾Ê´Â´Ù.
    else if ((DefenderZoneLevel & VAMPIRE_SAFE_ZONE) && pTargetCreature->isVampire())
        return false;
    // ¹æ¾îÀÚ°¡ ¼­ ÀÖ´Â À§Ä¡°¡ ¾Æ¿ì½ºÅÍ½º ¾ÈÀüÁö´ëÀÌ°í,
    // ¹æ¾îÀÚ°¡ ¾Æ¿ì½ºÅÍ½º¶ó¸é ±â¼úÀº ¸ÂÁö ¾Ê´Â´Ù.
    else if ((DefenderZoneLevel & OUSTERS_SAFE_ZONE) && pTargetCreature->isOusters())
        return false;
    // ¿ÏÀü ¾ÈÀüÁö´ë¶ó¸é ±â¼úÀ» »ç¿ëÇÒ ¼ö ¾ø´Ù.
    else if (DefenderZoneLevel & COMPLETE_SAFE_ZONE)
        return false;

    if ((abs(tx - ox) <= Dist) && (abs(ty - oy) <= Dist))
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////////
// ÁöÁ¤µÈ ÁÂÇ¥ ÁÖÀ§ÀÇ ½ºÇÃ·¡½Ã µ¥¹ÌÁö¸¦ ¸ÂÀ» Å©¸®ÃÄ¸¦ »Ì¾Æ¿Â´Ù.
//////////////////////////////////////////////////////////////////////////////
int getSplashVictims(Zone* pZone, int cx, int cy, Creature::CreatureClass CClass, list<Creature*>& creatureList,
                     int splash) {
    VSRect rect(0, 0, pZone->getWidth() - 1, pZone->getHeight() - 1);

    // ÇØ´ç Å©¸®ÃÄ°¡ ½½·¹ÀÌ¾î¶ó¸é, ±× ½½·¹ÀÌ¾î¸¸ ¸Â°í,
    // ÁÖÀ§ÀÇ ´Ù¸¥ ½½·¹ÀÌ¾îµéÀº ¸ÂÁö ¾Ê´Â´Ù.
    if (CClass == Creature::CREATURE_CLASS_SLAYER) {
        if (rect.ptInRect(cx, cy)) {
            Tile& rTile = pZone->getTile(cx, cy);

            if (rTile.hasCreature(Creature::MOVE_MODE_WALKING)) {
                Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_WALKING);
                if (pCreature->getCreatureClass() == CClass) {
                    creatureList.push_back(pCreature);
                }
            }
            // ÇöÀç·Î¼­´Â ³¯¾Æ´Ù´Ï´Â ½½·¹ÀÌ¾î´Â ¾øÁö¸¸...
            if (rTile.hasCreature(Creature::MOVE_MODE_FLYING)) {
                Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_FLYING);
                if (pCreature->getCreatureClass() == CClass) {
                    creatureList.push_back(pCreature);
                }
            }
        }

        return (int)creatureList.size();
    }

    vector<Creature*> creatureVector;
    vector<int> pickedVector;

    for (int i = 0; i < 9; i++) {
        int tilex = cx + dirMoveMask[i].x;
        int tiley = cy + dirMoveMask[i].y;

        if (rect.ptInRect(tilex, tiley)) {
            Tile& rTile = pZone->getTile(tilex, tiley);

            if (rTile.hasCreature(Creature::MOVE_MODE_WALKING)) {
                Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_WALKING);

                if (CClass == Creature::CREATURE_CLASS_MAX) {
                    // CREATURE_CLASS_MAX°¡ ÆÄ¶ó¹ÌÅÍ·Î ³Ñ¾î¿À´Â °æ¿ì¿¡´Â ¹«Á¶°Ç ´õÇÏÀÚ.
                    creatureVector.push_back(pCreature);
                } else if (pCreature->getCreatureClass() == CClass) {
                    // ¾Æ´Ñ °æ¿ì¿¡´Â CreatureClass°¡ °°Àº °æ¿ì¿¡¸¸ ´õÇÑ´Ù.
                    creatureVector.push_back(pCreature);
                }
            }

            if (rTile.hasCreature(Creature::MOVE_MODE_FLYING)) {
                Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_FLYING);
                if (CClass == Creature::CREATURE_CLASS_MAX) {
                    // CREATURE_CLASS_MAX°¡ ÆÄ¶ó¹ÌÅÍ·Î ³Ñ¾î¿À´Â °æ¿ì¿¡´Â ¹«Á¶°Ç ´õÇÏÀÚ.
                    creatureVector.push_back(pCreature);
                } else if (pCreature->getCreatureClass() == CClass) {
                    // ¾Æ´Ñ °æ¿ì¿¡´Â CreatureClass°¡ °°Àº °æ¿ì¿¡¸¸ ´õÇÑ´Ù.
                    creatureVector.push_back(pCreature);
                }
            }
        }
    }

    // ½ºÇÃ·¡½Ã µ¥¹ÌÁö¸¦ ÀÔÈú ³ðµéÀÇ ¼ýÀÚº¸´Ù ÇöÀç ÀÖ´Â Å©¸®ÃÄ°¡ Àû´Ù¸é,
    // ¸ðµÎ ½ºÇÃ·¡½Ã µ¥¹ÌÁö¸¦ ÀÔÈ÷¸é µÈ´Ù.
    if ((int)creatureVector.size() <= splash) {
        for (int i = 0; i < (int)creatureVector.size(); i++) {
            creatureList.push_back(creatureVector[i]);
        }
    }
    // ½ºÇÃ·¡½Ã µ¥¹ÌÁö¸¦ ÀÔÈú ³ðº¸´Ù ÇöÀç Á¸ÀçÇÏ´Â Å©¸®ÃÄµéÀÌ ¸¹´Ù¸é,
    // ÀÌ Áß¿¡ splash ¼ýÀÚ¸¸Å­ÀÇ Å©¸®ÃÄ¸¦ ÀÓÀÇ·Î »Ì¾Æ¾ß ÇÑ´Ù.
    else {
        // Á¦ÀÏ Ã³À½¿¡ 6³ðÀÌ ÀÖ°í, ÀÌ Áß¿¡ 4³ðÀ» »Ì¾Æ¾ß ÇÑ´Ù°í
        // °¡Á¤ÇÏ¸é, size = 6ÀÌ µÈ´Ù.
        // Indexes ¹è¿­¿¡´Â (0, 1, 2, 3, 4, 5, -1...)ÀÌ µé¾î°£´Ù.
        // ÀÌ Áß¿¡ 2¸¦ »Ì¾Ò´Ù°í °¡Á¤ÇÏÀÚ.
        // ±×·¯¸é ÀÌ ¹è¿­¿¡¼­ 2¸¦ Á¦°ÅÇØ Áà¾ß ÇÑ´Ù.
        // µÚ¿¡¼­ºÎÅÍ ¾ÕÀ¸·Î ÇÑÄ­¾¿ ¿Å°ÜÁà¾ß ÇÑ´Ù.
        // (0, 1, 3, 4, 5, 5...)
        // ±× ´ÙÀ½ »çÀÌÁî¸¦ ÁÙÀÌ°í, ´Ù½Ã ±× Áß¿¡¼­ ÇÏ³ª¸¦ ·£´ýÀ¸·Î
        // »Ì¾Æ°¡¸é °ãÄ¡Áö ¾Ê´Â Å©¸®ÃÄÀÇ ¸®½ºÆ®¸¦ ¾òÀ» ¼ö ÀÖ´Ù.
        std::vector<int> Indexes(creatureVector.size(), -1);
        int i;
        int size = creatureVector.size();
        for (i = 0; i < size; i++) {
            Indexes[i] = i;
        }

        for (i = 0; i < splash; i++) {
            int index = rand() % size;
            int realIndex = Indexes[index];
            creatureList.push_back(creatureVector[realIndex]);

            for (int m = index + 1; m < size; m++) {
                Indexes[m - 1] = Indexes[m];
            }

            size--;
        }
    }

    return (int)creatureList.size();
}

//////////////////////////////////////////////////////////////////////////////
// ÁöÁ¤µÈ ÁÂÇ¥ ÁÖÀ§ÀÇ Å©¸®Ã³¸¦ Ã£¾Æ¼­ ³Ñ°ÜÁØ´Ù.
//////////////////////////////////////////////////////////////////////////////
int getSplashVictims(Zone* pZone, int cx, int cy, Creature::CreatureClass CClass, list<Creature*>& creatureList,
                     int splash, int range) {
    VSRect rect(0, 0, pZone->getWidth() - 1, pZone->getHeight() - 1);

    vector<Creature*> creatureVector;
    vector<int> pickedVector;

    for (int y = -range; y <= range; y++) {
        for (int x = -range; x <= range; x++) {
            int tilex = cx + x;
            int tiley = cy + y;
            ;

            if (rect.ptInRect(tilex, tiley)) {
                Tile& rTile = pZone->getTile(tilex, tiley);

                if (rTile.hasCreature(Creature::MOVE_MODE_WALKING)) {
                    Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_WALKING);

                    if (CClass == Creature::CREATURE_CLASS_MAX) {
                        // CREATURE_CLASS_MAX°¡ ÆÄ¶ó¹ÌÅÍ·Î ³Ñ¾î¿À´Â °æ¿ì¿¡´Â ¹«Á¶°Ç ´õÇÏÀÚ.
                        creatureVector.push_back(pCreature);
                    } else if (pCreature->getCreatureClass() == CClass) {
                        // ¾Æ´Ñ °æ¿ì¿¡´Â CreatureClass°¡ °°Àº °æ¿ì¿¡¸¸ ´õÇÑ´Ù.
                        creatureVector.push_back(pCreature);
                    }
                }

                if (rTile.hasCreature(Creature::MOVE_MODE_FLYING)) {
                    Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_FLYING);
                    if (CClass == Creature::CREATURE_CLASS_MAX) {
                        // CREATURE_CLASS_MAX°¡ ÆÄ¶ó¹ÌÅÍ·Î ³Ñ¾î¿À´Â °æ¿ì¿¡´Â ¹«Á¶°Ç ´õÇÏÀÚ.
                        creatureVector.push_back(pCreature);
                    } else if (pCreature->getCreatureClass() == CClass) {
                        // ¾Æ´Ñ °æ¿ì¿¡´Â CreatureClass°¡ °°Àº °æ¿ì¿¡¸¸ ´õÇÑ´Ù.
                        creatureVector.push_back(pCreature);
                    }
                }
            }
        }
    }

    // ½ºÇÃ·¡½Ã µ¥¹ÌÁö¸¦ ÀÔÈú ³ðµéÀÇ ¼ýÀÚº¸´Ù ÇöÀç ÀÖ´Â Å©¸®ÃÄ°¡ Àû´Ù¸é,
    // ¸ðµÎ ½ºÇÃ·¡½Ã µ¥¹ÌÁö¸¦ ÀÔÈ÷¸é µÈ´Ù.
    if ((int)creatureVector.size() <= splash) {
        for (int i = 0; i < (int)creatureVector.size(); i++) {
            creatureList.push_back(creatureVector[i]);
        }
    }
    // ½ºÇÃ·¡½Ã µ¥¹ÌÁö¸¦ ÀÔÈú ³ðº¸´Ù ÇöÀç Á¸ÀçÇÏ´Â Å©¸®ÃÄµéÀÌ ¸¹´Ù¸é,
    // ÀÌ Áß¿¡ splash ¼ýÀÚ¸¸Å­ÀÇ Å©¸®ÃÄ¸¦ ÀÓÀÇ·Î »Ì¾Æ¾ß ÇÑ´Ù.
    else {
        // Á¦ÀÏ Ã³À½¿¡ 6³ðÀÌ ÀÖ°í, ÀÌ Áß¿¡ 4³ðÀ» »Ì¾Æ¾ß ÇÑ´Ù°í
        // °¡Á¤ÇÏ¸é, size = 6ÀÌ µÈ´Ù.
        // Indexes ¹è¿­¿¡´Â (0, 1, 2, 3, 4, 5, -1...)ÀÌ µé¾î°£´Ù.
        // ÀÌ Áß¿¡ 2¸¦ »Ì¾Ò´Ù°í °¡Á¤ÇÏÀÚ.
        // ±×·¯¸é ÀÌ ¹è¿­¿¡¼­ 2¸¦ Á¦°ÅÇØ Áà¾ß ÇÑ´Ù.
        // µÚ¿¡¼­ºÎÅÍ ¾ÕÀ¸·Î ÇÑÄ­¾¿ ¿Å°ÜÁà¾ß ÇÑ´Ù.
        // (0, 1, 3, 4, 5, 5...)
        // ±× ´ÙÀ½ »çÀÌÁî¸¦ ÁÙÀÌ°í, ´Ù½Ã ±× Áß¿¡¼­ ÇÏ³ª¸¦ ·£´ýÀ¸·Î
        // »Ì¾Æ°¡¸é °ãÄ¡Áö ¾Ê´Â Å©¸®ÃÄÀÇ ¸®½ºÆ®¸¦ ¾òÀ» ¼ö ÀÖ´Ù.
        std::vector<int> Indexes(creatureVector.size(), -1);
        int i;
        int size = creatureVector.size();
        for (i = 0; i < size; i++) {
            Indexes[i] = i;
        }

        for (i = 0; i < splash; i++) {
            int index = rand() % size;
            int realIndex = Indexes[index];
            creatureList.push_back(creatureVector[realIndex]);

            for (int m = index + 1; m < size; m++) {
                Indexes[m - 1] = Indexes[m];
            }

            size--;
        }
    }

    return (int)creatureList.size();
}

//----------------------------------------------------------------------
// Set Direction To Creature
//----------------------------------------------------------------------
// ´Ù¸¥ Creature¸¦ ÇâÇØ¼­ ¹Ù¶óº»´Ù.
//----------------------------------------------------------------------
// 8¹æÇâ¿¡ µû¸¥ ±âÁØÀÌ µÇ´Â ±â¿ï±â : °¡·Î/¼¼·Î ºñÀ²°ú °ü·Ã
//----------------------------------------------------------------------
const float BASIS_DIRECTION_LOW = 0.35f;

const float BASIS_DIRECTION_HIGH = 3.0f;

Dir_t getDirectionToPosition(int originX, int originY, int destX, int destY) {
    int stepX = destX - originX, stepY = destY - originY;

    // 0ÀÏ ¶§ check
    float k = (stepX == 0) ? 0 : (float)(stepY) / stepX; // ±â¿ï±â

    //--------------------------------------------------
    // ¹æÇâÀ» Á¤ÇØ¾ß ÇÑ´Ù.
    //--------------------------------------------------
    if (stepY == 0) {
        // XÃà
        // - -;;
        if (stepX == 0)
            return DOWN;
        else if (stepX > 0)
            return RIGHT;
        else
            return LEFT;
    } else if (stepY < 0) // UPÂÊÀ¸·Î
    {
        // yÃà À§
        if (stepX == 0) {
            return UP;
        }
        // 1»çºÐ¸é
        else if (stepX > 0) {
            if (k < -BASIS_DIRECTION_HIGH)
                return UP;
            else if (k <= -BASIS_DIRECTION_LOW)
                return RIGHTUP;
            else
                return RIGHT;
        }
        // 2»çºÐ¸é
        else {
            if (k > BASIS_DIRECTION_HIGH)
                return UP;
            else if (k >= BASIS_DIRECTION_LOW)
                return LEFTUP;
            else
                return LEFT;
        }
    }
    // ¾Æ·¡ÂÊ
    else {
        // yÃà ¾Æ·¡
        if (stepX == 0) {
            return DOWN;
        }
        // 4»çºÐ¸é
        else if (stepX > 0) {
            if (k > BASIS_DIRECTION_HIGH)
                return DOWN;
            else if (k >= BASIS_DIRECTION_LOW)
                return RIGHTDOWN;
            else
                return RIGHT;
        }
        // 3»çºÐ¸é
        else {
            if (k < -BASIS_DIRECTION_HIGH)
                return DOWN;
            else if (k <= -BASIS_DIRECTION_LOW)
                return LEFTDOWN;
            else
                return LEFT;
        }
    }
}

// Á¡°ú Á¡»çÀÌ¸¦ °É¾î¼­ °¥ ¼ö ÀÖ´Â°¡? ( Å©¸®ÃÄ·Î ¸·Èù °æ¿ì´Â Á¦¿Ü )
bool isPassLine(Zone* pZone, ZoneCoord_t sX, ZoneCoord_t sY, ZoneCoord_t eX, ZoneCoord_t eY, bool blockByCreature) {
    list<TPOINT> tpList;

    if (pZone == NULL)
        return false;

    // µÎ Á¡»çÀÌÀÇ Áø¼±À» ÀÌ·ç´Â Á¡µéÀ» ±¸ÇÑ´Ù.
    getLinePoint(sX, sY, eX, eY, tpList);

    if (tpList.empty())
        return false;

    VSRect rect(0, 0, pZone->getWidth() - 1, pZone->getHeight() - 1);

    list<TPOINT>::const_iterator itr = tpList.begin();
    TPOINT prev = (*itr);
    for (; itr != tpList.end(); ++itr) {
        TPOINT tp = (*itr);
        if (!rect.ptInRect(tp.x, tp.y))
            return false;

        if (tp.x == sX && tp.y == sY) {
            // ½ÃÀÛÁ¡Àº Ã¼Å© ¾ÈÇÑ´Ù.
            continue;
        }

        Tile& tile = pZone->getTile(tp.x, tp.y);

        if (blockByCreature) {
            if (tile.isGroundBlocked())
                return false;
        } else if (tile.isFixedGroundBlocked()) {
            return false;
        }

        // ´ë°¢¼±À¸·Î ¹Ù²ï °æ¿ì, ÇÑÂÊ ¹æÇâÀ¸·Î¸¸ °¥¼ö ÀÖ¾îµµ °¡´ÉÇÏ´Ù.
        // (1,1) -> (2,2) ÀÎ °æ¿ì, (1,2) ³ª (2,1) µÑ Áß¿¡ ÇÏ³ª¸¸ Áö³ª°¥ ¼ö ÀÖ¾îµµ Áö³ª°¥ ¼ö ÀÖ´Ù°í º»´Ù.
        if (prev.x != tp.x && prev.y != tp.y) {
            if (!rect.ptInRect(tp.x, prev.y))
                return false;
            if (!rect.ptInRect(prev.x, tp.y))
                return false;

            Tile& tile1 = pZone->getTile(tp.x, prev.y);
            Tile& tile2 = pZone->getTile(prev.x, tp.y);

            if (tile1.isFixedGroundBlocked() && tile2.isFixedGroundBlocked()) {
                return false;
            }
        }

        prev = tp;
    }

    return true;
}

// µÎ Á¡»çÀÌÀÇ Áø¼±À» ÀÌ·ç´Â Á¡µéÀ» ±¸ÇÑ´Ù.
void getLinePoint(ZoneCoord_t sX, ZoneCoord_t sY, ZoneCoord_t eX, ZoneCoord_t eY, list<TPOINT>& tpList) {
    int xLength = abs(sX - eX);
    int yLength = abs(sY - eY);

    if (xLength == 0 && yLength == 0)
        return;

    if (xLength > yLength) {
        if (sX > eX) {
            int tmpX = sX;
            sX = eX;
            eX = tmpX;
            int tmpY = sY;
            sY = eY;
            eY = tmpY;
        }

        float yStep = (float)(eY - sY) / (float)(eX - sX);

        for (int i = sX; i <= eX; i++) {
            TPOINT pt;
            pt.x = i;
            pt.y = sY + (int)(yStep * (float)(i - sX));

            tpList.push_back(pt);
        }
    } else {
        if (sY > eY) {
            int tmpX = sX;
            sX = eX;
            eX = tmpX;
            int tmpY = sY;
            sY = eY;
            eY = tmpY;
        }

        float xStep = (float)(eX - sX) / (float)(eY - sY);

        for (int i = sY; i <= eY; i++) {
            TPOINT pt;
            pt.x = sX + (int)(xStep * (float)(i - sY));
            pt.y = i;

            tpList.push_back(pt);
        }
    }
}
