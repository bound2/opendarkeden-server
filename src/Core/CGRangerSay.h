//////////////////////////////////////////////////////////////////////
//
// Filename    : CGRangerSay.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_RANGER_SAY_H__
#define __CG_RANGER_SAY_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class CGRangerSay;
//
// Packet used when a user holding a DragonEye does Ranger zone chat
//
//////////////////////////////////////////////////////////////////////

class CGRangerSay : public Packet {
public:
    CGRangerSay(){};
    ~CGRangerSay(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_RANGER_SAY;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_Message);
    }

    // get packet name
    string getPacketName() const {
        return "CGRangerSay";
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


private:
    // chatting message
    string m_Message;
};


//////////////////////////////////////////////////////////////////////
//
// class CGRangerSayFactory;
//
// Factory for CGRangerSay
//
//////////////////////////////////////////////////////////////////////

class CGRangerSayFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_RANGER_SAY;
    static constexpr std::string_view kName = "CGRangerSay";
    static constexpr PacketSize_t kMaxSize{szBYTE + 128};

    // create packet
    Packet* createPacket() override {
        return new CGRangerSay();
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
// class CGRangerSayHandler;
//
//////////////////////////////////////////////////////////////////////

class CGRangerSayHandler {
public:
    // execute packet's handler
    static void execute(CGRangerSay* pPacket, Player* pPlayer);
};

#endif
