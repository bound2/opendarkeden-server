//////////////////////////////////////////////////////////////////////////////
// Filename    : CGAddItemToItemHandler.cc
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////
/*

// Take one item and put it into another item.
 [Client]                                               [Server]
           -- CGAddItemToItem
              (mouseItem_ObjectID, invenX, invenY)
              =(star held on the mouse, XY of the item to upgrade) -->

            <-- GCAddItemToItemVerify(result code, related value) ---

//-------------+---------------------------------------------
// used packet | client handling
//-------------+---------------------------------------------
// impossible  | print the impossible message
// fail        | print the fail message, delete the star, [remove the item]
// success     | delete the star, change the option
//-------------+---------------------------------------------
*/


#include "CGAddItemToItem.h"

#ifdef __GAME_SERVER__
#include <stdio.h>

#include "CreatureUtil.h"
#include "EventStar.h"
#include "GCAddItemToItemVerify.h"
#include "GCCreateItem.h"
#include "GCDeleteInventoryItem.h"
#include "GamePlayer.h"
#include "Inventory.h"
#include "Item.h"
#include "ItemInfoManager.h"
#include "ItemUtil.h"
#include "MixingItem.h"
#include "OptionInfo.h"
#include "PacketUtil.h"
#include "PetAttrInfo.h"
#include "PetEnchantItem.h"
#include "PetInfo.h"
#include "PetItem.h"
#include "PetTypeInfo.h"
#include "PlayerCreature.h"
#include "SystemAvailabilitiesManager.h"

void executeUpGrade(GamePlayer* pGamePlayer, Item* pMouseItem, Item* pItem);
void executeEnchantOption(GamePlayer* pGamePlayer, Item* pMouseItem, Item* pItem, CoordInven_t invenX,
                          CoordInven_t invenY);
void executeEnchantRareOption(GamePlayer* pGamePlayer, Item* pMouseItem, Item* pItem, CoordInven_t invenX,
                              CoordInven_t invenY);
void executeEnchantRareThreeOption(GamePlayer* pGamePlayer, Item* pMouseItem, Item* pItem, CoordInven_t invenX,
                                   CoordInven_t invenY);
void executeAddOption(GamePlayer* pGamePlayer, Item* pMouseItem, Item* pItem, OptionType_t addOptionType);
void executeTransKit(GamePlayer* pGamePlayer, Item* pMouseItem, Item* pItem);
extern bool isPossibleOptionItemClass(Item::ItemClass IClass);

OptionInfo* getOptionInfo(OptionType_t optionType);

void processUpgradeOptionType(Item* pItem, OptionType_t currentOptionType, OptionInfo* pOptionInfo);
void crashItem(Item* pItem, Inventory* pInventory, CoordInven_t invenX, CoordInven_t invenY);
void downgradeOptionType(Item* pItem, OptionType_t currentOptionType, OptionInfo* pOptionInfo);

void sendEnchantImpossible(GamePlayer* pGamePlayer);
void sendEnchantOK(GamePlayer* pGamePlayer, DWORD optionChange);
// Packet that returns the three-option enchant result
void sendEnchantOK(GamePlayer* pGamePlayer, DWORD optionChange, DWORD optionCHange2);

