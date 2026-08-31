//////////////////////////////////////////////////////////////////////////////
// Filename    : GCShopListFill.cpp
// Description : gameserver-side half of GCShopList — setShopItem()
//               converts a live Item into the packet's wire fields, so its
//               definition lives with the game objects, out of the wire
//               library (see src/Core/GCShopList.cpp).
//////////////////////////////////////////////////////////////////////////////

#include "GCShopList.h"

#include "Assert1.h"
#include "Item.h"

void GCShopList::setShopItem(BYTE index, const Item* pItem)

{
    // check bound
    if (index >= SHOP_RACK_INDEX_MAX)
        throw("GCShopList::setShopItem() : Out of Bound!");

    // check pointer
    Assert(pItem != NULL);

    // set shop item info
    m_pBuffer[index].bExist = true;
    m_pBuffer[index].objectID = pItem->getObjectID();
    m_pBuffer[index].itemClass = pItem->getItemClass();
    m_pBuffer[index].itemType = pItem->getItemType();
    m_pBuffer[index].optionType = pItem->getOptionTypeList();
    m_pBuffer[index].durability = pItem->getDurability();
    m_pBuffer[index].silver = pItem->getSilver();
    m_pBuffer[index].grade = pItem->getGrade();
    m_pBuffer[index].enchantLevel = pItem->getEnchantLevel();
}
