//////////////////////////////////////////////////////////////////////
//
// Filename    : LCQueryResultCharacterName.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_QUERY_RESULT_CHARACTER_NAME_H__
#define __LC_QUERY_RESULT_CHARACTER_NAME_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class LCQueryResultCharacterName;
//
// The first packet the client sends to the login server.
// The id and the password are encrypted.
//
//////////////////////////////////////////////////////////////////////

class LCQueryResultCharacterName : public Packet {
public:
    // constructor
    LCQueryResultCharacterName() : m_bExist(false) {}
    ~LCQueryResultCharacterName(){};

public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_QUERY_RESULT_CHARACTER_NAME;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szbool + de::wire::stringWireSize(m_CharacterName);
    }

    // get packet name
    string getPacketName() const {
        return "LCQueryResultCharacterName";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set player id
    string getCharacterName() const {
        return m_CharacterName;
    }
    void setCharacterName(const string& playerID) {
        m_CharacterName = playerID;
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
    string m_CharacterName;

    // player id's existence
    bool m_bExist;
};


//////////////////////////////////////////////////////////////////////
//
// class LCQueryResultCharacterNameFactory;
//
// Factory for LCQueryResultCharacterName
//
//////////////////////////////////////////////////////////////////////

class LCQueryResultCharacterNameFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_QUERY_RESULT_CHARACTER_NAME;
    static constexpr std::string_view kName = "LCQueryResultCharacterName";
    static constexpr PacketSize_t kMaxSize{szbool + szBYTE + 20};

    // create packet
    Packet* createPacket() override {
        return new LCQueryResultCharacterName();
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
