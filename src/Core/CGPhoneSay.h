//////////////////////////////////////////////////////////////////////
//
// Filename    : CGPhoneSay.h
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_PHONE_SAY_H__
#define __CG_PHONE_SAY_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class CGPhoneSay;
//
// The PhoneSay packet the client sends to the server.
// It holds only the PhoneSay string as its data field.
//
//////////////////////////////////////////////////////////////////////

class CGPhoneSay : public Packet {
public:
    CGPhoneSay(){};
    ~CGPhoneSay(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_PHONE_SAY;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szSlotID + de::wire::stringWireSize(m_Message);
    }

    // get packet name
    string getPacketName() const {
        return "CGPhoneSay";
    }

    // get packet's debug string
    string toString() const;

    // get/set SlotID
    SlotID_t getSlotID() const {
        return m_SlotID;
    }
    void setSlotID(SlotID_t SlotID) {
        m_SlotID = SlotID;
    }

    // get/set chatting message
    string getMessage() const {
        return m_Message;
    }
    void setMessage(const string& msg) {
        m_Message = msg;
    }


private:
    // SlotID
    SlotID_t m_SlotID;

    // chatting message
    string m_Message;
};


//////////////////////////////////////////////////////////////////////
//
// class CGPhoneSayFactory;
//
// Factory for CGPhoneSay
//
//////////////////////////////////////////////////////////////////////

class CGPhoneSayFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_PHONE_SAY;
    static constexpr std::string_view kName = "CGPhoneSay";
    static constexpr PacketSize_t kMaxSize{szSlotID + szBYTE + 128};

    // create packet
    Packet* createPacket() override {
        return new CGPhoneSay();
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
// class CGPhoneSayHandler;
//
//////////////////////////////////////////////////////////////////////

class CGPhoneSayHandler {
public:
    // execute packet's handler
    static void execute(CGPhoneSay* pPacket, Player* pPlayer);
};

#endif
