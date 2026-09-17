//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSay.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_SAY_H__
#define __GC_SAY_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"


//////////////////////////////////////////////////////////////////////
//
// class GCSay;
//
// Packet the game server sends when broadcasting a particular player's
// Say to the other players. It holds the character name and that string
// as data fields.
//
//////////////////////////////////////////////////////////////////////

class GCSay : public Packet {
public:
    GCSay(){};
    ~GCSay(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SAY;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID + szuint + de::wire::stringWireSize(m_Message);
    }

    // get packet name
    string getPacketName() const {
        return "GCSay";
    }

    // get packet's debug string
    string toString() const;

    // get/set sender's creature id
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(const ObjectID_t& creatureID) {
        m_ObjectID = creatureID;
    }

    // get/set text color
    uint getColor() const {
        return m_Color;
    }
    void setColor(uint color) {
        m_Color = color;
    }

    // get/set chatting message
    string getMessage() const {
        return m_Message;
    }
    void setMessage(const string& msg) {
        m_Message = msg;
    }


private:
    // character's creature id
    ObjectID_t m_ObjectID = 0;

    // text color
    uint m_Color = 0;

    // chatting message
    string m_Message;
};


//////////////////////////////////////////////////////////////////////
//
// class GCSayFactory;
//
// Factory for GCSay
//
//////////////////////////////////////////////////////////////////////

class GCSayFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SAY;
    static constexpr std::string_view kName = "GCSay";
    static constexpr PacketSize_t kMaxSize{szObjectID + szuint + szBYTE + 128};

    // create packet
    Packet* createPacket() override {
        return new GCSay();
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
    // Define and return const static GCSayPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
