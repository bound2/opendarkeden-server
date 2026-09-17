//////////////////////////////////////////////////////////////////////
//
// Filename    : CGWhisper.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_WHISPER_H__
#define __CG_WHISPER_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class CGWhisper;
//
// The Whisper packet the client sends to the server.
// It holds only the Whisper string as its data field.
//
//////////////////////////////////////////////////////////////////////

class CGWhisper : public Packet {
public:
    CGWhisper(){};
    virtual ~CGWhisper(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_WHISPER;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_Name) + szuint + de::wire::stringWireSize(m_Message);
    }

    // get packet name
    string getPacketName() const {
        return "CGWhisper";
    }

    // get packet's debug string
    string toString() const;

    // get/set Name
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


private:
    string m_Name;

    // text color
    uint m_Color;

    // chatting message
    string m_Message;
};


//////////////////////////////////////////////////////////////////////
//
// class CGWhisperFactory;
//
// Factory for CGWhisper
//
//////////////////////////////////////////////////////////////////////

class CGWhisperFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_WHISPER;
    static constexpr std::string_view kName = "CGWhisper";
    static constexpr PacketSize_t kMaxSize{szBYTE + 10 + szuint + szBYTE + 128};

    // create packet
    Packet* createPacket() override {
        return new CGWhisper();
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
// class CGWhisperHandler;
//
//////////////////////////////////////////////////////////////////////

class CGWhisperHandler {
public:
    // execute packet's handler
    static void execute(CGWhisper* pPacket, Player* pPlayer);
};

#endif
