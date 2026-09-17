//////////////////////////////////////////////////////////////////////////////
// Filename    : CGAddZoneToMouseHandler.cc
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGAddZoneToMouse.h"
#include "ItemInfoManager.h"

#ifdef __GAME_SERVER__
#include "CreatureUtil.h"
#include "EffectManager.h"
#include "EffectPrecedence.h"
#include "GamePlayer.h"
#include "Inventory.h"
#include "Item.h"
#include "ItemUtil.h"
#include "Relic.h"
#include "RelicUtil.h"
#include "Slayer.h"
#include "Timeval.h"
#include "Zone.h"
// #include "EffectCombatMessage2.h"
#include <stdio.h>

#include "Belt.h"
#include "BloodBible.h"
#include "CombatInfoManager.h"
#include "EffectHasSlayerRelic.h"
#include "EffectHasSweeper.h"
#include "EffectHasVampireRelic.h"
#include "EffectRelicPosition.h"
#include "GCAddEffect.h"
#include "GCCannotAdd.h"
#include "GCDeleteObject.h"
#include "GCDeleteandPickUpOK.h"
#include "GCSystemMessage.h"
#include "OustersArmsband.h"
#include "ShrineInfoManager.h"
#include "StringPool.h"
#include "Sweeper.h"
#include "ZoneGroupManager.h"
#include "ZoneUtil.h"
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGAddZoneToMouseHandler::execute(CGAddZoneToMouse* pPacket, Player* pPlayer) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    bool Success = false;

    try {
        GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);
        Creature* pCreature = pGamePlayer->getCreature();

        if (pCreature == NULL)
            return;
        PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);

        Zone* pZone = pPC->getZone();
        ZoneCoord_t ZoneX = pPacket->getZoneX();
        ZoneCoord_t ZoneY = pPacket->getZoneY();

        // Check that the bounds are not crossed.
        if (!isValidZoneCoord(pZone, ZoneX, ZoneY))
            goto ERROR;

        Tile& _Tile = pZone->getTile(ZoneX, ZoneY);


        // With no item on the tile there is of course nothing to add.
        if (!_Tile.hasItem())
            goto ERROR;

        // A null item pointer, or an item that cannot be picked up, cannot be taken.
        Item* pItem = _Tile.getItem();
        if (pItem == NULL || !isPortableItem(pItem))
            goto ERROR;
        if (!isAbleToPickupItem(pPC, pItem))
            goto ERROR;

        // For the blood bible, check whether it can be picked up. --> moved inside isAbleToPickupItem.

        ObjectID_t ItemObjectID = pItem->getObjectID();

        // Check that the item's ObjectID matches.
        if (ItemObjectID == pPacket->getObjectID()) {
            Item* pExtraItem = pPC->getExtraInventorySlotItem();

            // If something is already held, the item cannot be added.
            if (pExtraItem != NULL)
                goto ERROR;

            // An item carrying precedence can be picked up only by its owner or the owner's party.
            if (pItem->isFlag(Effect::EFFECT_CLASS_PRECEDENCE)) {
                Timeval currentTime;
                getCurrentTime(currentTime);

                EffectManager& rEffectManager = pItem->getEffectManager();

                EffectPrecedence* pEffectPrecedence =
                    dynamic_cast<EffectPrecedence*>(rEffectManager.findEffect(Effect::EFFECT_CLASS_PRECEDENCE));
                Assert(pEffectPrecedence != NULL);

                // A Relic can be picked up by anyone.
                if (isRelicItem(pItem) || pEffectPrecedence->getDeadline() < currentTime) {
                    // Once the time has passed anyone may pick it up. The effect is deleted here as well.
                    rEffectManager.deleteEffect(Effect::EFFECT_CLASS_PRECEDENCE);
                    pItem->removeFlag(Effect::EFFECT_CLASS_PRECEDENCE);
                } else {
                    // While the time has not passed, only the owner or the owner's party may pick it up.
                    if ((pEffectPrecedence->getHostName() == pPC->getName()) ||
                        (pPC->getPartyID() != 0 && pPC->getPartyID() == pEffectPrecedence->getHostPartyID())) {
                        // It can be picked up. Delete the effect.
                        rEffectManager.deleteEffect(Effect::EFFECT_CLASS_PRECEDENCE);
                        pItem->removeFlag(Effect::EFFECT_CLASS_PRECEDENCE);
                    } else {
                        // It cannot be picked up.
                        goto ERROR;
                    }
                }
            }

            /*
            #ifdef __XMAS_EVENT_CODE__
                        Inventory* pInventory = pPC->getInventory();
                        // If the item being picked up is a green gift box,
                        // it cannot be picked up while a green gift box is in the inventory.
                        if (pItem->getItemClass() == Item::ITEM_CLASS_EVENT_GIFT_BOX &&
                            pItem->getItemType() == 0 &&
                            pInventory->hasGreenGiftBox()) goto ERROR;
            #endif
            */
            pItem->whenPCTake(pPC);

            Item::ItemClass itemclass = pItem->getItemClass();
            // ItemType_t itemtype = pItem->getItemType();

            // For a relic, a relic kind already held cannot be held again.
            // If it can be held, attach the effect that says the relic is held and
            // set the owner value in the CombatInfoManager.
            if (isRelicItem(itemclass)) {
                addRelicEffect(pPC, pItem);

                deleteEffectRelicPosition(pItem);
            }

            // For a Flag, attach the Flag.
            if (pItem->isFlagItem()) {
                addSimpleCreatureEffect(pPC, Effect::EFFECT_CLASS_HAS_FLAG);
            }

            if (pItem->getItemClass() == Item::ITEM_CLASS_SWEEPER) {
                EffectHasSweeper* pEffect = new EffectHasSweeper(pPC);
                pEffect->setPart(pItem->getItemType());

                pPC->setFlag(pEffect->getEffectClass());
                pPC->addEffect(pEffect);
                //				addSimpleCreatureEffect( pPC, (Effect::EffectClass)(Effect::EFFECT_CLASS_HAS_SWEEPER +
                // pItem->getItemType()) );

                GCAddEffect gcAddEffect;
                gcAddEffect.setObjectID(pPC->getObjectID());
                gcAddEffect.setEffectID(pEffect->getSendEffectClass());

                pZone->broadcastPacket(pPC->getX(), pPC->getY(), &gcAddEffect);

                // Once picked up, broadcast a system message to the zone
                char race[15];
                if (pCreature->isSlayer()) {
                    sprintf(race, g_pStringPool->c_str(STRID_SLAYER));
                } else if (pCreature->isVampire()) {
                    sprintf(race, g_pStringPool->c_str(STRID_VAMPIRE));
                } else if (pCreature->isOusters()) {
                    sprintf(race, g_pStringPool->c_str(STRID_OUSTERS));
                } else {
                    Assert(false);
                }

                const SweeperInfo* pSweeperInfo = dynamic_cast<SweeperInfo*>(
                    g_pItemInfoManager->getItemInfo(Item::ITEM_CLASS_SWEEPER, pItem->getItemType()));

                char msg[100];
                sprintf(msg, g_pStringPool->c_str(STRID_PICK_UP_SWEEPER), pSweeperInfo->getName().c_str(),
                        pCreature->getName().c_str(), race);
                GCSystemMessage gcSystemMessage;
                gcSystemMessage.setMessage(msg);
                pZone->broadcastPacket(&gcSystemMessage);
            }

            pZone->deleteItem(pItem, ZoneX, ZoneY);
            pPC->addItemToExtraInventorySlot(pItem);

            // Tell the one who picked it up that it worked.
            GCDeleteandPickUpOK _GCDeleteandPickUpOK;
            GCDeleteObject _GCDeleteObject;
            _GCDeleteandPickUpOK.setObjectID(pItem->getObjectID());
            pPlayer->sendPacket(&_GCDeleteandPickUpOK);

            // Tell the others nearby that the item has disappeared.
            _GCDeleteObject.setObjectID(pItem->getObjectID());
            //			pZone->broadcastPacket(pPC->getX(), pPC->getY(), &_GCDeleteObject, pPC);
            //			pZone->broadcastPacket(ZoneX, ZoneY, &_GCDeleteObject, pPC);
            pZone->broadcastPacket(ZoneX, ZoneY, &_GCDeleteObject);

            Success = true;

            // Save the item.
            // pItem->save(pPC->getName(), STORAGE_EXTRASLOT, 0, 0, 0);
            // Item save optimization.
            char pField[80];
            sprintf(pField, "OwnerID='%s', Storage=%d", pPC->getName().c_str(), STORAGE_EXTRASLOT);
            pItem->tinysave(pField);

            // For a belt, ownership of the items inside it must transfer too.
            if (pItem->getItemClass() == Item::ITEM_CLASS_BELT) {
                sprintf(pField, "OwnerID='%s'", pPC->getName().c_str());

                Belt* pBelt = dynamic_cast<Belt*>(pItem);
                Assert(pBelt != NULL);

                Inventory* pBeltInventory = pBelt->getInventory();
                PocketNum_t num = pBelt->getPocketCount();

                for (SlotID_t count = 0; count < num; ++count) {
                    Item* pBeltItem = pBeltInventory->getItem(count, 0);
                    if (pBeltItem != NULL) {
                        pBeltItem->tinysave(pField);
                    }
                }
            }
            // For an armsband, ownership of the items inside it must transfer too.
            if (pItem->getItemClass() == Item::ITEM_CLASS_OUSTERS_ARMSBAND) {
                sprintf(pField, "OwnerID='%s'", pPC->getName().c_str());

                OustersArmsband* pOustersArmsband = dynamic_cast<OustersArmsband*>(pItem);
                Assert(pOustersArmsband != NULL);

                Inventory* pOustersArmsbandInventory = pOustersArmsband->getInventory();
                PocketNum_t num = pOustersArmsband->getPocketCount();

                for (SlotID_t count = 0; count < num; ++count) {
                    Item* pOustersArmsbandItem = pOustersArmsbandInventory->getItem(count, 0);
                    if (pOustersArmsbandItem != NULL) {
                        pOustersArmsbandItem->tinysave(pField);
                    }
                }
            }

            if (pItem->getItemClass() == Item::ITEM_CLASS_EVENT_ITEM && pItem->getItemType() == 30) {
                unsigned long timeLimit = 3600 * 24;

                pPC->addTimeLimitItem(pItem, timeLimit);
                pPC->sendTimeLimitItemInfo();
                pPC->setBaseLuck(10);
                pPC->initAllStatAndSend();
            }

        } else {
            goto ERROR;
        }

        if (pItem != NULL && pItem->isTraceItem()) {
            char zoneName[15];
            sprintf(zoneName, "%4d%3d%3d", pZone->getZoneID(), ZoneX, ZoneY);
            remainTraceLog(pItem, zoneName, pCreature->getName(), ITEM_LOG_MOVE, DETAIL_PICKUP);
        }
    } catch (Throwable& t) {
        // cerr << t.toString();
    }

