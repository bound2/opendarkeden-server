//////////////////////////////////////////////////////////////////////
//
// Filename    : GCDisconnect.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_DISCONNECT_H__
#define __GC_DISCONNECT_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class GCDisconnect;
//
//////////////////////////////////////////////////////////////////////

class GCDisconnect : public Packet {
public:
    GCDisconnect(){};
    ~GCDisconnect(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_DISCONNECT;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_Message);
    }

    // get packet name
    string getPacketName() const {
        return "GCDisconnect";
    }

    // get packet's debug string
    string toString() const;

    // get/set chatting message
    string getMessage() const {
        return m_Message;
    }
    void setMessage(const string& msg) {
        m_Message = msg;
    }

private:
    // chatting message
    string m_Message;
};


//////////////////////////////////////////////////////////////////////
//
// class GCDisconnectFactory;
//
// Factory for GCDisconnect
//
//////////////////////////////////////////////////////////////////////

class GCDisconnectFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_DISCONNECT;
    static constexpr std::string_view kName = "GCDisconnect";
    static constexpr PacketSize_t kMaxSize{szBYTE + 128};

    // create packet
    Packet* createPacket() override {
        return new GCDisconnect();
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
    // Define and return const static GCDisconnectPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
