//////////////////////////////////////////////////////////////////////////////
// Filename    : GCStashList.h
// Written By  : 김성민
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_STASH_LIST_H__
#define __GC_STASH_LIST_H__

#include <list>

#include "Packet.h"
#include "PacketFactory.h"
#include "SubItemInfo.h"

typedef struct _STASHITEM {
    // The options one slot's item carries. The count travels in a BYTE
    // and the widest option list an item holds is a code sheet's grid.
    static constexpr uint kMaxOptionCount = MAX_ITEM_OPTION_NUM;

    int getPacketSize() const {
        return szObjectID + szBYTE + szItemType + szBYTE + szOptionType * optionType.size() + szDurability + szItemNum +
               szSilver + szGrade + szEnchantLevel;
    }

    static constexpr int getPacketMaxSize() {
        return szObjectID + szBYTE + szItemType + szBYTE + szOptionType * kMaxOptionCount + szDurability + szItemNum +
               szSilver + szGrade + szEnchantLevel;
    }

    ObjectID_t objectID = 0;
    BYTE itemClass = 0;
    ItemType_t itemType = 0;
    list<OptionType_t> optionType;
    Durability_t durability = 0;
    ItemNum_t num = 0;
    Silver_t silver = 0;
    Grade_t grade = 0;
    EnchantLevel_t enchantLevel = 0;
} STASHITEM;

//////////////////////////////////////////////////////////////////////////////
// class GCStashList;
//////////////////////////////////////////////////////////////////////////////

class Item;

class GCStashList : public Packet {
public:
    // The items a belt or an armsband keeps in one stash slot. The
    // widest pocket count an item of either class has is this many, and
    // the factory max budgets that many records per slot.
    static constexpr uint kMaxSubItemCount = 8;

    GCStashList();
    virtual ~GCStashList();

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_STASH_LIST;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCStashList";
    }
    string toString() const;

public:
    bool isExist(BYTE rack, BYTE index) const;

    STASHITEM getStashItem(BYTE rack, BYTE index) const;
    void setStashItem(BYTE rack, BYTE index, Item* pItem);

    list<SubItemInfo*>& getSubItems(BYTE rack, BYTE index);

    // The count on the wire, which is the list itself.
    BYTE getSubItemCount(BYTE rack, BYTE index) const;

    Gold_t getStashGold() const {
        return m_StashGold;
    }
    void setStashGold(Gold_t gold) {
        m_StashGold = gold;
    }

    BYTE getStashNum() const {
        return m_StashNum;
    }
    void setStashNum(BYTE num) {
        m_StashNum = num;
    }

private:
    bool m_bExist[STASH_RACK_MAX][STASH_INDEX_MAX] = {};
    STASHITEM m_pItems[STASH_RACK_MAX][STASH_INDEX_MAX];
    list<SubItemInfo*> m_pSubItems[STASH_RACK_MAX][STASH_INDEX_MAX];
    Gold_t m_StashGold = 0;
    BYTE m_StashNum = 0;
};


//////////////////////////////////////////////////////////////////////////////
// class GCStashListFactory;
//////////////////////////////////////////////////////////////////////////////

class GCStashListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_STASH_LIST;
    static constexpr std::string_view kName = "GCStashList";
    static constexpr PacketSize_t kMaxSize{[] {
        PacketSize_t size = 0;
        PacketSize_t unit_size = szBYTE * 2 +                    // rack and index
                                 STASHITEM::getPacketMaxSize() + // the item itself
                                 szBYTE +                        // how many items the belt holds
                                 SubItemInfo::getSize() * GCStashList::kMaxSubItemCount; // the belt's items

        size += szBYTE;                                       // number of racks
        size += szBYTE;                                       // total number of items
        size += unit_size * STASH_RACK_MAX * STASH_INDEX_MAX; // every slot occupied
        size += szGold;                                       // money

        return size;
    }()};

    Packet* createPacket() override {
        return new GCStashList();
    }
    string getPacketName() const override {
        return string(kName);
    }
    PacketID_t getPacketID() const override {
        return kPacketID;
    }
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


#endif
