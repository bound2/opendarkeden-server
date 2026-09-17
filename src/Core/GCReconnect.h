//////////////////////////////////////////////////////////////////////
//
// Filename    : GCReconnect.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_RECONNECT_H__
#define __GC_RECONNECT_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class GCReconnect;
//
// When moving between servers, the packet with which the old server tells the
// client to connect to the next one and close the connection. On receiving it the client
// closes the connection to that server and connects to the IP/Port in the packet.
//
//////////////////////////////////////////////////////////////////////

class GCReconnect : public Packet {
public:
    GCReconnect(){};
    ~GCReconnect(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_RECONNECT;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_Name)       // Character name
               + de::wire::stringWireSize(m_ServerIP) // IP of the game server to connect to
               + szDWORD;                             // Authentication key
    }

    // get packet name
    string getPacketName() const {
        return "GCReconnect";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set creature name
    string getName() const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
    }

    // get/set server ip
    string getServerIP() const {
        return m_ServerIP;
    }
    void setServerIP(const string& serverIP) {
        m_ServerIP = serverIP;
    }

    // get/set key
    DWORD getKey() const {
        return m_Key;
    }
    void setKey(DWORD key) {
        m_Key = key;
    }

private:
    // creature name
    string m_Name;

    // New Server IP
    string m_ServerIP;

    // authentication key
    DWORD m_Key = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCReconnectFactory;
//
// Factory for GCReconnect
//
//////////////////////////////////////////////////////////////////////

class GCReconnectFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_RECONNECT;
    static constexpr std::string_view kName = "GCReconnect";
    static constexpr PacketSize_t kMaxSize{szBYTE + 20   // Character name
                                           + szBYTE + 15 // IP of the game server to connect to
                                           + szDWORD};   // Authentication key

    // create packet
    Packet* createPacket() override {
        return new GCReconnect();
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
    // Define and return const static GCReconnectPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
