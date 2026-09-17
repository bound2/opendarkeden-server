//////////////////////////////////////////////////////////////////////
//
// Filename    : CLLogout.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CL_LOGOUT_H__
#define __CL_LOGOUT_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class CLLogout;
//
// The packet with which the client tells the server it is logging out.
// It has no data field, so getSize() returns 0 and the read() and write()
// methods do nothing.
//
//////////////////////////////////////////////////////////////////////

class CLLogout : public Packet {
public:
    CLLogout(){};
    virtual ~CLLogout(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CL_LOGOUT;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet name
    string getPacketName() const {
        return "CLLogout";
    }

    // get packet's debug string
    string toString() const {
        return "CLLogout";
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CLLogoutFactory;
//
// Factory for CLLogout
//
//////////////////////////////////////////////////////////////////////

class CLLogoutFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CL_LOGOUT;
    static constexpr std::string_view kName = "CLLogout";
    static constexpr PacketSize_t kMaxSize{0};

    // create packet
    Packet* createPacket() override {
        return new CLLogout();
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
// class CLLogoutHandler;
//
//////////////////////////////////////////////////////////////////////

class CLLogoutHandler {
public:
    // execute packet's handler
    static void execute(CLLogout* pPacket, Player* player);
};

#endif
