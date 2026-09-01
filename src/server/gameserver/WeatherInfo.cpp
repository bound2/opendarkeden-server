////////////////////////////////////////////////////////////////////////////////
// Filename    : WeatherInfo.cpp
// Written By  : Reiot
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "WeatherInfo.h"

#include "Assert.h"
#include "repository/GameInfoRepository.h"

////////////////////////////////////////////////////////////////////////////////
// global variable definition
////////////////////////////////////////////////////////////////////////////////
WeatherInfoManager* g_pWeatherInfoManager = NULL;

////////////////////////////////////////////////////////////////////////////////
// class WeatherInfo member methods
////////////////////////////////////////////////////////////////////////////////

Weather WeatherInfo::getWeather(uint probability) const

{
    __BEGIN_TRY

    Assert(m_Probabilities[WEATHER_CLEAR] + m_Probabilities[WEATHER_RAINY] + m_Probabilities[WEATHER_SNOWY] == 100);
    Assert(probability <= 100);

    if (probability < m_Probabilities[WEATHER_CLEAR])
        return WEATHER_CLEAR;
    else if (probability < +m_Probabilities[WEATHER_CLEAR] + m_Probabilities[WEATHER_RAINY])
        return WEATHER_RAINY;
    else
        return WEATHER_SNOWY;

    __END_CATCH
}

string WeatherInfo::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "WeatherInfo(" << "Clear:" << m_Probabilities[WEATHER_CLEAR] << "%"
        << ",Rainy:" << m_Probabilities[WEATHER_RAINY] << "%" << ",Snowy:" << m_Probabilities[WEATHER_SNOWY] << "%"
        << ")";
    return msg.toString();

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////////////
// class WeatherInfoManager member methods
////////////////////////////////////////////////////////////////////////////////

void WeatherInfoManager::load()

{
    __BEGIN_TRY

    vector<WeatherRow> rows = defaultGameInfoRepository().loadWeather();

    Assert(rows.size() == 12);

    for (size_t r = 0; r < rows.size(); r++) {
        int month = rows[r].month;
        m_WeatherInfos[month - 1].setProbability(WEATHER_CLEAR, rows[r].clear);
        m_WeatherInfos[month - 1].setProbability(WEATHER_RAINY, rows[r].rainy);
        m_WeatherInfos[month - 1].setProbability(WEATHER_SNOWY, rows[r].snowy);
    }

    __END_CATCH
}

string WeatherInfoManager::toString() const

{
    __BEGIN_TRY
    StringStream msg;

    msg << "WeatherInfoManager(";

    for (int i = 0; i < 12; i++) {
        msg << (int)(i + 1) << " 월 : " << m_WeatherInfos[i].toString();
    }

    msg << ")";

    return msg.toString();
    __END_CATCH
}
