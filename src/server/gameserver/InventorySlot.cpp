//////////////////////////////////////////////////////////////////////////////
// Filename    : InventorySlot.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "InventorySlot.h"

//////////////////////////////////////////////////////////////////////////////
// class InventorySlot member methods
//////////////////////////////////////////////////////////////////////////////

InventorySlot::~InventorySlot()

{
    __BEGIN_TRY

    Assert(m_pItem == NULL);

    __END_CATCH_NO_RETHROW
}

void InventorySlot::addItem(Item* pItem)

{
    __BEGIN_TRY

    Assert(pItem != NULL);
    m_pItem = pItem;

    __END_CATCH
}

void InventorySlot::deleteItem()

{
    __BEGIN_TRY

    Assert(m_pItem != NULL);
    m_pItem = NULL;

    __END_CATCH
}
