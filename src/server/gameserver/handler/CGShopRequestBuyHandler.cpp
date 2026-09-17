//////////////////////////////////////////////////////////////////////////////
// Filename    : CGShopRequestBuyHandler.cpp
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGShopRequestBuy.h"

#ifdef __GAME_SERVER__
#include "GamePlayer.h"
#include "ItemFactoryManager.h"
#include "ItemInfo.h"
#include "ItemInfoManager.h"
#include "ItemUtil.h"
#include "NPC.h"
#include "ParkingCenter.h"
#include "PriceManager.h"
#include "Slayer.h"
#include "Tile.h"
#include "Vampire.h"
// #include "LogClient.h"
#include <stdio.h>

#include "CastleInfoManager.h"
#include "GCShopBuyFail.h"
#include "GCShopBuyOK.h"
#include "GCShopSold.h"
#include "GuildManager.h"
#include "SystemAvailabilitiesManager.h"
#include "Utility.h"
#include "ZoneUtil.h"
#include "item/Key.h"
#include "item/Magazine.h"
#include "item/Potion.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// Check that the NPC selling what the player wants, and that item, exist,
// then branch into the ordinary item and the motorcycle handling.
//////////////////////////////////////////////////////////////////////////////
void CGShopRequestBuyHandler::execute(CGShopRequestBuy* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    // Pull the packet information out.
    ObjectID_t NPCID = pPacket->getObjectID();
    ShopRackType_t shopType = pPacket->getShopType();
    BYTE shopIndex = pPacket->getShopIndex();
    ItemNum_t itemNum = pPacket->getItemNum();
    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);
    Creature* pNPCBase = NULL;

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pGamePlayer->getCreature());
    Assert(pPC != NULL);

    Zone* pZone = pPC->getZone();
    Assert(pZone != NULL);

    if (shopType == SHOP_RACK_MYSTERIOUS) {
        SYSTEM_ASSERT(SYSTEM_GAMBLE);
    }

    // try
    //{

    // NoSuch removed.
    pNPCBase = pZone->getCreature(NPCID);
    //}
    if (pNPCBase == NULL) // catch (NoSuchElementException)
    {
        GCShopBuyFail gcShopBuyFail;
        gcShopBuyFail.setCode(GC_SHOP_BUY_FAIL_NPC_NOT_EXIST);
        gcShopBuyFail.setAmount(0);
        pPlayer->sendPacket(&gcShopBuyFail);
        return;
    }

    if (!pNPCBase->isNPC()) {
        GCShopBuyFail gcShopBuyFail;
        gcShopBuyFail.setCode(GC_SHOP_BUY_FAIL_NOT_NPC);
        gcShopBuyFail.setAmount(0);
        pPlayer->sendPacket(&gcShopBuyFail);
        return;
    }

    NPC* pNPC = dynamic_cast<NPC*>(pNPCBase);

    /*
    // Used to check whether the store has the item
    for (int i=0; i<=2; i++)
    {
        for (int j=0; j<20; j++)
        {
            cout << (int)pNPC->isExistShopItem(i, j) << " ";
        }

        cout << endl;
    }
    */

    if (pNPC->getShopType() == SHOPTYPE_NORMAL) {
        // Check that the NPC holds the item the player wants to buy
        if (pNPC->isExistShopItem(shopType, shopIndex) == false) {
            GCShopBuyFail gcShopBuyFail;
            gcShopBuyFail.setCode(GC_SHOP_BUY_FAIL_ITEM_NOT_EXIST);
            gcShopBuyFail.setAmount(0);
            pPlayer->sendPacket(&gcShopBuyFail);
            return;
        }

        Item* pItem = pNPC->getShopItem(shopType, shopIndex);

        if (pItem->getItemClass() == Item::ITEM_CLASS_TRAP_ITEM) {
            if (!g_pGuildManager->isGuildMaster(pPC->getGuildID(), pPC) ||
                !pPC->isFlag(Effect::EFFECT_CLASS_SIEGE_DEFENDER)) {
                GCShopBuyFail gcShopBuyFail;
                gcShopBuyFail.setCode(GC_SHOP_BUY_FAIL_ITEM_NOT_EXIST);
                gcShopBuyFail.setAmount(0);
                pPlayer->sendPacket(&gcShopBuyFail);
                return;
            }
        }

        // Check that the item count is valid
        if (itemNum < 1 || itemNum > ItemMaxStack[pItem->getItemClass()]) {
            throw ProtocolException("CGShopRequestBuyHandler::execute() : invalid item count!");
        }

        // With an item count of 2 or more, check that the item is a valid one
        // A non-stackable item has its count set to 1.
        if (itemNum > 1 && !isStackable(pItem->getItemClass()))
            pPacket->setItemNum(1);

        if (pItem->getItemClass() == Item::ITEM_CLASS_MOTORCYCLE)
            executeMotorcycle(pPacket, pPlayer);
        else
            executeNormal(pPacket, pPlayer);
    }