#endif // __GAME_SERVER__

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CGAddItemToItemHandler::execute(CGAddItemToItem* pPacket, Player* pPlayer) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    // cout << pPacket->toString().c_str() << endl;

    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);

    Assert(pGamePlayer != NULL); // by sigi

    if (pGamePlayer->getPlayerStatus() == GPS_NORMAL) {
        SYSTEM_ASSERT(SYSTEM_ENCHANT);

        Creature* pCreature = pGamePlayer->getCreature();
        PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
        ObjectID_t objectID = pPacket->getObjectID();
        Inventory* pInventory = pPC->getInventory();
        CoordInven_t invenX = pPacket->getX();
        CoordInven_t invenY = pPacket->getY();

        // The inventory has no item there (this checks the coordinates too).
        if (!pInventory->hasItem(invenX, invenY)) {
            GCAddItemToItemVerify gcAddItemToItemVerify;
            gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ERROR);
            pGamePlayer->sendPacket(&gcAddItemToItemVerify);
            return;
        }

        // Mouse item
        Item* pMouseItem = pPC->getExtraInventorySlotItem();

        // Item in the inventory
        Item* pItem = pInventory->getItem(invenX, invenY);

        // No item, or
        // a wrong objectID
        if (pMouseItem == NULL || pItem == NULL || pMouseItem->getObjectID() != objectID || pMouseItem->getNum() != 1) {
            // cout << "Wrong Item" << endl;

            GCAddItemToItemVerify gcAddItemToItemVerify;
            gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ERROR);
            pGamePlayer->sendPacket(&gcAddItemToItemVerify);
            return;
        }


        switch (pMouseItem->getItemClass()) {
        //---------------------------------------------------------
        //
        // 				ITEM_CLASS_EVENT_STAR
        //
        //---------------------------------------------------------
        // Upgrade the item's option.
        //---------------------------------------------------------
        case Item::ITEM_CLASS_EVENT_STAR: {
            ItemInfo* pItemInfo =
                g_pItemInfoManager->getItemInfo(Item::ITEM_CLASS_EVENT_STAR, pMouseItem->getItemType());
            Assert(pItemInfo != NULL);

            EventStarInfo* pEventStarInfo = dynamic_cast<EventStarInfo*>(pItemInfo);
            Assert(pEventStarInfo != NULL);

            int optionSize = pItem->getOptionTypeSize();
            // Upgrade the equipment level
            if ((pEventStarInfo->getFunctionFlag() & EventStarInfo::FUNCTION_UP_GRADE) != 0) {
                executeUpGrade(pGamePlayer, pMouseItem, pItem);
            } else if (pEventStarInfo->isFunctionTransKit()) {
                executeTransKit(pGamePlayer, pMouseItem, pItem);
            }
            // Enchant Option
            else if (optionSize > 1) // by sigi. 2002.10.21
            {
                if (pEventStarInfo->isFunctionEnchantRareOption()) {
                    if (pMouseItem->getItemType() == 6) // three-option enchant
                        executeEnchantRareThreeOption(pGamePlayer, pMouseItem, pItem, invenX, invenY);
                    else
                        executeEnchantRareOption(pGamePlayer, pMouseItem, pItem, invenX, invenY);
                } else {
                    GCAddItemToItemVerify gcAddItemToItemVerify;
                    gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_IMPOSSIBLE);
                    pGamePlayer->sendPacket(&gcAddItemToItemVerify);
                    return;
                }
            } else if (pEventStarInfo->isFunctionEnchantOption()) {
                executeEnchantOption(pGamePlayer, pMouseItem, pItem, invenX, invenY);
            }
            // Add Option
            else if (pEventStarInfo->isFunctionAddOption()) {
                int addOptionType = pEventStarInfo->getFunctionValue();
                executeAddOption(pGamePlayer, pMouseItem, pItem, addOptionType);
            }
        } break;

        case Item::ITEM_CLASS_PET_ENCHANT_ITEM: {
            // cout << "Trying Pet Enchant... " << endl;
            if (pItem->getItemClass() != Item::ITEM_CLASS_PET_ITEM) {
                // cout << "It was not done on a pet item." << endl;
                GCAddItemToItemVerify gcAddItemToItemVerify;
                gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ERROR);
                pGamePlayer->sendPacket(&gcAddItemToItemVerify);
                return;
            }

            PetEnchantItemInfo* pItemInfo = dynamic_cast<PetEnchantItemInfo*>(
                g_pItemInfoManager->getItemInfo(Item::ITEM_CLASS_PET_ENCHANT_ITEM, pMouseItem->getItemType()));
            Assert(pItemInfo != NULL);

            PetItem* pPetItem = dynamic_cast<PetItem*>(pItem);
            PetInfo* pPetInfo = NULL;
            if (pPetItem == NULL || (pPetInfo = pPetItem->getPetInfo()) == NULL) {
                // cout << "Not a pet item, or the pet item carries no pet state." << endl;
                GCAddItemToItemVerify gcAddItemToItemVerify;
                gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ERROR);
                pGamePlayer->sendPacket(&gcAddItemToItemVerify);
                return;
            }

            /*
                ENCHANT_FUNCTION = 0,
                SPEC_OPTION_FUNCTION = 1,
                REVIVAL_FUNCTION = 13,
                RARE_FUNCTION = 14
            */
            switch (pItemInfo->getFunction()) {
            case PetEnchantItemInfo::ENCHANT_FUNCTION: {
                if (pPetInfo->getPetAttr() != 0xff) {
                    // cout << "The item is already enchanted." << endl;
                    GCAddItemToItemVerify gcAddItemToItemVerify;
                    gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ERROR);
                    pGamePlayer->sendPacket(&gcAddItemToItemVerify);
                    return;
                }
                // cout << "A standard enchant is in progress." << endl;
                pPC->deleteItemFromExtraInventorySlot();
                pMouseItem->destroy();
                SAFE_DELETE(pMouseItem);

                if (PetAttrInfoManager::Instance().enchantRandomAttr(pPetInfo, pItemInfo->getFunctionGrade())) {
                    // cout << "Enchant succeeded." << endl;
                    pPetItem->savePetInfo();

                    GCAddItemToItemVerify gcAddItemToItemVerify;
                    gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_OK);
                    gcAddItemToItemVerify.setParameter(((uint)pPetInfo->getPetAttr()) << 16 |
                                                       (uint)pPetInfo->getPetAttrLevel());

                    pGamePlayer->sendPacket(&gcAddItemToItemVerify);

                    if (pPC->getPetInfo() == pPetInfo) {
                        // cout << "Already summoned, so the packet is being sent." << endl;
                        sendPetInfo(pGamePlayer, true);
                        pPC->initAllStatAndSend();
                    }
                    return;
                } else {
                    // cout << "Enchant failed" << endl;
                    GCAddItemToItemVerify gcAddItemToItemVerify;
                    gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_FAIL);
                    pGamePlayer->sendPacket(&gcAddItemToItemVerify);
                    return;
                }
            } break;
            case PetEnchantItemInfo::SPEC_OPTION_FUNCTION: {
                if (pPetInfo->getPetAttr() != 0xff) {
                    // cout << "The item is already enchanted." << endl;
                    GCAddItemToItemVerify gcAddItemToItemVerify;
                    gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ERROR);
                    pGamePlayer->sendPacket(&gcAddItemToItemVerify);
                    return;
                }
                pPC->deleteItemFromExtraInventorySlot();
                pMouseItem->destroy();
                SAFE_DELETE(pMouseItem);

                if (PetAttrInfoManager::Instance().enchantSpecAttr(pPetInfo,
                                                                   (PetAttr_t)pItemInfo->getFunctionGrade())) {
                    pPetItem->savePetInfo();
                    GCAddItemToItemVerify gcAddItemToItemVerify;
                    gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_OK);
                    gcAddItemToItemVerify.setParameter(((uint)pPetInfo->getPetAttr()) << 16 |
                                                       (uint)pPetInfo->getPetAttrLevel());
                    //							gcAddItemToItemVerify.setParameter( pPetInfo->getPetAttr() );

                    pGamePlayer->sendPacket(&gcAddItemToItemVerify);

                    if (pPC->getPetInfo() == pPetInfo) {
                        sendPetInfo(pGamePlayer, true);
                        pPC->initAllStatAndSend();
                    }
                    return;
                } else {
                    GCAddItemToItemVerify gcAddItemToItemVerify;
                    gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_FAIL);
                    pGamePlayer->sendPacket(&gcAddItemToItemVerify);
                    return;
                }
            } break;
            case PetEnchantItemInfo::RARE_FUNCTION: {
                // cout << "A pet rare enchant is in progress." << endl;
                if (pPetInfo->getPetLevel() != 49) {
                    // cout << "Do not enchant one that is not at level" << endl;
                    GCAddItemToItemVerify gcAddItemToItemVerify;
                    gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ERROR);
                    pGamePlayer->sendPacket(&gcAddItemToItemVerify);
                    return;
                }

                pPC->deleteItemFromExtraInventorySlot();
                pMouseItem->destroy();
                SAFE_DELETE(pMouseItem);

                /*
                OptionType_t targetOption = rand()%87;
                targetOption++;
                */

                OptionType_t targetOption = 0;
                int ratio = rand() % 100000;
                const list<PetEnchantOption*>& petEnchantOptions = g_pOptionInfoManager->getPetEnchantOptionList();
                list<PetEnchantOption*>::const_iterator itr = petEnchantOptions.begin();
                list<PetEnchantOption*>::const_iterator endItr = petEnchantOptions.end();

                for (; itr != endItr; itr++) {
                    PetEnchantOption* pPetEnchantOption = *itr;
                    if (pPetEnchantOption->getRatio() > ratio) {
                        targetOption = pPetEnchantOption->getOptionType();
                        break;
                    }
                }

                pPetInfo->setPetOption(targetOption);
                sendPetInfo(pGamePlayer);

                GCAddItemToItemVerify gcAddItemToItemVerify;
                gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_OK);
                gcAddItemToItemVerify.setParameter(targetOption);
                pGamePlayer->sendPacket(&gcAddItemToItemVerify);

                char query[100];
                sprintf(query, "PetOption=%u", (uint)targetOption);
                pItem->tinysave(query);
            } break;
            case PetEnchantItemInfo::REVIVAL_FUNCTION: {
                // cout << "A revive is being attempted."  << endl;
                if (pPetInfo->getPetHP() != 0) {
                    // cout << "Do not revive one that is not dead" << endl;
                    GCAddItemToItemVerify gcAddItemToItemVerify;
                    gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ERROR);
                    pGamePlayer->sendPacket(&gcAddItemToItemVerify);
                    return;
                }

                pPC->deleteItemFromExtraInventorySlot();
                pMouseItem->destroy();
                SAFE_DELETE(pMouseItem);

                VSDateTime current = VSDateTime::currentDateTime();
                int Days = pPetInfo->getLastFeedTime().daysTo(current);
                if (Days < 0)
                    Days = 0;

                if (pItemInfo->getItemType() != 14) {
                    int value = rand() % 100;
                    if (Days >= 8 || value >= (10 - Days / 2) * 10) {
                        GCAddItemToItemVerify gcAddItemToItemVerify;
                        gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_FAIL);
                        pGamePlayer->sendPacket(&gcAddItemToItemVerify);
                        return;
                    }
                }

                pPetInfo->setPetHP(pItemInfo->getFunctionGrade());
                pPetInfo->setFeedTime(VSDateTime::currentDateTime());
                pPetInfo->getPetItem()->savePetInfo();

                GCAddItemToItemVerify gcAddItemToItemVerify;
                gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_REVIVAL_OK);
                gcAddItemToItemVerify.setParameter(pPetInfo->getPetHP());

                pGamePlayer->sendPacket(&gcAddItemToItemVerify);
            } break;
            case PetEnchantItemInfo::MUTANT_FUNCTION: {
                if (pPetInfo->getPetType() != 1) {
                    cout << "¿ïÇÁµ¶ÀÌ ¾Æ´Ï¶ø´Ï´Ù." << endl;
                    GCAddItemToItemVerify gcAddItemToItemVerify;
                    gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ERROR);
                    pGamePlayer->sendPacket(&gcAddItemToItemVerify);
                    return;
                }

                bool summonPet = pPC->getPetInfo() == pPetInfo;

                pPC->deleteItemFromExtraInventorySlot();
                pMouseItem->destroy();
                SAFE_DELETE(pMouseItem);

                PetTypeInfo* pPetTypeInfo = PetTypeInfoManager::getInstance()->getPetTypeInfo(2);
                Assert(pPetTypeInfo != NULL);

                pPetInfo->setPetType(2);
                pPetInfo->setPetCreatureType(pPetTypeInfo->getPetCreatureType(pPetInfo->getPetLevel()));
                pPetItem->setItemType(2);

                pPetItem->tinysave("ItemType=2");

                //						if ( pPC->getPetInfo() == pPetInfo ) sendPetInfo( pGamePlayer );
                if (summonPet) {
                    pPC->setPetInfo(NULL);
                    sendPetInfo(pGamePlayer);
                }

                GCAddItemToItemVerify gcAddItemToItemVerify;
                gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_OK);
                gcAddItemToItemVerify.setParameter(0);

                pGamePlayer->sendPacket(&gcAddItemToItemVerify);

                GCDeleteInventoryItem gcDI;
                gcDI.setObjectID(pPetItem->getObjectID());
                pGamePlayer->sendPacket(&gcDI);

                CoordInven_t X = -1, Y;
                pPC->getInventory()->findItemOID(pPetItem->getObjectID(), X, Y);
                Assert(X != -1);

                GCCreateItem gcCI;
                makeGCCreateItem(&gcCI, pPetItem, X, Y);
                pGamePlayer->sendPacket(&gcCI);

                if (summonPet) {
                    pPC->setPetInfo(pPetInfo);
                    sendPetInfo(pGamePlayer);
                }
            } break;
            default: {
                GCAddItemToItemVerify gcAddItemToItemVerify;
                gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ERROR);
                pGamePlayer->sendPacket(&gcAddItemToItemVerify);
                return;
            }
            }
        } break;

        case Item::ITEM_CLASS_MIXING_ITEM: {
            if (pItem->getItemClass() != Item::ITEM_CLASS_PET_ITEM) {
                // cout << "It was not done on a pet item." << endl;
                GCAddItemToItemVerify gcAddItemToItemVerify;
                gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ERROR);
                pGamePlayer->sendPacket(&gcAddItemToItemVerify);
                return;
            }

            MixingItemInfo* pItemInfo = dynamic_cast<MixingItemInfo*>(
                g_pItemInfoManager->getItemInfo(Item::ITEM_CLASS_MIXING_ITEM, pMouseItem->getItemType()));
            Assert(pItemInfo != NULL);

            if (pItemInfo->getType() != MixingItemInfo::TYPE_DETACH) {
                // cout << "It was not done on a pet item." << endl;
                GCAddItemToItemVerify gcAddItemToItemVerify;
                gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ERROR);
                pGamePlayer->sendPacket(&gcAddItemToItemVerify);
                return;
            }

            PetItem* pPetItem = dynamic_cast<PetItem*>(pItem);
            PetInfo* pPetInfo = NULL;
            if (pPetItem == NULL || (pPetInfo = pPetItem->getPetInfo()) == NULL || pPetInfo->getPetOption() == 0) {
                // cout << "Not a pet item, or the pet item carries no pet state." << endl;
                GCAddItemToItemVerify gcAddItemToItemVerify;
                gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ERROR);
                pGamePlayer->sendPacket(&gcAddItemToItemVerify);
                return;
            }

            pPC->deleteItemFromExtraInventorySlot();
            pMouseItem->destroy();
            SAFE_DELETE(pMouseItem);

            pPetInfo->setPetOption(0);
            pPetItem->tinysave("PetOption=0");

            GCAddItemToItemVerify gcAddItemToItemVerify;
            gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_OK);
            gcAddItemToItemVerify.setParameter(0);

            pGamePlayer->sendPacket(&gcAddItemToItemVerify);

            if (pPC->getPetInfo() == pPetInfo) {
                // cout << "Already summoned, so the packet is being sent." << endl;
                sendPetInfo(pGamePlayer, true);
                pPC->initAllStatAndSend();
            }
            return;
        } break;

        default: {
            // cout << "upgrade error" << endl;

            GCAddItemToItemVerify gcAddItemToItemVerify;
            gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ERROR);
            pGamePlayer->sendPacket(&gcAddItemToItemVerify);
            return;
        }
        }
    }

