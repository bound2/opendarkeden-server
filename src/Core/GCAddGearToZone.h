//////////////////////////////////////////////////////////////////////////////
// Filename    : GCAddGearToZone.h
// Written By  : crazydog
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_ADD_GEAR_TO_ZONE_H__
#define __GC_ADD_GEAR_TO_ZONE_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class GCAddGearToZone;
//////////////////////////////////////////////////////////////////////////////

class GCAddGearToZone : public Packet {
public:
    GCAddGearToZone();
    ~GCAddGearToZone() noexcept;

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_GEAR_TO_ZONE;
    }
    PacketSize_t getPacketSize() const {
        return szSlotID;
    }
    string getPacketName() const {
        return "GCAddGearToZone";
    }
    string toString() const;

public:
    SlotID_t getSlotID() {
        return m_SlotID;
    }
    void setSlotID(SlotID_t SlotID) {
        m_SlotID = SlotID;
    }

private:
    SlotID_t m_SlotID; // SlotID
};


//////////////////////////////////////////////////////////////////////////////
// class GCAddGearToZoneFactory;
//////////////////////////////////////////////////////////////////////////////

class GCAddGearToZoneFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_GEAR_TO_ZONE;
    static constexpr std::string_view kName = "GCAddGearToZone";
    static constexpr PacketSize_t kMaxSize{szSlotID};

    Packet* createPacket() override {
        return new GCAddGearToZone();
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
