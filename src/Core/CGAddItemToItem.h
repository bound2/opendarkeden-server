//////////////////////////////////////////////////////////////////////////////
// Filename    : CGAddItemToItem.h
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_ADD_ITEM_TO_ITEM_H__
#define __CG_ADD_ITEM_TO_ITEM_H__

#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////////////
// class CGAddItemToItem;
//////////////////////////////////////////////////////////////////////////////

class CGAddItemToItem : public Packet {
public:
    CGAddItemToItem(){};
    ~CGAddItemToItem(){};

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_ADD_ITEM_TO_ITEM;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szCoordInven + szCoordInven;
    }
    string getPacketName() const {
        return "CGAddItemToItem";
    }
    string toString() const;

public:
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    CoordInven_t getX() const {
        return m_X;
    }
    void setX(Coord_t X) {
        m_X = X;
    }

    CoordInven_t getY() const {
        return m_Y;
    }
    void setY(Coord_t Y) {
        m_Y = Y;
    }

private:
    ObjectID_t m_ObjectID; // ObjectID
    CoordInven_t m_X;      // Coord X
    CoordInven_t m_Y;      // Coord Y
};

//////////////////////////////////////////////////////////////////////////////
// class CGAddItemToItemFactory;
//////////////////////////////////////////////////////////////////////////////

class CGAddItemToItemFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_ADD_ITEM_TO_ITEM;
    static constexpr std::string_view kName = "CGAddItemToItem";
    static constexpr PacketSize_t kMaxSize{szObjectID + szCoordInven + szCoordInven};

    Packet* createPacket() override {
        return new CGAddItemToItem();
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

//////////////////////////////////////////////////////////////////////////////
// class CGAddItemToItemHandler;
//////////////////////////////////////////////////////////////////////////////

class CGAddItemToItemHandler {
public:
    static void execute(CGAddItemToItem* pCGAddItemToItem, Player* pPlayer);
};

#endif
