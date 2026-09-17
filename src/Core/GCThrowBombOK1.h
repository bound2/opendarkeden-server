//////////////////////////////////////////////////////////////////////
//
// Filename    : GCThrowBombOK1.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
// 				Packet sent to the skill user
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_THROW_BOMB_OK_1_H__
#define __GC_THROW_BOMB_OK_1_H__


// include files
#include "Exception.h"
#include "ModifyInfo.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCThrowBombOK1;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCThrowBombOK1 : public ModifyInfo {
public:
    // constructor
    GCThrowBombOK1();

    // destructor
    ~GCThrowBombOK1();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_THROW_BOMB_OK_1;
    }

    // The creature list is counted in a BYTE, and the factory max budgets
    // this many ids.
    static constexpr uint kMaxCount = 255;

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return (PacketSize_t)(szCoord * 2 + szBYTE + szDir + szItemType + szObjectID * m_CList.size() +
                              ModifyInfo::getPacketSize());
    }

    // get packet's name
    string getPacketName() const {
        return "GCThrowBombOK1";
    }

    // get packet's debug string
    string toString() const;

    // get / set X, Y
    Coord_t getX() {
        return m_X;
    }
    void setX(Coord_t X) {
        m_X = X;
    }

    Coord_t getY() {
        return m_Y;
    }
    void setY(Coord_t Y) {
        m_Y = Y;
    }

    Dir_t getDir() {
        return m_Dir;
    }
    void setDir(Dir_t r) {
        m_Dir = r;
    }

    ItemType_t getItemType() const {
        return m_ItemType;
    }
    void setItemType(ItemType_t i) {
        m_ItemType = i;
    }


    void setXYDir(Coord_t X, Coord_t Y, Dir_t R) {
        m_X = X;
        m_Y = Y;
        m_Dir = R;
    }

    // get Creature List Number
    BYTE getCListNum() const {
        return (BYTE)m_CList.size();
    }

    // add / delete  Creature List
    void addCListElement(ObjectID_t ObjectID);
    void clearCList() {
        m_CList.clear();
    }

    // pop front Element in Status List
    ObjectID_t popCListElement() {
        if (m_CList.empty())
            throw InvalidProtocolException("no creature left");
        ObjectID_t CreatureList = m_CList.front();
        m_CList.pop_front();
        return CreatureList;
    }

private:
    // Dir
    Dir_t m_Dir = 0;
    ItemType_t m_ItemType = 0;

    // X, Y Position
    Coord_t m_X = 0;
    Coord_t m_Y = 0;

    // Creature List
    list<ObjectID_t> m_CList;
};


//////////////////////////////////////////////////////////////////////
//
// class GCThrowBombOK1Factory;
//
// Factory for GCThrowBombOK1
//
//////////////////////////////////////////////////////////////////////

class GCThrowBombOK1Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_THROW_BOMB_OK_1;
    static constexpr std::string_view kName = "GCThrowBombOK1";
    static constexpr PacketSize_t kMaxSize{szCoord * 2 + szBYTE + szBYTE + +szDir + szItemType + szWORD +
                                           szObjectID * GCThrowBombOK1::kMaxCount + ModifyInfo::getPacketMaxSize()};

    // constructor
    GCThrowBombOK1Factory() {}

    // destructor
    virtual ~GCThrowBombOK1Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCThrowBombOK1();
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
#endif