///////////////////////////////////////////////////////////////////////
// This function was used for the 2001 Christmas event.
// Reused for the 2002 Children's Day event.
// It is expected to be renamed from XMAS_EVENT to STAR_EVENT.
//////////////////////////////////////////////////////////////////////
#ifdef __XMAS_EVENT_CODE__
    else if (pNPC->getShopType() == SHOPTYPE_EVENT) {
        // Check that the NPC holds the item the player wants to buy
        if (pNPC->isExistShopItem(shopType, shopIndex) == false) {
            GCShopBuyFail gcShopBuyFail;
            gcShopBuyFail.setCode(GC_SHOP_BUY_FAIL_ITEM_NOT_EXIST);
            gcShopBuyFail.setAmount(0);
            pPlayer->sendPacket(&gcShopBuyFail);
            return;
        }

        // Check that the item count is valid
        if (itemNum != 1)
            throw ProtocolException("CGShopRequestBuyHandler::execute() : invalid item count!");

        executeEvent(pPacket, pPlayer);
    }
#endif

    // cout << "Buy ok" << endl;

#endif

    __END_DEBUG_EX __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// Handles buying an ordinary item.
//////////////////////////////////////////////////////////////////////////////
void CGShopRequestBuyHandler::executeNormal(CGShopRequestBuy* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        ObjectID_t NPCID = pPacket->getObjectID();
    ShopRackType_t shopType = pPacket->getShopType();
    BYTE shopIndex = pPacket->getShopIndex();
    ItemNum_t itemNum = pPacket->getItemNum();
    Coord_t x = pPacket->getX();
    Coord_t y = pPacket->getY();
    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);
    Creature* pCreature = pGamePlayer->getCreature();
    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
    Zone* pZone = pPC->getZone();
    NPC* pNPC = dynamic_cast<NPC*>(pZone->getCreature(NPCID));
    Gold_t itemTax = 0;

    // NoSuch removed.
    if (pNPC == NULL)
        return;

    Item* pItem = pNPC->getShopItem(shopType, shopIndex);
    Price_t itemMoney;
    bool bMysteriousRack = (shopType == SHOP_RACK_MYSTERIOUS);

    // A Mysterious item is priced differently.
    if (bMysteriousRack) {
        // cout << pPacket->toString().c_str() << endl;
        itemMoney = g_pPriceManager->getMysteriousPrice(pItem->getItemClass(), pCreature);
        // cout << "CGShopRequestBuyHandler::MysteriousItem Price = " << itemMoney << endl;
    } else {
        itemMoney = g_pPriceManager->getPrice(pItem, pNPC->getMarketCondSell(), shopType, pPC) * itemNum;
        // cout << "CGShopRequestBuyHandler::normalItem Price = " << itemMoney << endl;
    }

    if (pNPC->getTaxingCastleZoneID() != 0) {
        int itemTaxRatio = pNPC->getTaxRatio(pPC);
        if (itemTaxRatio > 100) {
            //			int NewItemMoney = getPercentValue((int)itemMoney, itemTaxRatio);
            int NewItemMoney = (int)(itemMoney * (itemTaxRatio / 100.0));
            itemTax = (NewItemMoney - itemMoney);

            itemMoney = NewItemMoney;
        }
    }

    Item::ItemClass IClass = pItem->getItemClass();
    ItemType_t IType = pItem->getItemType();
    const list<OptionType_t>& OType = pItem->getOptionTypeList();

    Inventory* pInventory = pPC->getInventory();
    Gold_t playerMoney = pPC->getGold();

    if (playerMoney < itemMoney) {
        GCShopBuyFail gcShopBuyFail;
        gcShopBuyFail.setCode(GC_SHOP_BUY_FAIL_NOT_ENOUGH_MONEY);
        gcShopBuyFail.setAmount(0);
        pPlayer->sendPacket(&gcShopBuyFail);

        // cout << "Not Enough Money for Mysterious Item" << endl;

        return;
    }

    // Create the mysterious item.
    if (bMysteriousRack) {
        if (pZone->isPremiumZone() || pZone->isPayPlay()) {
            pItem = getRandomMysteriousItem(pCreature, IClass);
        } else {
            // Outside a premium zone the gamble level is limited to 30.
            pItem = getRandomMysteriousItem(pCreature, IClass, 30);
        }

        Assert(pItem != NULL);
        (pZone->getObjectRegistry()).registerObject(pItem);
    }

    // For a potion or a magazine the item count is set first, and then...
    if (isStackable(pItem)) {
        pItem->setNum(itemNum);
    } else {
        pItem->setNum(1);
    }

    if (!pInventory->canAddingEx(x, y, pItem)) {
        GCShopBuyFail gcShopBuyFail;
        gcShopBuyFail.setCode(GC_SHOP_BUY_FAIL_NOT_ENOUGH_SPACE);
        gcShopBuyFail.setAmount(0);
        pPlayer->sendPacket(&gcShopBuyFail);

        // The mysterious item was created above through getRandomMysteriousItem(),
        // so it has to be deleted.
        if (bMysteriousRack)
            SAFE_DELETE(pItem);

        // cout << "Can't Add to Inventory" << endl;

        return;
    }

    // if (pItem != NULL ) pItem->whenPCTake(pPC);

    // When the player bought a special item the shop version goes up.
    if (shopType == SHOP_RACK_SPECIAL) {
        pNPC->increaseShopVersion(shopType);
    }

    // Take the player's money.
    // pPC->setGoldEx(playerMoney - itemMoney);

    // by sigi. 2002.9.4
    pPC->decreaseGoldEx(itemMoney);
    filelog("Tax.log", "%s 가 %s 에게 %u 만큼을 세금으로 냈습니다.", pPC->getName().c_str(), pNPC->getName().c_str(),
            itemTax);
    g_pCastleInfoManager->increaseTaxBalance(pNPC->getTaxingCastleZoneID(), itemTax);

    // cout << "addItemEx" << endl;

    Item* pReturnItem = pInventory->addItemEx(x, y, pItem);
    if (pReturnItem == pItem) {
        // cout << "add ok" << endl;
        //  pReturnItem equal to pItem means the item was not
        //  a stacking item.


        // Keep the existing ItemID.
        // An ItemID of 0 means create() hands out a new ItemID.
        // by sigi. 2002.10.28
        pItem->create(pPC->getName(), STORAGE_INVENTORY, 0, x, y, pItem->getItemID());
        // Calling create alone makes the DB put a count like a potion's at 1.
        // So save has to be called again to set the real count.
        // pItem->save(pPC->getName(), STORAGE_INVENTORY, 0, x, y);

        // create was changed to store the count right away.
        // Item save optimization.

        // Send the OK packet.
        GCShopBuyOK OKPacket;
        OKPacket.setObjectID(NPCID);
        OKPacket.setShopVersion(pNPC->getShopVersion(shopType));
        OKPacket.setItemObjectID(pItem->getObjectID());
        OKPacket.setItemClass(pItem->getItemClass());
        OKPacket.setItemType(pItem->getItemType());
        OKPacket.setOptionType(pItem->getOptionTypeList());
        OKPacket.setDurability(pItem->getDurability());
        OKPacket.setItemNum(pItem->getNum());
        OKPacket.setSilver(pItem->getSilver());
        OKPacket.setGrade(pItem->getGrade());
        OKPacket.setEnchantLevel(pItem->getEnchantLevel());
        OKPacket.setPrice(playerMoney - itemMoney);
        pPlayer->sendPacket(&OKPacket);

        // log(LOG_BUY_ITEM, pPC->getName(), "", pItem->toString());
    } else {
        // cout << "pile ok" << endl;
        // log(LOG_BUY_ITEM, pPC->getName(), "", pItem->toString());

        // pReturnItem different from pItem means the item was
        // a stacking item. So the pItem sent in to be added
        // has to be deleted.
        SAFE_DELETE(pItem);
        // pReturnItem->save(pPC->getName(), STORAGE_INVENTORY, 0, x, y);
        //  Item save optimization.
        char pField[80];
        sprintf(pField, "Num=%d", pReturnItem->getNum());
        pReturnItem->tinysave(pField);


        // Send the OK packet.
        GCShopBuyOK OKPacket;
        OKPacket.setObjectID(NPCID);
        OKPacket.setShopVersion(pNPC->getShopVersion(shopType));
        OKPacket.setItemObjectID(pReturnItem->getObjectID());
        OKPacket.setItemClass(pReturnItem->getItemClass());
        OKPacket.setItemType(pReturnItem->getItemType());
        OKPacket.setOptionType(pReturnItem->getOptionTypeList());
        OKPacket.setDurability(pReturnItem->getDurability());
        OKPacket.setItemNum(pReturnItem->getNum());
        OKPacket.setSilver(pReturnItem->getSilver());
        OKPacket.setGrade(pReturnItem->getGrade());
        OKPacket.setEnchantLevel(pReturnItem->getEnchantLevel());
        OKPacket.setPrice(playerMoney - itemMoney);
        pPlayer->sendPacket(&OKPacket);
    }

    // Leave an ItemTrace log
    if (pItem != NULL && pItem->isTraceItem()) {
        remainTraceLog(pItem, pNPC->getName(), pCreature->getName(), ITEM_LOG_CREATE, DETAIL_SHOPBUY);
    }
    // cout << "send OK" << endl;

    // When it is not a mysterious item..
    if (!bMysteriousRack) {
        pNPC->removeShopItem(shopType, shopIndex);
    }

    if (shopType == SHOP_RACK_NORMAL) {
        // When the sold item is a normal item, create an item of the same type and class.
        Item* pNewItem = g_pItemFactoryManager->createItem(IClass, IType, OType);
        Assert(pNewItem != NULL);
        (pZone->getObjectRegistry()).registerObject(pNewItem);
        pNPC->insertShopItem(shopType, shopIndex, pNewItem);
    } else if (bMysteriousRack) {
        // For a mysterious item the shop can be left as it is.
        // cout << "mysterious item" << endl;
    } else {
        // When the sold item is not a normal item,
        // the players nearby have to be told that a shop item was sold.
        int CenterX = pNPC->getX();
        int CenterY = pNPC->getY();
        GCShopSold soldpkt;
        soldpkt.setObjectID(NPCID);
        soldpkt.setShopVersion(pNPC->getShopVersion(shopType));
        soldpkt.setShopType(shopType);
        soldpkt.setShopIndex(shopIndex);

        try {
            for (int zx = CenterX - 5; zx <= CenterX + 5; zx++) {
                for (int zy = CenterY - 5; zy <= CenterY + 5; zy++) {
                    // Check that the bounds are not crossed
                    if (!isValidZoneCoord(pZone, zx, zy))
                        continue;

                    Tile& tile = pZone->getTile(zx, zy);

                    // Search the walking creatures
                    if (tile.hasCreature(Creature::MOVE_MODE_WALKING)) {
                        Creature* pNearCreature = tile.getCreature(Creature::MOVE_MODE_WALKING);
                        if (pNearCreature != NULL) {
                            // Skip the player that just bought the item
                            if (pNearCreature->getObjectID() == pPC->getObjectID())
                                continue;
                            // Send the packet if it is a player.
                            if (pNearCreature->isPC()) {
                                Player* pNearPlayer = pNearCreature->getPlayer();
                                if (pNearPlayer != NULL)
                                    pNearPlayer->sendPacket(&soldpkt);
                            }
                        }
                    }
                    // Search the flying creatures
                    if (tile.hasCreature(Creature::MOVE_MODE_FLYING)) {
                        Creature* pNearCreature = tile.getCreature(Creature::MOVE_MODE_FLYING);
                        if (pNearCreature != NULL) {
                            // Skip the player that just bought the item
                            if (pNearCreature->getObjectID() == pPC->getObjectID())
                                continue;
                            // Send the packet if it is a player.
                            if (pNearCreature->isPC()) {
                                Player* pNearPlayer = pNearCreature->getPlayer();
                                if (pNearPlayer != NULL)
                                    pNearPlayer->sendPacket(&soldpkt);
                            }
                        }
                    }
                } // for (ZoneCoord_t zy...) end
            } // for (ZoneCoord_t zx...) end
        } catch (Throwable& t) {
            filelog("shopbug_packet.log", "%s", t.toString().c_str());
        }
    }

