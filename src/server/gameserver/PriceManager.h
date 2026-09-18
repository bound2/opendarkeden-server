//////////////////////////////////////////////////////////////////////////////
// Filename    : PriceManager.h
// Description :
// Decides the price at which an item is bought or sold in a shop.
// Internally it computes from the base price in ItemInfoManager.
//////////////////////////////////////////////////////////////////////////////

#ifndef __PRICEMANAGER_H__
#define __PRICEMANAGER_H__

#include "Exception.h"
#include "Item.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class PriceManager
//////////////////////////////////////////////////////////////////////////////

// class Item;
class Creature;

class PriceManager {
public:
    // Determine the actual price from the item info.
    Price_t getPrice(Item* pItem, MarketCond_t nDiscount, ShopRackType_t shopType, Creature* pCreature) const;

    // Price of a Mysterious Item
    Price_t getMysteriousPrice(Item::ItemClass itemClass, Creature* pCreature) const;

    // Cost of repairing an item
    Price_t getRepairPrice(Item* pItem, Creature* pCreature = NULL) const;

    // Cost of silver coating an item
    Price_t getSilverCoatingPrice(Item* pItem, Creature* pCreature = NULL) const;

    // Price of a stash
    Price_t getStashPrice(BYTE index, Creature* pCreature = NULL) const;

    // Price functions for events
    int getStarPrice(Item* pItem, XMAS_STAR& star) const;
    int getBallPrice(int price, XMAS_STAR& star) const;
};

// global variable declaration
extern PriceManager* g_pPriceManager;

#endif
