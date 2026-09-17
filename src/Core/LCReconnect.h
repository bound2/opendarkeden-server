//////////////////////////////////////////////////////////////////////
//
// Filename    : LCReconnect.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_RECONNECT_H__
#define __LC_RECONNECT_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class LCReconnect;
//
// For a client that has been authenticated by the login server and has chosen a
// character, the zone the character was in on the previous connection is found and
// this is the packet that tells it to reconnect to that zone's game server.
//
//////////////////////////////////////////////////////////////////////

class LCReconnect : public Packet {
public:
    LCReconnect(){};
    ~LCReconnect(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_RECONNECT;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_GameServerIP) // Game server IP
               + szuint                                 // Game server port
               + szDWORD;                               // Authentication key
    }

    // get packet name
    string getPacketName() const {
        return "LCReconnect";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set game server's ip
    string getGameServerIP() const {
        return m_GameServerIP;
    }
    void setGameServerIP(const string& ip) {
        m_GameServerIP = ip;
    }

    // get/set game server's port
    uint getGameServerPort() const {
        return m_GameServerPort;
    }
    void setGameServerPort(uint port) {
        m_GameServerPort = port;
    }

    // get/set key
    DWORD getKey() const {
        return m_Key;
    }
    void setKey(DWORD key) {
        m_Key = key;
    }

private:
    // New GameServer's IP
    string m_GameServerIP;

    // New GameServer's TCP Port
    uint m_GameServerPort;

    // authentication key
    DWORD m_Key;
};


//////////////////////////////////////////////////////////////////////
//
// class LCReconnectFactory;
//
// Factory for LCReconnect
//
//////////////////////////////////////////////////////////////////////

class LCReconnectFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_RECONNECT;
    static constexpr std::string_view kName = "LCReconnect";
    static constexpr PacketSize_t kMaxSize{szBYTE + 15 // Game server IP
                                           + szuint    // Game server port
                                           + szDWORD}; // Authentication key

    // create packet
    Packet* createPacket() override {
        return new LCReconnect();
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
    // Define and return const static LCReconnectPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
