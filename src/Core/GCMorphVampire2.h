//----------------------------------------------------------------------
//
// Filename    : GCMorphVampire2.h
// Written By  : crazydog
//
//----------------------------------------------------------------------

#ifndef __GC_MORPH_VAMPIRE2_H__
#define __GC_MORPH_VAMPIRE2_H__

// include files
#include "PCSlayerInfo3.h"
#include "PCVampireInfo3.h"
#include "Packet.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class GCMorphVampire2;
//
// Sent to those watching a slayer or the like transform into a vampire.
//----------------------------------------------------------------------

class GCMorphVampire2 : public Packet {
public:
    // constructor
    GCMorphVampire2() {}
    GCMorphVampire2(PCVampireInfo3 info) : m_VampireInfo3(info) {}
    ~GCMorphVampire2(){};

public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_MORPH_VAMPIRE_2;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return m_VampireInfo3.getSize();
    }

    // get packet's name
    string getPacketName() const {
        return "GCMorphVampire2";
    }

    // get packet's debug string
    string toString() const;


public:
    // get/set vampire info
    const PCVampireInfo3& getVampireInfo() {
        return m_VampireInfo3;
    }
    void setVampireInfo(PCVampireInfo3 info) {
        m_VampireInfo3 = info;
    }


private:
    // Appearance information
    PCVampireInfo3 m_VampireInfo3;
};


//--------------------------------------------------------------------------------
//
// class GCMorphVampire2Factory;
//
// Factory for GCMorphVampire2
//
//--------------------------------------------------------------------------------

class GCMorphVampire2Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MORPH_VAMPIRE_2;
    static constexpr std::string_view kName = "GCMorphVampire2";
    static constexpr PacketSize_t kMaxSize{PCVampireInfo3::getMaxSize()};

    // create packet
    Packet* createPacket() override {
        return new GCMorphVampire2();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's body size
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
//
//--------------------------------------------------------------------------------

#endif