ERROR:
    if (!Success) {
        GCCannotAdd _GCCannotAdd;
        _GCCannotAdd.setObjectID(pPacket->getObjectID());
        pPlayer->sendPacket(&_GCCannotAdd);
    }

#endif // __GAME_SERVER__

    __END_DEBUG_EX __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// This is the version with the event code in it.
// The same event may be run again, so do not delete this!
//////////////////////////////////////////////////////////////////////////////
/*
void CGAddZoneToMouseHandler::execute (CGAddZoneToMouse* pPacket , Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

    Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    try
    {
        GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);
        Creature*   pCreature   = pGamePlayer->getCreature();
        bool        Success     = false;

        if (pCreature == NULL) return;
        PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);

        Zone*       pZone   = pPC->getZone();
        ZoneCoord_t ZoneX   = pPacket->getZoneX();
        ZoneCoord_t ZoneY   = pPacket->getZoneY();

        // Check that the bounds are not crossed.
        if (!isValidZoneCoord(pZone, ZoneX, ZoneY)) goto ERROR;

        Tile& _Tile = pZone->getTile(ZoneX, ZoneY);

        // With no item on the tile there is of course nothing to add.
        if (!_Tile.hasItem())
        {
            GCCannotAdd _GCCannotAdd;
            _GCCannotAdd.setObjectID(pPacket->getObjectID());
            pPlayer->sendPacket(&_GCCannotAdd);
            return;
        }

        // A null item pointer of course cannot be added.
        Item* pItem = _Tile.getItem();
        if (pItem == NULL)
        {
            GCCannotAdd _GCCannotAdd;
            _GCCannotAdd.setObjectID(pPacket->getObjectID());
            pPlayer->sendPacket(&_GCCannotAdd);
            return;
        }

        ObjectID_t ItemObjectID = pItem->getObjectID();

        // Check that the item's ObjectID matches.
        if (ItemObjectID == pPacket->getObjectID())
        {
            Item* pExtraItem = pPC->getExtraInventorySlotItem();

            // If something is already held, the item cannot be added.
            if (pExtraItem != NULL)
            {
                GCCannotAdd _GCCannotAdd;
                _GCCannotAdd.setObjectID(pPacket->getObjectID());
                pPlayer->sendPacket(&_GCCannotAdd);
                return;
            }

            // If the item lying on the ground is an event skull...
            if (pItem->getItemClass() == Item::ITEM_CLASS_SKULL &&
                12 <= pItem->getItemType() && pItem->getItemType() <= 16)
            {
                uint scount = pGamePlayer->getSpecialEventCount();
                int  prev   = (int)(scount/10);
                int  cur    = 0;

                switch (pItem->getItemType())
                {
                    case 12: scount += 1; break; // golden skull
                    case 15: scount += 4; break; // crystal skull
                    case 14: scount += 9; break; // black skull
                    default: break;
                }

                // Save the count.
                pGamePlayer->setSpecialEventCount(scount);
                pGamePlayer->saveSpecialEventCount();
                cur = scount/10;

                // Delete the item from the zone.
                pZone->deleteItem(pItem, ZoneX, ZoneY);

                // Tell the one who picked it up that it worked.
                GCDeleteandPickUpOK _GCDeleteandPickUpOK;
                GCDeleteObject _GCDeleteObject;
                _GCDeleteandPickUpOK.setObjectID(pItem->getObjectID());
                pPlayer->sendPacket(&_GCDeleteandPickUpOK);
                // Tell the others nearby that the item has disappeared.
                _GCDeleteObject.setObjectID(pItem->getObjectID());
                pZone->broadcastPacket(pPC->getX(), pPC->getY(), &_GCDeleteObject, pPC);

                // Finally delete the actual item object.
                SAFE_DELETE(pItem);

                // Report the score at regular intervals.
                StringStream msg;
                msg << "Your current event points are " << pGamePlayer->getSpecialEventCount() << " points.";
                GCSystemMessage gcMsg;
                gcMsg.setMessage(msg.toString());
                pPlayer->sendPacket(&gcMsg);

                // Broadcast the score at regular intervals.
                if (prev != cur)
                {
                    StringStream msg;
                    msg << pPC->getName() << " has gained " << pGamePlayer->getSpecialEventCount() << " event
points."; GCSystemMessage gcMsg; gcMsg.setMessage(msg.toString()); pPlayer->sendPacket(&gcMsg);
                    pZone->broadcastPacket(pPC->getX(), pPC->getY(), &gcMsg , pPC);
                }

                return;
            }

            // Delete the item from the zone and hang it on the mouse.
            pZone->deleteItem(pItem, ZoneX, ZoneY);
            pPC->addItemToExtraInventorySlot(pItem);

            // Tell the one who picked it up that it worked.
            GCDeleteandPickUpOK _GCDeleteandPickUpOK;
            GCDeleteObject _GCDeleteObject;
            _GCDeleteandPickUpOK.setObjectID(pItem->getObjectID());
            pPlayer->sendPacket(&_GCDeleteandPickUpOK);

            // Tell the others nearby that the item has disappeared.
            _GCDeleteObject.setObjectID(pItem->getObjectID());
            pZone->broadcastPacket(pPC->getX(), pPC->getY(), &_GCDeleteObject, pPC);

            Success = true;

            // Save the item.
            pItem->save(pPC->getName(), STORAGE_EXTRASLOT, 0, 0, 0);
        }

        if (!Success)
        {
            GCCannotAdd _GCCannotAdd;
            _GCCannotAdd.setObjectID(pPacket->getObjectID());
            pPlayer->sendPacket(&_GCCannotAdd);
        }
    }
    catch (Throwable & t)
    {
        //cerr << t.toString();
    }

#endif	// __GAME_SERVER__

    __END_DEBUG_EX __END_CATCH

}
*/
