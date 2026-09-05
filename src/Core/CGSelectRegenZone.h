//////////////////////////////////////////////////////////////////////////////
// Filename    : CGSelectRegenZone.h
// Written By  : excel96
// Description : Packet for requesting a new regen zone when moving to another zone.
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_SELECT_REGEN_ZONE_H__
#define __CG_SELECT_REGEN_ZONE_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class CGSelectRegenZone;
//////////////////////////////////////////////////////////////////////////////

class CGSelectRegenZone : public Packet {
public:
    CGSelectRegenZone(){};
    virtual ~CGSelectRegenZone(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_SELECT_REGEN_ZONE;
    }
    PacketSize_t getPacketSize() const {
        return szBYTE;
    }
    string getPacketName() const {
        return "CGSelectRegenZone";
    }
    string toString() const;

public:
    BYTE getRegenZoneID() const {
        return m_RegenZoneID;
    }
    void setRegenZoneID(BYTE RegenZoneID) {
        m_RegenZoneID = RegenZoneID;
    }

private:
    BYTE m_RegenZoneID;
};

//////////////////////////////////////////////////////////////////////////////
// class CGSelectRegenZoneFactory;
//////////////////////////////////////////////////////////////////////////////

class CGSelectRegenZoneFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_SELECT_REGEN_ZONE;
    static constexpr std::string_view kName = "CGSelectRegenZone";
    static constexpr PacketSize_t kMaxSize{szBYTE};

    Packet* createPacket() override {
        return new CGSelectRegenZone();
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
// class CGSelectRegenZoneHandler;
//////////////////////////////////////////////////////////////////////////////

class CGSelectRegenZoneHandler {
public:
    static void execute(CGSelectRegenZone* pCGSelectRegenZone, Player* pPlayer);
};

#endif
