//////////////////////////////////////////////////////////////////////
//
// Filename    : CGBloodDrain
// Written By  : crazydog
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_BLOOD_DRAIN_H__
#define __CG_BLOOD_DRAIN_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

#ifdef __LINUX__
// #include "GCGlobalHandler.h"
#endif // __LINUX__


//////////////////////////////////////////////////////////////////////
//
// class CGBloodDrain;
//
//////////////////////////////////////////////////////////////////////

class CGBloodDrain : public Packet {
public:
    // constructor
    CGBloodDrain();

    // destructor
    ~CGBloodDrain();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_BLOOD_DRAIN;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }

    // get packet name
    string getPacketName() const {
        return "CGBloodDrain";
    }

    // get packet's debug string
    string toString() const;
    // get/set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

private:
    ObjectID_t m_ObjectID = 0; // ObjectID
    /*
        Coord_t m_X;			// X coordinate
        Coord_t m_Y;			// Y coordinate
        Dir_t m_Dir;			// Direction
    */
};


//////////////////////////////////////////////////////////////////////
//
// class CGBloodDrainFactory;
//
// Factory for CGBloodDrain
//
//////////////////////////////////////////////////////////////////////

class CGBloodDrainFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_BLOOD_DRAIN;
    static constexpr std::string_view kName = "CGBloodDrain";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    // constructor
    CGBloodDrainFactory() {}

    // destructor
    virtual ~CGBloodDrainFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGBloodDrain();
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
// class CGBloodDrainHandler;
//
//////////////////////////////////////////////////////////////////////

class CGBloodDrainHandler {
public:
    // execute packet's handler
    static void execute(CGBloodDrain* pCGBloodDrain, Player* player);
};

#endif
