//////////////////////////////////////////////////////////////////////
//
// Filename    : CLGetPCList.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CL_GET_PC_LIST_H__
#define __CL_GET_PC_LIST_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class CLGetPCList;
//
//////////////////////////////////////////////////////////////////////

class CLGetPCList : public Packet {
public:
    CLGetPCList(){};
    virtual ~CLGetPCList(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CL_GET_PC_LIST;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet name
    string getPacketName() const {
        return "CLGetPCList";
    }

    // get packet's debug string
    string toString() const {
        return "CLGetPCList";
    }

private:
};


//////////////////////////////////////////////////////////////////////
//
// class CLGetPCListFactory;
//
// Factory for CLGetPCList
//
//////////////////////////////////////////////////////////////////////

class CLGetPCListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CL_GET_PC_LIST;
    static constexpr std::string_view kName = "CLGetPCList";
    static constexpr PacketSize_t kMaxSize{0};

    // create packet
    Packet* createPacket() override {
        return new CLGetPCList();
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
// class CLGetPCListHandler;
//
//////////////////////////////////////////////////////////////////////

class CLGetPCListHandler {
public:
    // execute packet's handler
    static void execute(CLGetPCList* pPacket, Player* player);
};

#endif
