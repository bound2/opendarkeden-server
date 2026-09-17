//////////////////////////////////////////////////////////////////////
//
// Filename    : GCRemoveInjuriousCreature.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_REMOVE_INJURIOUS_CREATURE_H__
#define __GC_REMOVE_INJURIOUS_CREATURE_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class GCRemoveInjuriousCreature;
//
// The RemoveInjuriousCreature packet the client sends to the server.
// It holds only the RemoveInjuriousCreature string as a data field.
//
//////////////////////////////////////////////////////////////////////

class GCRemoveInjuriousCreature : public Packet {
public:
    GCRemoveInjuriousCreature(){};
    ~GCRemoveInjuriousCreature(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_REMOVE_INJURIOUS_CREATURE;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_Name);
    }

    // get packet name
    string getPacketName() const {
        return "GCRemoveInjuriousCreature";
    }

    // get packet's debug string
    string toString() const;

    // get/set Name
    string getName() const {
        return m_Name;
    }
    void setName(const string& Name) {
        m_Name = Name;
    }

private:
    string m_Name;
};


//////////////////////////////////////////////////////////////////////
//
// class GCRemoveInjuriousCreatureFactory;
//
// Factory for GCRemoveInjuriousCreature
//
//////////////////////////////////////////////////////////////////////

class GCRemoveInjuriousCreatureFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_REMOVE_INJURIOUS_CREATURE;
    static constexpr std::string_view kName = "GCRemoveInjuriousCreature";
    static constexpr PacketSize_t kMaxSize{szBYTE + 10};

    // create packet
    Packet* createPacket() override {
        return new GCRemoveInjuriousCreature();
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
    // The maximum size of the message has to be set.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
