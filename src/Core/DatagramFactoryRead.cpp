//////////////////////////////////////////////////////////////////////
//
// Filename    : DatagramFactoryRead.cpp
// Description : The factory-backed receive paths of Datagram and
//               SerialDatagram, split out of their own .cpps (task
//               2.4): constructing the packet object goes through
//               g_pPacketFactoryManager, which the kernel must not
//               depend on, so these two functions compile in Core
//               while the rest of the framing lives in de-kernel.
//
//////////////////////////////////////////////////////////////////////

// include files
#include <stdio.h>

#include <exception>

#include "Assert.h"
#include "Datagram.h"
#include "DatagramPacket.h"
#include "Packet.h"
#include "PacketFactoryManager.h"
#include "SerialDatagram.h"
#include "SerialDatagramPacket.h"

//////////////////////////////////////////////////////////////////////
//
// Reconstruct a Packet object from a Datagram.
// If the data received by DatagramSocket is larger than expected, the
// packet from the peer may have been truncated in transit.
//
// (Though for our game the UDP links run on a LAN, so it can hardly
// ever happen in practice..)
//
// *CAUTION*
//
// By the algorithm below, (1) two different packets arriving from the
// same address must be returned by separate recvfrom() calls, and
// (2) one packet must arrive in one piece.
//
//////////////////////////////////////////////////////////////////////
void Datagram::read(DatagramPacket*& pPacket) {
    __BEGIN_TRY

    Assert(pPacket == NULL);

    PacketID_t packetID;
    PacketSize_t packetSize;

    // initialize packet header
    read((char*)&packetID, szPacketID);
    read((char*)&packetSize, szPacketSize);

    // cout << "DatagramPacket I  D : " << packetID << endl;

    // invalid packet id
    if (packetID >= Packet::PACKET_MAX)
        throw InvalidProtocolException("invalid packet id");

    // abnormal packet size
    if (packetSize > g_pPacketFactoryManager->getPacketMaxSize(packetID))
        throw InvalidProtocolException("too large packet size");

    // the datagram is smaller than the packet claims to be
    if (m_Length < szPacketHeader + packetSize)
        throw Error("datagram packet did not arrive in one piece");

    // the datagram is larger than the packet claims to be
    if (m_Length > szPacketHeader + packetSize)
        throw Error("more than one datagram packet arrived in a single datagram");

    // reject ids that are not legitimate UDP packets
    if (!isDatagram(packetID)) {
        filelog("datagram.txt", "id:%u host:%s", packetID, getHost().c_str());
        throw InvalidProtocolException("packet is not UDP");
    }

    pPacket = (DatagramPacket*)g_pPacketFactoryManager->createPacket(packetID);

    Assert(pPacket != NULL);

    // Initialize the packet.
    // filelog("datagram.txt","id:%u host:%s",packetID,getHost().c_str());
    pPacket->read(*this);

    // Record the peer address/port on the packet.
    pPacket->setHost(getHost());
    pPacket->setPort(getPort());

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// Reconstruct a Packet object from a SerialDatagram.
// If the data received by SerialDatagramSocket is larger than
// expected, the packet from the peer may have been truncated in
// transit.
//
// (Though for our game the UDP links run on a LAN, so it can hardly
// ever happen in practice..)
//
// *CAUTION*
//
// By the algorithm below, (1) two different packets arriving from the
// same address must be returned by separate recvfrom() calls, and
// (2) one packet must arrive in one piece.
//
//////////////////////////////////////////////////////////////////////
void SerialDatagram::read(SerialDatagramPacket*& pPacket) {
    __BEGIN_TRY

    Assert(pPacket == NULL);

    PacketID_t packetID;
    PacketSize_t packetSize;
    uint serial;

    // initialize packet header
    read((char*)&packetID, szPacketID);
    read((char*)&packetSize, szPacketSize);
    read((char*)&serial, szuint);

    cout << "SerialDatagramPacket I  D : " << packetID << endl;

    // invalid packet id
    if (packetID >= Packet::PACKET_MAX)
        throw InvalidProtocolException("invalid packet id");

    // abnormal packet size
    if (packetSize > g_pPacketFactoryManager->getPacketMaxSize(packetID))
        throw InvalidProtocolException("too large packet size");

    // the datagram is smaller than the packet claims to be
    if (m_Length < szPacketHeader + packetSize)
        throw Error("datagram packet did not arrive in one piece");

    // the datagram is larger than the packet claims to be
    if (m_Length > szPacketHeader + packetSize)
        throw Error("more than one datagram packet arrived in a single datagram");

    // Create the packet.
    pPacket = (SerialDatagramPacket*)g_pPacketFactoryManager->createPacket(packetID);

    Assert(pPacket != NULL);

    // Set the serial.
    pPacket->setSerial(serial);

    // Initialize the packet.
    pPacket->read(*this);

    // Record the peer address/port on the packet.
    pPacket->setHost(getHost());
    pPacket->setPort(getPort());

    __END_CATCH
}
