//////////////////////////////////////////////////////////////////////////////
// Filename    : ConnectionInfoManager.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CONNECTION_INFO_MANAGER_H__
#define __CONNECTION_INFO_MANAGER_H__

#include <unordered_map>

#include "ConnectionInfo.h"
#include "Exception.h"
#include "Mutex.h"
#include "Types.h"

typedef unordered_map<string, ConnectionInfo*> HashMapConnectionInfo;

//////////////////////////////////////////////////////////////////////////////
// class ConnectionInfoManager;
// Holds an unordered_map internally, keyed by the IP string.
//////////////////////////////////////////////////////////////////////////////

class ConnectionInfoManager {
public:
    ConnectionInfoManager();
    ~ConnectionInfoManager();

public:
    void addConnectionInfo(ConnectionInfo* pConnectionInfo);
    void deleteConnectionInfo(const string& ip);
    ConnectionInfo* getConnectionInfo(const string& ip);
    void heartbeat();
    string toString() const;

private:
    // hash map of ConnectionInfo
    // key is ip-string
    HashMapConnectionInfo m_ConnectionInfos;

    Timeval m_NextHeartbeat;
    Timeval m_UpdateUserStatusTime; // For Netmarble

    mutable Mutex m_Mutex;
};

#endif
