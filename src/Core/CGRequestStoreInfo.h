//////////////////////////////////////////////////////////////////////
//
// Filename    : CGRequestStoreInfo.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_REQUEST_STORE_INFO_H__
#define __CG_REQUEST_STORE_INFO_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class CGRequestStoreInfo;
//
//////////////////////////////////////////////////////////////////////

class CGRequestStoreInfo : public Packet {
public:
    CGRequestStoreInfo(){};
    virtual ~CGRequestStoreInfo(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_REQUEST_STORE_INFO;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }

    // get packet name
    string getPacketName() const {
        return "CGRequestStoreInfo";
    }

    // get packet's debug string
    string toString() const;

    ObjectID_t getOwnerObjectID() const {
        return m_OwnerObjectID;
    }
    void setOwnerObjectID(ObjectID_t oid) {
        m_OwnerObjectID = oid;
    }

private:
    ObjectID_t m_OwnerObjectID = 0; // 0 means the player's own store information
};


//////////////////////////////////////////////////////////////////////
//
// class CGRequestStoreInfoFactory;
//
// Factory for CGRequestStoreInfo
//
//////////////////////////////////////////////////////////////////////

class CGRequestStoreInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_REQUEST_STORE_INFO;
    static constexpr std::string_view kName = "CGRequestStoreInfo";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    // constructor
    CGRequestStoreInfoFactory() {}

    // destructor
    virtual ~CGRequestStoreInfoFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGRequestStoreInfo();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get Packet Max Size
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGRequestStoreInfoHandler;
//
//////////////////////////////////////////////////////////////////////

class CGRequestStoreInfoHandler {
public:
    // execute packet's handler
    static void execute(CGRequestStoreInfo* pCGRequestStoreInfo, Player* pPlayer);
};

#endif
