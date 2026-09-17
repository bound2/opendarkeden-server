//////////////////////////////////////////////////////////////////////
//
// Filename    : CGSelectPortal.h
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_SELECT_PORTAL_H__
#define __CG_SELECT_PORTAL_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class CGSelectPortal;
//
//////////////////////////////////////////////////////////////////////

class CGSelectPortal : public Packet {
public:
    // constructor
    CGSelectPortal();

    // destructor
    ~CGSelectPortal();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_SELECT_PORTAL;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szZoneID;
    }

    // get packet name
    string getPacketName() const {
        return "CGSelectPortal";
    }

    // get / set ZoneID
    ObjectID_t getZoneID() const {
        return m_ZoneID;
    }
    void setZoneID(ZoneID_t ZoneID) {
        m_ZoneID = ZoneID;
    }

    // get packet's debug string
    string toString() const;

private:
    ZoneID_t m_ZoneID = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class CGSelectPortalFactory;
//
// Factory for CGSelectPortal
//
//////////////////////////////////////////////////////////////////////

class CGSelectPortalFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_SELECT_PORTAL;
    static constexpr std::string_view kName = "CGSelectPortal";
    static constexpr PacketSize_t kMaxSize{szZoneID};

    // constructor
    CGSelectPortalFactory() {}

    // destructor
    virtual ~CGSelectPortalFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGSelectPortal();
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
// class CGSelectPortalHandler;
//
//////////////////////////////////////////////////////////////////////

class CGSelectPortalHandler {
public:
    // execute packet's handler
    static void execute(CGSelectPortal* pCGSelectPortal, Player* pPlayer);
};

#endif
