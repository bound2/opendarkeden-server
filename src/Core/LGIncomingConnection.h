//----------------------------------------------------------------------
//
// Filename    : LGIncomingConnection.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __LG_INCOMING_CONNECTION_H__
#define __LG_INCOMING_CONNECTION_H__

// include files
#include "DatagramPacket.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class LGIncomingConnection;
//
// When a user tries to connect to a game server, the login server tells
// that game server which user is logging in from which address and as which
// creature.
//
// *CAUTION*
//
// Is the creature name really needed? It is, and here is why.
// Slot3 can be chosen on the login server,
// and then SLOT2 asked for once the game server itself is reached.
// To stop that, the character chosen with CLSelectPC has to be told
// to the game server, and CGConnect carries the character id too so that
// the right character is loaded straight away.
//
//----------------------------------------------------------------------

class LGIncomingConnection : public DatagramPacket {
public:
    LGIncomingConnection(){};
    ~LGIncomingConnection(){};
    // Read data from the Datagram object and initialise the packet.
    void read(Datagram& iDatagram);

    // Send the packet's binary image to the Datagram object.
    void write(Datagram& oDatagram) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LG_INCOMING_CONNECTION;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return +szBYTE + m_PlayerID.size()   // Player ID
               + szBYTE + m_PCName.size()    // PC name
               + szBYTE + m_ClientIP.size(); // client ip
    }

    // get packet name
    string getPacketName() const {
        return "LGIncomingConnection";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set playerID
    string getPlayerID() const {
        return m_PlayerID;
    }
    void setPlayerID(const string& playerID) {
        m_PlayerID = playerID;
    }

    // get/set pcName
    string getPCName() const {
        return m_PCName;
    }
    void setPCName(const string& pcName) {
        m_PCName = pcName;
    }

    // get/set client ip
    string getClientIP() const {
        return m_ClientIP;
    }
    void setClientIP(const string& ip) {
        m_ClientIP = ip;
    }

private:
    // Player ID
    string m_PlayerID;

    // PC name
    string m_PCName;

    // Client IP
    string m_ClientIP;
};


//////////////////////////////////////////////////////////////////////
//
// class LGIncomingConnectionFactory;
//
// Factory for LGIncomingConnection
//
//////////////////////////////////////////////////////////////////////

class LGIncomingConnectionFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LG_INCOMING_CONNECTION;
    static constexpr std::string_view kName = "LGIncomingConnection";
    static constexpr PacketSize_t kMaxSize{+szBYTE + 20    // creature name
                                           + szBYTE + 20   // PC name
                                           + szBYTE + 15}; // client ip

    // create packet
    Packet* createPacket() override {
        return new LGIncomingConnection();
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
    // Define and return const static LGIncomingConnectionPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class LGIncomingConnectionHandler;
//
//////////////////////////////////////////////////////////////////////

class LGIncomingConnectionHandler {
public:
    // execute packet's handler
    static void execute(LGIncomingConnection* pPacket);
};

#endif
