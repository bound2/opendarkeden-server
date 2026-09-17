//////////////////////////////////////////////////////////////////////
//
// Filename    : GCChangeShape.h
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_CHANGE_SHAPE_H__
#define __GC_CHANGE_SHAPE_H__

// include files
#include <list>

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

#define SHAPE_FLAG_QUEST 0x01

//////////////////////////////////////////////////////////////////////
//
// class GCChangeShape;
//
// Packet object used when the game server tells the client that a particular
// user has moved. It holds (CreatureID,X,Y,DIR).
//
//////////////////////////////////////////////////////////////////////

class GCChangeShape : public Packet {
public:
    // constructor
    GCChangeShape();

    // destructor
    ~GCChangeShape();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_CHANGE_SHAPE;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szObjectID + szBYTE + szItemType + szOptionType + szSpeed + szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "GCChangeShape";
    }

    // get packet's debug string
    string toString() const;

    // get Object ID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t objectID) {
        m_ObjectID = objectID;
    }

    // get/set ItemClass
    BYTE getItemClass() const {
        return m_ItemClass;
    }
    void setItemClass(BYTE ItemClass) {
        m_ItemClass = ItemClass;
    }

    // get / set ItemType
    ItemType_t getItemType() const {
        return m_ItemType;
    }
    void setItemType(ItemType_t ItemType) {
        m_ItemType = ItemType;
    }

    // get / set OptionType
    void setOptionType(OptionType_t optionType) {
        m_OptionType = optionType;
    }
    OptionType_t getOptionType() const {
        return m_OptionType;
    }

    // get / set AttackSpeed
    Speed_t getAttackSpeed() const {
        return m_AttackSpeed;
    }
    void setAttackSpeed(Speed_t AttackSpeed) {
        m_AttackSpeed = AttackSpeed;
    }

    BYTE getFlag() const {
        return m_Flag;
    }
    void setFlag(BYTE flag) {
        m_Flag = flag;
    }

private:
    // Creature id
    ObjectID_t m_ObjectID = 0;

    // Item Class
    BYTE m_ItemClass = 0;

    // Item Type
    ItemType_t m_ItemType = 0;

    // Option Type
    OptionType_t m_OptionType = 0;

    // Attack Speed
    Speed_t m_AttackSpeed = 0;

    BYTE m_Flag = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCChangeShapeFactory;
//
// Factory for GCChangeShape
//
//////////////////////////////////////////////////////////////////////

class GCChangeShapeFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_CHANGE_SHAPE;
    static constexpr std::string_view kName = "GCChangeShape";
    static constexpr PacketSize_t kMaxSize{szObjectID + szBYTE + szItemType + szOptionType + szSpeed + szBYTE};

    // constructor
    GCChangeShapeFactory() {}

    // destructor
    virtual ~GCChangeShapeFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCChangeShape();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get Packet Max Size
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
