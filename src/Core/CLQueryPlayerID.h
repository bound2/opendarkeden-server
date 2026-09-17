//--------------------------------------------------------------------------------
//
// Filename    : CLQueryPlayerID.h
// Written By  : Reiot
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __CL_QUERY_PLAYER_ID_H__
#define __CL_QUERY_PLAYER_ID_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//--------------------------------------------------------------------------------
//
// class CLQueryPlayerID;
//
// The first packet the client sends to the login server.
// The id and the password are encrypted.
//
//--------------------------------------------------------------------------------

class CLQueryPlayerID : public Packet {
public:
    CLQueryPlayerID(){};
    virtual ~CLQueryPlayerID(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return Packet::PACKET_CL_QUERY_PLAYER_ID;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_PlayerID);
    }

    // get packet name
    string getPacketName() const {
        return "CLQueryPlayerID";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set player's id
    string getPlayerID() const {
        return m_PlayerID;
    }
    void setPlayerID(const string& playerID) {
        m_PlayerID = playerID;
    }

private:
    // Player id
    string m_PlayerID;
};


//--------------------------------------------------------------------------------
//
// class CLQueryPlayerIDFactory;
//
// Factory for CLQueryPlayerID
//
//--------------------------------------------------------------------------------

class CLQueryPlayerIDFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CL_QUERY_PLAYER_ID;
    static constexpr std::string_view kName = "CLQueryPlayerID";
    static constexpr PacketSize_t kMaxSize{szBYTE + 20};

    // create packet
    Packet* createPacket() override {
        return new CLQueryPlayerID();
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


//--------------------------------------------------------------------------------
//
// class CLQueryPlayerIDHandler;
//
//--------------------------------------------------------------------------------

class CLQueryPlayerIDHandler {
public:
    // execute packet's handler
    static void execute(CLQueryPlayerID* pPacket, Player* pPlayer);
};

#endif
