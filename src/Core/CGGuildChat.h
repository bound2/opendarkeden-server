//////////////////////////////////////////////////////////////////////
//
// Filename    : CGGuildChat.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_GUILD_CHAT_H__
#define __CG_GUILD_CHAT_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class CGGuildChat;
//
//////////////////////////////////////////////////////////////////////

class CGGuildChat : public Packet {
public:
    // Initialize packet by reading data from the incoming stream.
    void read(SocketInputStream& iStream);

    // Serialize packet data to the outgoing stream.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_GUILD_CHAT;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE + szuint + // text color
               szBYTE +          // message size
               m_Message.size(); // chatting message
    }

    // get packet name
    string getPacketName() const {
        return "CGGuildChat";
    }

    // get packet's debug string
    string toString() const;

    BYTE getType() const {
        return m_Type;
    }
    void setType(BYTE type) {
        m_Type = type;
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
    BYTE m_Type;
    // text color
    uint m_Color;

    // chatting message
    string m_Message;
};


//////////////////////////////////////////////////////////////////////
//
// class CGGuildChatFactory;
//
// Factory for CGGuildChat
//
//////////////////////////////////////////////////////////////////////

class CGGuildChatFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_GUILD_CHAT;
    static constexpr std::string_view kName = "CGGuildChat";
    static constexpr PacketSize_t kMaxSize{szBYTE + szuint + // text color
                                           szBYTE +          // message size
                                           128};             // chatting message

    // create packet
    Packet* createPacket() override {
        return new CGGuildChat();
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
    // Depends on the maximum message length.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGGuildChatHandler;
//
//////////////////////////////////////////////////////////////////////

class CGGuildChatHandler {
public:
    // execute packet's handler
    static void execute(CGGuildChat* pPacket, Player* pPlayer);
};

#endif
