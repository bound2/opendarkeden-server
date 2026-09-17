//////////////////////////////////////////////////////////////////////
//
// Filename    : GCKickMessage.h
// Written By  : Reiot
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_KICK_MESSAGE_H__
#define __GC_KICK_MESSAGE_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


enum KickMessageType { KICK_MESSAGE_PAY_TIMEOUT = 0, KICK_MESSAGE_EXPIRE_FREEPLAY, KICK_MESSAGE_MAX };

//////////////////////////////////////////////////////////////////////
//
// class GCKickMessage;
//
// Sent when the game server broadcasts one player's KickMessage to
// the other players. It holds the character name and the KickMessage string as its data
// fields.
//
//////////////////////////////////////////////////////////////////////

class GCKickMessage : public Packet {
public:
    GCKickMessage() : m_Type(KICK_MESSAGE_PAY_TIMEOUT) {}
    ~GCKickMessage(){};

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_KICK_MESSAGE;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE + szuint;
    }

    // get packet name
    string getPacketName() const {
        return "GCKickMessage";
    }

    // get packet's debug string
    string toString() const;

    // get/set Kick Message Type
    BYTE getType() const {
        return m_Type;
    }
    // The enum declares fewer values than the byte carries, so the raw
    // byte is tested before it reaches it.
    void setType(BYTE type) {
        if (type >= KICK_MESSAGE_MAX)
            throw InvalidProtocolException("kick message type out of range");
        m_Type = (KickMessageType)type;
    }

    // get/set seconds
    uint getSeconds() const {
        return m_Seconds;
    }
    void setSeconds(uint seconds) {
        m_Seconds = seconds;
    }

private:
    KickMessageType m_Type;

    // seconds
    uint m_Seconds = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCKickMessageFactory;
//
// Factory for GCKickMessage
//
//////////////////////////////////////////////////////////////////////

class GCKickMessageFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_KICK_MESSAGE;
    static constexpr std::string_view kName = "GCKickMessage";
    static constexpr PacketSize_t kMaxSize{szBYTE + szuint};

    // create packet
    Packet* createPacket() override {
        return new GCKickMessage();
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
    // Define and return const static GCKickMessagePacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
