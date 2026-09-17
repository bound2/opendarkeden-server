//----------------------------------------------------------------------
//
// Filename    : GGGuildChat.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __GG_GUILD_CHAT_H__
#define __GG_GUILD_CHAT_H__

// include files
#include "DatagramPacket.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class GGGuildChat;
//
//----------------------------------------------------------------------

class GGGuildChat : public DatagramPacket {
public:
    // Read data from the Datagram object and initialise the packet.
    void read(Datagram& iDatagram);

    // Send the packet's binary image to the Datagram object.
    void write(Datagram& oDatagram) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GG_GUILD_CHAT;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE + szGuildID +       // GuildID
               szBYTE + m_Sender.size() + // Sender
               szuint +                   // Color
               szBYTE + m_Message.size(); // Message
    }

    // get packet name
    string getPacketName() const {
        return "GGGuildChat";
    }

    // get packet's debug string
    string toString() const;

public:
    BYTE getType() const {
        return m_Type;
    }
    void setType(BYTE type) {
        m_Type = type;
    }

    // get/set Guild ID
    GuildID_t getGuildID() const {
        return m_GuildID;
    }
    void setGuildID(GuildID_t guildID) {
        m_GuildID = guildID;
    }

    // get/set Sender
    const string& getSender() const {
        return m_Sender;
    }
    void setSender(const string& sender) {
        m_Sender = sender;
    }

    // get/set text color
    uint getColor() const {
        return m_Color;
    }
    void setColor(uint color) {
        m_Color = color;
    }

    // get/set message
    const string& getMessage() const {
        return m_Message;
    }
    void setMessage(const string& message) {
        m_Message = message;
    }

private:
    BYTE m_Type;
    // Guild ID
    GuildID_t m_GuildID;

    // Sender
    string m_Sender;

    // text color
    uint m_Color;

    // message
    string m_Message;
};


//////////////////////////////////////////////////////////////////////
//
// class GGGuildChatFactory;
//
// Factory for GGGuildChat
//
//////////////////////////////////////////////////////////////////////

class GGGuildChatFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GG_GUILD_CHAT;
    static constexpr std::string_view kName = "GGGuildChat";
    static constexpr PacketSize_t kMaxSize{szBYTE + szGuildID + // GuildID
                                           szBYTE + 10 +        // Sender
                                           szuint +             // Color
                                           szBYTE + 128};       // Message

    // create packet
    Packet* createPacket() override {
        return new GGGuildChat();
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
    // Define and return const static GGGuildChatPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class GGGuildChatHandler;
//
//////////////////////////////////////////////////////////////////////

class GGGuildChatHandler {
public:
    // execute packet's handler
    static void execute(GGGuildChat* pPacket);
};

#endif
