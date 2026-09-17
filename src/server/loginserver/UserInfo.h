//----------------------------------------------------------------------
//
// Filename    : UserInfo.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __USER_INFO_H__
#define __USER_INFO_H__

// include files
#include "Exception.h"
#include "Types.h"

//----------------------------------------------------------------------
//
// class UserInfo;
//
// Zone group information object for the login server.
//
// Only covers which zone group is served by which server.
//
//----------------------------------------------------------------------

class UserInfo {
public:
    // get/set zone group id
    WorldID_t getWorldID() const {
        return m_WorldID;
    }
    void setWorldID(WorldID_t WorldID) {
        m_WorldID = WorldID;
    }

    // get/set zone group id
    ZoneGroupID_t getServerGroupID() const {
        return m_ServerGroupID;
    }
    void setServerGroupID(ZoneGroupID_t GroupID) {
        m_ServerGroupID = GroupID;
    }

    // get/set zone group id
    UserNum_t getUserNum() const {
        return m_UserNum;
    }
    void setUserNum(UserNum_t UserNum) {
        m_UserNum = UserNum;
    }


    // get debug string
    string toString() const {
        StringStream msg;

        msg << "UserInfo(" << "WorldID:" << m_WorldID << "ServerGroupID:" << m_ServerGroupID << ",UserNum:" << m_UserNum
            << ")";

        return msg.toString();
    }

private:
    // World ID
    WorldID_t m_WorldID;

    // Game server
    ZoneGroupID_t m_ServerGroupID;

    // Number
    UserNum_t m_UserNum;
};

#endif
