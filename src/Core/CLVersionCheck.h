//--------------------------------------------------------------------------------
//
// Filename    : CLVersionCheck.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __CL_VERSION_CHECK_H__
#define __CL_VERSION_CHECK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//--------------------------------------------------------------------------------
//
// class CLVersionCheck;
//
// The first packet the client sends to the login server.
// The id and the password are encrypted.
//
//--------------------------------------------------------------------------------

class CLVersionCheck : public Packet {
public:
    CLVersionCheck(){};
    ~CLVersionCheck(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CL_VERSION_CHECK;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szDWORD;
    }

    // get packet name
    string getPacketName() const {
        return "CLVersionCheck";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set Client Version
    DWORD getVersion() const {
        return m_Version;
    }
    void setVersion(DWORD Version) {
        m_Version = Version;
    }

private:
    // Client version
    DWORD m_Version;
};


//--------------------------------------------------------------------------------
//
// class CLVersionCheckFactory;
//
// Factory for CLVersionCheck
//
//--------------------------------------------------------------------------------

class CLVersionCheckFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CL_VERSION_CHECK;
    static constexpr std::string_view kName = "CLVersionCheck";
    static constexpr PacketSize_t kMaxSize{szDWORD};

    // create packet
    Packet* createPacket() override {
        return new CLVersionCheck();
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


//--------------------------------------------------------------------------------
//
// class CLVersionCheckHandler;
//
//--------------------------------------------------------------------------------

class CLVersionCheckHandler {
public:
    // execute packet's handler
    static void execute(CLVersionCheck* pPacket, Player* pPlayer);
};

#endif
