//////////////////////////////////////////////////////////////////////
//
// Filename    : GCAddInjuriousCreature.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_ADD_INJURIOUS_CREATURE_H__
#define __GC_ADD_INJURIOUS_CREATURE_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class GCAddInjuriousCreature;
//
// The AddInjuriousCreature packet the client sends to the server.
// It holds only the AddInjuriousCreature string as its data field.
//
//////////////////////////////////////////////////////////////////////

class GCAddInjuriousCreature : public Packet {
public:
    GCAddInjuriousCreature(){};
    ~GCAddInjuriousCreature(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_INJURIOUS_CREATURE;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_Name);
    }

    // get packet name
    string getPacketName() const {
        return "GCAddInjuriousCreature";
    }

    // get packet's debug string
    string toString() const;

    // get/set Name
    // The field carries a character name, so it stops where one does.
    string getName() const {
        return m_Name;
    }
    void setName(const string& Name) {
        m_Name = (Name.size() > maxNameLength) ? Name.substr(0, maxNameLength) : Name;
    }

private:
    string m_Name;
};


//////////////////////////////////////////////////////////////////////
//
// class GCAddInjuriousCreatureFactory;
//
// Factory for GCAddInjuriousCreature
//
//////////////////////////////////////////////////////////////////////

class GCAddInjuriousCreatureFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_INJURIOUS_CREATURE;
    static constexpr std::string_view kName = "GCAddInjuriousCreature";
    static constexpr PacketSize_t kMaxSize{szBYTE + maxNameLength};

    // create packet
    Packet* createPacket() override {
        return new GCAddInjuriousCreature();
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
    // The maximum size of message needs to be configured.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
