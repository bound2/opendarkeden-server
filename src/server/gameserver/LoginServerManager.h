//////////////////////////////////////////////////////////////////////////////
// Filename    : LoginServerManager.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __LOGIN_SERVER_MANANGER_H__
#define __LOGIN_SERVER_MANANGER_H__

#include <cstdint>

#include "DatagramAdmission.h"
#include "DatagramSocket.h"
#include "Exception.h"
#include "ManagedThread.h"
#include "Mutex.h"
#include "RepeatedErrorReport.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class LoginServerManager;
//
// Thread dedicated to communication with the login server.
// Holds one datagram server socket internally and works in blocking mode.
// A received datagram is read only when its sender may send its packet's
// link: the loginserver's packets from the configured loginserver, the
// other gameservers' from the catalogue's gameservers, a client's port
// check from anywhere (DatagramAdmission.h).
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

    // The datagrams the gate admitted and refused since the start.
    std::uint64_t admittedDatagrams() const noexcept {
        return m_Gate.admitted();
    }
    std::uint64_t refusedDatagrams() const noexcept {
        return m_Gate.refused();
    }

private:
    DatagramSocket* m_pDatagramSocket; // UDP server socket

    mutable Mutex m_Mutex;

    // One line per 30 seconds about refused datagrams, however many arrive.
    de::RepeatedErrorReport m_RefusalReport;
    de::DatagramGate m_Gate;
};

#endif
