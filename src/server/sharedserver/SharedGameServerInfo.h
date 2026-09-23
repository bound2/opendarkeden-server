//----------------------------------------------------------------------
//
// Filename    : SharedGameServerInfo.h
// Written By  : Reiot
// Description : Information the login server holds about each game server
//
//----------------------------------------------------------------------

#ifndef __SHARED_SERVER_GAME_SERVER_INFO_H__
#define __SHARED_SERVER_GAME_SERVER_INFO_H__

// include files
#include "Exception.h"
#include "StringStream.h"
#include "Types.h"


//----------------------------------------------------------------------
//
// class SharedGameServerInfo;
//
// Class holding each game server's information read from the SharedGameServerInfo
// table of the GAME DB.
//
//----------------------------------------------------------------------

class SharedGameServerInfo {
public:
    // get/set GameServerID
    ServerID_t getServerID() const {
        return m_ServerID;
    }
    void setServerID(ServerID_t ServerID) {
        m_ServerID = ServerID;
    }

    // get/set host name
    string getNickname() const {
        return m_Nickname;
    }
    void setNickname(string nickname) {
        m_Nickname = nickname;
    }

    // get/set ip address
    string getIP() const {
        return m_IP;
    }
    void setIP(string ip) {
        m_IP = ip;
    }

    // get/set port
    uint getTCPPort() const {
        return m_TCPPort;
    }
    void setTCPPort(uint port) {
        m_TCPPort = port;
    }

    // get/set UDP port
    uint getUDPPort() const {
        return m_UDPPort;
    }
    void setUDPPort(uint port) {
        m_UDPPort = port;
    }

    // get/set GameServerGroupID
    ServerGroupID_t getGroupID() const {
        return m_GroupID;
    }
    void setGroupID(ServerGroupID_t GroupID) {
        m_GroupID = GroupID;
    }

    // get/set GameWorldID
    WorldID_t getWorldID() const {
        return m_WorldID;
    }
    void setWorldID(WorldID_t WorldID) {
        m_WorldID = WorldID;
    }

    // get/set ServerStat
    ServerStatus getServerStat() const {
        return m_ServerStat;
    }
    void setServerStat(ServerStatus Stat) {
        m_ServerStat = Stat;
    }

    // get debug string
    string toString() const {
        StringStream msg;
        msg << "GameServerInfo(" << "ServerID: " << (int)m_ServerID << ",Nickname:" << m_Nickname << ",IP: " << m_IP
            << ",TCPPort:" << m_TCPPort << ",UDPPort:" << m_UDPPort << ",GroupID:" << (int)m_GroupID
            << ",WorldID:" << (int)m_WorldID << ",ServerStat:" << (int)m_GroupID << ")";
        return msg.toString();
    }

private:
    // GameServer ID
    ServerID_t m_ServerID;

    // GameServer Process's nick name
    string m_Nickname;

    // Host's IP address
    string m_IP;

    // GameServer's port
    uint m_TCPPort;
    uint m_UDPPort;

    // GameServerGroupID
    ServerGroupID_t m_GroupID;

    // GameServerWorld
    WorldID_t m_WorldID;

    // Server Stat
    ServerStatus m_ServerStat;
};

#endif