#endif // __GAME_SERVER__

    __END_DEBUG_EX __END_CATCH
}


#ifdef __GAME_SERVER__
void executeUpGrade(GamePlayer* pGamePlayer, Item* pMouseItem, Item* pItem) {
    __BEGIN_TRY

    Creature* pCreature = pGamePlayer->getCreature();
    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
    Inventory* pInventory = pPC->getInventory();

    const ItemInfo* pItemInfo = g_pItemInfoManager->getItemInfo(pItem->getItemClass(), pItem->getItemType());

    int Ratio = 50;

    if (pPC->getLevel() >= 100 && pMouseItem->getNum() == 1 && pItem->getGrade() < 10 && rand() % 100 < Ratio) {
        pItem->setGrade(pItem->getGrade() + 1);
        char buffer[80];
        snprintf(buffer, 80, "Grade=%u", pItem->getGrade());
        pItem->tinysave(buffer);

        pPC->deleteItemFromExtraInventorySlot();
        pMouseItem->destroy();
        SAFE_DELETE(pMouseItem);

        GCAddItemToItemVerify gcAddItemToItemVerify;
        gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_UP_GRADE_OK);
        gcAddItemToItemVerify.setParameter(pItem->getGrade());
        pGamePlayer->sendPacket(&gcAddItemToItemVerify);
    } else {
        GCAddItemToItemVerify gcAddItemToItemVerify;
        gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_FAIL);
        pGamePlayer->sendPacket(&gcAddItemToItemVerify);
        return;
    }

    __END_CATCH
}
void executeEnchantRareThreeOption(GamePlayer* pGamePlayer, Item* pMouseItem, Item* pItem, CoordInven_t invenX,
                                   CoordInven_t invenY) {
    __BEGIN_TRY

    Creature* pCreature = pGamePlayer->getCreature();
    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
    //		Inventory* 		pInventory  = pPC->getInventory();

    // The failure chance is decided by itemType.
    const ItemInfo* pItemInfo = g_pItemInfoManager->getItemInfo(pItem->getItemClass(), pItem->getItemType());

    // The item to enchant does not have exactly two options,
    // is a unique item,
    // or two or more stars are held for the enchant
    if (pItem->getOptionTypeSize() != 3 || pItemInfo->isUnique() || pMouseItem->getNum() != 1) {
        // cout << "no Option " << endl;
        sendEnchantImpossible(pGamePlayer);
        return;
    }

    bool bFirstSucceed = false;
    uint optionChange = 0;
    uint optionChange2 = 0;
    int shiftValue = 16;
    int diceValue = 3500;

    list<OptionType_t> optionList = pItem->getOptionTypeList();
    list<OptionType_t>::const_iterator itr = optionList.begin();

    OptionType_t thirdOption = *itr;
    itr++;
    OptionType_t secondOption = *itr;
    itr++;
    OptionType_t firstOption = *itr;
    itr--;
    itr--;

    /*
    OptionType_t thirdOption  = *itr++;
    OptionType_t secondOption = *itr;
    OptionType_t firstOption  = *itr++;
    */

    OptionInfo* pFirstOptionInfo = getOptionInfo(firstOption);
    OptionInfo* pSecondOptionInfo = getOptionInfo(secondOption);
    OptionInfo* pThirdOptionInfo = getOptionInfo(thirdOption);
    //		cout << "firstOption : " << pFirstOptionInfo->getName() <<  endl;
    //		cout << "secondOption : " << pSecondOptionInfo->getName() <<  endl;
    //		cout << "thirdOption : " << pThirdOptionInfo->getName() <<  endl;
    // Check the item options for the upgrade
    if (pFirstOptionInfo == NULL || pSecondOptionInfo == NULL || pThirdOptionInfo == NULL) {
        sendEnchantImpossible(pGamePlayer);
        return;
    }
    // Test whether the item can be upgraded
    bool bFirstUpgradePossible = pFirstOptionInfo->isUpgradePossible();
    bool bSecondUpgradePossible = pSecondOptionInfo->isUpgradePossible();
    bool bThirdUpgradePossible = pThirdOptionInfo->isUpgradePossible();
    // Test whether an option upgrade is possible
    if (!bFirstUpgradePossible && !bSecondUpgradePossible && !bThirdUpgradePossible) {
        sendEnchantImpossible(pGamePlayer);
        return;
    }
    // if ((rand() %6000) > diceValue)
    // diceValue*=2;
    if (bFirstUpgradePossible) {
        // The first option's enchant chance follows the Blue Drop 2 chance.
        int succeedRatio = pFirstOptionInfo->getUpgradeSecondRatio() * (pItemInfo->getUpgradeRatio() + (rand() % 50));
        // int dice = rand()%10000;
        int dice = rand() % diceValue;
        cout << "dice : " << (int)dice << endl;
        cout << "diceValue : " << (int)diceValue << endl;

        // cout << "First option: " << pFirstOptionInfo->getHName() << " enchant chance " << succeedRatio << endl;
        // dice =succeedRatio-1;
        if (dice < succeedRatio) // on success, upgrade the equipment
        {
            // cout << "Enchant succeeded" << endl;
            processUpgradeOptionType(pItem, firstOption, pFirstOptionInfo);

            OptionType_t upgradeOptionType = pFirstOptionInfo->getUpgradeType();
            optionChange = (firstOption << (shiftValue + 8)) | (upgradeOptionType << shiftValue);

            bFirstSucceed = true;
            goto EnOK;
        } else {
            if (pFirstOptionInfo->getUpgradeSecondRatio() < 5) {
                int noChangeRatio = 33;
                int downgradeRatio = 46;
                dice = rand() % 100;

                cout << "downgradeRatio" << pItemInfo->getName() << " ¿É¼Ç¶³¾îÁú È®·ü " << downgradeRatio << endl;

                if (dice < noChangeRatio) {
                    // No change
                } else if (dice < noChangeRatio + downgradeRatio) {
                    // Option dropped
                    downgradeOptionType(pItem, firstOption, pFirstOptionInfo);

                    OptionType_t previousOptionType = pFirstOptionInfo->getPreviousType();
                    optionChange = (firstOption << (shiftValue + 8)) | (previousOptionType << shiftValue);
                }
            }
        }
    }
    shiftValue -= 16;

    if (bSecondUpgradePossible) {
        // The second option's enchant chance depends on whether the first option succeeded.

        int succeedRatio = g_pOptionInfoManager->getRareUpgradeRatio(secondOption, bFirstSucceed) *
                           (pItemInfo->getUpgradeRatio() + (rand() % 50));

        // int dice = rand() %10000;
        int dice = rand() % diceValue;

        // cout << "Second option: " << pSecondOptionInfo->getHName() << " enchant chance " << succeedRatio << endl;
        // dice =succeedRatio-1;
        if (dice < succeedRatio) {
            // cout << "Enchant succeeded" << endl;
            processUpgradeOptionType(pItem, secondOption, pSecondOptionInfo);

            OptionType_t upgradeOptionType = pSecondOptionInfo->getUpgradeType();
            optionChange |= (secondOption << (shiftValue + 8)) | (upgradeOptionType << shiftValue);

            bFirstSucceed = true;
            goto EnOK;
        } else {
            if (pSecondOptionInfo->getUpgradeSecondRatio() < 5) {
                int noChangeRatio = 33;
                int downgradeRatio = 46;
                dice = rand() % 100;

                cout << "downgradeRatio" << pItemInfo->getName() << " ¿É¼Ç¶³¾îÁú È®·ü " << downgradeRatio << endl;

                if (dice < noChangeRatio) {
                    // No change
                } else if (dice < noChangeRatio + downgradeRatio) {
                    // Option dropped
                    downgradeOptionType(pItem, secondOption, pSecondOptionInfo);

                    OptionType_t previousOptionType = pSecondOptionInfo->getPreviousType();
                    optionChange = (secondOption << (shiftValue + 8)) | (previousOptionType << shiftValue);
                }
            }
        }
    }
    shiftValue = 0;
    if (bThirdUpgradePossible) {
        // The second option's enchant chance depends on whether the first option succeeded.
        int succeedRatio = g_pOptionInfoManager->getRareUpgradeRatio(thirdOption, bFirstSucceed) *
                           (pItemInfo->getUpgradeRatio() + (rand() % 50));
        // int dice = rand() %10000;

        int dice = rand() % diceValue;
        // cout << "Second option: " << pSecondOptionInfo->getHName() << " enchant chance " << succeedRatio << endl;
        // dice =succeedRatio-1;
        if (dice < succeedRatio) {
            // cout << "Enchant succeeded" << endl;
            processUpgradeOptionType(pItem, thirdOption, pThirdOptionInfo);

            OptionType_t upgradeOptionType = pThirdOptionInfo->getUpgradeType();
            optionChange2 |= (thirdOption << (shiftValue + 8)) | (upgradeOptionType << shiftValue);

            bFirstSucceed = true;
        } else {
            if (pThirdOptionInfo->getUpgradeSecondRatio() < 5) {
                int noChangeRatio = 33;
                int downgradeRatio = 46;
                dice = rand() % 100;

                cout << "downgradeRatio" << pItemInfo->getName() << " ¿É¼Ç¶³¾îÁú È®·ü " << downgradeRatio << endl;

                if (dice < noChangeRatio) {
                    // No change
                } else if (dice < noChangeRatio + downgradeRatio) {
                    // Option dropped
                    downgradeOptionType(pItem, thirdOption, pThirdOptionInfo);

                    OptionType_t previousOptionType = pThirdOptionInfo->getPreviousType();
                    optionChange2 |= (thirdOption << (shiftValue + 8)) | (previousOptionType << shiftValue);
                }
            }
        }
    }
EnOK:
    //{
    // Delete the held star
    pPC->deleteItemFromExtraInventorySlot();

    // Delete from the DB
    pMouseItem->destroy();

    // Delete from memory
    SAFE_DELETE(pMouseItem);

    // Send the result packet
    cout << "optionChange : " << optionChange << endl;
    sendEnchantOK(pGamePlayer, optionChange, optionChange2);

    //}

    __END_CATCH
}
void executeEnchantRareOption(GamePlayer* pGamePlayer, Item* pMouseItem, Item* pItem, CoordInven_t invenX,
                              CoordInven_t invenY) {
    __BEGIN_TRY

    Creature* pCreature = pGamePlayer->getCreature();
    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
    //		Inventory* 		pInventory  = pPC->getInventory();

    // The failure chance is decided by itemType.
    const ItemInfo* pItemInfo = g_pItemInfoManager->getItemInfo(pItem->getItemClass(), pItem->getItemType());

    // The item to enchant does not have exactly two options,
    // is a unique item,
    // or two or more stars are held for the enchant
    /*		// Decide whether the item has three options; if so, call the new three-option routine
            if (pItem->getOptionTypeSize() ==3);
            {
                //Three-option enchant
                executeEnchantRareThreeOption(pGamePlayer, pMouseItem, pItem, invenX, invenY);
                return;
            }
            // end */
    if (pItem->getOptionTypeSize() != 2 || pItemInfo->isUnique() || pMouseItem->getNum() != 1) {
        cout << "no Option 1" << endl;
        sendEnchantImpossible(pGamePlayer);
        return;
    }

    bool bFirstSucceed = false;
    uint optionChange = 0;
    int shiftValue = 16;

    list<OptionType_t> optionList = pItem->getOptionTypeList();
    list<OptionType_t>::const_iterator itr = optionList.begin();

    OptionType_t firstOption = *itr++;
    OptionType_t secondOption = *itr;

    OptionInfo* pFirstOptionInfo = getOptionInfo(firstOption);
    OptionInfo* pSecondOptionInfo = getOptionInfo(secondOption);

    if (pFirstOptionInfo == NULL || pSecondOptionInfo == NULL) {
        cout << "no Option 2" << endl;
        sendEnchantImpossible(pGamePlayer);
        return;
    }

    bool bFirstUpgradePossible = pFirstOptionInfo->isUpgradePossible();
    bool bSecondUpgradePossible = pSecondOptionInfo->isUpgradePossible();

    if (!bFirstUpgradePossible && !bSecondUpgradePossible) {
        cout << "no Option 3" << endl;
        sendEnchantImpossible(pGamePlayer);
        return;
    }

    if (bFirstUpgradePossible) {
        // The first option's enchant chance follows the Blue Drop 2 chance.
        int succeedRatio = pFirstOptionInfo->getUpgradeSecondRatio() * pItemInfo->getUpgradeRatio();
        int dice = rand() % 10000;

        // cout << "First option: " << pFirstOptionInfo->getHName() << " enchant chance " << succeedRatio << endl;

        if (dice < succeedRatio) {
            // cout << "Enchant succeeded" << endl;
            processUpgradeOptionType(pItem, firstOption, pFirstOptionInfo);

            OptionType_t upgradeOptionType = pFirstOptionInfo->getUpgradeType();
            optionChange = (firstOption << (shiftValue + 8)) | (upgradeOptionType << shiftValue);

            bFirstSucceed = true;
        } else {
            int noChangeRatio = 33;
            int downgradeRatio = pItemInfo->getDowngradeRatio();
            dice = rand() % 100;

            // cout << "Enchant item: " << pItemInfo->getName() << " option drop chance " << downgradeRatio << endl;

            if (dice < noChangeRatio) {
                // No change
            } else if (dice < noChangeRatio + downgradeRatio) {
                // Option dropped
                downgradeOptionType(pItem, firstOption, pFirstOptionInfo);

                OptionType_t previousOptionType = pFirstOptionInfo->getPreviousType();
                optionChange = (firstOption << (shiftValue + 8)) | (previousOptionType << shiftValue);
            } else {
                // Option removed
                pItem->removeOptionType(firstOption);
                optionChange = (firstOption << (shiftValue + 8));

                // DB change
                string optionField;
                setOptionTypeToField(pItem->getOptionTypeList(), optionField);

                char pField[80];
                sprintf(pField, "OptionType='%s'", optionField.c_str());
                pItem->tinysave(pField);
            }
        }
    } else {
        OptionType_t prev = pFirstOptionInfo->getPreviousType();

        if (prev != 0) {
            // The option drop chance has to be computed.

            int noChangeRatio = 33;
            int downgradeRatio = pItemInfo->getDowngradeRatio();
            int dice = rand() % 100;

            // cout << "First option: " << pFirstOptionInfo->getHName() << " option drop chance: " << downgradeRatio <<
            // endl;

            if (dice < noChangeRatio) {
                // No change
            } else if (dice < noChangeRatio + downgradeRatio) {
                // Option dropped
                downgradeOptionType(pItem, firstOption, pFirstOptionInfo);

                OptionType_t previousOptionType = pFirstOptionInfo->getPreviousType();
                optionChange = (firstOption << (shiftValue + 8)) | (previousOptionType << shiftValue);
            } else {
                // Option removed
                pItem->removeOptionType(firstOption);
                optionChange = (firstOption << (shiftValue + 8));

                // DB change
                string optionField;
                setOptionTypeToField(pItem->getOptionTypeList(), optionField);

                char pField[80];
                sprintf(pField, "OptionType='%s'", optionField.c_str());
                pItem->tinysave(pField);
            }
        }
    }

    shiftValue -= 16;

    if (bSecondUpgradePossible) {
        // The second option's enchant chance depends on whether the first option succeeded.
        int succeedRatio =
            g_pOptionInfoManager->getRareUpgradeRatio(secondOption, bFirstSucceed) * pItemInfo->getUpgradeRatio();
        int dice = rand() % 10000;

        // cout << "Second option: " << pSecondOptionInfo->getHName() << " enchant chance " << succeedRatio << endl;

        if (dice < succeedRatio) {
            // cout << "Enchant succeeded" << endl;
            processUpgradeOptionType(pItem, secondOption, pSecondOptionInfo);

            OptionType_t upgradeOptionType = pSecondOptionInfo->getUpgradeType();
            optionChange |= (secondOption << (shiftValue + 8)) | (upgradeOptionType << shiftValue);

            bFirstSucceed = true;
        } else {
            int noChangeRatio = 33;
            int downgradeRatio = pItemInfo->getDowngradeRatio();
            dice = rand() % 100;

            // cout << "Enchant item: " << pItemInfo->getName() << " option drop chance " << downgradeRatio << endl;

            if (dice < noChangeRatio) {
                // No change
            } else if (dice < noChangeRatio + downgradeRatio) {
                // Option dropped
                downgradeOptionType(pItem, secondOption, pSecondOptionInfo);

                OptionType_t previousOptionType = pSecondOptionInfo->getPreviousType();
                optionChange |= (secondOption << (shiftValue + 8)) | (previousOptionType << shiftValue);
            } else {
                // Option removed
                pItem->removeOptionType(secondOption);
                optionChange |= (secondOption << (shiftValue + 8));

                // DB change
                string optionField;
                setOptionTypeToField(pItem->getOptionTypeList(), optionField);

                char pField[80];
                sprintf(pField, "OptionType='%s'", optionField.c_str());
                pItem->tinysave(pField);
            }
        }
    } else {
        OptionType_t prev = pSecondOptionInfo->getPreviousType();

        if (prev != 0) {
            // The option drop chance has to be computed.
            int noChangeRatio = 33;
            int downgradeRatio = pItemInfo->getDowngradeRatio();
            int dice = rand() % 100;

            // cout << "Second option: " << pSecondOptionInfo->getHName() << " option drop chance: " << downgradeRatio <<
            // endl;

            if (dice < noChangeRatio) {
                // No change
            } else if (dice < noChangeRatio + downgradeRatio) {
                // Option dropped
                downgradeOptionType(pItem, secondOption, pSecondOptionInfo);

                OptionType_t previousOptionType = pSecondOptionInfo->getPreviousType();
                optionChange |= (secondOption << (shiftValue + 8)) | (previousOptionType << shiftValue);
            } else {
                // Option removed
                pItem->removeOptionType(secondOption);
                optionChange |= (secondOption << (shiftValue + 8));

                // DB change
                string optionField;
                setOptionTypeToField(pItem->getOptionTypeList(), optionField);

                char pField[80];
                sprintf(pField, "OptionType='%s'", optionField.c_str());
                pItem->tinysave(pField);
            }
        }
    }

    /*		// Having several options is not taken to be impossible..
            // check that the held item is the right one, and
            // when the option can be upgraded...


            //const list<OptionType_t>& optionList = pItem->getOptionTypeList();
            // Copy it and work on the copy..
            list<OptionType_t>::const_iterator itr = optionList.begin();

            for (; itr!=optionList.end(); itr++)
            {
                OptionType_t currentOptionType = *itr;

                OptionInfo* pOptionInfo = getOptionInfo( currentOptionType );

                if (pOptionInfo==NULL)
                {
                    sendEnchantImpossible( pGamePlayer );
                    return;
                }

                bool bUpgradePossible = pOptionInfo->isUpgradePossible();

                if (bUpgradePossible)
                {
                    //---------------------------------------------------------
                    // Only an item count of 1 is allowed.
                    // To allow several at once,
                    // the item count would have to be reduced.. the client would need changing too.
                    // Will the upgrade succeed?
                    //---------------------------------------------------------
                    // option chance * type chance / 10000
                    //---------------------------------------------------------
                    // First option
                    //---------------------------------------------------------

                    // If the first option succeeds, compute with the second option chance.
                    int upgradeRatio = (bFirstSucceed? pOptionInfo->getUpgradeSecondRatio()
                                                    : pOptionInfo->getUpgradeRatio());


                    int succeedRatio = upgradeRatio * pItemInfo->getUpgradeRatio();

                    int dice = rand()%10000;
                    bool bSucceed = (dice < succeedRatio);

                    //cout << "EnchantCheck = "
                    //	<< pOptionInfo->getUpgradeRatio() << " * "
                    //	<< pItemInfo->getUpgradeRatio() << " = "
                    //	<< succeedRatio << ", dice = " << dice << " , succeed=" << (int)bSucceed << endl;

                    //cout << "Before = " << getOptionTypeToString( pItem->getOptionTypeList() ) << endl;
                    //cout << "CurrentOptionType = " << (int)currentOptionType << endl;


                    if (pMouseItem->getNum()==1
                        && bSucceed)
                    {
                        processUpgradeOptionType( pItem, currentOptionType, pOptionInfo );

                        OptionType_t upgradeOptionType = pOptionInfo->getUpgradeType();
                        optionChange = (currentOptionType << (shiftValue+8)) | (upgradeOptionType << shiftValue);

                        bFirstSucceed = true;
                        //cout << gcAddItemToItemVerify.toString().c_str() << endl;
                    }
                    //---------------------------------------------------------
                    // upgrade failed
                    //---------------------------------------------------------
                    else
                    {
                        //cout << "Failed!" << endl;
                        bool bCrashItem = pItemInfo->isUpgradeCrash();

                        if ( bCrashItem )
                        {
                            // Leave an ItemTrace log before breaking the item
                            if ( pItem != NULL && pItem->isTraceItem() )
                            {
                                remainTraceLog( pItem, pCreature->getName(), "GOD", ITEM_LOG_DELETE, DETAIL_ENCHANT);
                            }
                            crashItem(pItem, pInventory, invenX, invenY);

                            GCAddItemToItemVerify gcAddItemToItemVerify;
                            gcAddItemToItemVerify.setCode( ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_FAIL_CRASH );
                            pGamePlayer->sendPacket( &gcAddItemToItemVerify );

                            return;
                        }
                        else
                        {
                            // Taking the list by reference above lets an option be removed here, so
                            // the list iterator can go stale.
                            downgradeOptionType( pItem, currentOptionType, pOptionInfo );

                            OptionType_t previousOptionType = pOptionInfo->getPreviousType();
                            optionChange = (currentOptionType << (shiftValue+8)) | (previousOptionType << shiftValue);
                        }
                    }
                }
                else
                {
                    impossibleCount ++;
                }

                shiftValue -= 16;
            }*/

    {
        // Delete the held star
        pPC->deleteItemFromExtraInventorySlot();

        // Delete from the DB
        pMouseItem->destroy();

        // Delete from memory
        SAFE_DELETE(pMouseItem);

        // Send the result packet
        sendEnchantOK(pGamePlayer, optionChange);
    }

    __END_CATCH
}

