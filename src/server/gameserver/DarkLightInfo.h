//////////////////////////////////////////////////////////////////////////////
// Filename    : DarkLightInfo.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __DARK_LIGHT_INFO_H__
#define __DARK_LIGHT_INFO_H__

#include "Exception.h"
#include "Types.h"

class Zone;

//////////////////////////////////////////////////////////////////////////////
// class DarkLightInfo
//////////////////////////////////////////////////////////////////////////////

class DarkLightInfo {
public:
    DarkLightInfo();
    DarkLightInfo(const DarkLightInfo& dli);

public:
    DarkLevel_t getDarkLevel() const {
        return m_DarkLevel;
    }
    void setDarkLevel(DarkLevel_t darkLevel) {
        m_DarkLevel = darkLevel;
    }

    LightLevel_t getLightLevel() const {
        return m_LightLevel;
    }
    void setLightLevel(LightLevel_t lightLevel) {
        m_LightLevel = lightLevel;
    }

    string toString() const;

private:
    // Darkness level
    // 0 - 15; the higher the value, the darker the screen.
    DarkLevel_t m_DarkLevel;

    // Size of the light
    // 1 - 13; the higher the value, the wider the lit area.
    LightLevel_t m_LightLevel;
};


////////////////////////////////////////////////////////////////////////////////
// Size of the DLI array inside the DLIM
////////////////////////////////////////////////////////////////////////////////
const uint nDarkLightInfos = 12 * 24 * 6;


////////////////////////////////////////////////////////////////////////////////
// class DarkLightInfoManager
////////////////////////////////////////////////////////////////////////////////

class DarkLightInfoManager {
public:
    DarkLightInfoManager();
    ~DarkLightInfoManager();

public:
    void init();
    void load();

    DarkLightInfo* getDarkLightInfo(BYTE month, BYTE hour, BYTE minute);
    const DarkLightInfo* getDarkLightInfo(BYTE month, BYTE hour, BYTE minute) const;

    DarkLightInfo* getCurrentDarkLightInfo(Zone* pZone);
    const DarkLightInfo* getCurrentDarkLightInfo(Zone* pZone) const;

    string toString() const;

private:
    DarkLightInfo* m_DarkLightInfos[nDarkLightInfos];
};


#endif
