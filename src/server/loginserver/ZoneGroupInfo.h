//----------------------------------------------------------------------
//
// Filename    : ZoneGroupInfo.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __ZONE_GROUP_INFO_H__
#define __ZONE_GROUP_INFO_H__

// include files
#include "Exception.h"
#include "Types.h"

//----------------------------------------------------------------------
//
// class ZoneGroupInfo;
//
// Zone group information object for the login server.
//
// Only covers which zone group is served by which server.
//
//----------------------------------------------------------------------

class ZoneGroupInfo {
public:
    // get/set zone group id
    ZoneGroupID_t getZoneGroupID() const {
        return m_ZoneGroupID;
    }
    void setZoneGroupID(ZoneGroupID_t zoneGroupID) {
        m_ZoneGroupID = zoneGroupID;
    }

    // get/set game server's nick name
    ServerID_t getServerID() const {
        return m_ServerID;
    }
    void setServerID(const ServerID_t ServerID) {
        m_ServerID = ServerID;
    }

    // get debug string
    string toString() const {
        StringStream msg;

        msg << "ZoneGroupInfo(" << "ZoneGroupID:" << m_ZoneGroupID << ",ServerID:" << m_ServerID << ")";

        return msg.toString();
    }

private:
    // Zone group id
    ZoneGroupID_t m_ZoneGroupID;

    // Game server
    ServerID_t m_ServerID;
};

#endif
