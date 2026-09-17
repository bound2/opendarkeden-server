//////////////////////////////////////////////////////////////////////
//
// Filename    : GCPartySay.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_PARTY_SAY_H__
#define __GC_PARTY_SAY_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"


//////////////////////////////////////////////////////////////////////
//
// class GCPartySay;
//
// Packet used when the game server tells the client that a particular user
// has moved. It carries (ObjectID, X, Y, DIR).
//
//////////////////////////////////////////////////////////////////////

class GCPartySay : public Packet {
public:
    GCPartySay(){};
    ~GCPartySay(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_PARTY_SAY;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCPartySayPacketSize.
    PacketSize_t getPacketSize() const {
        return szBYTE + m_Name.size() + szDWORD + de::wire::stringWireSize(m_Message);
    }

    // get packet's name
    string getPacketName() const {
        return "GCPartySay";
    }

    // get packet's debug string
    string toString() const;


public:
    string getName() const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
    }

    DWORD getColor() const {
        return m_Color;
    }
    void setColor(DWORD color) {
        m_Color = color;
    }

    string getMessage() const {
        return m_Message;
    }
    // Truncates to the width the factory max budgets.
    void setMessage(const string& msg) {
        m_Message = (msg.size() > 128) ? msg.substr(0, 128) : msg;
    }

private:
    string m_Name;
    DWORD m_Color;
    string m_Message;
};


//////////////////////////////////////////////////////////////////////
//
// class GCPartySayFactory;
//
// Factory for GCPartySay
//
//////////////////////////////////////////////////////////////////////

class GCPartySayFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_PARTY_SAY;
    static constexpr std::string_view kName = "GCPartySay";
    static constexpr PacketSize_t kMaxSize{szBYTE + 20 + szDWORD + szBYTE + 128};

    // create packet
    Packet* createPacket() override {
        return new GCPartySay();
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
    // Define and return const static GCPartySayPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
