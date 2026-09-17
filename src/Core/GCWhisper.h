//////////////////////////////////////////////////////////////////////
//
// Filename    : GCWhisper.h
// Written By  : elca
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_WHISPER_H__
#define __GC_WHISPER_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"


//////////////////////////////////////////////////////////////////////
//
// class GCWhisper;
//
// Packet the game server sends when broadcasting a particular player's
// Whisper to the other players. It holds the character name and that string
// as data fields.
//
//////////////////////////////////////////////////////////////////////

class GCWhisper : public Packet {
public:
    GCWhisper(){};
    ~GCWhisper(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_WHISPER;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_Name) + szuint + de::wire::stringWireSize(m_Message) + szBYTE;
    }

    // get packet name
    string getPacketName() const {
        return "GCWhisper";
    }

    // get packet's debug string
    string toString() const;

    // get/set sender's creature id
    string getName() const {
        return m_Name;
    }
    void setName(const string& Name) {
        m_Name = Name;
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

    // get/set chatting message
    Race_t getRace() const {
        return m_Race;
    }
    void setRace(Race_t race) {
        m_Race = race;
    }

private:
    // character's creature Name
    string m_Name;

    // text color
    uint m_Color = 0;

    // chatting message
    string m_Message;

    // Race
    Race_t m_Race = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCWhisperFactory;
//
// Factory for GCWhisper
//
//////////////////////////////////////////////////////////////////////

class GCWhisperFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_WHISPER;
    static constexpr std::string_view kName = "GCWhisper";
    static constexpr PacketSize_t kMaxSize{szBYTE + 10 + szuint + szBYTE + 128 + szBYTE};

    // create packet
    Packet* createPacket() override {
        return new GCWhisper();
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
    // Define and return const static GCWhisperPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
