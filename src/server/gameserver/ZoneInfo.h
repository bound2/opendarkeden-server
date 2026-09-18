//////////////////////////////////////////////////////////////////////////////
// Filename    : ZoneInfo.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __ZONEINFO_H__
#define __ZONEINFO_H__

#include "Exception.h"
#include "Types.h"
#include "Zone.h"

//////////////////////////////////////////////////////////////////////////////
// class ZoneInfo;
//////////////////////////////////////////////////////////////////////////////

class ZoneInfo {
public:
    ZoneInfo();
    ~ZoneInfo();

    void load(ZoneID_t zoneID);

public:
    ZoneID_t getZoneID() const {
        return m_ZoneID;
    }
    void setZoneID(ZoneID_t zoneID) {
        m_ZoneID = zoneID;
    }

    ZoneGroupID_t getZoneGroupID() const {
        return m_ZoneGroupID;
    }
    void setZoneGroupID(ZoneGroupID_t zoneGroupID) {
        m_ZoneGroupID = zoneGroupID;
    }

    ZoneType getZoneType() const {
        return m_ZoneType;
    }
    void setZoneType(ZoneType zoneType) {
        m_ZoneType = zoneType;
    }
    void setZoneType(const string& zoneType);

    ZoneLevel_t getZoneLevel() const {
        return m_ZoneLevel;
    }
    void setZoneLevel(ZoneLevel_t zoneLevel) {
        m_ZoneLevel = zoneLevel;
    }

    ZoneAccessMode getZoneAccessMode() const {
        return m_ZoneAccessMode;
    }
    void setZoneAccessMode(ZoneAccessMode zoneAccessMode) {
        m_ZoneAccessMode = zoneAccessMode;
    }
    void setZoneAccessMode(const string& zoneAccessMode);

    bool isPayPlay() const {
        return m_bPayPlay;
    }
    void setPayPlay(bool bPayPlay = true) {
        m_bPayPlay = bPayPlay;
    }

    bool isPremiumZone() const {
        return m_bPremiumZone;
    }
    void setPremiumZone(bool bPremiumZone = true) {
        m_bPremiumZone = bPremiumZone;
    }

    string getZoneOwnerID() const {
        return m_OwnerID;
    }
    void setZoneOwnerID(const string& ownerID) {
        m_OwnerID = ownerID;
    }

    string getSMPFilename(void) const {
        return m_SMPFilename;
    }
    void setSMPFilename(const string& name) {
        m_SMPFilename = name;
    }

    string getSSIFilename(void) const {
        return m_SSIFilename;
    }
    void setSSIFilename(const string& name) {
        m_SSIFilename = name;
    }

    string getFullName(void) const {
        return m_FullName;
    }
    void setFullName(const string& name) {
        m_FullName = name;
    }

    string getShortName(void) const {
        return m_ShortName;
    }
    void setShortName(const string& name) {
        m_ShortName = name;
    }

    bool isPKZone() const {
        return m_bPKZone;
    }
    void setPKZone(bool bPKZone = true) {
        m_bPKZone = bPKZone;
    }

    bool isNoPortalZone() const {
        return m_bNoPortalZone;
    }
    void setNoPortalZone(bool bNoPortalZone = true) {
        m_bNoPortalZone = bNoPortalZone;
    }

    bool isMasterLair() const {
        return m_bMasterLair;
    }
    void setMasterLair(bool bMasterLair = true) {
        m_bMasterLair = bMasterLair;
    }

    bool isCastle() const {
        return m_bCastle;
    }
    void setCastle(bool bCastle = true) {
        m_bCastle = bCastle;
    }

    bool isHolyLand() const {
        return m_bHolyLand;
    }
    void setHolyLand(bool bHolyLand = true) {
        m_bHolyLand = bHolyLand;
    }

    bool isLevelWarZone() const {
        switch (m_ZoneID) {
        case 1131:
        case 1132:
        case 1133:
        case 1134: {
            return true;
        } break;
        default: {
            return false;
        } break;
        }
    }

    bool isAvailable() const {
        return m_bAvailable;
    }
    void setAvailable(bool bAvailable = true) {
        m_bAvailable = bAvailable;
    }

    int getOpenLevel() const {
        return m_OpenLevel;
    }
    void setOpenLevel(int OpenLevel) {
        m_OpenLevel = OpenLevel;
    }

    string toString() const;

private:
    ZoneID_t m_ZoneID;               // Zone id
    ZoneGroupID_t m_ZoneGroupID;     // Zone group id
    ZoneType m_ZoneType;             // Zone type
    ZoneLevel_t m_ZoneLevel;         // Zone level (0 - 10)
    ZoneAccessMode m_ZoneAccessMode; // Zone access mode
    bool m_bPayPlay;                 // Pay zone? = a place you must pay to enter
    bool m_bPremiumZone;             // Zone covered by the premium service
    string m_OwnerID;                // Zone owner id
    string m_SMPFilename;            // SMP file name
    string m_SSIFilename;            // SSI file name
    string m_FullName;               // Zone name
    string m_ShortName;              // Short name

    bool m_bPKZone;       // Zone where alignment does not drop
    bool m_bNoPortalZone; // Zone that cannot be reached by portal
    bool m_bMasterLair;   // Is it a master lair?
    bool m_bCastle;       // Is it a Castle?
    bool m_bHolyLand;     // Is it Adam's holy land?
    bool m_bAvailable;    // Is this zone meant to show up?

    int m_OpenLevel; // Open level of the zone. It appears only when OpenDegree is higher than this.
};

#endif
