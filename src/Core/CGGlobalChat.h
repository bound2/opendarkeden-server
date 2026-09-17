//////////////////////////////////////////////////////////////////////
//
// Filename    : CGGlobalChat.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_GLOBAL_CHAT_H__
#define __CG_GLOBAL_CHAT_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class CGGlobalChat;
//
// The GlobalChat packet the client sends to the server.
// It holds only the GlobalChat string as its data field.
//
//////////////////////////////////////////////////////////////////////

class CGGlobalChat : public Packet {
public:
    CGGlobalChat(){};
    ~CGGlobalChat(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_GLOBAL_CHAT;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szuint + de::wire::stringWireSize(m_Message);
    }

    // get packet name
    string getPacketName() const {
        return "CGGlobalChat";
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


private:
    // text color
    uint m_Color = 0;

    // chatting message
    string m_Message;
};


//////////////////////////////////////////////////////////////////////
//
// class CGGlobalChatFactory;
//
// Factory for CGGlobalChat
//
//////////////////////////////////////////////////////////////////////

class CGGlobalChatFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_GLOBAL_CHAT;
    static constexpr std::string_view kName = "CGGlobalChat";
    static constexpr PacketSize_t kMaxSize{szuint + szBYTE + 128};

    // create packet
    Packet* createPacket() override {
        return new CGGlobalChat();
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
    // The maximum size of message needs to be configured.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGGlobalChatHandler;
//
//////////////////////////////////////////////////////////////////////

class CGGlobalChatHandler {
public:
    // execute packet's handler
    static void execute(CGGlobalChat* pPacket, Player* pPlayer);
};

#endif
