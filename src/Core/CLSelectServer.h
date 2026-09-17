//////////////////////////////////////////////////////////////////////
//
// Filename    : CLSelectServer.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CL_SELECT_SERVER_H__
#define __CL_SELECT_SERVER_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class CLSelectServer;
//
//////////////////////////////////////////////////////////////////////

class CLSelectServer : public Packet {
public:
    CLSelectServer(){};
    virtual ~CLSelectServer(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CL_SELECT_SERVER;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szServerGroupID;
    }

    // get packet name
    string getPacketName() const {
        return "CLSelectServer";
    }

    // get / set ServerGroupID
    ServerGroupID_t getServerGroupID() const {
        return m_ServerGroupID;
    }
    void setServerGroupID(ServerGroupID_t ServerGroupID) {
        m_ServerGroupID = ServerGroupID;
    }

    // get packet's debug string
    string toString() const {
        return "CLSelectServer";
    }

private:
    ServerGroupID_t m_ServerGroupID;
};


//////////////////////////////////////////////////////////////////////
//
// class CLSelectServerFactory;
//
// Factory for CLSelectServer
//
//////////////////////////////////////////////////////////////////////

class CLSelectServerFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CL_SELECT_SERVER;
    static constexpr std::string_view kName = "CLSelectServer";
    static constexpr PacketSize_t kMaxSize{szServerGroupID};

    // create packet
    Packet* createPacket() override {
        return new CLSelectServer();
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
// class CLSelectServerHandler;
//
//////////////////////////////////////////////////////////////////////

class CLSelectServerHandler {
public:
    // execute packet's handler
    static void execute(CLSelectServer* pPacket, Player* player);
};

#endif
