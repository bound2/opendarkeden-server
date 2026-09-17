//----------------------------------------------------------------------
//
// Filename    : CGPortCheck.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __CG_PORT_CHECK_H__
#define __CG_PORT_CHECK_H__

// include files
#include "DatagramPacket.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class CGPortCheck;
//
// When a user is about to connect from the login server to a game server,
// the login server tells that game server which user, from which address,
// is going to log in as which creature.
//
// *CAUTION*
//
// Is the creature name really needed? It is, when the following case is
// considered. A player can pick the Slot3 character at the login server and
// then, once actually connected to the game server, ask for the SLOT2
// character to be loaded. To prevent that, the character picked with
// CLSelectPC must be told to the game server, and CGConnect also carries the
// character id so that it can be loaded straight away.
//
//----------------------------------------------------------------------

class CGPortCheck : public DatagramPacket {
public:
    CGPortCheck(){};
    ~CGPortCheck(){};
    // Read data from the Datagram object and initialise the packet.
    void read(Datagram& iDatagram);

    // Send the packet's binary image to the Datagram object.
    void write(Datagram& oDatagram) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_PORT_CHECK;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE + m_PCName.size(); // PC name
    }

    // get packet name
    string getPacketName() const {
        return "CGPortCheck";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set pcName
    string getPCName() const {
        return m_PCName;
    }
    void setPCName(const string& pcName) {
        m_PCName = pcName;
    }

private:
    // PC name
    string m_PCName;
};


//////////////////////////////////////////////////////////////////////
//
// class CGPortCheckFactory;
//
// Factory for CGPortCheck
//
//////////////////////////////////////////////////////////////////////

class CGPortCheckFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_PORT_CHECK;
    static constexpr std::string_view kName = "CGPortCheck";
    static constexpr PacketSize_t kMaxSize{szBYTE + 20}; // PC name

    // create packet
    Packet* createPacket() override {
        return new CGPortCheck();
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
    // Define and return const static CGPortCheckPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGPortCheckHandler;
//
//////////////////////////////////////////////////////////////////////

class CGPortCheckHandler {
public:
    // execute packet's handler
    static void execute(CGPortCheck* pPacket);
};

#endif
