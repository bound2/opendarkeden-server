//////////////////////////////////////////////////////////////////////
//
// Filename    : GameServerManager.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GAME_SERVER_MANANGER_H__
#define __GAME_SERVER_MANANGER_H__

// include files
#include <cstdint>

#include "DatagramAdmission.h"
#include "DatagramSocket.h"
#include "Exception.h"
#include "ManagedThread.h"
#include "RepeatedErrorReport.h"
#include "Types.h"

class Datagram;
class DatagramPacket;

//////////////////////////////////////////////////////////////////////
//
// class GameServerManager;
//
// Worker thread dedicated to communication with the game servers.
//
// It owns a single datagram server socket. The socket is nonblocking, so an
// idle link cannot keep the worker from observing a shutdown request. A
// received datagram is read only when its sender may send its packet's
// link: the gameservers' packets from the catalogue's gameservers, a
// client's port check from anywhere (DatagramAdmission.h).
//
//////////////////////////////////////////////////////////////////////

class GameServerManager : public ManagedThread {
public:
    // constructor
    GameServerManager();

    // destructor
    ~GameServerManager() noexcept;

    //
    void init() {}

    // stop thread
    void stop() override;

    // main method
    void run() override;

    void sendDatagram(Datagram* pDatagram);
    // Normal return means a complete local UDP send. A reported send failure
    // throws ConnectException so callers cannot advance a failed handshake.
    void sendPacket(const string& host, uint port, const DatagramPacket* pPacket);

    // The datagrams the gate admitted and refused since the start.
    std::uint64_t admittedDatagrams() const noexcept {
        return m_Gate.admitted();
    }
    std::uint64_t refusedDatagrams() const noexcept {
        return m_Gate.refused();
    }

private:
    // UDP server socket
    DatagramSocket* m_pDatagramSocket;

    // One line per 30 seconds about refused datagrams, however many arrive.
    de::RepeatedErrorReport m_RefusalReport;
    de::DatagramGate m_Gate;
};

#endif
