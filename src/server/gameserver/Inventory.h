//////////////////////////////////////////////////////////////////////////////
// Filename    : Inventory.h
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __INVENTORY_H__
#define __INVENTORY_H__

#include <list>

#include "InventorySlot.h"
#include "Item.h"
#include "Types.h"
#include "VolumeInfo.h"

class PetItem;

#define MAX_INVEN_WIDTH 10
#define MAX_INVEN_HEIGHT 6

//////////////////////////////////////////////////////////////////////////////
// Class Inventory
//////////////////////////////////////////////////////////////////////////////

class Inventory {
    // constructor & destructor
public:
    Inventory(CoordInven_t X, CoordInven_t Y, bool bDeleteAll = true);
    Inventory(const Inventory* pInventory);
    ~Inventory();

    // identity methods
public:
    string getOwner(void) const {
        return m_Owner;
    }
    void setOwner(const string& owner) {
        m_Owner = owner;
    }

    CoordInven_t getWidth() const {
        return m_Width;
    }
    CoordInven_t getHeight() const {
        return m_Height;
    }

    ItemNum_t getItemNum() const {
        return m_TotalNum;
    }
    void increaseNum() {
        m_TotalNum++;
    }
    void increaseNum(ItemNum_t ItemNum) {
        m_TotalNum += ItemNum;
    }
    void decreaseItemNum() {
        m_TotalNum--;
    }
    void decreaseItemNum(ItemNum_t ItemNum) {
        m_TotalNum -= ItemNum;
    }
    void setItemNum(ItemNum_t ItemNum) {
        m_TotalNum = ItemNum;
    }

    Weight_t getWeight() const {
        return m_TotalWeight;
    }
    void increaseWeight(Weight_t Weight) {
        m_TotalWeight += Weight;
    }
    void decreaseWeight(Weight_t Weight) {
        m_TotalWeight -= Weight;
    }

    bool getDeleteAllFlag(void) const {
        return m_bDeleteAll;
    }
    void setDeleteAllFlag(bool value) {
        m_bDeleteAll = value;
    }

    // check methods
public:
    bool hasItem(CoordInven_t X, CoordInven_t Y);
    bool hasItem(ObjectID_t ObjectID);
    bool hasItemWithItemID(ItemID_t ItemID);
    bool hasKey(ItemID_t TargetItemID);

    bool canAdding(CoordInven_t X, CoordInven_t Y, Item* pItem);
    bool canAddingEx(CoordInven_t X, CoordInven_t Y, Item* pItem);

    // addition & deletion related methods
public:
    bool addItem(CoordInven_t X, CoordInven_t Y, Item* pItem);
    Item* addItemEx(CoordInven_t X, CoordInven_t Y, Item* pItem);
    bool addItem(Item* pItem);
    bool addItem(Item* pItem, TPOINT& pt);
    bool getEmptySlot(Item* pItem, _TPOINT& p) {
        Assert(pItem != NULL);
        return getEmptySlot(pItem->getVolumeWidth(), pItem->getVolumeHeight(), p);
    }
    bool getEmptySlot(VolumeWidth_t ItemWidth, VolumeHeight_t ItemHeight, _TPOINT& p);
    void deleteItem(ObjectID_t ObjectID);
    void deleteItem(CoordInven_t X, CoordInven_t Y);

    // find methods
public:
    Item* searchItem(CoordInven_t X, CoordInven_t Y, Item* pItem, TPOINT& pt);

    Item* getItemWithItemID(ItemID_t itemID);
    Item* getBeltWithItemID(ItemID_t itemID);
    Item* getItemWithObjectID(ObjectID_t objectID);

    Item* findItemOID(ObjectID_t objectID, CoordInven_t& X, CoordInven_t& Y);
    Item* findItemIID(ItemID_t ItemID, CoordInven_t& X, CoordInven_t& Y);

    Item* findItemOID(ObjectID_t objectID, Item::ItemClass IClass, CoordInven_t& X, CoordInven_t& Y);
    Item* findItemIID(ItemID_t ItemID, Item::ItemClass IClass, CoordInven_t& X, CoordInven_t& Y);

    Item* findItem(Item::ItemClass IClass, ItemType_t itemType = 0xFFFF);
    Item* findItem(Item::ItemClass IClass, ItemType_t itemType, CoordInven_t& X, CoordInven_t& Y);

    // item manipulation related methods
public:
    Item* getItem(CoordInven_t X, CoordInven_t Y) const;

    InventorySlot& getInventorySlot(CoordInven_t X, CoordInven_t Y) const {
        return m_pInventorySlot[X][Y];
    }

protected:
    void setItem(CoordInven_t X, CoordInven_t Y, Item* pItem);

    // packing related methods
public:
    void clear();
    list<Item*> getList() const;
    int calc2x2Item(void) const;

    // misc methods
public:
    void save(const string& owner);
    string toString() const;


    // #ifdef __XMAS_EVENT_CODE__
    /////////////////////////////////////////////////////////////////////////
    // Functions for the 2001 Christmas event.
    // Reused for the 2002 Children's Day star event (GiftBox is not used).
    // Renaming XMAS_STAR to STAR_EVENT should be considered for further reuse.
public:
    bool hasEnoughStar(const XMAS_STAR& star);
    void decreaseStar(const XMAS_STAR& star);
    bool hasRedGiftBox(void);
    bool hasGreenGiftBox(void);
    // #endif

public:
    void clearQuestItem(list<Item*>& iList);

    ////////////////////////////////////////
    // member data
    ////////////////////////////////////////
private:
    string m_Owner;                   // Inventory owner
    CoordInven_t m_Width;             // Inventory Width
    CoordInven_t m_Height;            // Inventory Height
    ItemNum_t m_TotalNum;             // Total Count
    Weight_t m_TotalWeight;           // Total Weight
    InventorySlot** m_pInventorySlot; // Two-dimensional array of InventorySlot.
    bool m_bDeleteAll;                // Delete the items too when the object is deleted.
};

#endif // __INVENTORY_H__
