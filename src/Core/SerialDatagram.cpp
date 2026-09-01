//////////////////////////////////////////////////////////////////////
//
// Filename    : SerialDatagram.cpp
// Written By  : reiot@ewestsoft.com
// Description : Pure wire framing for serial-numbered UDP datagrams.
//               The factory-backed receive path
//               (read(SerialDatagramPacket*&)) lives in
//               DatagramFactoryRead.cpp: it needs PacketFactoryManager,
//               which the kernel must not depend on (rule K1/K2).
//
//////////////////////////////////////////////////////////////////////

// include files
#include "SerialDatagram.h"

#include <exception>

#include "Assert.h"
#include "SerialDatagramPacket.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
SerialDatagram::SerialDatagram() : m_Length(0), m_InputOffset(0), m_OutputOffset(0), m_Data(NULL) {
    __BEGIN_TRY

    memset(&m_SockAddr, 0, sizeof(m_SockAddr));
    m_SockAddr.sin_family = AF_INET;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
SerialDatagram::~SerialDatagram() noexcept {
    try {
        if (m_Data != NULL) {
            SAFE_DELETE_ARRAY(m_Data);
            m_Data = NULL;
        }
    } catch (const std::exception&) {
        // ignore
    }
}


//////////////////////////////////////////////////////////////////////
// Copy data from the internal buffer to an external buffer.
//////////////////////////////////////////////////////////////////////
void SerialDatagram::read(char* buf, uint len) {
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
void SerialDatagram::read(string& str, uint len) {
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
void SerialDatagram::write(const char* buf, uint len) {
    __BEGIN_TRY

    // boundary check
    Assert(m_OutputOffset + len <= m_Length);
    //	if (m_OutputOffset + len > m_Length)
    //	{
    //		throw Error( "SerialDatagram::write(): data is larger than the buffer.");
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
void SerialDatagram::write(const string& str) {
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
void SerialDatagram::write(const SerialDatagramPacket* pPacket) {
    __BEGIN_TRY

    Assert(pPacket != NULL);

    PacketID_t packetID = pPacket->getPacketID();
    PacketSize_t packetSize = pPacket->getPacketSize();
    uint serial = pPacket->getSerial();

    // Size the datagram buffer to the packet.
    setData(szPacketHeader + packetSize);

    // Write the packet header.
    write((char*)&packetID, szPacketID);
    write((char*)&packetSize, szPacketSize);
    write((char*)&serial, szuint);

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
void SerialDatagram::setData(char* data, uint len) {
    __BEGIN_TRY

    Assert(data != NULL && m_Data == NULL);

    m_Length = len;
    m_Data = new char[m_Length];
    memcpy(m_Data, data, m_Length);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void SerialDatagram::setData(uint len) {
    __BEGIN_TRY

    Assert(m_Data == NULL);

    m_Length = len;
    m_Data = new char[m_Length];

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// set address
//////////////////////////////////////////////////////////////////////
void SerialDatagram::setAddress(SOCKADDR_IN* pSockAddr) {
    __BEGIN_TRY

    Assert(pSockAddr != NULL);

    memcpy(&m_SockAddr, pSockAddr, szSOCKADDR_IN);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// get debug string
//////////////////////////////////////////////////////////////////////
string SerialDatagram::toString() const {
    StringStream msg;
    msg << "SerialDatagram(" << "Length:" << m_Length << ",InputOffset:" << m_InputOffset
        << ",OutputOffset:" << m_OutputOffset << ")";
    return msg.toString();
}
