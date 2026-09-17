#include "WaitForApart.h"

#include <stdio.h>

#include "Assert.h"
#include "CoupleManager.h"
#include "FlagSet.h"
#include "GCCreateItem.h"
#include "GCDeleteInventoryItem.h"
#include "GCNPCResponse.h"
#include "GCRemoveFromGear.h"
#include "GCSystemMessage.h"
#include "ItemFactoryManager.h"
#include "ItemNameInfo.h"
#include "PacketUtil.h"
#include "Player.h"
#include "PlayerCreature.h"
#include "Slayer.h"
#include "StringPool.h"
#include "Vampire.h"
#include "item/CoupleRing.h"
#include "item/CoupleRingBase.h"
#include "item/VampireCoupleRing.h"

uint WaitForApart::waitPartner(PlayerCreature* pTargetPC) {
    __BEGIN_TRY

    if (pTargetPC == NULL)
        return COUPLE_MESSAGE_LOGOFF;

    PlayerCreature* pWaitingPC = getWaitingPC();
    if (pWaitingPC == NULL)
        return COUPLE_MESSAGE_LOGOFF;

    if (!g_pCoupleManager->isCouple(pTargetPC, pWaitingPC)) {
        return COUPLE_MESSAGE_NOT_COUPLE;
    }

    GCSystemMessage gcSystemMessage;
    //	StringStream msg;
    //	msg << pWaitingPC->getName() << " sent a parting request.";

    char msg[100];
    sprintf(msg, g_pStringPool->c_str(STRID_REQUEST_APART), pWaitingPC->getName().c_str());
    gcSystemMessage.setMessage(msg);

    pTargetPC->getPlayer()->sendPacket(&gcSystemMessage);

    return 0;

    __END_CATCH
}

uint WaitForApart::acceptPartner(PlayerCreature* pRequestedPC) {
    __BEGIN_TRY

    Assert(pRequestedPC != NULL);

    PlayerCreature* pWaitingPC = getWaitingPC();
    if (pWaitingPC == NULL)
        return COUPLE_MESSAGE_LOGOFF;

    if (!g_pCoupleManager->isCouple(pRequestedPC, pWaitingPC))
        return COUPLE_MESSAGE_NOT_COUPLE;

    if (!hasCoupleItem(pRequestedPC)) {
        filelog("CoupleRing.txt", "don't have coupleRing : %s", pRequestedPC->getName().c_str());
        return COUPLE_MESSAGE_NOT_COUPLE;
    }
    if (!hasCoupleItem(pWaitingPC)) {
        filelog("CoupleRing.txt", "don't have coupleRing : %s", pWaitingPC->getName().c_str());
        return COUPLE_MESSAGE_NOT_COUPLE;
    }

    Assert(hasCoupleItem(pRequestedPC));
    Assert(hasCoupleItem(pWaitingPC));

    // The couple ring has to be removed
    Assert(removeCoupleItem(pRequestedPC));
    Assert(removeCoupleItem(pWaitingPC));

    // Record the parting with the couple manager
    g_pCoupleManager->removeCouple(pRequestedPC, pWaitingPC);

    // The couple is broken, so give the Flag back.
    pRequestedPC->getFlagSet()->turnOff(FLAGSET_IS_COUPLE);
    pWaitingPC->getFlagSet()->turnOff(FLAGSET_IS_COUPLE);

    pRequestedPC->getFlagSet()->save(pRequestedPC->getName());
    pWaitingPC->getFlagSet()->save(pWaitingPC->getName());

    return 0;
    /*	Inventory* pRequestedPCInven = pRequestedPC->getInventory();
        Inventory* pWaitingPCInven = pWaitingPC->getInventory();

        Item::ItemClass IClass = Item::ITEM_CLASS_COUPLE_ITEM;
        Item* pRequestedPCCoupleItem = pRequestedPCInven->findItem(IClass , 0);
        Item* pWaitingPCCoupleItem = pWaitingPCInven->findItem(IClass , 0);

        if (pRequestedPCCoupleItem != NULL && pWaitingPCCoupleItem != NULL)
        {
            pRequestedPCInven->deleteItem(pRequestedPCCoupleItem->getObjectID());
            pWaitingPCInven->deleteItem(pWaitingPCCoupleItem->getObjectID());

            GCDeleteInventoryItem gcDeleteRequestedPCInventoryCoupleItem;
            GCDeleteInventoryItem gcDeleteWaitingPCInventoryCoupleItem;

            gcDeleteRequestedPCInventoryCoupleItem.setObjectID( pRequestedPCCoupleItem->getObjectID() );
            gcDeleteWaitingPCInventoryCoupleItem.setObjectID( pWaitingPCCoupleItem->getObjectID() );

            pRequestedPC->getPlayer()->sendPacket( &gcDeleteRequestedPCInventoryCoupleItem );
            pWaitingPC->getPlayer()->sendPacket( &gcDeleteWaitingPCInventoryCoupleItem );

            // The couple is broken, so the PlayerCreature's ItemNameInfo has to go as well
            // Just set it to NULL
            pRequestedPC->deleteItemNameInfoList( pRequestedPCCoupleItem->getObjectID() );
            pWaitingPC->deleteItemNameInfoList( pWaitingPCCoupleItem->getObjectID() );

            // Remove the item.
            pRequestedPCCoupleItem->destroy();
            SAFE_DELETE( pRequestedPCCoupleItem );
            pWaitingPCCoupleItem->destroy();
            SAFE_DELETE( pWaitingPCCoupleItem );

            // Because one has to go and find another man or woman.

            return true;
        }

        return false;
        */

    __END_CATCH
}
void WaitForApart::timeExpired() {
    __BEGIN_TRY

    // Report that the parting was refused.
    GCNPCResponse gcNPCResponse;
    gcNPCResponse.setCode(NPC_RESPONSE_APART_WAIT_TIME_EXPIRED);

    PlayerCreature* pWaitingPC = getWaitingPC();
    if (pWaitingPC != NULL)
        pWaitingPC->getPlayer()->sendPacket(&gcNPCResponse);

    __END_CATCH
}

