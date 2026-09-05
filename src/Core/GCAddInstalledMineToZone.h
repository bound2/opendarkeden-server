//////////////////////////////////////////////////////////////////////////////
// Filename    : GCAddInstalledMineToZone.h
// Written By  : elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_ADD_INSTALLED_MINE_TO_ZONE_H__
#define __GC_ADD_INSTALLED_MINE_TO_ZONE_H__

#include "GCAddItemToZone.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "SubItemInfo.h"

//////////////////////////////////////////////////////////////////////////////
//
// class GCAddInstalledMineToZone;
//
//////////////////////////////////////////////////////////////////////////////

class GCAddInstalledMineToZone : public GCAddItemToZone {
public:
    GCAddInstalledMineToZone();
    ~GCAddInstalledMineToZone();

public:
    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_INSTALLED_MINE_TO_ZONE;
    }
    string getPacketName() const {
        return "GCAddInstalledMineToZone";
    }
    string toString() const;
};

//////////////////////////////////////////////////////////////////////////////
// class GCAddInstalledMineToZoneFactory;
//////////////////////////////////////////////////////////////////////////////

class GCAddInstalledMineToZoneFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_INSTALLED_MINE_TO_ZONE;
    static constexpr std::string_view kName = "GCAddInstalledMineToZone";
    static constexpr PacketSize_t kMaxSize{szObjectID + szCoord + szCoord + szBYTE + szItemType + szBYTE + 255 +
                                           szDurability + szItemNum + szBYTE +
                                           (szObjectID + szBYTE + szItemType + szItemNum + szSlotID) * 12};

    Packet* createPacket() override {
        return new GCAddInstalledMineToZone();
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