void executeEnchantOption(GamePlayer* pGamePlayer, Item* pMouseItem, Item* pItem, CoordInven_t invenX,
                          CoordInven_t invenY) {
    __BEGIN_TRY

    Creature* pCreature = pGamePlayer->getCreature();
    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
    Inventory* pInventory = pPC->getInventory();

    // Get the Item's OptionInfo.
    OptionType_t currentOptionType = // pItem->getFirstOptionType();
        pItem->getRandomOptionType();

    int optionSize = pItem->getOptionTypeSize();

    // The failure chance is decided by itemType.
    const ItemInfo* pItemInfo = g_pItemInfoManager->getItemInfo(pItem->getItemClass(), pItem->getItemType());

    // The item to enchant has no option,
    // is a rare item,
    // is a unique item,
    if (optionSize != 1 || pItemInfo->isUnique()) {
        // cout << "no Option " << endl;
        sendEnchantImpossible(pGamePlayer);
        return;
    }

    OptionInfo* pOptionInfo = getOptionInfo(currentOptionType);

    if (pOptionInfo == NULL) {
        sendEnchantImpossible(pGamePlayer);
        return;
    }

    uint optionChange = 0;

    // Having several options is not taken to be impossible..
    // check that the held item is the right one, and
    // when the option can be upgraded...
    bool bUpgradePossible = pOptionInfo->isUpgradePossible();
    if (optionSize > 1 || bUpgradePossible) {
        //---------------------------------------------------------
        // Only an item count of 1 is allowed.
        // To allow several at once,
        // the item count would have to be reduced.. the client would need changing too.
        // Will the upgrade succeed?
        //---------------------------------------------------------
        // option chance * type chance / 10000
        int succeedRatio = 0;

        if (pMouseItem->getItemType() == 15) {
            // Blue Drop 2 onwards
            succeedRatio = pOptionInfo->getUpgradeSecondRatio() * pItemInfo->getUpgradeRatio();
        } else {
            // The existing enchant items
            succeedRatio = pOptionInfo->getUpgradeRatio() * pItemInfo->getUpgradeRatio();
        }

        //			cout << "Option " << pOptionInfo->getHName() << " enchant chance : " << succeedRatio << endl;

        int dice = rand() % 10000;
        bool bSucceed = (dice < succeedRatio);

        /*
        cout << "EnchantCheck = "
            << pOptionInfo->getUpgradeRatio() << " * "
            << pItemInfo->getUpgradeRatio() << " = "
            << succeedRatio << ", dice = " << dice << " , succeed=" << (int)bSucceed << endl;
        */

        // cout << "Before = " << getOptionTypeToString( pItem->getOptionTypeList() ) << endl;
        // cout << "CurrentOptionType = " << (int)currentOptionType << endl;

        // With several options attached, a chosen option that cannot be upgraded counts as a failure.
        // The if above checked optionSize>1, so when bUpgradePossible does not hold,
        // optionSize>1 can be assumed.
        if (bUpgradePossible && pMouseItem->getNum() == 1 && bSucceed) {
            //				cout << "Enchant succeeded" << endl;
            processUpgradeOptionType(pItem, currentOptionType, pOptionInfo);

            OptionType_t upgradeOptionType = pOptionInfo->getUpgradeType();
            uint optionChange = (currentOptionType << 24) | (upgradeOptionType << 16);

            sendEnchantOK(pGamePlayer, optionChange);

            // cout << gcAddItemToItemVerify.toString().c_str() << endl;
        }
        //---------------------------------------------------------
        // upgrade failed
        //---------------------------------------------------------
        else {
            // cout << "Failed!" << endl;
            bool bCrashItem = pItemInfo->isUpgradeCrash();

            // fail packet: decide whether the item breaks
            GCAddItemToItemVerify gcAddItemToItemVerify;

            if (bCrashItem) {
                // Leave an ItemTrace log before breaking the item
                if (pItem != NULL && pItem->isTraceItem()) {
                    remainTraceLog(pItem, pCreature->getName(), "GOD", ITEM_LOG_DELETE, DETAIL_ENCHANT);
                }

                crashItem(pItem, pInventory, invenX, invenY);

                gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_FAIL_CRASH);

                // cout << "Broken" << endl;
            } else {
                int noChangeRatio = 33;
                int downgradeRatio = pItemInfo->getDowngradeRatio();
                int dice = rand() % 100;

                // cout << "Enchant item: " << pItemInfo->getName() << " option drop chance " << downgradeRatio << endl;

                if (dice < noChangeRatio) {
                    // No change
                } else if (dice < noChangeRatio + downgradeRatio) {
                    // Option dropped
                    downgradeOptionType(pItem, currentOptionType, pOptionInfo);

                    OptionType_t previousOptionType = pOptionInfo->getPreviousType();
                    optionChange = (currentOptionType << 24) | (previousOptionType << 16);
                }

                gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_FAIL_DECREASE);
                gcAddItemToItemVerify.setParameter(optionChange);
            }

            pGamePlayer->sendPacket(&gcAddItemToItemVerify);

            // cout << "Upgrade Failed " << (bCrashItem? ": Crash!":" ") << endl;
        }

        // Delete the held star
        pPC->deleteItemFromExtraInventorySlot();

        // Delete from the DB
        pMouseItem->destroy();

        // Delete from memory
        SAFE_DELETE(pMouseItem);
    } else {
        sendEnchantImpossible(pGamePlayer);
        // cout << "upgrade impossible" << endl;
    }

    __END_CATCH
}

