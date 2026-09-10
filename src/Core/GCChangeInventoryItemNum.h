//////////////////////////////////////////////////////////////////////
//
// Filename    : GCChangeInventoryItemNum.h
// Written By  : elca@ewestsoft.com
// Description : The materials a craft consumed, as ids and new counts.
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_CHANGE_INVENTORY_ITEM_NUM_H__
#define __GC_CHANGE_INVENTORY_ITEM_NUM_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCChangeInventoryItemNum;
//
// The changed-material record GCMakeItemOK and GCMakeItemFail carry.
//
//////////////////////////////////////////////////////////////////////

class GCChangeInventoryItemNum {
public:
    // constructor
    GCChangeInventoryItemNum();

    // destructor
    ~GCChangeInventoryItemNum();

public:
    // The entries the count byte in front of the two runs can describe.
    static constexpr uint kMaxCount = 255;

    // What the record occupies with a full list.
    static constexpr PacketSize_t getPacketMaxSize() {
        return szBYTE + kMaxCount * (szObjectID + szItemNum);
    }

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;

    PacketSize_t getPacketSize() const {
        return szBYTE + (szObjectID + szItemNum) * m_ChangedItemList.size();
    }

    // get packet's debug string
    string toString() const;

    // The count is the list.
    BYTE getChangedItemListNum() const {
        return (BYTE)m_ChangedItemList.size();
    }

    // add one (item, new count) pair
    void addChangedItemListElement(ObjectID_t objectID, ItemNum_t itemNum);

    // ClearList
    void clearChangedItemList() {
        m_ChangedItemList.clear();
        m_ChangedItemNumList.clear();
    }

    // pop front Element in Object List
    ObjectID_t popFrontChangedItemListElement() {
        if (m_ChangedItemList.empty())
            throw InvalidProtocolException("no changed item left");
        ObjectID_t item = m_ChangedItemList.front();
        m_ChangedItemList.pop_front();
        return item;
    }
    ItemNum_t popFrontChangedItemNumListElement() {
        if (m_ChangedItemNumList.empty())
            throw InvalidProtocolException("no changed item count left");
        ItemNum_t itemNum = m_ChangedItemNumList.front();
        m_ChangedItemNumList.pop_front();
        return itemNum;
    }

protected:
    // The inventory items whose count changed, and what it changed to.
    list<ObjectID_t> m_ChangedItemList;
    list<ItemNum_t> m_ChangedItemNumList;
};

#endif
