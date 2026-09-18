//////////////////////////////////////////////////////////////////////////
// Filename			: GoodsInventory.h
// Written By		: bezz
// Description		: Inventory holding the items purchased on the website
//////////////////////////////////////////////////////////////////////////

#ifndef __GOODS_INVENTORY_H__
#define __GOODS_INVENTORY_H__

#include <list>

#include "Item.h"
#include "Types.h"

typedef struct {
    string m_ID;
    Item* m_pItem;
} BuyItem;

//////////////////////////////////////////////////////////////
// Class GoodsInventory
//////////////////////////////////////////////////////////////

class GoodsInventory {
public:
    typedef list<BuyItem> ListItem;
    typedef ListItem::iterator ListItemItr;
    typedef ListItem::const_iterator ListItemConstItr;

public:
    GoodsInventory(){};
    ~GoodsInventory();

public:
    ListItem& getGoods() {
        return m_Goods;
    }

    // Add an item
    void addItem(string ID, Item* pItem);

    // Take an item out
    Item* popItem(ObjectID_t oid);

    // Just look at an item
    Item* getItem(ObjectID_t oid);

    // Is the inventory empty?
    bool empty() {
        return m_Goods.empty();
    }

    void clear();

    // Number of items in the inventory
    int getNum() const {
        return m_Goods.size();
    }

private:
    ListItem m_Goods;
};

#endif