bool WaitForApart::removeCoupleItem(PlayerCreature* pPC) {
    __BEGIN_TRY

    // Search the fourth finger first.
    if (pPC->isSlayer()) {
        Slayer* pSlayer = dynamic_cast<Slayer*>(pPC);
        Assert(pSlayer != NULL);

        for (Slayer::WearPart i = Slayer::WEAR_FINGER1; i <= Slayer::WEAR_FINGER4;
             i = static_cast<Slayer::WearPart>(static_cast<int>(i) + 1)) {
            Item* pRing = pSlayer->getWearItem(i);
            if (pRing != NULL) {
                if (isMatchCoupleRing(pPC, pRing)) {
                    //					pSlayer->deleteWearItem( i );
                    pSlayer->takeOffItem(i, false, true);
                    pRing->destroy();
                    SAFE_DELETE(pRing);

                    GCRemoveFromGear gcRemoveFromGear;
                    gcRemoveFromGear.setSlotID(i);

                    pPC->getPlayer()->sendPacket(&gcRemoveFromGear);

                    return true;
                }
            }
        }
    } else if (pPC->isVampire()) {
        Vampire* pVampire = dynamic_cast<Vampire*>(pPC);
        Assert(pVampire != NULL);

        for (Vampire::WearPart i = Vampire::WEAR_FINGER1; i <= Vampire::WEAR_FINGER4;
             i = static_cast<Vampire::WearPart>(static_cast<int>(i) + 1)) {
            Item* pRing = pVampire->getWearItem(i);
            if (pRing != NULL) {
                if (isMatchCoupleRing(pPC, pRing)) {
                    //					pVampire->deleteWearItem( i );
                    pVampire->takeOffItem(i, false, true);
                    pRing->destroy();
                    SAFE_DELETE(pRing);

                    GCRemoveFromGear gcRemoveFromGear;
                    gcRemoveFromGear.setSlotID(i);

                    pPC->getPlayer()->sendPacket(&gcRemoveFromGear);

                    return true;
                }
            }
        }
    } else
        Assert(false);

    // There is an item on the mouse.
    Item* pCoupleItem = pPC->getExtraInventorySlotItem();
    if (pCoupleItem == NULL || !isMatchCoupleRing(pPC, pCoupleItem)) {
        // With no item on the mouse, or one that is not a couple ring, search the inventory.
        pCoupleItem = pPC->getInventory()->findItem(getItemClass(pPC), getItemType(pPC));
        if (pCoupleItem != NULL)
            pPC->getInventory()->deleteItem(pCoupleItem->getObjectID());
    } else {
        pPC->deleteItemFromExtraInventorySlot();
    }

    if (pCoupleItem == NULL)
        return false;

    GCDeleteInventoryItem gcDeleteInventoryItem;
    gcDeleteInventoryItem.setObjectID(pCoupleItem->getObjectID());

    pPC->getPlayer()->sendPacket(&gcDeleteInventoryItem);

    pCoupleItem->destroy();
    SAFE_DELETE(pCoupleItem);

    return true;
    /*	Item* pPCCoupleItem = getCoupleItem( pPC );
        if ( pPCCoupleItem == NULL ) return false;

        pPC->getInventory()->deleteItem( pPCCoupleItem->getObjectID() );
        GCDeleteInventoryItem gcDeleteInventoryItem;
        gcDeleteInventoryItem.setObjectID( pPCCoupleItem->getObjectID() );

        pPC->getPlayer()->sendPacket( &gcDeleteInventoryItem );

    //	pPC->deleteItemNameInfoList( pPCCoupleItem->getObjectID() );

        pPCCoupleItem->destroy();
        SAFE_DELETE( pPCCoupleItem );

        return true;*/

    __END_CATCH
}

Item* WaitForApart::getCoupleItem(PlayerCreature* pPC) {
    if (pPC->isSlayer()) {
        Slayer* pSlayer = dynamic_cast<Slayer*>(pPC);
        Assert(pSlayer != NULL);

        for (Slayer::WearPart i = Slayer::WEAR_FINGER1; i <= Slayer::WEAR_FINGER4;
             i = static_cast<Slayer::WearPart>(static_cast<int>(i) + 1)) {
            Item* pRing = pSlayer->getWearItem(i);
            if (pRing != NULL) {
                if (isMatchCoupleRing(pPC, pRing)) {
                    return pRing;
                }
            }
        }
    } else if (pPC->isVampire()) {
        Vampire* pVampire = dynamic_cast<Vampire*>(pPC);
        Assert(pVampire != NULL);

        for (Vampire::WearPart i = Vampire::WEAR_FINGER1; i <= Vampire::WEAR_FINGER4;
             i = static_cast<Vampire::WearPart>(static_cast<int>(i) + 1)) {
            Item* pRing = pVampire->getWearItem(i);
            if (pRing != NULL) {
                if (isMatchCoupleRing(pPC, pRing)) {
                    return pRing;
                }
            }
        }
    } else
        Assert(false);

    return pPC->getInventory()->findItem(getItemClass(pPC), getItemType(pPC));
}
