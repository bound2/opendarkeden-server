//////////////////////////////////////////////////////////////////////
//
// Filename   : DatagramSocket.h
// Written By : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __DATAGRAM_SOCKET_H__
#define __DATAGRAM_SOCKET_H__

// include files
#include "Datagram.h"
#include "Exception.h"
#include "SocketAPI.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class DatagramSocket;
//
// If the datagram socket's internal buffer is larger than the maximum size
// of a datagram packet, packets will probably never be truncated.
//
//////////////////////////////////////////////////////////////////////

const uint DATAGRAM_SOCKET_BUFFER_LEN = 65536;

class DatagramSocket {
public:
    // constructor for UDP Client Socket
    DatagramSocket();

    // constructor for UDP Server Socket
    DatagramSocket(uint port);

    // destructor
    ~DatagramSocket();

    // send datagram to peer
    uint send(Datagram* pDatagram);

    // receive datagram from peer
    Datagram* receive();

    // get socket descriptor
    SOCKET getSOCKET() const {
        return m_SocketID;
    }

private:
    // socket descriptor
    SOCKET m_SocketID;

    // socket address
    SOCKADDR_IN m_SockAddr;

    // internal socket input buffer
    char m_Buffer[DATAGRAM_SOCKET_BUFFER_LEN];
};

#endif