// Add an option to an item that has none.
void executeAddOption(GamePlayer* pGamePlayer, Item* pMouseItem, Item* pItem, OptionType_t addOptionType) {
    __BEGIN_TRY

    Creature* pCreature = pGamePlayer->getCreature();
    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);

    // Not allowed outside the premium service..
    /*		if (!pGamePlayer->isPayPlaying()
                && !pGamePlayer->isPremiumPlay())
            {
                //cout << "no Option " << endl;
                GCAddItemToItemVerify gcAddItemToItemVerify;
                gcAddItemToItemVerify.setCode( ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_IMPOSSIBLE_NO_PREMIUM );
                pGamePlayer->sendPacket( &gcAddItemToItemVerify );
                return;
            }
    */

    // The option class can attach to this class,
    // there is no option,
    // it is not unique,
    // and only one is held on the mouse: then an option can be added.
    if (isPossibleOptionItemClass(pItem->getItemClass()) && pItem->getOptionTypeSize() == 0 && !pItem->isUnique() &&
        pMouseItem->getNum() == 1) {
        // Option change
        pItem->addOptionType(addOptionType);

        string optionField;
        setOptionTypeToField(pItem->getOptionTypeList(), optionField);

        // DB change
        char pField[80];
        sprintf(pField, "OptionType='%s'", optionField.c_str());
        pItem->tinysave(pField);

        uint optionChange = addOptionType;

        // Delete the held star
        pPC->deleteItemFromExtraInventorySlot();
        pMouseItem->destroy();   // delete from the DB
        SAFE_DELETE(pMouseItem); // delete from memory


        // success packet: put in the option that will be upgraded.
        GCAddItemToItemVerify gcAddItemToItemVerify;
        gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_OK);
        gcAddItemToItemVerify.setParameter(optionChange);
        pGamePlayer->sendPacket(&gcAddItemToItemVerify);

        // cout << gcAddItemToItemVerify.toString().c_str() << endl;
    } else {
        // cout << "Enchant Impossible" << endl;
        // cout << "impossibleOption : " << (int)currentOptionType << endl;
        //  This option cannot be upgraded.
        //  impossible packet
        GCAddItemToItemVerify gcAddItemToItemVerify;
        gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_IMPOSSIBLE);
        pGamePlayer->sendPacket(&gcAddItemToItemVerify);

        // cout << "upgrade impossible" << endl;
    }

    __END_CATCH
}

