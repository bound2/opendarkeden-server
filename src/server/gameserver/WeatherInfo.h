////////////////////////////////////////////////////////////////////////////////
// Filename    : WeatherInfo.h
// Written By  : Reiot
// Description :
////////////////////////////////////////////////////////////////////////////////

#ifndef __WEATHER_INFO_H__
#define __WEATHER_INFO_H__

#include "Exception.h"
#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
// class WeatherInfo;
////////////////////////////////////////////////////////////////////////////////

class WeatherInfo {
public:
    // Given a probability value, returns today's weather.
    Weather getWeather(uint probability) const;

    // Returns the probability of a given weather.
    uint getProbability(Weather weather) const {
        return m_Probabilities[weather];
    }

    // Sets the probability of a given weather.
    void setProbability(Weather weather, uint prob) {
        m_Probabilities[weather] = prob;
    }

    // get debug string
    string toString() const;

private:
    uint m_Probabilities[WEATHER_MAX];
};


////////////////////////////////////////////////////////////////////////////////
// class WeatherInfoManager;
//
// Manager class for the per-month weather info.
////////////////////////////////////////////////////////////////////////////////

class WeatherInfoManager {
public:
    // init vision info
    void init() {
        load();
    }

    // load from database
    void load();

    // save to database
    void save() {
        throw UnsupportedError();
    }

    // get vision info
    const WeatherInfo& getWeatherInfo(int month) const {
        if (month > 12)
            throw OutOfBoundException("too large month value");
        return m_WeatherInfos[month - 1];
    }

    // get debug string
    string toString() const;

private:
    // WeatherInfo for each month
    WeatherInfo m_WeatherInfos[12];
};

#endif
