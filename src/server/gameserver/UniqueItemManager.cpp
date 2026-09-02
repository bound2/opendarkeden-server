//////////////////////////////////////////////////////////////////////////////
// Filename    : UniqueItemManager.cpp
// Written By  : suigui
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "UniqueItemManager.h"

#include "Item.h"
#include "ItemInfoManager.h"
#include "repository/ItemRepository.h"


UniqueItemManager* g_pUniqueItemManager = NULL;

//----------------------------------------------------------------------
// init
//----------------------------------------------------------------------
void UniqueItemManager::init()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    // Read the current rows and mark each (itemClass, itemType) unique.
    vector<UniqueItemRow> rows = defaultItemRepository().loadUniqueItems();

    for (size_t r = 0; r < rows.size(); r++) {
        Item::ItemClass itemClass = (Item::ItemClass)rows[r].itemClass;
        int itemType = rows[r].itemType;

        ItemInfo* pItemInfo = g_pItemInfoManager->getItemInfo(itemClass, itemType);
        Assert(pItemInfo != NULL);

        pItemInfo->setUnique();
    }

    __END_CATCH
    __END_DEBUG
}

//----------------------------------------------------------------------
// is Possible Create
//----------------------------------------------------------------------
bool UniqueItemManager::isPossibleCreate(Item::ItemClass itemClass, ItemType_t itemType)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    // Read the current numbers; an item without a row falls through to the
    // false below, as before.
    int limitNumber = 0;
    int currentNumber = 0;

    if (defaultItemRepository().loadUniqueItemNumbers((int)itemClass, (int)itemType, limitNumber, currentNumber)) {
        return limitNumber == 0 || currentNumber < limitNumber;
    }

    __END_CATCH
    __END_DEBUG

    return false;
}

//----------------------------------------------------------------------
// createItem
//----------------------------------------------------------------------
// DB에서 개수 증가
//----------------------------------------------------------------------
void UniqueItemManager::createItem(Item::ItemClass itemClass, ItemType_t itemType)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    defaultItemRepository().incrementUniqueItemCount((int)itemClass, (int)itemType);

    __END_CATCH
    __END_DEBUG
}

//----------------------------------------------------------------------
// deleteItem
//----------------------------------------------------------------------
// DB에서 개수 증가
//----------------------------------------------------------------------
void UniqueItemManager::deleteItem(Item::ItemClass itemClass, ItemType_t itemType)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    defaultItemRepository().decrementUniqueItemCount((int)itemClass, (int)itemType);

    __END_CATCH
    __END_DEBUG
}
