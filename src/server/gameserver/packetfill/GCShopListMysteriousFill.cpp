//////////////////////////////////////////////////////////////////////////////
// Filename    : GCShopListMysteriousFill.cpp
// Description : gameserver-side half of GCShopListMysterious —
//               setShopItem() converts a live Item into the packet's wire
//               fields, so its definition lives with the game objects, out
//               of the wire library (see src/Core/GCShopListMysterious.cpp).
//////////////////////////////////////////////////////////////////////////////

#include "Assert1.h"
#include "GCShopListMysterious.h"
#include "Item.h"

void GCShopListMysterious::setShopItem(BYTE index, const Item* pItem)

{
    // check bound
    if (index >= SHOP_RACK_INDEX_MAX)
        throw "GCShopListMysterious::setShopItem() : Out of Bound!";

    // check pointer
    Assert(pItem != NULL);

    // set shop item info
    m_pBuffer[index].bExist = true;
    m_pBuffer[index].itemClass = pItem->getItemClass();
    m_pBuffer[index].itemType = pItem->getItemType();
}
