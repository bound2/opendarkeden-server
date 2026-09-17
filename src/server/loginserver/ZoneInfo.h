//----------------------------------------------------------------------
//
// Filename    : ZoneInfo.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __ZONE_INFO_H__
#define __ZONE_INFO_H__

// include files
#include "Exception.h"
#include "Types.h"

//----------------------------------------------------------------------
//
// class ZoneInfo;
//
// Zone information object for the login server.
//
// Only covers which zone belongs to which zone group.
//
//----------------------------------------------------------------------

class ZoneInfo {
public:
    // get/set zone id
    ZoneID_t getZoneID() const {
        return m_ZoneID;
    }
    void setZoneID(ZoneID_t zoneID) {
        m_ZoneID = zoneID;
    }

    // get/set zone group id
    ZoneGroupID_t getZoneGroupID() const {
        return m_ZoneGroupID;
    }
    void setZoneGroupID(ZoneGroupID_t zoneGroupID) {
        m_ZoneGroupID = zoneGroupID;
    }

    // get debug string
    string toString() const {
        StringStream msg;
        msg << "ZoneInfo(ZoneID:" << m_ZoneID << ",ZoneGroupID:" << m_ZoneGroupID << ")";
        return msg.toString();
    }

private:
    // Zone id
    ZoneID_t m_ZoneID;

    // Zone group id
    ZoneGroupID_t m_ZoneGroupID;
};

#endif
