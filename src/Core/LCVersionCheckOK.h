//////////////////////////////////////////////////////////////////////
//
// Filename    : LCVersionCheckOK.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_VERSION_CHECK_OK_H__
#define __LC_VERSION_CHECK_OK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class LCVersionCheckOK;
//
// Packet with which the login server tells the client that login succeeded.
//
//////////////////////////////////////////////////////////////////////

class LCVersionCheckOK : public Packet {
public:
    LCVersionCheckOK(){};
    virtual ~LCVersionCheckOK(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_VERSION_CHECK_OK;
    }

    // get packet body size
    // *OPTIMIZATION HINT*
    // Define and return const static LCVersionCheckOKPacketSize.
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet's name
    string getPacketName() const {
        return "LCVersionCheckOK";
    }

    // get packet's debug string
    string toString() const {
        return "LCVersionCheckOK";
    }
};


//////////////////////////////////////////////////////////////////////
//
// class LCVersionCheckOKFactory;
//
// Factory for LCVersionCheckOK
//
//////////////////////////////////////////////////////////////////////

class LCVersionCheckOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_VERSION_CHECK_OK;
    static constexpr std::string_view kName = "LCVersionCheckOK";
    static constexpr PacketSize_t kMaxSize{0};

    // create packet
    Packet* createPacket() override {
        return new LCVersionCheckOK();
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
