//////////////////////////////////////////////////////////////////////////////
// Filename    : ConnectionInfo.h
// Written by  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CONNECTION_INFO_H__
#define __CONNECTION_INFO_H__

#include "Exception.h"
#include "Timeval.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ConnectionInfo;
// Describes a connection that is valid for a move between servers.
//////////////////////////////////////////////////////////////////////////////

class ConnectionInfo {
public:
    ConnectionInfo();
    ~ConnectionInfo();

public:
    string getClientIP() const {
        return m_ClientIP;
    }
    void setClientIP(const string& clientIP) {
        m_ClientIP = clientIP;
    }

    string getPlayerID() const {
        return m_PlayerID;
    }
    void setPlayerID(const string& PlayerID) {
        m_PlayerID = PlayerID;
    }

    string getPCName() const {
        return m_PCName;
    }
    void setPCName(const string& name) {
        m_PCName = name;
    }

    Timeval getExpireTime() const {
        return m_ExpireTime;
    }
    void setExpireTime(Timeval tv) {
        m_ExpireTime = tv;
    }

    DWORD getKey() const {
        return m_Key;
    }
    void setKey(DWORD key) {
        m_Key = key;
    }

    string toString() const;

private:
    string m_ClientIP;    // valid client's ip
    string m_PlayerID;    // PlayerID
    string m_PCName;      // PC name
    Timeval m_ExpireTime; // expire time : how long this info has to be kept
    DWORD m_Key;          // validation key
};

//////////////////////////////////////////////////////////////////////////////
// CompareConnectionInfo
//
// Class used to compare ConnectionInfo objects in a priority_queue
//
// *CAUTION*
//
// The entry with the earlier time (the smaller value) must sit at the
// top of the PQ; that is, the PQ must be in ascending order. Look at
// the comparison closely to see why.
//////////////////////////////////////////////////////////////////////////////

class CompareConnectionInfo {
public:
    // compare which is more recent
    bool operator()(const ConnectionInfo& left, const ConnectionInfo& right) {
        // Ascending Order
        return left.getExpireTime() > right.getExpireTime();

        // Descending Order
        // return left.getExpireTime() < right.getExpireTime();
    }
};

#endif
