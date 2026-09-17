//////////////////////////////////////////////////////////////////////
//
// Filename    : CGConnect.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_CONNECT_H__
#define __CG_CONNECT_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class CGConnect;
//
// The connect packet the client sends to the server.
// It is used when moving between servers: the Key the previous server handed
// out is sent to the new server for authentication. It also carries the creature id to use.
//
//////////////////////////////////////////////////////////////////////

class CGConnect : public Packet {
public:
    CGConnect(){};
    ~CGConnect(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_CONNECT;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szDWORD                                   // authentication key
               + szPCType                                // Slayer or Vampire?
               + de::wire::stringWireSize(m_PCName) + 6; // name
    }

    // get packet name
    string getPacketName() const {
        return "CGConnect";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set key
    DWORD getKey() const {
        return m_Key;
    }
    void setKey(DWORD key) {
        m_Key = key;
    }

    // get/set PCType
    PCType getPCType() const {
        return m_PCType;
    }
    void setPCType(PCType pcType) {
        m_PCType = pcType;
    }

    // get/set pc name
    string getPCName() const {
        return m_PCName;
    }
    void setPCName(string pcName) {
        m_PCName = pcName;
    }

    const BYTE* getMacAddress() const {
        return m_MacAddress;
    }

private:
    // authentication key
    DWORD m_Key;

    // Slayer or Vampire?
    PCType m_PCType;

    // Name of the PC
    string m_PCName;

    BYTE m_MacAddress[6];
};


//////////////////////////////////////////////////////////////////////
//
// class CGConnectFactory;
//
// Factory for CGConnect
//
//////////////////////////////////////////////////////////////////////

class CGConnectFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_CONNECT;
    static constexpr std::string_view kName = "CGConnect";
    static constexpr PacketSize_t kMaxSize{szDWORD             // authentication key
                                           + szPCType          // Slayer or Vampire
                                           + szBYTE + 20 + 6}; // name

    // create packet
    Packet* createPacket() override {
        return new CGConnect();
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
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};

//////////////////////////////////////////////////////////////////////
//
// class CGConnectHandler;
//
//////////////////////////////////////////////////////////////////////

class CGConnectHandler {
public:
    // execute packet's handler
    static void execute(CGConnect* pPacket, Player* pPlayer);
};

#endif
