//--------------------------------------------------------------------------------
//
// Filename    : GCChangeWeather.h
// Written By  : reiot
//
//--------------------------------------------------------------------------------

#ifndef __GC_CHANGE_WEATHER_H__
#define __GC_CHANGE_WEATHER_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//--------------------------------------------------------------------------------
//
// class GCChangeWeather;
//
//--------------------------------------------------------------------------------

class GCChangeWeather : public Packet {
public:
    GCChangeWeather(){};
    ~GCChangeWeather(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_CHANGE_WEATHER;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCChangeWeatherPacketSize.
    PacketSize_t getPacketSize() const {
        return szWeather + szWeatherLevel;
    }

    // get packet's name
    string getPacketName() const {
        return "GCChangeWeather";
    }

    // get packet's debug string
    string toString() const;

public:
    Weather getWeather() const {
        return m_Weather;
    }
    void setWeather(Weather weather) {
        m_Weather = weather;
    }

    WeatherLevel_t getWeatherLevel() const {
        return m_WeatherLevel;
    }
    void setWeatherLevel(WeatherLevel_t weatherLevel) {
        m_WeatherLevel = weatherLevel;
    }

public:
    Weather m_Weather = WEATHER_CLEAR;

    WeatherLevel_t m_WeatherLevel = 0;
};


//--------------------------------------------------------------------------------
//
// class GCChangeWeatherFactory;
//
// Factory for GCChangeWeather
//
//--------------------------------------------------------------------------------

class GCChangeWeatherFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_CHANGE_WEATHER;
    static constexpr std::string_view kName = "GCChangeWeather";
    static constexpr PacketSize_t kMaxSize{szWeather + szWeatherLevel};

    // create packet
    Packet* createPacket() override {
        return new GCChangeWeather();
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
    // Define and return const static GCChangeWeatherPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
//
//--------------------------------------------------------------------------------

#endif
