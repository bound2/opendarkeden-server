//////////////////////////////////////////////////////////////////////
//
// Filename    : CGUsePotionFromQuickSlot.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_USE_POTION_FROM_QUICKSLOT_H__
#define __CG_USE_POTION_FROM_QUICKSLOT_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class CGUsePotionFromQuickSlot;
//
//////////////////////////////////////////////////////////////////////

class CGUsePotionFromQuickSlot : public Packet {
public:
    // constructor
    CGUsePotionFromQuickSlot();

    // destructor
    ~CGUsePotionFromQuickSlot();

public:
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_USE_POTION_FROM_QUICKSLOT;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // const static CGUsePotionFromQuickSlotPacketSize 를 정의해서 리턴하라.
    PacketSize_t getPacketSize() const {
        return szObjectID + szSlotID;
    }

    // get packet name
    string getPacketName() const {
        return "CGUsePotionFromQuickSlot";
    }

    // get packet's debug string
    string toString() const;

public:
    // get / set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    // get / set QuickSlotID
    SlotID_t getSlotID() const {
        return m_SlotID;
    }
    void setSlotID(SlotID_t SlotID) {
        m_SlotID = SlotID;
    }


private:
    // ObjectID
    ObjectID_t m_ObjectID;

    // QuickSlot의 ID
    SlotID_t m_SlotID;
};


//////////////////////////////////////////////////////////////////////
//
// class CGUsePotionFromQuickSlotFactory;
//
// Factory for CGUsePotionFromQuickSlot
//
//////////////////////////////////////////////////////////////////////

class CGUsePotionFromQuickSlotFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_USE_POTION_FROM_QUICKSLOT;
    static constexpr std::string_view kName = "CGUsePotionFromQuickSlot";
    static constexpr PacketSize_t kMaxSize{szObjectID + szSlotID};

    // create packet
    Packet* createPacket() override {
        return new CGUsePotionFromQuickSlot();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's max body size
    // *OPTIMIZATION HINT*
    // const static CGUsePotionFromQuickSlotPacketSize 를 정의해서 리턴하라.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGUsePotionFromQuickSlotHandler;
//
//////////////////////////////////////////////////////////////////////

class CGUsePotionFromQuickSlotHandler {
public:
    // execute packet's handler
    static void execute(CGUsePotionFromQuickSlot* pPacket, Player* player);
};

#endif