#endif

    __END_DEBUG_EX __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Handles buying a motorcycle.
//////////////////////////////////////////////////////////////////////////////
void CGShopRequestBuyHandler::executeMotorcycle(CGShopRequestBuy* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        // Pull the packet information out.
        ObjectID_t NPCID = pPacket->getObjectID();
    ShopRackType_t shopType = pPacket->getShopType();
    BYTE shopIndex = pPacket->getShopIndex();
    ZoneCoord_t x = pPacket->getX();
    ZoneCoord_t y = pPacket->getY();
    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);
    Creature* pCreature = pGamePlayer->getCreature();
    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
    Zone* pZone = pPC->getZone();
    NPC* pNPC = dynamic_cast<NPC*>(pZone->getCreature(NPCID));

    // NoSuch removed.
    if (pNPC == NULL)
        return;

    Item* pItem = pNPC->getShopItem(shopType, shopIndex);
    Price_t itemMoney = g_pPriceManager->getPrice(pItem, pNPC->getMarketCondSell(), shopType, pPC);
    Item::ItemClass IClass;
    ItemType_t IType;

    Inventory* pInventory = pPC->getInventory();
    Gold_t playerMoney = pPC->getGold();

    list<OptionType_t> optionNULL;
    Item* pTestKey = g_pItemFactoryManager->createItem(Item::ITEM_CLASS_KEY, 0, optionNULL);
    Assert(pTestKey != NULL);

    if (playerMoney < itemMoney) {
        GCShopBuyFail gcShopBuyFail;
        gcShopBuyFail.setCode(GC_SHOP_BUY_FAIL_NOT_ENOUGH_MONEY);
        gcShopBuyFail.setAmount(0);
        pPlayer->sendPacket(&gcShopBuyFail);
        SAFE_DELETE(pTestKey);
        return;
    }

    if (!pInventory->canAddingEx(x, y, pTestKey)) {
        GCShopBuyFail gcShopBuyFail;
        gcShopBuyFail.setCode(GC_SHOP_BUY_FAIL_NOT_ENOUGH_SPACE);
        gcShopBuyFail.setAmount(0);
        pPlayer->sendPacket(&gcShopBuyFail);
        SAFE_DELETE(pTestKey);
        return;
    }

    // When the player bought a special item the shop version goes up.
    if (shopType == SHOP_RACK_SPECIAL)
        pNPC->increaseShopVersion(shopType);

    // Take the player's money.
    // pPC->setGoldEx(playerMoney - itemMoney);
    // by sigi. 2002.9.4
    pPC->decreaseGoldEx(itemMoney);

    // First take the motorcycle out of the NPC's rack and put it in the zone.
    // Then write to the DB that the motorcycle was sold.
    TPOINT pt = pZone->addItem(pItem, pPC->getX(), pPC->getY(), false);
    if (pt.x == -1) {
        // Erase the motorcycle just sold from the NPC on the server side.
        // pNPC->removeShopItem(shopType, shopIndex);

        // SAFE_DELETE(pItem);
        //  The motorcycle could not be added to the zone. Just return.
        cerr << "######################################################" << endl;
        cerr << "# CRITICAL ERROR!!! Cannot add MOTORCYCLE to ZONE!!! #" << endl;
        cerr << "######################################################" << endl;
        return;
    }
    pItem->create(pPC->getName(), STORAGE_ZONE, pZone->getZoneID(), pt.x, pt.y);
    ItemID_t MotorcycleID = pItem->getItemID();

    // Register the motorcycle with the parking center.
    MotorcycleBox* pBox = new MotorcycleBox(dynamic_cast<Motorcycle*>(pItem), pZone, pt.x, pt.y);
    Assert(pBox != NULL);
    g_pParkingCenter->addMotorcycleBox(pBox);

    // Next create the key that matches the motorcycle just created.
    // Then write to the DB that the motorcycle key passed to the player.
    Item* pKey = g_pItemFactoryManager->createItem(Item::ITEM_CLASS_KEY, 2, optionNULL);
    Assert(pKey != NULL);
    (pZone->getObjectRegistry()).registerObject(pKey);
    dynamic_cast<Key*>(pKey)->setTarget(MotorcycleID);
    pKey->create(pPC->getName(), STORAGE_INVENTORY, 0, x, y);

    // Add the motorcycle key to the player's inventory.
    pInventory->addItemEx(x, y, pKey);

    // Erase the motorcycle just sold from the NPC on the server side.
    pNPC->removeShopItem(shopType, shopIndex);

    // Send the OK packet.
    GCShopBuyOK OKPacket;
    OKPacket.setObjectID(NPCID);
    OKPacket.setShopVersion(pNPC->getShopVersion(shopType));
    OKPacket.setItemObjectID(pKey->getObjectID());
    OKPacket.setItemClass(pKey->getItemClass());
    OKPacket.setItemType(pKey->getItemType());
    OKPacket.setOptionType(pKey->getOptionTypeList());
    OKPacket.setDurability(pKey->getDurability());
    OKPacket.setItemNum(1);
    OKPacket.setSilver(pKey->getSilver());
    OKPacket.setGrade(pKey->getGrade());
    OKPacket.setEnchantLevel(pKey->getEnchantLevel());
    OKPacket.setPrice(playerMoney - itemMoney);
    pPlayer->sendPacket(&OKPacket);

    // When the sold motorcycle is a normal motorcycle...
    // create a motorcycle of the same type and class.
    if (shopType == SHOP_RACK_NORMAL) {
        IClass = Item::ITEM_CLASS_MOTORCYCLE;
        IType = pItem->getItemType();
        const list<OptionType_t>& OType = pItem->getOptionTypeList();
        Item* pNewItem = g_pItemFactoryManager->createItem(IClass, IType, OType);
        Assert(pNewItem != NULL);
        (pZone->getObjectRegistry()).registerObject(pNewItem);
        pNPC->insertShopItem(shopType, shopIndex, pNewItem);
    } else if (shopType == SHOP_RACK_MYSTERIOUS) {
    } else {
        // When the sold motorcycle is not a normal one, the players nearby
        // have to be told that the shop's motorcycle was sold.

        int CenterX = pNPC->getX();
        int CenterY = pNPC->getY();
        Creature* pNearCreature = NULL;
        Player* pNearPlayer = NULL;
        GCShopSold soldpkt;

        soldpkt.setObjectID(NPCID);
        soldpkt.setShopVersion(pNPC->getShopVersion(shopType));
        soldpkt.setShopType(shopType);
        soldpkt.setShopIndex(shopIndex);

        try {
            for (int zx = CenterX - 5; zx <= CenterX + 5; zx++) {
                for (int zy = CenterY - 5; zy <= CenterY + 5; zy++) {
                    // Check that the bounds are not crossed
                    if (!isValidZoneCoord(pZone, zx, zy))
                        continue;

                    Tile& tile = pZone->getTile(zx, zy);

                    if (tile.hasCreature(Creature::MOVE_MODE_WALKING)) {
                        // Search the walking creatures
                        pNearCreature = tile.getCreature(Creature::MOVE_MODE_WALKING);
                        if (pNearCreature != NULL) {
                            // Skip the player that just bought the item
                            if (pNearCreature->getObjectID() == pPC->getObjectID())
                                continue;
                            // Send the packet if it is a player.
                            if (pNearCreature->isPC()) {
                                pNearPlayer = pNearCreature->getPlayer();
                                if (pNearPlayer != NULL)
                                    pNearPlayer->sendPacket(&soldpkt);
                            }
                        }
                    }
                    if (tile.hasCreature(Creature::MOVE_MODE_FLYING)) {
                        // Search the flying creatures
                        pNearCreature = tile.getCreature(Creature::MOVE_MODE_FLYING);
                        if (pNearCreature != NULL) {
                            // Skip the player that just bought the item
                            if (pNearCreature->getObjectID() == pPC->getObjectID())
                                continue;
                            // Send the packet if it is a player.
                            if (pNearCreature->isPC()) {
                                pNearPlayer = pNearCreature->getPlayer();
                                if (pNearPlayer != NULL)
                                    pNearPlayer->sendPacket(&soldpkt);
                            }
                        }
                    }
                } // for (ZoneCoord_t zy...) end
            } // for (ZoneCoord_t zx...) end
        } catch (Throwable& t) {
            filelog("shopbug_packet.log", "%s", t.toString().c_str());
        }
    } // if (shopType == SHOP_RACK_NORMAL) else end

    SAFE_DELETE(pTestKey);

