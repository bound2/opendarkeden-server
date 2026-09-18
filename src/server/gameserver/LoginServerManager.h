//////////////////////////////////////////////////////////////////////////////
// Filename    : LoginServerManager.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __LOGIN_SERVER_MANANGER_H__
#define __LOGIN_SERVER_MANANGER_H__

#include "DatagramSocket.h"
#include "Exception.h"
#include "ManagedThread.h"
#include "Mutex.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class LoginServerManager;
//
// Thread dedicated to communication with the login server.
// Holds one datagram server socket internally and works in blocking mode.
//////////////////////////////////////////////////////////////////////////////

class LoginServerManager : public ManagedThread {
public:
    LoginServerManager();
    ~LoginServerManager() noexcept;

public:
    void init() {}

    void stop();

    void run();

    void sendDatagram(Datagram* pDatagram);

    void sendPacket(const string& host, uint port, DatagramPacket* pPacket);

    void lock() const {
        m_Mutex.lock();
    }
    void unlock() const {
        m_Mutex.unlock();
    }

private:
    DatagramSocket* m_pDatagramSocket; // UDP server socket

    mutable Mutex m_Mutex;
};

// global variable declaration
extern LoginServerManager* g_pLoginServerManager;

#endif
