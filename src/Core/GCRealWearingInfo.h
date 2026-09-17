//////////////////////////////////////////////////////////////////////
//
// Filename    : GCRealWearingInfo.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_REAL_WEARING_INFO_H__
#define __GC_REAL_WEARING_INFO_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCRealWearingInfo;
//
//////////////////////////////////////////////////////////////////////

class GCRealWearingInfo : public Packet {
public:
    GCRealWearingInfo(){};
    ~GCRealWearingInfo(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_REAL_WEARING_INFO;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szDWORD;
    }

    // get packet name
    string getPacketName() const {
        return "GCRealWearingInfo";
    }

    // get packet's debug string
    string toString() const;

public:
    DWORD getInfo(void) const {
        return m_Info;
    }
    void setInfo(DWORD info) {
        m_Info = info;
    }

private:
    DWORD m_Info = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCRealWearingInfoFactory;
//
// Factory for GCRealWearingInfo
//
//////////////////////////////////////////////////////////////////////

class GCRealWearingInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_REAL_WEARING_INFO;
    static constexpr std::string_view kName = "GCRealWearingInfo";
    static constexpr PacketSize_t kMaxSize{szDWORD};

    // create packet
    Packet* createPacket() override {
        return new GCRealWearingInfo();
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
    // Define and return const static GCRealWearingInfoPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
