//////////////////////////////////////////////////////////////////////
//
// Filename    : CLGetWorldList.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CL_GET_WORLD_LIST_H__
#define __CL_GET_WORLD_LIST_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class CLGetWorldList;
//
//////////////////////////////////////////////////////////////////////

class CLGetWorldList : public Packet {
public:
    CLGetWorldList(){};
    virtual ~CLGetWorldList(){};
    // Initialize the packet by reading data from the input stream.
    void read(SocketInputStream& iStream);

    // Serialize the packet into the output stream.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CL_GET_WORLD_LIST;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return 0;
    }

    // get packet name
    string getPacketName() const {
        return "CLGetWorldList";
    }

    // get packet's debug string
    string toString() const {
        return "CLGetWorldList";
    }

private:
};


//////////////////////////////////////////////////////////////////////
//
// class CLGetWorldListFactory;
//
// Factory for CLGetWorldList
//
//////////////////////////////////////////////////////////////////////

class CLGetWorldListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CL_GET_WORLD_LIST;
    static constexpr std::string_view kName = "CLGetWorldList";
    static constexpr PacketSize_t kMaxSize{0};

    // create packet
    Packet* createPacket() override {
        return new CLGetWorldList();
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
// class CLGetWorldListHandler;
//
//////////////////////////////////////////////////////////////////////

class CLGetWorldListHandler {
public:
    // execute packet's handler
    static void execute(CLGetWorldList* pPacket, Player* player);
};

#endif
