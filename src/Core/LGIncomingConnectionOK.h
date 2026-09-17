//////////////////////////////////////////////////////////////////////
//
// Filename    : LGIncomingConnectionOK.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LG_INCOMING_CONNECTION_OK_H__
#define __LG_INCOMING_CONNECTION_OK_H__

// include files
#include "DatagramPacket.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class LGIncomingConnectionOK;
//
//////////////////////////////////////////////////////////////////////

class LGIncomingConnectionOK : public DatagramPacket {
public:
    LGIncomingConnectionOK(){};
    ~LGIncomingConnectionOK(){};
    // Read data from the Datagram object and initialise the packet.
    void read(Datagram& iDatagram);

    // Send the packet's binary image to the Datagram object.
    void write(Datagram& oDatagram) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LG_INCOMING_CONNECTION_OK;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE + m_PlayerID.size() + szuint + szDWORD;
    }

    // get packet name
    string getPacketName() const {
        return "LGIncomingConnectionOK";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set player id
    string getPlayerID() const {
        return m_PlayerID;
    }
    void setPlayerID(string playerID) {
        m_PlayerID = playerID;
    }

    // get/set tcp port
    uint getTCPPort() const {
        return m_TCPPort;
    }
    void setTCPPort(uint tcpPort) {
        m_TCPPort = tcpPort;
    }

    // get/set auth key
    DWORD getKey() const {
        return m_Key;
    }
    void setKey(DWORD key) {
        m_Key = key;
    }

private:
    // Error ID
    string m_PlayerID;

    // The game server tells the login server its own TCP port, so
    // the login server does not need to know the game server's TCP port.
    uint m_TCPPort;

    // Authentication key created by the game server
    DWORD m_Key;
};


//////////////////////////////////////////////////////////////////////
//
// class LGIncomingConnectionOKFactory;
//
// Factory for LGIncomingConnectionOK
//
//////////////////////////////////////////////////////////////////////

class LGIncomingConnectionOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LG_INCOMING_CONNECTION_OK;
    static constexpr std::string_view kName = "LGIncomingConnectionOK";
    static constexpr PacketSize_t kMaxSize{szBYTE + 20 + szuint + szDWORD};

    // create packet
    Packet* createPacket() override {
        return new LGIncomingConnectionOK();
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
    // Define and return const static LGIncomingConnectionOKPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class LGIncomingConnectionOKHandler;
//
//////////////////////////////////////////////////////////////////////

class LGIncomingConnectionOKHandler {
public:
    // execute packet's handler
    static void execute(LGIncomingConnectionOK* pPacket);
};

#endif
