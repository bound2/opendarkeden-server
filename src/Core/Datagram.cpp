//////////////////////////////////////////////////////////////////////
//
// Filename    : Datagram.cpp
// Written By  : reiot@ewestsoft.com
// Description : Pure wire framing for UDP datagrams. The factory-backed
//               receive path (read(DatagramPacket*&)) lives in
//               DatagramFactoryRead.cpp: it needs PacketFactoryManager,
//               which the kernel must not depend on (rule K1/K2).
//
//////////////////////////////////////////////////////////////////////

// include files
#include "Datagram.h"

#include <stdio.h>

#include <exception>

#include "Assert.h"
#include "DatagramPacket.h"
#include "Packet.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
Datagram::Datagram() : m_Length(0), m_InputOffset(0), m_OutputOffset(0), m_Data(NULL) {
    __BEGIN_TRY

    memset(&m_SockAddr, 0, sizeof(m_SockAddr));
    m_SockAddr.sin_family = AF_INET;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
Datagram::~Datagram() noexcept {
    try {
        if (m_Data != NULL) {
            SAFE_DELETE_ARRAY(m_Data);
            m_Data = NULL;
        }
    } catch (const std::exception&) {
        // ignore during teardown
    }
}


bool Datagram::isDatagram(PacketID_t packetID) {
    switch (packetID) {
    case Packet::PACKET_CG_PORT_CHECK:
    case Packet::PACKET_GG_COMMAND:
    case Packet::PACKET_GG_GUILD_CHAT:
    case Packet::PACKET_GG_SERVER_CHAT:
    case Packet::PACKET_GL_INCOMING_CONNECTION:
    case Packet::PACKET_GL_INCOMING_CONNECTION_ERROR:
    case Packet::PACKET_GL_INCOMING_CONNECTION_OK:
    case Packet::PACKET_GL_KICK_VERIFY:
    case Packet::PACKET_GM_SERVER_INFO:
    case Packet::PACKET_LG_KICK_CHARACTER:
    case Packet::PACKET_LG_INCOMING_CONNECTION:
    case Packet::PACKET_LG_INCOMING_CONNECTION_ERROR:
    case Packet::PACKET_LG_INCOMING_CONNECTION_OK:
        return true;
    default:
        return false;
    }
}

//////////////////////////////////////////////////////////////////////
// Copy data from the internal buffer to an external buffer.
//////////////////////////////////////////////////////////////////////
void Datagram::read(char* buf, uint len) {
    __BEGIN_TRY

    // boundary check
    Assert(m_InputOffset + len <= m_Length);

    memcpy(buf, &m_Data[m_InputOffset], len);

    m_InputOffset += len;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Copy data from the internal buffer to an external string.
//////////////////////////////////////////////////////////////////////
void Datagram::read(string& str, uint len) {
    __BEGIN_TRY

    // boundary check
    Assert(m_InputOffset + len <= m_Length);

    str.reserve(len);
    str.assign(&m_Data[m_InputOffset], len);

    m_InputOffset += len;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Copy data from an external buffer into the internal buffer.
//////////////////////////////////////////////////////////////////////
void Datagram::write(const char* buf, uint len) {
    __BEGIN_TRY

    // boundary check
    Assert(m_OutputOffset + len <= m_Length);
    //	if (m_OutputOffset + len > m_Length)
    //	{
    //		throw Error( "Datagram::write(): data is larger than the buffer.");
    //	}

    memcpy(&m_Data[m_OutputOffset], buf, len);

    m_OutputOffset += len;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Copy an external string into the internal buffer.
//
// *CAUTION*
//
// Every write() goes through write(const char*,uint), so m_OutputOffset
// does not need adjusting here.
//
//////////////////////////////////////////////////////////////////////
void Datagram::write(const string& str) {
    __BEGIN_TRY

    // boundary check
    Assert(m_OutputOffset + str.size() <= m_Length);

    // write string body
    write(str.c_str(), str.size());

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// write packet
//
// Serializes the packet's binary image into the datagram. Call this on
// the sending side; the datagram's internal buffer must be NULL before
// the call and is allocated by it.
//
//////////////////////////////////////////////////////////////////////
void Datagram::write(const DatagramPacket* pPacket) {
    __BEGIN_TRY

    Assert(pPacket != NULL);

    PacketID_t packetID = pPacket->getPacketID();
    PacketSize_t packetSize = pPacket->getPacketSize();

    // Size the datagram buffer to the packet.
    setData(szPacketHeader + packetSize);

    // Write the packet header.
    write((char*)&packetID, szPacketID);
    write((char*)&packetSize, szPacketSize);

    // Write the packet body.
    pPacket->write(*this);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// set data
//
// Stores data read from the datagram socket into the internal buffer.
//
//////////////////////////////////////////////////////////////////////
void Datagram::setData(char* data, uint len) {
    __BEGIN_TRY

    Assert(data != NULL && m_Data == NULL);

    m_Length = len;
    m_Data = new char[m_Length];
    memcpy(m_Data, data, m_Length);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void Datagram::setData(uint len) {
    __BEGIN_TRY

    Assert(m_Data == NULL);

    m_Length = len;
    m_Data = new char[m_Length];

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// set address
//////////////////////////////////////////////////////////////////////
void Datagram::setAddress(SOCKADDR_IN* pSockAddr) {
    __BEGIN_TRY

    Assert(pSockAddr != NULL);

    memcpy(&m_SockAddr, pSockAddr, szSOCKADDR_IN);

    // char str[80];
    // sprintf(str, "0x%X - 0x%X", m_SockAddr.sin_port, ntohs(m_SockAddr.sin_port));
    // cout << "[Datagram::setAddress] " << inet_ntoa(m_SockAddr.sin_addr) << ":" << ntohs(m_SockAddr.sin_port) << " - "
    // << str << endl;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get debug string
//////////////////////////////////////////////////////////////////////
string Datagram::toString() const {
    StringStream msg;
    msg << "Datagram(" << "Length:" << m_Length << ",InputOffset:" << m_InputOffset
        << ",OutputOffset:" << m_OutputOffset << ")";
    return msg.toString();
}
