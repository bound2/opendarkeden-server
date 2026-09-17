//----------------------------------------------------------------------
//
// Filename    : GLIncomingConnection.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __GL_INCOMING_CONNECTION_H__
#define __GL_INCOMING_CONNECTION_H__

// include files
#include "DatagramPacket.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class GLIncomingConnection;
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

class GLIncomingConnection : public DatagramPacket {
public:
    GLIncomingConnection(){};
    ~GLIncomingConnection(){};
    // Read data from the Datagram object and initialise the packet.
    void read(Datagram& iDatagram);

    // Send the packet's binary image to the Datagram object.
    void write(Datagram& oDatagram) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GL_INCOMING_CONNECTION;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return +szBYTE + m_PlayerID.size()   // Player ID
               + szBYTE + m_ClientIP.size(); // client ip
    }

    // get packet name
    string getPacketName() const {
        return "GLIncomingConnection";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set playerID
    const string& getPlayerID() const {
        return m_PlayerID;
    }
    void setPlayerID(const string& playerID) {
        m_PlayerID = playerID;
    }

    // get/set client ip
    const string& getClientIP() const {
        return m_ClientIP;
    }
    void setClientIP(const string& ip) {
        m_ClientIP = ip;
    }

private:
    // Player ID
    string m_PlayerID;

    // Client IP
    string m_ClientIP;
};


//////////////////////////////////////////////////////////////////////
//
// class GLIncomingConnectionFactory;
//
// Factory for GLIncomingConnection
//
//////////////////////////////////////////////////////////////////////

class GLIncomingConnectionFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GL_INCOMING_CONNECTION;
    static constexpr std::string_view kName = "GLIncomingConnection";
    static constexpr PacketSize_t kMaxSize{+szBYTE + 20    // creature name
                                           + szBYTE + 15}; // client ip

    // create packet
    Packet* createPacket() override {
        return new GLIncomingConnection();
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
    // Define and return const static GLIncomingConnectionPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class GLIncomingConnectionHandler;
//
//////////////////////////////////////////////////////////////////////

class GLIncomingConnectionHandler {
public:
    // execute packet's handler
    static void execute(GLIncomingConnection* pPacket);
};

#endif
