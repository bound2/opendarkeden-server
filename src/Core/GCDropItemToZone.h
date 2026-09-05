//////////////////////////////////////////////////////////////////////
//
// Filename    : GCDropItemToZone.h
// Written By  : elca
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_DROP_ITEM_TO_ZONE_H__
#define __GC_DROP_ITEM_TO_ZONE_H__

// include files
#include "GCAddItemToZone.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "SubItemInfo.h"


//////////////////////////////////////////////////////////////////////
//
// class GCDropItemToZone;
//
////////////////////////////////////////////////////////////////////

class GCDropItemToZone : public GCAddItemToZone {
public:
    GCDropItemToZone();
    ~GCDropItemToZone();

    PacketSize_t getPacketSize() const {
        return GCAddItemToZone::getPacketSize() + szObjectID;
    }

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_DROP_ITEM_TO_ZONE;
    }

    // get packet's name
    string getPacketName() const {
        return "GCDropItemToZone";
    }

    // get packet's debug string
    string toString() const;

public:
    ObjectID_t getDropPetOID() const {
        return m_DropPetOID;
    }
    void setDropPetOID(ObjectID_t PetOID) {
        m_DropPetOID = PetOID;
    }

private:
    ObjectID_t m_DropPetOID;
};


//////////////////////////////////////////////////////////////////////
//
// class GCDropItemToZoneFactory;
//
// Factory for GCDropItemToZone
//
//////////////////////////////////////////////////////////////////////

class GCDropItemToZoneFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_DROP_ITEM_TO_ZONE;
    static constexpr std::string_view kName = "GCDropItemToZone";
    static constexpr PacketSize_t kMaxSize{szObjectID + szCoord + szCoord + szBYTE + szItemType + szBYTE + 255 +
                                           szDurability + szItemNum + szBYTE +
                                           (szObjectID + szBYTE + szItemType + szItemNum + szSlotID) * 12 + szObjectID};

    // create packet
    Packet* createPacket() override {
        return new GCDropItemToZone();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // const static GCDropItemToZonePacketSize 를 정의, 리턴하라.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