void executeTransKit(GamePlayer* pGamePlayer, Item* pMouseItem, Item* pItem) {
    __BEGIN_TRY

    GCAddItemToItemVerify gcResult;
    uint optionChange;

    ItemInfo* pItemInfo = g_pItemInfoManager->getItemInfo(pItem->getItemClass(), pItem->getItemType());
    if (pItemInfo->getReqGender() == GENDER_BOTH || pItemInfo->getReqGender() == GENDER_MAX || pItemInfo->isUnique() ||
        pMouseItem->getNum() != 1) {
        gcResult.setCode(ADD_ITEM_TO_ITEM_VERIFY_TRANS_IMPOSSIBLE);
        pGamePlayer->sendPacket(&gcResult);
        return;
    }

    ItemType_t newItemType = pItem->getItemType() ^ 1;
    optionChange = (pItem->getItemType() << 16) | newItemType;
    pItem->setItemType(newItemType);

    char query[50];
    sprintf(query, "ItemType=%u", newItemType);
    pItem->tinysave(query);

    gcResult.setCode(ADD_ITEM_TO_ITEM_VERIFY_TRANS_OK);
    gcResult.setParameter(optionChange);
    pGamePlayer->sendPacket(&gcResult);

    Creature* pCreature = pGamePlayer->getCreature();
    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);

    pPC->deleteItemFromExtraInventorySlot();
    pMouseItem->destroy();
    SAFE_DELETE(pMouseItem);

    __END_CATCH
}

