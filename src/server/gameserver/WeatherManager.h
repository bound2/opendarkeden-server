//--------------------------------------------------------------------------------
//
// Filename   : WeatherManager.h
// Written By : Reiot
//
//--------------------------------------------------------------------------------

#ifndef __WEATHER_MANAGER_H__
#define __WEATHER_MANAGER_H__

// include files
#include <time.h>

#include "Assert.h"
#include "Exception.h"
#include "Types.h"

class Zone;

//--------------------------------------------------------------------------------
//
// class WeatherManager;
//
// Manages a zone's weather; every zone must own one WeatherManager.
//
// A zone's weather is decided once a day. First the current month is taken
// from GameTime, then the matching WeatherInfo is fetched from
// WeatherInfoManager. A dice roll then picks today's weather as one of
// CLEAR, RAIN or SNOW, after which the day's detail is worked out.
//
// The basic unit of weather is one game hour.
//
// WEATHER_CLEAR : no rain or snow falls.
// WEATHER_RAINY, WEATHER_SNOWY : a dice roll each hour, based on the day's
// chance of rain, starts the rain; the time it stops is rolled as well.
//(The duration of the rain is also measured in units of one game hour.)
//
// Lightning is checked once per real minute, with a chance of (rain level * 5 - 30) %.
//
//--------------------------------------------------------------------------------
class WeatherManager {
public:
    // constructor
    WeatherManager(Zone* pZone) : m_pZone(pZone) {
        Assert(m_pZone != NULL);
    }

    // destructor
    virtual ~WeatherManager();

    // initialize
    void init();

    // Changes the weather when its time comes. Must be called from the zone's heartbeat.
    void heartbeat();

    // Return today's weather.
    Weather getTodayWeather() const {
        return m_TodayWeather;
    }

    // Return today's chance of rain or snow.
    uint getProbability() const {
        return m_Probability;
    }

    // Return the current weather.
    Weather getCurrentWeather() const {
        return m_CurrentWeather;
    }

    // Return the current weather level.
    WeatherLevel_t getWeatherLevel() const {
        return m_WeatherLevel;
    }

    void resetDarkLightInfo() {
        m_Next10Min = time(0);
    }

    // get debug string
    string toString() const;

private:
    // The zone this manager is attached to.
    Zone* m_pZone;

    // Today's weather (CLEAR/RAINY/SNOWY)
    Weather m_TodayWeather;

    // Chance of rain or snow (0 - 100)
    uint m_Probability;

    // The current weather
    Weather m_CurrentWeather;

    // Weather level (1 - 20)
    WeatherLevel_t m_WeatherLevel;


    //--------------------------------------------------
    // Time of the next XXX (second resolution is plenty)
    //--------------------------------------------------
private:
    // Tomorrow
    time_t m_Tomorrow;

    // Time of the next weather change
    time_t m_NextWeatherChangingTime;

    // Time of the next lightning
    time_t m_NextLightning;

    // The next 10-minute mark
    time_t m_Next10Min;
};

#endif
