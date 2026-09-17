//////////////////////////////////////////////////////////////////////
//
// Filename    :  GCGetDamage.h
// Written By  :  elca@ewestsoft.com
// Description :  Packet class sent when a CGMove packet has arrived from
//                the client, to grant the client that sent it permission to
//                move.
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_GET_DAMAGE_H__
#define __GC_GET_DAMAGE_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class  GCGetDamage;
//
// Packet object used when the game server tells the client that a particular
// user has moved. It holds (CreatureID,X,Y,DIR).
//
//////////////////////////////////////////////////////////////////////

class GCGetDamage : public Packet {
public:
    // constructor
    GCGetDamage();

    // destructor
    ~GCGetDamage();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_GET_DAMAGE;
    }

    // get packet size
    PacketSize_t getPacketSize() const {
        return szObjectID + szWORD;
    }

    // get packet's name
    string getPacketName() const {
        return "GCGetDamage";
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

    // get/set Damage
    WORD getDamage() const {
        return m_GetDamage;
    }
    void setDamage(WORD GetDamage) {
        m_GetDamage = GetDamage;
    }

private:
    ObjectID_t m_ObjectID; // ObjectID..
    WORD m_GetDamage;      // Damage..
};


//////////////////////////////////////////////////////////////////////
//
// class  GCGetDamageFactory;
//
// Factory for  GCGetDamage
//
//////////////////////////////////////////////////////////////////////

class GCGetDamageFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_GET_DAMAGE;
    static constexpr std::string_view kName = "GCGetDamage";
    static constexpr PacketSize_t kMaxSize{szObjectID + szWORD};

    // constructor
    GCGetDamageFactory() {}

    // destructor
    virtual ~GCGetDamageFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCGetDamage();
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


#endif
