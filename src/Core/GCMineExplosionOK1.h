//////////////////////////////////////////////////////////////////////
//
// Filename    : GCMineExplosionOK1.h
// Written By  : elca@ewestsoft.com
// Description : Packet received by the one hit by a skill
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_MINE_EXPLOSION_OK_1_H__
#define __GC_MINE_EXPLOSION_OK_1_H__

// include files
#include "Exception.h"
#include "ModifyInfo.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCMineExplosionOK1;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCMineExplosionOK1 : public ModifyInfo {
public:
    // constructor
    GCMineExplosionOK1();

    // destructor
    ~GCMineExplosionOK1();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_MINE_EXPLOSION_OK_1;
    }

    // The creature list is counted in a BYTE, and the factory max budgets
    // this many ids.
    static constexpr uint kMaxCount = 255;

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return (PacketSize_t)(szCoord * 2 + szDir + szItemType + szBYTE + szObjectID * m_CList.size() +
                              ModifyInfo::getPacketSize());
    }
    // CListNum, SListNum, ListEle* CListNum, ListEle* SListNum* 2

    // get packet's name
    string getPacketName() const {
        return "GCMineExplosionOK1";
    }

    // get packet's debug string
    string toString() const;

    // get / set X
    Coord_t getX() const {
        return m_X;
    }
    void setX(Coord_t X) {
        m_X = X;
    }

    // get / set Y
    Coord_t getY() const {
        return m_Y;
    }
    void setY(Coord_t Y) {
        m_Y = Y;
    }

    // get / set Dir
    Dir_t getDir() const {
        return m_Dir;
    }
    void setDir(Dir_t r) {
        m_Dir = r;
    }

    // get / set ItemType
    ItemType_t getItemType() const {
        return m_ItemType;
    }
    void setItemType(ItemType_t r) {
        m_ItemType = r;
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

    // Clear Creature List
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
    // X, Y
    Coord_t m_X = 0;
    Coord_t m_Y = 0;

    // Dir
    Dir_t m_Dir = 0;

    ItemType_t m_ItemType = 0;

    // Creature List
    list<ObjectID_t> m_CList;
};


//////////////////////////////////////////////////////////////////////
//
// class GCMineExplosionOK1Factory;
//
// Factory for GCMineExplosionOK1
//
//////////////////////////////////////////////////////////////////////

class GCMineExplosionOK1Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MINE_EXPLOSION_OK_1;
    static constexpr std::string_view kName = "GCMineExplosionOK1";
    static constexpr PacketSize_t kMaxSize{szCoord * 2 + szDir + szItemType + szBYTE + szWORD +
                                           szObjectID * GCMineExplosionOK1::kMaxCount + ModifyInfo::getPacketMaxSize()};

    // constructor
    GCMineExplosionOK1Factory() {}

    // destructor
    virtual ~GCMineExplosionOK1Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCMineExplosionOK1();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get Pakcet Max Size
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
