//----------------------------------------------------------------------
//
// Filename    : ReconnectLoginInfo.h
// Written by  : reiot@ewestsoft.com
// Description :
//
//----------------------------------------------------------------------

#ifndef __RECONNECT_LOGIN_INFO_H__
#define __RECONNECT_LOGIN_INFO_H__

// include files
#include "Exception.h"
#include "StringStream.h"
#include "Timeval.h"
#include "Types.h"

//----------------------------------------------------------------------
//
// class ReconnectLoginInfo;
//
// Object describing a connection authorized for a server-to-server move
//
//----------------------------------------------------------------------

class ReconnectLoginInfo {
public:
    // constructor
    ReconnectLoginInfo() : m_Key(0) {
        m_ExpireTime.tv_sec = 0;
        m_ExpireTime.tv_usec = 0;
    }

    // destructor
    ~ReconnectLoginInfo() {}

    // get/set client ip
    string getClientIP() const {
        return m_ClientIP;
    }
    void setClientIP(const string& clientIP) {
        m_ClientIP = clientIP;
    }

    // get/set PlayerID
    string getPlayerID() const {
        return m_PlayerID;
    }
    void setPlayerID(const string& PlayerID) {
        m_PlayerID = PlayerID;
    }

    // get/set expire time
    Timeval getExpireTime() const {
        return m_ExpireTime;
    }
    void setExpireTime(Timeval tv) {
        m_ExpireTime = tv;
    }

    // get/set validation key
    DWORD getKey() const {
        return m_Key;
    }
    void setKey(DWORD key) {
        m_Key = key;
    }

    // get debug string
    string toString() const {
        StringStream msg;
        msg << "ReconnectLoginInfo(ClientIP:" << m_ClientIP << ",ExpireTime:" << m_ExpireTime.tv_sec << "."
            << m_ExpireTime.tv_usec << ",KEY: " << m_Key << ")";
        return msg.toString();
    }

private:
    // valid client's ip
    string m_ClientIP;

    // PlayerID
    string m_PlayerID;

    // expire time : how long this information has to be kept..
    Timeval m_ExpireTime;

    // validation key
    DWORD m_Key;
};

//----------------------------------------------------------------------
//
// CompareReconnectLoginInfo
//
// Class used when comparing ReconnectLoginInfo objects in a priority_queue
//
// *CAUTION*
//
// The earlier one (the smaller time value) must sit at the top of the PQ.
// That is, the PQ must be in ascending order. Look closely to see why..
//
//----------------------------------------------------------------------
class CompareReconnectLoginInfo {
public:
    // compare which is more recent
    bool operator()(const ReconnectLoginInfo& left, const ReconnectLoginInfo& right) {
        // Ascending Order
        return left.getExpireTime() > right.getExpireTime();

        // Descending Order
        // return left.getExpireTime() < right.getExpireTime();
    }
};

#endif
