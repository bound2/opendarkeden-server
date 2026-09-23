//////////////////////////////////////////////////////////////////////////////
// Filename    : ConnectionInfoManager.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "ConnectionInfoManager.h"

#include <stdio.h>

#include "Assert.h"
#include "GMServerInfo.h"
#include "GameContext.h"
#include "LogDef.h"
#include "LoginServerManager.h"
#include "Properties.h"
#include "StringStream.h"
#include "ZoneGroup.h"
#include "ZoneGroupManager.h"
#include "ZonePlayerManager.h"
#include "repository/SessionRepository.h"
#include "repository/ZoneInfoRepository.h"

// global variable definition
//////////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////////
ConnectionInfoManager::ConnectionInfoManager()

{
    __BEGIN_TRY

    m_Mutex.setName("ConnectionInfoManager");

    // Set the time of the next heartbeat.
    getCurrentTime(m_NextHeartbeat);
    m_NextHeartbeat.tv_sec += 10;

    // The user count goes in after 30 seconds.
    m_UpdateUserStatusTime.tv_sec = m_NextHeartbeat.tv_sec + 20;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////////////
ConnectionInfoManager::~ConnectionInfoManager()

{
    __BEGIN_TRY

    // Every ConnectionInfo must be deleted.
    HashMapConnectionInfo::iterator itr = m_ConnectionInfos.begin();
    for (; itr != m_ConnectionInfos.end(); itr++) {
        SAFE_DELETE(itr->second);
    }

    // Delete every pair in the hash map.
    m_ConnectionInfos.clear();

    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////////////
// add connection info to connection info manager
//////////////////////////////////////////////////////////////////////////////
void ConnectionInfoManager::addConnectionInfo(ConnectionInfo* pConnectionInfo) {
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    Assert(pConnectionInfo != NULL);

    HashMapConnectionInfo::iterator itr = m_ConnectionInfos.find(pConnectionInfo->getClientIP());

    if (itr != m_ConnectionInfos.end()) {
        // An entry with the same id already exists.
        // throw DuplicatedException("duplicated connection info id");

        // Remove the existing information and set the new information.
        // by sigi. 2002.12.7
        // throw DuplicatedException("duplicated connection info id");
        ConnectionInfo* pOldConnectionInfo = itr->second;

        FILELOG_INCOMING_CONNECTION("connectionInfo.log", "DupDelete [%s:%s] %s (%u)",
                                    pOldConnectionInfo->getPlayerID().c_str(), pOldConnectionInfo->getPCName().c_str(),
                                    pOldConnectionInfo->getClientIP().c_str(), pOldConnectionInfo->getKey());

        SAFE_DELETE(pOldConnectionInfo);

        itr->second = pConnectionInfo;


        return;
    }

    m_ConnectionInfos[pConnectionInfo->getClientIP()] = pConnectionInfo;

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Delete connection info from connection info manager
//////////////////////////////////////////////////////////////////////////////
void ConnectionInfoManager::deleteConnectionInfo(const string& clientIP) {
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    HashMapConnectionInfo::iterator itr = m_ConnectionInfos.find(clientIP);

    if (itr != m_ConnectionInfos.end()) {
        Assert(itr->second != NULL);

        // Delete the ConnectionInfo.
        SAFE_DELETE(itr->second);

        // Delete the pair.
        m_ConnectionInfos.erase(itr);
    } else {
        throw NoSuchElementException(clientIP);
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// get connection info from connection info manager
//////////////////////////////////////////////////////////////////////////////
ConnectionInfo* ConnectionInfoManager::getConnectionInfo(const string& clientIP) {
    __BEGIN_TRY

    ConnectionInfo* pConnectionInfo = NULL;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    HashMapConnectionInfo::iterator itr = m_ConnectionInfos.find(clientIP);

    if (itr != m_ConnectionInfos.end()) {
        pConnectionInfo = itr->second;
    } else {
        throw NoSuchElementException(clientIP);
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return pConnectionInfo;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// Delete expired Connection Info objects.
//////////////////////////////////////////////////////////////////////////////
void ConnectionInfoManager::heartbeat()

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    Timeval currentTime;
    getCurrentTime(currentTime);

    if (m_NextHeartbeat < currentTime) {
        m_NextHeartbeat = currentTime;
        m_NextHeartbeat.tv_sec += 10;

        HashMapConnectionInfo::iterator before = m_ConnectionInfos.end();
        HashMapConnectionInfo::iterator current = m_ConnectionInfos.begin();

        while (current != m_ConnectionInfos.end()) {
            if (current->second->getExpireTime() < currentTime) {
                ConnectionInfo* pConnectionInfo = current->second;

                m_ConnectionInfos.erase(current);

                // by sigi. 2002.12.7
                FILELOG_INCOMING_CONNECTION("connectionInfo.log", "Expire [%s:%s] %s (%u)",
                                            pConnectionInfo->getPlayerID().c_str(),
                                            pConnectionInfo->getPCName().c_str(),
                                            pConnectionInfo->getClientIP().c_str(), pConnectionInfo->getKey());


                SAFE_DELETE(pConnectionInfo);

                if (before == m_ConnectionInfos.end()) // case of first
                {
                    current = m_ConnectionInfos.begin();
                } else // case of not first
                {
                    current = before;
                    current++;
                }
            } else {
                before = current++;
            }
        }

        // Report the user count.
        static int GroupCount = 0;

        if (GroupCount == 0) {
            int maxZoneGroupID = 0;
            if (!defaultZoneInfoRepository().loadMaxZoneGroupID(maxZoneGroupID)) {
                throw Error("Critical Error : ZoneGroupInfo table is empty.");
            }

            GroupCount = maxZoneGroupID + 1;
        }

        GMServerInfo gmServerInfo;

        static int worldID = g_pConfig->getPropertyInt("WorldID");
        static int serverID = g_pConfig->getPropertyInt("ServerID");

        gmServerInfo.setWorldID(worldID);
        gmServerInfo.setServerID(serverID);

        // cout << "GroupCount: " << GroupCount << endl;
        uint numPC = 0;

        for (int i = 1; i < GroupCount; i++) {
            ZoneGroup* pZoneGroup = NULL;

            try {
                pZoneGroup = de::gameContext().zoneGroups().getZoneGroupByGroupID(i);
            } catch (NoSuchElementException& t) {
                throw Error("Critical Error : ZoneInfoManager has no such zone group.");
            }

            pZoneGroup->makeZoneUserInfo(gmServerInfo);

            numPC += pZoneGroup->getZonePlayerManager()->size();
        }

        // Store in the DB when running for Netmarble.
        if (currentTime > m_UpdateUserStatusTime) {
            // Every 30 seconds
            m_UpdateUserStatusTime.tv_sec = currentTime.tv_sec + 30;

            if (g_pConfig->getPropertyInt("IsNetMarble") == 1) {
                if (!defaultSessionRepository().updateUserStatus(numPC, worldID, serverID)) {
                    // No row yet: add one.
                    defaultSessionRepository().insertUserStatus(worldID, serverID, numPC);
                }
            }
        }

        // MonitorClient no longer takes this value.

        static int portNum = g_pConfig->getPropertyInt("LoginServerUDPPortNum");
        static const string& loginServerIP = g_pConfig->getProperty("LoginServerIP");
        static int loginServerUDPPort = g_pConfig->getPropertyInt("LoginServerUDPPort");
        static int loginServerBaseUDPPort = g_pConfig->getPropertyInt("LoginServerBaseUDPPort");

        // Default
        de::gameContext().loginServer().sendPacket(loginServerIP, loginServerUDPPort, &gmServerInfo);

        // The other ports
        if (portNum > 1) {
            for (int j = 0; j < portNum; j++) {
                de::gameContext().loginServer().sendPacket(loginServerIP, loginServerBaseUDPPort + j, &gmServerInfo);
            }
        }
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// get debug string
//////////////////////////////////////////////////////////////////////////////
string ConnectionInfoManager::toString() const

{
    StringStream msg;

    msg << "ConnectionInfoManager(";

    for (unordered_map<string, ConnectionInfo*>::const_iterator itr = m_ConnectionInfos.begin();
         itr != m_ConnectionInfos.end(); itr++) {
        Assert(itr->second != NULL);
        msg << itr->second->toString();
    }

    msg << ")";

    return msg.toString();
}
