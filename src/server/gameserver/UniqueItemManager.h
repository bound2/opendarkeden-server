//////////////////////////////////////////////////////////////////////////////
// Filename    : UniqueItemInfo.h
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __UNIQUE_ITEM_MANAGER_H__
#define __UNIQUE_ITEM_MANAGER_H__

#include "Item.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class UniqueItemInfo
//////////////////////////////////////////////////////////////////////////////

class UniqueItemManager {
public:
    void init();

    // Create one unique item.
    // Item* getRandomUniqueitem() ;

    // Can the item be created? (item count limit)
    static bool isPossibleCreate(Item::ItemClass itemClass, ItemType_t itemType);

    // An item was created. (updates the count)
    static void createItem(Item::ItemClass itemClass, ItemType_t itemType);

    // An item was deleted. (updates the count)
    static void deleteItem(Item::ItemClass itemClass, ItemType_t itemType);

private:
    // Ratio_t 	m_TotalUniqueItemRatio;
    // Ratio_t*	m_Ratios[Item::ITEM_CLASS_MAX];
};

#endif // __UNIQUE_ITEM_MANAGER_H__
