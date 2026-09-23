//////////////////////////////////////////////////////////////////////////////
// Filename    : ItemRack.cpp
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "ItemRack.h"

#include "Assert.h"
#include "GameContext.h"
#include "Item.h"
#include "ParkingCenter.h"
#include "item/Key.h"

//////////////////////////////////////////////////////////////////////////////
// class ItemRack member methods
//////////////////////////////////////////////////////////////////////////////

ItemRack::ItemRack() {
    m_ppItem = NULL;
    m_nSize = 0;
}

ItemRack::ItemRack(int size) {
    // Allocate the pointer array.
    m_ppItem = new Item*[size];
    Assert(m_ppItem != NULL);

    // Initialize the pointer array.
    for (int i = 0; i < size; i++)
        m_ppItem[i] = NULL;

    m_nSize = size;
}

ItemRack::~ItemRack() {
    if (m_ppItem != NULL) {
        for (int i = 0; i < m_nSize; i++) {
            Item* pItem = m_ppItem[i];

            if (pItem != NULL && pItem->getItemClass() == Item::ITEM_CLASS_KEY) {
                Key* pKey = dynamic_cast<Key*>(pItem);
                // Simply remove it from the zone here.
                if (de::gameContext().parking().hasMotorcycleBox(pKey->getTarget())) {
                    de::gameContext().parking().deleteMotorcycleBox(pKey->getTarget());
                }
            }

            SAFE_DELETE(m_ppItem[i]);
        }

        SAFE_DELETE_ARRAY(m_ppItem);
    }
}

void ItemRack::init(int size) {
    // Clear the previous contents first.
    if (m_ppItem != NULL) {
        for (int i = 0; i < m_nSize; i++)
            SAFE_DELETE(m_ppItem[i]);

        SAFE_DELETE_ARRAY(m_ppItem);
    }

    // Allocate the pointer array.
    m_ppItem = new Item*[size];
    Assert(m_ppItem != NULL);

    // Initialize the pointer array.
    for (int i = 0; i < size; i++)
        m_ppItem[i] = NULL;

    m_nSize = size;
}

bool ItemRack::isFull(void) const {
    // If even one slot is empty it is not full.
    for (int i = 0; i < m_nSize; i++)
        if (m_ppItem[i] == NULL)
            return false;

    return true;
}

bool ItemRack::isEmpty(void) const {
    // If even one slot holds an item it is not empty.
    for (int i = 0; i < m_nSize; i++)
        if (m_ppItem[i] != NULL)
            return false;

    return true;
}

bool ItemRack::isExist(BYTE index) const {
    // Verify the index.
    Assert(verifyIndex(index));

    // False if there is no item, true if there is.
    return (m_ppItem[index] == NULL ? false : true);
}

void ItemRack::insert(BYTE index, Item* pItem) {
    // Verify the index.
    Assert(verifyIndex(index));

    // Check first that no item is already there.
    Assert(m_ppItem[index] == NULL);

    // Put it in.
    m_ppItem[index] = pItem;
}

void ItemRack::remove(BYTE index) {
    // Verify the index.
    Assert(verifyIndex(index));

    // Clear the pointer.
    m_ppItem[index] = NULL;
}

Item* ItemRack::get(BYTE index) {
    // Verify the index.
    Assert(verifyIndex(index));

    return m_ppItem[index];
}

void ItemRack::clear(void) {
    for (int i = 0; i < m_nSize; i++)
        SAFE_DELETE(m_ppItem[i]);
}

BYTE ItemRack::getFirstEmptySlot(void) const {
    // Search from the front.
    for (int i = 0; i < m_nSize; i++)
        if (m_ppItem[i] == NULL)
            return i;

    return m_nSize;
}

BYTE ItemRack::getLastEmptySlot(void) const {
    // Search from the back.
    for (int i = m_nSize - 1; i >= 0; i--)
        if (m_ppItem[i] == NULL)
            return i;

    return m_nSize;
}

bool ItemRack::verifyIndex(BYTE index) const {
    if (index >= m_nSize)
        return false;
    return true;
}
