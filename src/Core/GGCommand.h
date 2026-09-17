//----------------------------------------------------------------------
//
// Filename    : GGCommand.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __GG_COMMAND_H__
#define __GG_COMMAND_H__

// include files
#include "DatagramPacket.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class GGCommand;
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

class GGCommand : public DatagramPacket {
public:
    GGCommand(){};
    ~GGCommand(){};
    // Read data from the Datagram object and initialise the packet.
    void read(Datagram& iDatagram);

    // Send the packet's binary image to the Datagram object.
    void write(Datagram& oDatagram) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GG_COMMAND;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE + m_Command.size();
    }

    // get packet name
    string getPacketName() const {
        return "GGCommand";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set playerID
    const string& getCommand() const {
        return m_Command;
    }
    void setCommand(const string& command) {
        m_Command = command;
    }

private:
    string m_Command;
};


//////////////////////////////////////////////////////////////////////
//
// class GGCommandFactory;
//
// Factory for GGCommand
//
//////////////////////////////////////////////////////////////////////

class GGCommandFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GG_COMMAND;
    static constexpr std::string_view kName = "GGCommand";
    static constexpr PacketSize_t kMaxSize{szBYTE + 80};

    // create packet
    Packet* createPacket() override {
        return new GGCommand();
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
    // Define and return const static GGCommandPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class GGCommandHandler;
//
//////////////////////////////////////////////////////////////////////

class GGCommandHandler {
public:
    // execute packet's handler
    static void execute(GGCommand* pPacket);
};

#endif
