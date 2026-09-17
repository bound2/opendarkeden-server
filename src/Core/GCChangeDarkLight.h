//--------------------------------------------------------------------------------
//
// Filename    : GCChangeDarkLight.h
// Written By  : reiot
//
//--------------------------------------------------------------------------------

#ifndef __GC_CHANGE_DARK_LIGHT_H__
#define __GC_CHANGE_DARK_LIGHT_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//--------------------------------------------------------------------------------
//
// class GCChangeDarkLight;
//
//--------------------------------------------------------------------------------

class GCChangeDarkLight : public Packet {
public:
    GCChangeDarkLight(){};
    ~GCChangeDarkLight(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_CHANGE_DARK_LIGHT;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCChangeDarkLightPacketSize.
    PacketSize_t getPacketSize() const {
        return szDarkLevel + szLightLevel;
    }

    // get packet's name
    string getPacketName() const {
        return "GCChangeDarkLight";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set dark level
    DarkLevel_t getDarkLevel() const {
        return m_DarkLevel;
    }
    void setDarkLevel(DarkLevel_t darkLevel) {
        m_DarkLevel = darkLevel;
    }

    // get/set light level
    LightLevel_t getLightLevel() const {
        return m_LightLevel;
    }
    void setLightLevel(LightLevel_t lightLevel) {
        m_LightLevel = lightLevel;
    }


public:
    // Zone darkness (0 - 15)
    DarkLevel_t m_DarkLevel = 0;

    // Size of the zone's light (1 - 13)
    LightLevel_t m_LightLevel = 0;
};


//--------------------------------------------------------------------------------
//
// class GCChangeDarkLightFactory;
//
// Factory for GCChangeDarkLight
//
//--------------------------------------------------------------------------------

class GCChangeDarkLightFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_CHANGE_DARK_LIGHT;
    static constexpr std::string_view kName = "GCChangeDarkLight";
    static constexpr PacketSize_t kMaxSize{szDarkLevel + szLightLevel};

    // create packet
    Packet* createPacket() override {
        return new GCChangeDarkLight();
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
    // Define and return const static GCChangeDarkLightPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
//
//--------------------------------------------------------------------------------

#endif
