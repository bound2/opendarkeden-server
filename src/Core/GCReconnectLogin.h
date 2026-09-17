//////////////////////////////////////////////////////////////////////
//
// Filename    : GCReconnectLogin.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_RECONNECT_LOGIN_H__
#define __GC_RECONNECT_LOGIN_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class GCReconnectLogin;
//
// For a client that has been authenticated by the login server and has chosen a
// character, the zone the character was in on the previous connection is found and
// this is the packet that tells it to reconnect to that zone's game server.
//
//////////////////////////////////////////////////////////////////////

class GCReconnectLogin : public Packet {
public:
    GCReconnectLogin(){};
    ~GCReconnectLogin(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_RECONNECT_LOGIN;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_LoginServerIP) // Game server IP
               + szuint                                  // Game server port
               + szDWORD;                                // Authentication key
    }

    // get packet name
    string getPacketName() const {
        return "GCReconnectLogin";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set game server's ip
    string getLoginServerIP() const {
        return m_LoginServerIP;
    }
    void setLoginServerIP(const string& ip) {
        m_LoginServerIP = ip;
    }

    // get/set game server's port
    uint getLoginServerPort() const {
        return m_LoginServerPort;
    }
    void setLoginServerPort(uint port) {
        m_LoginServerPort = port;
    }

    // get/set key
    DWORD getKey() const {
        return m_Key;
    }
    void setKey(DWORD key) {
        m_Key = key;
    }

private:
    // New LoginServer's IP
    string m_LoginServerIP;

    // New LoginServer's TCP Port
    uint m_LoginServerPort;

    // authentication key
    DWORD m_Key;
};


//////////////////////////////////////////////////////////////////////
//
// class GCReconnectLoginFactory;
//
// Factory for GCReconnectLogin
//
//////////////////////////////////////////////////////////////////////

class GCReconnectLoginFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_RECONNECT_LOGIN;
    static constexpr std::string_view kName = "GCReconnectLogin";
    static constexpr PacketSize_t kMaxSize{szBYTE + 15 // Game server IP
                                           + szuint    // Game server port
                                           + szDWORD}; // Authentication key

    // create packet
    Packet* createPacket() override {
        return new GCReconnectLogin();
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
    // Define and return const static GCReconnectLoginPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
