//////////////////////////////////////////////////////////////////////
//
// Filename    : CGDisplayItem.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_DISPLAY_ITEM_H__
#define __CG_DISPLAY_ITEM_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class CGDisplayItem;
//
//////////////////////////////////////////////////////////////////////

class CGDisplayItem : public Packet {
public:
    CGDisplayItem(){};
    ~CGDisplayItem(){};
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_DISPLAY_ITEM;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szCoordInven + szCoordInven + szObjectID + szGold + szBYTE;
    }

    // get packet name
    string getPacketName() const {
        return "CGDisplayItem";
    }

    // get packet's debug string
    string toString() const;

    CoordInven_t getX() const {
        return m_X;
    }
    CoordInven_t getY() const {
        return m_Y;
    }
    void setXY(CoordInven_t x, CoordInven_t y) {
        m_X = x;
        m_Y = y;
    }

    ObjectID_t getItemObjectID() const {
        return m_ItemObjectID;
    }
    void setItemObjectID(ObjectID_t oid) {
        m_ItemObjectID = oid;
    }

    Gold_t getPrice() const {
        return m_Price;
    }
    void setPrice(Gold_t price) {
        m_Price = price;
    }

    BYTE getIndex() const {
        return m_Index;
    }
    void setIndex(BYTE index) {
        m_Index = index;
    }

private:
    CoordInven_t m_X, m_Y;
    ObjectID_t m_ItemObjectID;
    Gold_t m_Price;
    BYTE m_Index;
};


//////////////////////////////////////////////////////////////////////
//
// class CGDisplayItemFactory;
//
// Factory for CGDisplayItem
//
//////////////////////////////////////////////////////////////////////

class CGDisplayItemFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_DISPLAY_ITEM;
    static constexpr std::string_view kName = "CGDisplayItem";
    static constexpr PacketSize_t kMaxSize{szCoordInven + szCoordInven + szObjectID + szGold + szBYTE};

    // constructor
    CGDisplayItemFactory() {}

    // destructor
    virtual ~CGDisplayItemFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGDisplayItem();
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
// class CGDisplayItemHandler;
//
//////////////////////////////////////////////////////////////////////

class CGDisplayItemHandler {
public:
    // execute packet's handler
    static void execute(CGDisplayItem* pCGDisplayItem, Player* pPlayer);
};

#endif
