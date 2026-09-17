//////////////////////////////////////////////////////////////////////
//
// Filename    : LCVersionCheckError.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_VERSION_CHECK_ERROR_H__
#define __LC_VERSION_CHECK_ERROR_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class LCVersionCheckError;
//
//
//////////////////////////////////////////////////////////////////////

class LCVersionCheckError : public Packet {
public:
    LCVersionCheckError(){};
    virtual ~LCVersionCheckError(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_VERSION_CHECK_ERROR;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet's name
    string getPacketName() const {
        return "LCVersionCheckError";
    }

    // get packet's debug string
    string toString() const;
};


//////////////////////////////////////////////////////////////////////
//
// class LCVersionCheckErrorFactory;
//
// Factory for LCVersionCheckError
//
//////////////////////////////////////////////////////////////////////

class LCVersionCheckErrorFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_VERSION_CHECK_ERROR;
    static constexpr std::string_view kName = "LCVersionCheckError";
    static constexpr PacketSize_t kMaxSize{0};

    // create packet
    Packet* createPacket() override {
        return new LCVersionCheckError();
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
