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
// Returns the distance between (OX,OY) and (TX,TY).
//////////////////////////////////////////////////////////////////////////////
Range_t getDistance(ZoneCoord_t Ox, ZoneCoord_t Oy, ZoneCoord_t Tx, ZoneCoord_t Ty) {
    // Pure geometry, implemented in de-core.
    return decore::tileDistance(Ox, Oy, Tx, Ty);
}

//////////////////////////////////////////////////////////////////////////////
// Checks whether the distance is close enough to use the skill.
//////////////////////////////////////////////////////////////////////////////
bool verifyDistance(Creature* pCreature, ZoneCoord_t X, ZoneCoord_t Y, Range_t Dist) {
    Assert(pCreature != NULL);

    Zone* pZone = pCreature->getZone();
    Assert(pZone != NULL);

    ZoneCoord_t cx = pCreature->getX();
    ZoneCoord_t cy = pCreature->getY();

    ZoneLevel_t AttackerZoneLevel = pZone->getZoneLevel(cx, cy);

    // Skills cannot be used in Adam's holy land or in a safe area inside a PK zone.
    if ((AttackerZoneLevel & SAFE_ZONE) && (g_pPKZoneInfoManager->isPKZone(pZone->getZoneID()) || pZone->isHolyLand()))
        return false;

    // If the attacker is standing in a Slayer safe area,
    // anyone who is not a Slayer cannot use the skill.
    if ((AttackerZoneLevel & SLAYER_SAFE_ZONE) && !pCreature->isSlayer())
        return false;
    // If the attacker is standing in a Vampire safe area,
    // anyone who is not a Vampire cannot use the skill.
    else if ((AttackerZoneLevel & VAMPIRE_SAFE_ZONE) && !pCreature->isVampire())
        return false;
    // If the attacker is standing in an Ousters safe area,
    // anyone who is not an Ousters cannot use the skill.
    else if ((AttackerZoneLevel & OUSTERS_SAFE_ZONE) && !pCreature->isOusters())
        return false;
    // In a complete safe area nobody may use the skill.
    else if (AttackerZoneLevel & COMPLETE_SAFE_ZONE)
        return false;

    if ((abs(cx - X) <= Dist) && (abs(cy - Y) <= Dist))
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Checks whether the distance is close enough to use the skill.
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

    // Skills cannot be used in Adam's holy land or in a safe area inside a PK zone.
    if ((AttackerZoneLevel & SAFE_ZONE) && (g_pPKZoneInfoManager->isPKZone(pZone->getZoneID()) || pZone->isHolyLand()))
        return false;

    // If the attacker is standing in a Slayer safe area,
    // anyone who is not a Slayer cannot use the skill.
    if ((AttackerZoneLevel & SLAYER_SAFE_ZONE) && !pCreature->isSlayer())
        return false;
    // If the attacker is standing in a Vampire safe area,
    // anyone who is not a Vampire cannot use the skill.
    else if ((AttackerZoneLevel & VAMPIRE_SAFE_ZONE) && !pCreature->isVampire())
        return false;
    // If the attacker is standing in an Ousters safe area,
    // anyone who is not an Ousters cannot use the skill.
    else if ((AttackerZoneLevel & OUSTERS_SAFE_ZONE) && !pCreature->isOusters())
        return false;
    // In a complete safe area nobody may use the skill.
    else if (AttackerZoneLevel & COMPLETE_SAFE_ZONE)
        return false;

    // If the defender is standing in a Slayer safe area
    // and the defender is a Slayer, the skill does not hit.
    if ((DefenderZoneLevel & SLAYER_SAFE_ZONE) && pTargetCreature->isSlayer())
        return false;
    // If the defender is standing in a Vampire safe area
    // and the defender is a Vampire, the skill does not hit.
    else if ((DefenderZoneLevel & VAMPIRE_SAFE_ZONE) && pTargetCreature->isVampire())
        return false;
    // If the defender is standing in an Ousters safe area
    // and the defender is an Ousters, the skill does not hit.
    else if ((DefenderZoneLevel & OUSTERS_SAFE_ZONE) && pTargetCreature->isOusters())
        return false;
    // In a complete safe area the skill cannot be used.
    else if (DefenderZoneLevel & COMPLETE_SAFE_ZONE)
        return false;

    if ((abs(tx - ox) <= Dist) && (abs(ty - oy) <= Dist))
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////////
// Collects the creatures that take splash damage around the given coordinates.
//////////////////////////////////////////////////////////////////////////////
int getSplashVictims(Zone* pZone, int cx, int cy, Creature::CreatureClass CClass, list<Creature*>& creatureList,
                     int splash) {
    VSRect rect(0, 0, pZone->getWidth() - 1, pZone->getHeight() - 1);

    // If the creature class is Slayer, only that one Slayer is hit,
    // and the other Slayers around it are not.
    if (CClass == Creature::CREATURE_CLASS_SLAYER) {
        if (rect.ptInRect(cx, cy)) {
            Tile& rTile = pZone->getTile(cx, cy);

            if (rTile.hasCreature(Creature::MOVE_MODE_WALKING)) {
                Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_WALKING);
                if (pCreature->getCreatureClass() == CClass) {
                    creatureList.push_back(pCreature);
                }
            }
            // There are no flying Slayers at the moment, but...
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
                    // Add unconditionally when CREATURE_CLASS_MAX is passed as the parameter.
                    creatureVector.push_back(pCreature);
                } else if (pCreature->getCreatureClass() == CClass) {
                    // Otherwise add only creatures of the same CreatureClass.
                    creatureVector.push_back(pCreature);
                }
            }

            if (rTile.hasCreature(Creature::MOVE_MODE_FLYING)) {
                Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_FLYING);
                if (CClass == Creature::CREATURE_CLASS_MAX) {
                    // Add unconditionally when CREATURE_CLASS_MAX is passed as the parameter.
                    creatureVector.push_back(pCreature);
                } else if (pCreature->getCreatureClass() == CClass) {
                    // Otherwise add only creatures of the same CreatureClass.
                    creatureVector.push_back(pCreature);
                }
            }
        }
    }

    // If fewer creatures are present than the splash damage count,
    // all of them take splash damage.
    if ((int)creatureVector.size() <= splash) {
        for (int i = 0; i < (int)creatureVector.size(); i++) {
            creatureList.push_back(creatureVector[i]);
        }
    }
    // If more creatures are present than the splash damage count,
    // splash of them have to be picked at random.
    else {
        // Suppose there are 6 of them at first and 4 have to be picked;
        // then size = 6.
        // The Indexes array holds (0, 1, 2, 3, 4, 5, -1...).
        // Suppose 2 is picked out of them.
        // Then 2 has to be removed from the array,
        // shifting the entries after it one slot forward.
        // (0, 1, 3, 4, 5, 5...)
        // Shrinking the size and picking one of the rest at random again
        // yields a list of creatures with no duplicates.
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
// Finds the creatures around the given coordinates and hands them back.
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
                        // Add unconditionally when CREATURE_CLASS_MAX is passed as the parameter.
                        creatureVector.push_back(pCreature);
                    } else if (pCreature->getCreatureClass() == CClass) {
                        // Otherwise add only creatures of the same CreatureClass.
                        creatureVector.push_back(pCreature);
                    }
                }

                if (rTile.hasCreature(Creature::MOVE_MODE_FLYING)) {
                    Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_FLYING);
                    if (CClass == Creature::CREATURE_CLASS_MAX) {
                        // Add unconditionally when CREATURE_CLASS_MAX is passed as the parameter.
                        creatureVector.push_back(pCreature);
                    } else if (pCreature->getCreatureClass() == CClass) {
                        // Otherwise add only creatures of the same CreatureClass.
                        creatureVector.push_back(pCreature);
                    }
                }
            }
        }
    }

    // If fewer creatures are present than the splash damage count,
    // all of them take splash damage.
    if ((int)creatureVector.size() <= splash) {
        for (int i = 0; i < (int)creatureVector.size(); i++) {
            creatureList.push_back(creatureVector[i]);
        }
    }
    // If more creatures are present than the splash damage count,
    // splash of them have to be picked at random.
    else {
        // Suppose there are 6 of them at first and 4 have to be picked;
        // then size = 6.
        // The Indexes array holds (0, 1, 2, 3, 4, 5, -1...).
        // Suppose 2 is picked out of them.
        // Then 2 has to be removed from the array,
        // shifting the entries after it one slot forward.
        // (0, 1, 3, 4, 5, 5...)
        // Shrinking the size and picking one of the rest at random again
        // yields a list of creatures with no duplicates.
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
// Faces another Creature.
//----------------------------------------------------------------------
// Slope thresholds for the 8 directions: relates to the width/height ratio.
//----------------------------------------------------------------------
const float BASIS_DIRECTION_LOW = 0.35f;

const float BASIS_DIRECTION_HIGH = 3.0f;

Dir_t getDirectionToPosition(int originX, int originY, int destX, int destY) {
    int stepX = destX - originX, stepY = destY - originY;

    // Check for 0.
    float k = (stepX == 0) ? 0 : (float)(stepY) / stepX; // Slope

    //--------------------------------------------------
    // Decide the direction.
    //--------------------------------------------------
    if (stepY == 0) {
        // X axis
        // - -;;
        if (stepX == 0)
            return DOWN;
        else if (stepX > 0)
            return RIGHT;
        else
            return LEFT;
    } else if (stepY < 0) // Upward
    {
        // Up the y axis
        if (stepX == 0) {
            return UP;
        }
        // Quadrant 1
        else if (stepX > 0) {
            if (k < -BASIS_DIRECTION_HIGH)
                return UP;
            else if (k <= -BASIS_DIRECTION_LOW)
                return RIGHTUP;
            else
                return RIGHT;
        }
        // Quadrant 2
        else {
            if (k > BASIS_DIRECTION_HIGH)
                return UP;
            else if (k >= BASIS_DIRECTION_LOW)
                return LEFTUP;
            else
                return LEFT;
        }
    }
    // Downward
    else {
        // Down the y axis
        if (stepX == 0) {
            return DOWN;
        }
        // Quadrant 4
        else if (stepX > 0) {
            if (k > BASIS_DIRECTION_HIGH)
                return DOWN;
            else if (k >= BASIS_DIRECTION_LOW)
                return RIGHTDOWN;
            else
                return RIGHT;
        }
        // Quadrant 3
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

// Can one walk from point to point? (cases blocked by a creature excluded)
bool isPassLine(Zone* pZone, ZoneCoord_t sX, ZoneCoord_t sY, ZoneCoord_t eX, ZoneCoord_t eY, bool blockByCreature) {
    list<TPOINT> tpList;

    if (pZone == NULL)
        return false;

    // Finds the points that make up the straight line between the two points.
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
            // The starting point is not checked.
            continue;
        }

        Tile& tile = pZone->getTile(tp.x, tp.y);

        if (blockByCreature) {
            if (tile.isGroundBlocked())
                return false;
        } else if (tile.isFixedGroundBlocked()) {
            return false;
        }

        // On a diagonal step, being able to pass on just one side is enough.
        // For (1,1) -> (2,2), passing through either (1,2) or (2,1) counts as passable.
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

// Finds the points that make up the straight line between the two points.
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
