//////////////////////////////////////////////////////////////////////
//
// Filename    : GCGlobalChat.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_GLOBAL_CHAT_H__
#define __GC_GLOBAL_CHAT_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"


//////////////////////////////////////////////////////////////////////
//
// class GCGlobalChat;
//
// Sent when the game server broadcasts one player's GlobalChat to
// the other players. It holds the character name and the GlobalChat string as its data
// fields.
//
//////////////////////////////////////////////////////////////////////

class GCGlobalChat : public Packet {
public:
    GCGlobalChat(){};
    ~GCGlobalChat(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_GLOBAL_CHAT;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szuint + de::wire::stringWireSize(m_Message) + szBYTE;
    }

    // get packet name
    string getPacketName() const {
        return "GCGlobalChat";
    }

    // get packet's debug string
    string toString() const;

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

    // get/set chatting message
    Race_t getRace() const {
        return m_Race;
    }
    void setRace(Race_t race) {
        m_Race = race;
    }


private:
    // chatting message
    string m_Message;

    // text color
    uint m_Color = 0;

    // race
    Race_t m_Race = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCGlobalChatFactory;
//
// Factory for GCGlobalChat
//
//////////////////////////////////////////////////////////////////////

class GCGlobalChatFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_GLOBAL_CHAT;
    static constexpr std::string_view kName = "GCGlobalChat";
    static constexpr PacketSize_t kMaxSize{szuint + szBYTE + 128 + szBYTE};

    // create packet
    Packet* createPacket() override {
        return new GCGlobalChat();
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
    // Define and return const static GCGlobalChatPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