#endif

    __END_DEBUG_EX __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Handles buying an event item.
//////////////////////////////////////////////////////////////////////////////
void CGShopRequestBuyHandler::executeEvent(CGShopRequestBuy* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

#ifdef __XMAS_EVENT_CODE__

        // cout << "CGShopRequestBuy::executeChildrenEvent() : BEGIN" << endl;

        ObjectID_t NPCID = pPacket->getObjectID();
    ShopRackType_t shopType = pPacket->getShopType();
    BYTE shopIndex = pPacket->getShopIndex();
    ItemNum_t itemNum = pPacket->getItemNum();
    Coord_t x = pPacket->getX();
    Coord_t y = pPacket->getY();
    GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);
    Creature* pCreature = pGamePlayer->getCreature();
    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
    Zone* pZone = pPC->getZone();
    NPC* pNPC = dynamic_cast<NPC*>(pZone->getCreature(NPCID));

    // NoSuch removed.
    if (pNPC == NULL)
        return;

    Item* pItem = pNPC->getShopItem(shopType, shopIndex);
    Item::ItemClass IClass = pItem->getItemClass();
    ItemType_t IType = pItem->getItemType();
    const list<OptionType_t>& OType = pItem->getOptionTypeList();

    Inventory* pInventory = pPC->getInventory();
    Gold_t playerMoney = pPC->getGold();
    XMAS_STAR star;

    // Get the event price of the event item.
    g_pPriceManager->getStarPrice(pItem, star);

    // cout << "Item to buy:" << endl << pItem->toString() << endl;
    // cout << "Price of the item to buy:" << endl
    //	<< "COLOR:" << star.color << ",AMOUNT:" << star.amount << endl;

    // For a potion or a magazine the item count is set first, and then...
    // In fact event items include no potion or magazine, but...
    switch (pItem->getItemClass()) {
    case Item::ITEM_CLASS_POTION:
        dynamic_cast<Potion*>(pItem)->setNum(itemNum);
        break;
    case Item::ITEM_CLASS_MAGAZINE:
        dynamic_cast<Magazine*>(pItem)->setNum(itemNum);
        break;
    default:
        break;
    }

    // Here it is checked whether enough stars for that price are held.
    if (!pInventory->hasEnoughStar(star)) {
        // cout << "The player does not hold that many stars." << endl;

        GCShopBuyFail gcShopBuyFail;
        gcShopBuyFail.setCode(GC_SHOP_BUY_FAIL_NOT_ENOUGH_MONEY);
        gcShopBuyFail.setAmount(0);
        pPlayer->sendPacket(&gcShopBuyFail);
        return;
    }

    // cout << "The player holds at least that many stars." << endl;

    // In case there is no room...
    if (!pInventory->canAddingEx(x, y, pItem)) {
        // cout << "There is no room in the inventory." << endl;

        GCShopBuyFail gcShopBuyFail;
        gcShopBuyFail.setCode(GC_SHOP_BUY_FAIL_NOT_ENOUGH_SPACE);
        gcShopBuyFail.setAmount(0);
        pPlayer->sendPacket(&gcShopBuyFail);
        return;
    }

    // When the player bought a special item the shop version goes up.
    if (shopType == SHOP_RACK_SPECIAL) {
        pNPC->increaseShopVersion(shopType);
    }

    // Here the player's stars are reduced.
    pInventory->decreaseStar(star);

    // cout << "The player's stars were reduced." << endl;

    Item* pReturnItem = pInventory->addItemEx(x, y, pItem);
    if (pReturnItem == pItem) {
        // pReturnItem equal to pItem means the item was not
        // a stacking item.
        pItem->create(pPC->getName(), STORAGE_INVENTORY, 0, x, y);
        // Calling create alone makes the DB put a count like a potion's at 1.
        // So save has to be called again to set the real count.
        // pItem->save(pPC->getName(), STORAGE_INVENTORY, 0, x, y);

        // item's create was changed to store the count.
        // Item save optimization.

        // Send the OK packet.
        GCShopBuyOK OKPacket;
        OKPacket.setObjectID(NPCID);
        OKPacket.setShopVersion(pNPC->getShopVersion(shopType));
        OKPacket.setItemObjectID(pItem->getObjectID());
        OKPacket.setItemClass(pItem->getItemClass());
        OKPacket.setItemType(pItem->getItemType());
        OKPacket.setOptionType(pItem->getOptionTypeList());
        OKPacket.setDurability(pItem->getDurability());
        OKPacket.setItemNum(pItem->getNum());
        OKPacket.setSilver(pItem->getSilver());
        OKPacket.setGrade(pItem->getGrade());
        OKPacket.setEnchantLevel(pItem->getEnchantLevel());
        OKPacket.setPrice(playerMoney);
        pPlayer->sendPacket(&OKPacket);

        // log(LOG_BUY_ITEM, pPC->getName(), "", pItem->toString());
    } else {
        // log(LOG_BUY_ITEM, pPC->getName(), "", pItem->toString());

        // pReturnItem different from pItem means the item was
        // a stacking item. So the pItem sent in to be added
        // has to be deleted.
        SAFE_DELETE(pItem);
        // pReturnItem->save(pPC->getName(), STORAGE_INVENTORY, 0, x, y);
        //  Item save optimization.
        char pField[80];
        sprintf(pField, "Num=%d", pReturnItem->getNum());
        pReturnItem->tinysave(pField);


        // Send the OK packet.
        GCShopBuyOK OKPacket;
        OKPacket.setObjectID(NPCID);
        OKPacket.setShopVersion(pNPC->getShopVersion(shopType));
        OKPacket.setItemObjectID(pReturnItem->getObjectID());
        OKPacket.setItemClass(pReturnItem->getItemClass());
        OKPacket.setItemType(pReturnItem->getItemType());
        OKPacket.setOptionType(pReturnItem->getOptionTypeList());
        OKPacket.setDurability(pReturnItem->getDurability());
        OKPacket.setItemNum(pReturnItem->getNum());
        OKPacket.setSilver(pReturnItem->getSilver());
        OKPacket.setGrade(pReturnItem->getGrade());
        OKPacket.setEnchantLevel(pReturnItem->getEnchantLevel());
        OKPacket.setPrice(playerMoney);
        pPlayer->sendPacket(&OKPacket);
    }

    pNPC->removeShopItem(shopType, shopIndex);

    if (shopType == SHOP_RACK_NORMAL) {
        // When the sold item is a normal item, create an item of the same type and class.
        Item* pNewItem = g_pItemFactoryManager->createItem(IClass, IType, OType);
        Assert(pNewItem != NULL);
        (pZone->getObjectRegistry()).registerObject(pNewItem);
        pNPC->insertShopItem(shopType, shopIndex, pNewItem);
    } else if (shopType == SHOP_RACK_MYSTERIOUS) {
    } else {
        // When the sold item is not a normal item,
        // the players nearby have to be told that a shop item was sold.
        int CenterX = pNPC->getX();
        int CenterY = pNPC->getY();
        GCShopSold soldpkt;
        soldpkt.setObjectID(NPCID);
        soldpkt.setShopVersion(pNPC->getShopVersion(shopType));
        soldpkt.setShopType(shopType);
        soldpkt.setShopIndex(shopIndex);

        // VSRect rect(0, 0, pZone->getWidth()-1, pZone->getHeight()-1);

        try {
            for (int zx = CenterX - 5; zx <= CenterX + 5; zx++) {
                for (int zy = CenterY - 5; zy <= CenterY + 5; zy++) {
                    // Check that the bounds are not crossed
                    if (!isValidZoneCoord(pZone, zx, zy))
                        continue;

                    Tile& tile = pZone->getTile(zx, zy);

                    // Search the walking creatures
                    if (tile.hasCreature(Creature::MOVE_MODE_WALKING)) {
                        Creature* pNearCreature = tile.getCreature(Creature::MOVE_MODE_WALKING);
                        if (pNearCreature != NULL) {
                            // Skip the player that just bought the item
                            if (pNearCreature->getObjectID() == pPC->getObjectID())
                                continue;
                            // Send the packet if it is a player.
                            if (pNearCreature->isPC()) {
                                Player* pNearPlayer = pNearCreature->getPlayer();
                                if (pNearPlayer != NULL)
                                    pNearPlayer->sendPacket(&soldpkt);
                            }
                        }
                    }
                    // Search the flying creatures
                    if (tile.hasCreature(Creature::MOVE_MODE_FLYING)) {
                        Creature* pNearCreature = tile.getCreature(Creature::MOVE_MODE_FLYING);
                        if (pNearCreature != NULL) {
                            // Skip the player that just bought the item
                            if (pNearCreature->getObjectID() == pPC->getObjectID())
                                continue;
                            // Send the packet if it is a player.
                            if (pNearCreature->isPC()) {
                                Player* pNearPlayer = pNearCreature->getPlayer();
                                if (pNearPlayer != NULL)
                                    pNearPlayer->sendPacket(&soldpkt);
                            }
                        }
                    }
                } // for (ZoneCoord_t zy...) end
            } // for (ZoneCoord_t zx...) end
        } catch (Throwable& t) {
            filelog("shopbug_packet.log", "%s", t.toString().c_str());
        }
    }

    // cout << "CGShopRequestBuy::executeEvent() : END" << endl;

#endif
#endif

    __END_DEBUG_EX __END_CATCH
}
