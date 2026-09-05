//////////////////////////////////////////////////////////////////////
//
// Filename    : GCPhoneSay.h
// Written By  : elca
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_PHONE_SAY_H__
#define __GC_PHONE_SAY_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCPhoneSay;
//
// Packet used to broadcast a player's PhoneSay chat text to other players.
// The phone chat text is included in the Message field.
//
//////////////////////////////////////////////////////////////////////

class GCPhoneSay : public Packet {
public:
    GCPhoneSay(){};
    ~GCPhoneSay(){};
    // Initialize packet by reading data from the incoming stream.
    void read(SocketInputStream& iStream);

    // Serialize packet data to the outgoing stream.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_PHONE_SAY;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szSlotID + szBYTE + m_Message.size();
    }

    // get packet name
    string getPacketName() const {
        return "GCPhoneSay";
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
// class GCPhoneSayFactory;
//
// Factory for GCPhoneSay
//
//////////////////////////////////////////////////////////////////////

class GCPhoneSayFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_PHONE_SAY;
    static constexpr std::string_view kName = "GCPhoneSay";
    static constexpr PacketSize_t kMaxSize{szSlotID + szBYTE + 128};

    // create packet
    Packet* createPacket() override {
        return new GCPhoneSay();
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
    // Use const static GCPhoneSayPacketMaxSize when possible.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
