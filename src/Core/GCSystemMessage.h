//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSystemMessage.h
// Written By  : Reiot
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_SYSTEM_MESSAGE_H__
#define __GC_SYSTEM_MESSAGE_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"


enum SystemMessageType {
    SYSTEM_MESSAGE_NORMAL = 0,
    SYSTEM_MESSAGE_OPERATOR,    // Word from the operator
    SYSTEM_MESSAGE_MASTER_LAIR, // Master lair related
    SYSTEM_MESSAGE_COMBAT,      // War related
    SYSTEM_MESSAGE_INFO,        // Particular information related
    SYSTEM_MESSAGE_HOLY_LAND,   // Adam's holy land related
    SYSTEM_MESSAGE_RANGER_SAY,  // Ranger Say, message of a Ranger who has DragonEye
    SYSTEM_MESSAGE_PLAYER,      // Player message
    SYSTEM_MESSAGE_MAX
};

//////////////////////////////////////////////////////////////////////
//
// class GCSystemMessage;
//
// Packet the game server sends when broadcasting a particular player's
// SystemMessage to the other players. It holds the character name and that string
// as data fields.
//
//////////////////////////////////////////////////////////////////////

class GCSystemMessage : public Packet {
public:
    GCSystemMessage() : m_Color(0x006040E8), m_Type(SYSTEM_MESSAGE_NORMAL) {}

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SYSTEM_MESSAGE;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_Message) + szuint + szBYTE;
    }

    // get packet name
    string getPacketName() const {
        return "GCSystemMessage";
    }

    // get packet's debug string
    string toString() const;

    // get/set chatting message
    string getMessage() const {
        return m_Message;
    }
    void setMessage(const string& msg) {
        m_Message = msg;
    }

    // get/set text color
    uint getColor() const {
        return m_Color;
    }
    void setColor(uint color) {
        m_Color = color;
    }

    SystemMessageType getType() const {
        return m_Type;
    }
    void setType(SystemMessageType Type) {
        m_Type = Type;
    }

private:
    // chatting message
    string m_Message;

    // text color
    uint m_Color;

    SystemMessageType m_Type;
};


//////////////////////////////////////////////////////////////////////
//
// class GCSystemMessageFactory;
//
// Factory for GCSystemMessage
//
//////////////////////////////////////////////////////////////////////

class GCSystemMessageFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SYSTEM_MESSAGE;
    static constexpr std::string_view kName = "GCSystemMessage";
    static constexpr PacketSize_t kMaxSize{szBYTE + 256 + szuint + szBYTE};

    // create packet
    Packet* createPacket() override {
        return new GCSystemMessage();
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
    // Define and return const static GCSystemMessagePacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
