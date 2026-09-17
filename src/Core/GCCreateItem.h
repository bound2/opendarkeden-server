//////////////////////////////////////////////////////////////////////////////
// Filename    : GCCreateItem.h
// Written By  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_CREATE_ITEM_H__
#define __GC_CREATE_ITEM_H__

#include <list>

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class GCCreateItem;
//////////////////////////////////////////////////////////////////////////////

class GCCreateItem : public Packet {
public:
    GCCreateItem();
    ~GCCreateItem(){};

public:
    // The options an item carries. The count travels in a BYTE and the
    // widest option list an item holds is a code sheet's grid.
    static constexpr uint kMaxOptionCount = MAX_ITEM_OPTION_NUM;

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_CREATE_ITEM;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID +                                  // item object id
               szBYTE +                                      // item class
               szItemType +                                  // item type
               szBYTE + szOptionType * m_OptionType.size() + // item options
               szDurability +                                // item durability
               szSilver +                                    // silver coating
               szGrade +                                     // item grade
               szEnchantLevel +                              // enchant level
               szItemNum +                                   // item count
               szCoordInven +                                // inventory x
               szCoordInven;                                 // inventory y
    }
    string getPacketName() const {
        return "GCCreateItem";
    }
    string toString() const;

public:
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    BYTE getItemClass() const {
        return m_ItemClass;
    }
    void setItemClass(BYTE ItemClass) {
        m_ItemClass = ItemClass;
    }

    ItemType_t getItemType() const {
        return m_ItemType;
    }
    void setItemType(ItemType_t ItemType) {
        m_ItemType = ItemType;
    }

    int getOptionTypeSize() const {
        return m_OptionType.size();
    }
    const list<OptionType_t>& getOptionType() const {
        return m_OptionType;
    }
    OptionType_t popOptionType() {
        if (m_OptionType.empty())
            return 0;
        OptionType_t optionType = m_OptionType.front();
        m_OptionType.pop_front();
        return optionType;
    }
    void addOptionType(OptionType_t OptionType) {
        if (m_OptionType.size() >= kMaxOptionCount)
            throw InvalidProtocolException("too many item options");
        m_OptionType.push_back(OptionType);
    }
    void setOptionType(const list<OptionType_t>& OptionTypes) {
        if (OptionTypes.size() > kMaxOptionCount)
            throw InvalidProtocolException("too many item options");
        m_OptionType = OptionTypes;
    }

    Durability_t getDurability() const {
        return m_Durability;
    }
    void setDurability(Durability_t Durability) {
        m_Durability = Durability;
    }

    Silver_t getSilver() const {
        return m_Silver;
    }
    void setSilver(Silver_t silver) {
        m_Silver = silver;
    }

    Grade_t getGrade() const {
        return m_Grade;
    }
    void setGrade(Grade_t silver) {
        m_Grade = silver;
    }

    EnchantLevel_t getEnchantLevel() const {
        return m_EnchantLevel;
    }
    void setEnchantLevel(EnchantLevel_t level) {
        m_EnchantLevel = level;
    }

    ItemNum_t getItemNum() const {
        return m_ItemNum;
    }
    void setItemNum(ItemNum_t num) {
        m_ItemNum = num;
    }

    CoordInven_t getInvenX() const {
        return m_InvenX;
    }
    void setInvenX(CoordInven_t InvenX) {
        m_InvenX = InvenX;
    }

    CoordInven_t getInvenY() const {
        return m_InvenY;
    }
    void setInvenY(CoordInven_t InvenY) {
        m_InvenY = InvenY;
    }

private:
    ObjectID_t m_ObjectID;           // Object ID
    BYTE m_ItemClass;                // Item class
    ItemType_t m_ItemType;           // Item type
    list<OptionType_t> m_OptionType; // Option type
    Durability_t m_Durability;       // Durability
    Silver_t m_Silver;               // Silver plating amount
    Grade_t m_Grade;                 // Item grade
    EnchantLevel_t m_EnchantLevel;   // Enchant level
    ItemNum_t m_ItemNum;             // Number of items
    CoordInven_t m_InvenX;           // Inventory X coordinate
    CoordInven_t m_InvenY;           // Inventory Y coordinate
};


//////////////////////////////////////////////////////////////////////////////
// class GCCreateItemFactory;
//////////////////////////////////////////////////////////////////////////////

class GCCreateItemFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_CREATE_ITEM;
    static constexpr std::string_view kName = "GCCreateItem";
    static constexpr PacketSize_t kMaxSize{szObjectID +                                            // item object id
                                           szBYTE +                                                // item class
                                           szItemType +                                            // item type
                                           szBYTE + szOptionType * GCCreateItem::kMaxOptionCount + // item options
                                           szDurability +                                          // item durability
                                           szSilver +                                              // silver coating
                                           szGrade +                                               // item grade
                                           szEnchantLevel +                                        // enchant level
                                           szItemNum +                                             // item count
                                           szCoordInven +                                          // inventory x
                                           szCoordInven};                                          // inventory y

    Packet* createPacket() override {
        return new GCCreateItem();
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
