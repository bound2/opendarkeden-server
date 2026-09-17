//////////////////////////////////////////////////////////////////////
//
// Filename    : LCSelectPCError.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_SELECT_PC_ERROR_H__
#define __LC_SELECT_PC_ERROR_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

enum SELECT_PC_ERROR {
    SELECT_PC_ERROR_NULL,

    SELECT_PC_CANNOT_PLAY,         // Character that cannot play (billing related)
    SELECT_PC_NOT_BILLING_CHECK,   // The payment information has not been checked yet.
    SELECT_PC_CANNOT_PLAY_BY_ATTR, // Cannot play for free any longer because of the attributes.
    SELECT_PC_DIDNOT_AGREE,        // Cannot play because the Netmarble terms were not accepted.
};

//////////////////////////////////////////////////////////////////////
//
// class LCSelectPCError;
//
// When the PC the player chose does not exist, or when the game server of
// the zone it logged out of is down, or when the DB server is down.
// The reason for the error is put in this packet and sent to the client.
//
//////////////////////////////////////////////////////////////////////

class LCSelectPCError : public Packet {
public:
    LCSelectPCError(){};
    ~LCSelectPCError(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_SELECT_PC_ERROR;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "LCSelectPCError";
    }

    // get packet's debug string
    string toString() const;

    // get/set error message
    BYTE getCode() const {
        return m_Code;
    }
    void setCode(BYTE code) {
        m_Code = code;
    }

private:
    // Error message
    BYTE m_Code;
};


//////////////////////////////////////////////////////////////////////
//
// class LCSelectPCErrorFactory;
//
// Factory for LCSelectPCError
//
//////////////////////////////////////////////////////////////////////

class LCSelectPCErrorFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_SELECT_PC_ERROR;
    static constexpr std::string_view kName = "LCSelectPCError";
    static constexpr PacketSize_t kMaxSize{szBYTE};

    // create packet
    Packet* createPacket() override {
        return new LCSelectPCError();
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
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
