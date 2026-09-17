//----------------------------------------------------------------------
//
// Filename    : GameServerGroupInfo.h
// Written By  : Reiot
// Description : Information the login server holds about each game server
//
//----------------------------------------------------------------------

#ifndef __LOGIN_SERVER_GAME_SERVER_GROUP_INFO_H__
#define __LOGIN_SERVER_GAME_SERVER_GROUP_INFO_H__

// include files
#include "Exception.h"
#include "StringStream.h"
#include "Types.h"


//----------------------------------------------------------------------
//
// class GameServerGroupInfo;
//
// Class holding each game server's information read from the GameServerGroupInfo
// table of the GAME DB.
//
//----------------------------------------------------------------------

class GameServerGroupInfo {
public:
    // get/set GameWorldID
    WorldID_t getWorldID() const {
        return m_WorldID;
    }
    void setWorldID(WorldID_t WorldID) {
        m_WorldID = WorldID;
    }

    // get/set GameServerGroupID
    ServerGroupID_t getGroupID() const {
        return m_GroupID;
    }
    void setGroupID(ServerGroupID_t GroupID) {
        m_GroupID = GroupID;
    }

    // get/set host name
    string getGroupName() const {
        return m_GroupName;
    }
    void setGroupName(string GroupName) {
        m_GroupName = GroupName;
    }

    // get/set host name
    BYTE getStat() const {
        return m_Stat;
    }
    void setStat(BYTE stat) {
        m_Stat = stat;
    }

    // get debug string
    string toString() const {
        StringStream msg;
        msg << "GameServerGroupInfo(" << "WorldID : " << (int)m_WorldID << "ServerGroupID: " << (int)m_GroupID
            << ",GroupName:" << m_GroupName << ")";
        return msg.toString();
    }

private:
    // WorldID
    WorldID_t m_WorldID;

    // GameServerGroup ID
    ServerGroupID_t m_GroupID;

    // GameServerGroup Process's nick name
    string m_GroupName;

    BYTE m_Stat;
};

#endif
