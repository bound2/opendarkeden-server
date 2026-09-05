//////////////////////////////////////////////////////////////////////
//
// Filename    : GCAddNewItemToZone.h
// Written By  : elca
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_ADD_NEW_ITEM_TO_ZONE_H__
#define __GC_ADD_NEW_ITEM_TO_ZONE_H__

// include files
#include "GCAddItemToZone.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "SubItemInfo.h"


//////////////////////////////////////////////////////////////////////
//
// class GCAddNewItemToZone;
//
////////////////////////////////////////////////////////////////////

class GCAddNewItemToZone : public GCAddItemToZone {
public:
    GCAddNewItemToZone();
    ~GCAddNewItemToZone();


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_NEW_ITEM_TO_ZONE;
    }

    // get packet's name
    string getPacketName() const {
        return "GCAddNewItemToZone";
    }

    // get packet's debug string
    string toString() const;
};


//////////////////////////////////////////////////////////////////////
//
// class GCAddNewItemToZoneFactory;
//
// Factory for GCAddNewItemToZone
//
//////////////////////////////////////////////////////////////////////

class GCAddNewItemToZoneFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_NEW_ITEM_TO_ZONE;
    static constexpr std::string_view kName = "GCAddNewItemToZone";
    static constexpr PacketSize_t kMaxSize{szObjectID + szCoord + szCoord + szBYTE + szItemType + szBYTE + 255 +
                                           szDurability + szItemNum + szBYTE +
                                           (szObjectID + szBYTE + szItemType + szItemNum + szSlotID) * 12};

    // create packet
    Packet* createPacket() override {
        return new GCAddNewItemToZone();
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
    // const static GCAddNewItemToZonePacketSize 를 정의, 리턴하라.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
