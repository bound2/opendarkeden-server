//////////////////////////////////////////////////////////////////////
//
// Filename    : LCQueryResultPlayerID.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_QUERY_RESULT_PLAYER_ID_H__
#define __LC_QUERY_RESULT_PLAYER_ID_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class LCQueryResultPlayerID;
//
// The first packet the client sends to the login server.
// The id and the password are encrypted.
//
//////////////////////////////////////////////////////////////////////

class LCQueryResultPlayerID : public Packet {
public:
    // constructor
    LCQueryResultPlayerID() : m_bExist(false) {}
    ~LCQueryResultPlayerID(){};

public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_QUERY_RESULT_PLAYER_ID;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szbool + de::wire::stringWireSize(m_PlayerID);
    }

    // get packet name
    string getPacketName() const {
        return "LCQueryResultPlayerID";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set player id
    string getPlayerID() const {
        return m_PlayerID;
    }
    void setPlayerID(const string& playerID) {
        m_PlayerID = playerID;
    }

    // get/set player id's existence
    bool isExist() const {
        return m_bExist;
    }
    void setExist(bool bExist = true) {
        m_bExist = bExist;
    }

private:
    // player id
    string m_PlayerID;

    // player id's existence
    bool m_bExist;
};


//////////////////////////////////////////////////////////////////////
//
// class LCQueryResultPlayerIDFactory;
//
// Factory for LCQueryResultPlayerID
//
//////////////////////////////////////////////////////////////////////

class LCQueryResultPlayerIDFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_QUERY_RESULT_PLAYER_ID;
    static constexpr std::string_view kName = "LCQueryResultPlayerID";
    static constexpr PacketSize_t kMaxSize{szbool + szBYTE + 20};

    // create packet
    Packet* createPacket() override {
        return new LCQueryResultPlayerID();
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
//
//////////////////////////////////////////////////////////////////////

#endif
