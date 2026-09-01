//////////////////////////////////////////////////////////////////////////
// Filename			: GoodsInventory.cpp
// Written By		: bezz
// Description		: Inventory holding the items purchased on the website
//////////////////////////////////////////////////////////////////////////

#include "GoodsInventory.h"

#include "repository/GoodsRepository.h"


GoodsInventory::~GoodsInventory()

{
    clear();
}

void GoodsInventory::clear()

{
    __BEGIN_TRY

    ListItemItr itr = m_Goods.begin();
    for (; itr != m_Goods.end(); itr++) {
        SAFE_DELETE((*itr).m_pItem);
    }

    m_Goods.clear();

    __END_CATCH
}

void GoodsInventory::addItem(string ID, Item* pItem)

{
    __BEGIN_TRY

    BuyItem buyItem;

    buyItem.m_ID = ID;
    buyItem.m_pItem = pItem;

    m_Goods.push_back(buyItem);

    __END_CATCH
}

Item* GoodsInventory::getItem(ObjectID_t oid)

{
    __BEGIN_TRY

    if (m_Goods.empty())
        return NULL;

    Item* pItem = NULL;

    ListItemItr itr = m_Goods.begin();

    for (; itr != m_Goods.end(); itr++) {
        if ((*itr).m_pItem->getObjectID() == oid) {
            pItem = (*itr).m_pItem;
            break;
        }
    }

    return pItem;

    __END_CATCH
}
Item* GoodsInventory::popItem(ObjectID_t oid)

{
    __BEGIN_TRY

    if (m_Goods.empty())
        return NULL;

    Item* pItem = NULL;

    ListItemItr itr = m_Goods.begin();

    for (; itr != m_Goods.end(); itr++) {
        if ((*itr).m_pItem->getObjectID() == oid) {
            pItem = (*itr).m_pItem;

            filelog("Goods.log", "The item was picked up. : [%s:%s]", (*itr).m_ID.c_str(),
                    (*itr).m_pItem->toString().c_str());

            if (!defaultGoodsRepository().takeOne((*itr).m_ID)) {
                filelog("Goods.log", "But the DB was not updated. : %s", (*itr).m_ID.c_str());
            }

            m_Goods.erase(itr);
            break;
        }
    }

    return pItem;

    __END_CATCH
}