OptionInfo* getOptionInfo(OptionType_t optionType) {
    OptionInfo* pOptionInfo = NULL;

    try {
        pOptionInfo = g_pOptionInfoManager->getOptionInfo(optionType);

        if (pOptionInfo == NULL) {
            filelog("optionError.txt", "CGAddItemToItemHandler: no option [%d]", optionType);

            return NULL;
        }
    } catch (NoSuchElementException& t) {
        filelog("optionError.txt", "CGAddItemToItemHandler: no option [%d]", optionType);

        return NULL;
    }

    return pOptionInfo;
}


void processUpgradeOptionType(Item* pItem, OptionType_t currentOptionType, OptionInfo* pOptionInfo) {
    __BEGIN_TRY

    // cout << "Succeed!" << endl;
    //  option that will be upgraded
    OptionType_t upgradeOptionType = pOptionInfo->getUpgradeType();

    // Option change
    pItem->changeOptionType(currentOptionType, upgradeOptionType);

    // cout << "Succeed = " << getOptionTypeToString( pItem->getOptionTypeList() );

    string optionField;
    setOptionTypeToField(pItem->getOptionTypeList(), optionField);

    // list<OptionType_t> op;
    // setOptionTypeFromField( op, optionField );
    // cout << "Check = " << getOptionTypeToString( op ) << endl;

    // DB change
    char pField[80];
    sprintf(pField, "OptionType='%s'", optionField.c_str());
    pItem->tinysave(pField);

    __END_CATCH
}

