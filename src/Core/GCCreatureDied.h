//////////////////////////////////////////////////////////////////////
//
// Filename    : GCCreatureDied.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_CREATURE_DIED_H__
#define __GC_CREATURE_DIED_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCCreatureDied;
//
// Sent when the game server broadcasts one player's CreatureDied to
// the other players. It holds the character name and the CreatureDied string as its data
// fields.
//
//////////////////////////////////////////////////////////////////////

class GCCreatureDied : public Packet {
public:
    GCCreatureDied() : m_ObjectID(0) {}
    ~GCCreatureDied(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_CREATURE_DIED;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }

    // get packet name
    string getPacketName() const {
        return "GCCreatureDied";
    }

    // get packet's debug string
    string toString() const;

    // get/set dead creature's creature id
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(const ObjectID_t& creatureID) {
        m_ObjectID = creatureID;
    }


private:
    // dead creature's creature id
    ObjectID_t m_ObjectID;
};


//////////////////////////////////////////////////////////////////////
//
// class GCCreatureDiedFactory;
//
// Factory for GCCreatureDied
//
//////////////////////////////////////////////////////////////////////

class GCCreatureDiedFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_CREATURE_DIED;
    static constexpr std::string_view kName = "GCCreatureDied";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    // create packet
    Packet* createPacket() override {
        return new GCCreatureDied();
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
    // *OPTIMIZATION HINT*
    // Define and return const static GCCreatureDiedPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
