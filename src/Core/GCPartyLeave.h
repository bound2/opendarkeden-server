//////////////////////////////////////////////////////////////////////////////
// Filename    : GCPartyLeave.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_PARTY_LEAVE_H__
#define __GC_PARTY_LEAVE_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class GCPartyLeave;
//////////////////////////////////////////////////////////////////////////////

class GCPartyLeave : public Packet {
public:
    GCPartyLeave(){};
    ~GCPartyLeave(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_PARTY_LEAVE;
    }
    PacketSize_t getPacketSize() const {
        return szBYTE * 2 + m_Expeller.size() + m_Expellee.size();
    }
    string getPacketName() const {
        return "GCPartyLeave";
    }
    string toString() const;

public:
    string getExpeller(void) const {
        return m_Expeller;
    }
    void setExpeller(const string& name) {
        m_Expeller = name;
    }

    string getExpellee(void) const {
        return m_Expellee;
    }
    void setExpellee(const string& name) {
        m_Expellee = name;
    }

private:
    // When somebody left the party on their own
    // The name of the one who threw them out is NULL.
    // When somebody was thrown out by somebody else
    // The name of the one who threw them out is not NULL.
    // In neither case is the name of the one thrown out NULL.
    string m_Expeller; // The one who threw them out
    string m_Expellee; // The one thrown out
};


//////////////////////////////////////////////////////////////////////////////
// class GCPartyLeaveFactory;
//////////////////////////////////////////////////////////////////////////////

class GCPartyLeaveFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_PARTY_LEAVE;
    static constexpr std::string_view kName = "GCPartyLeave";
    static constexpr PacketSize_t kMaxSize{szBYTE * 2 + 20};

    Packet* createPacket() override {
        return new GCPartyLeave();
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