void crashItem(Item* pItem, Inventory* pInventory, CoordInven_t invenX, CoordInven_t invenY) {
    __BEGIN_TRY

    // Remove the item from the inventory
    pInventory->deleteItem(invenX, invenY);

    // Remove from the DB
    pItem->destroy();

    // Remove from memory
    SAFE_DELETE(pItem);

    __END_CATCH
}

void downgradeOptionType(Item* pItem, OptionType_t currentOptionType, OptionInfo* pOptionInfo) {
    __BEGIN_TRY

    // Drop the item's option by one step.
    OptionType_t previousOptionType = pOptionInfo->getPreviousType();

    // cout << "FailBefore : " << getOptionTypeToString(pItem->getOptionTypeList()) << endl;

    // Option change
    // If there is no previous option, remove the option entirely.
    if (previousOptionType == 0) {
        pItem->removeOptionType(currentOptionType);
        // cout << "Remove = " << getOptionTypeToString( pItem->getOptionTypeList() ) << endl;
    } else {
        pItem->changeOptionType(currentOptionType, previousOptionType);
        // cout << "Down = " << getOptionTypeToString( pItem->getOptionTypeList() ) << endl;
    }

    // DB change
    string optionField;
    setOptionTypeToField(pItem->getOptionTypeList(), optionField);

    // cout << "FailAfter : " << getOptionTypeToString(pItem->getOptionTypeList()) << endl;

    char pField[80];
    sprintf(pField, "OptionType='%s'", optionField.c_str());
    pItem->tinysave(pField);


    __END_CATCH
}


void sendEnchantImpossible(GamePlayer* pGamePlayer) {
    __BEGIN_TRY

    GCAddItemToItemVerify gcAddItemToItemVerify;
    gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_IMPOSSIBLE);
    pGamePlayer->sendPacket(&gcAddItemToItemVerify);

    __END_CATCH
}

void sendEnchantOK(GamePlayer* pGamePlayer, DWORD optionChange) {
    __BEGIN_TRY

    GCAddItemToItemVerify gcAddItemToItemVerify;
    gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_OK);
    gcAddItemToItemVerify.setParameter(optionChange);
    gcAddItemToItemVerify.setParameter2(0);
    pGamePlayer->sendPacket(&gcAddItemToItemVerify);

    __END_CATCH
}

void sendEnchantOK(GamePlayer* pGamePlayer, DWORD optionChange, DWORD optionChange2) {
    __BEGIN_TRY

    GCAddItemToItemVerify gcAddItemToItemVerify;
    gcAddItemToItemVerify.setCode(ADD_ITEM_TO_ITEM_VERIFY_THREE_ENCHANT_OK);
    gcAddItemToItemVerify.setParameter(optionChange);
    gcAddItemToItemVerify.setParameter2(optionChange2);
    pGamePlayer->sendPacket(&gcAddItemToItemVerify);

    __END_CATCH
}

#endif
