//--------------------------------------------------------------------------------
//
// Filename    : GCChangeWeather.cpp
// Written By  : Reiot
//
//--------------------------------------------------------------------------------

// include files
#include "GCChangeWeather.h"


//--------------------------------------------------------------------------------
// Read data from the input stream (buffer) and initialise the packet.
//--------------------------------------------------------------------------------
void GCChangeWeather::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The enum declares fewer values than the byte carries, so the raw
    // byte is tested before it reaches it.
    BYTE weather = 0;
    iStream.read(weather);

    if (weather >= WEATHER_MAX)
        throw InvalidProtocolException("weather out of range");

    m_Weather = (Weather)weather;

    iStream.read(m_WeatherLevel);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Send the packet's binary image to the output stream (buffer).
//--------------------------------------------------------------------------------
void GCChangeWeather::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write((BYTE)m_Weather);
    oStream.write(m_WeatherLevel);

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get packet's debug string
//--------------------------------------------------------------------------------
string GCChangeWeather::toString() const

{
    __BEGIN_TRY

    // Weather2String names only the weathers below WEATHER_MAX; any
    // other value prints as its number.
    StringStream msg;
    msg << "GCChangeWeather("
        << "Weather:" << (m_Weather < WEATHER_MAX ? Weather2String[m_Weather] : std::to_string((int)m_Weather))
        << ",WeatherLevel:" << (int)m_WeatherLevel << ")";
    return msg.toString();

    __END_CATCH
}
