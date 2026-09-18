//////////////////////////////////////////////////////////////////////
//
// Filename    : GameServerPlayer.cpp
// Written By  : Reiot
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GameServerPlayer.h"

#include "Assert.h"
#include "Packet.h"
#include "PacketDispatcher.h"
#include "PacketFactoryManager.h"
#include "Socket.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

// by sigi. 2002.11.12
const int defaultGameServerPlayerInputStreamSize = 10240;
const int defaultGameServerPlayerOutputStreamSize = 163840;


//////////////////////////////////////////////////////////////////////
//
// constructor
//
//////////////////////////////////////////////////////////////////////
GameServerPlayer::GameServerPlayer() noexcept(false)
//: m_pSocket(NULL), m_pInputStream(NULL), m_pOutputStream(NULL)
{}


GameServerPlayer::GameServerPlayer(Socket* pSocket)
//: Player( pSocket )//m_pSocket(pSocket), m_pInputStream(NULL), m_pOutputStream(NULL)
{
    __BEGIN_TRY

    Assert(pSocket != NULL);
    m_pSocket = pSocket;

    // create socket input stream
    m_pInputStream = new SocketInputStream(m_pSocket, defaultGameServerPlayerInputStreamSize);

    Assert(m_pInputStream != NULL);

    // create socket output stream
    m_pOutputStream = new SocketOutputStream(m_pSocket, defaultGameServerPlayerOutputStreamSize);

    Assert(m_pOutputStream != NULL);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// destructor
//
//////////////////////////////////////////////////////////////////////
GameServerPlayer::~GameServerPlayer() noexcept {
    // destructor should never throw; guard any future cleanup
    try {
    } catch (...) {
        // swallow all exceptions to honor noexcept
    }
}


void GameServerPlayer::processInput() noexcept(false) {
    __BEGIN_TRY

    try {
        m_pInputStream->fill();
    } catch (NonBlockingIOException& nbie) {
    }

    __END_CATCH
}


void GameServerPlayer::processOutput() noexcept(false) {
    __BEGIN_TRY

    try {
        m_pOutputStream->flush();
    } catch (InvalidProtocolException&) {
        throw DisconnectException("Malformed packet");
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// parse packet and execute handler for the packet
//
//////////////////////////////////////////////////////////////////////
void GameServerPlayer::processCommand() noexcept(false) {
    __BEGIN_TRY

    try {
        // Buffer holding the header temporarily
        char header[szPacketHeader];
        PacketID_t packetID;
        PacketSize_t packetSize;
        Packet* pPacket;

        // Process every complete packet sitting in the input buffer.
        while (true) {
            // Read as many bytes as the packet header from the input stream.
            // If the requested number of bytes cannot be read from the stream,
            // an Insufficient exception is thrown and the loop is left.
            if (!m_pInputStream->peek(header, szPacketHeader))
                break;

            // Work out the packet id and the packet size.
            // The packet size includes the header here.
            memcpy(&packetID, &header[0], szPacketID);
            memcpy(&packetSize, &header[szPacketID], szPacketSize);

            // A strange packet id counts as a protocol error.
            if (packetID >= Packet::PACKET_MAX)
                throw InvalidProtocolException("invalid packet id");

            // A packet size that is too large counts as a protocol error.
            if (packetSize > g_pPacketFactoryManager->getPacketMaxSize(packetID))
                throw InvalidProtocolException("too large packet size");

            // Check that the input buffer holds as many bytes as the packet size.
            // break could be used when optimizing. (an exception is used here for now.)
            if (m_pInputStream->length() < szPacketHeader + packetSize)
                throw InsufficientDataException();

            // Getting here means the input buffer holds at least one complete packet.
            // The packet structure can be created from the packet factory manager with the packet id.
            // A wrong packet id is handled by the packet factory manager.
            pPacket = g_pPacketFactoryManager->createPacket(packetID);

            // Now initialize this packet structure.
            // The read() defined in the packet subclass is called through the virtual mechanism,
            // mechanism, so it is initialized automatically.
            m_pInputStream->readPacket(pPacket);

            PacketDispatcher::dispatch(pPacket, this);

            // Delete the packet
            delete pPacket;
        }
    } catch (NoSuchElementException& nsee) {
        // PacketFactoryManager::createPacket(PacketID_t)
        // PacketFactoryManager::getPacketMaxSize(PacketID_t)
        // may throw it.
        throw Error(nsee.toString());
    } catch (const InsufficientDataException&) {
        // do nothing
    }
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// send packet to player's output buffer
//
//////////////////////////////////////////////////////////////////////
void GameServerPlayer::sendPacket(Packet* pPacket) noexcept(false) {
    __BEGIN_TRY

    m_pOutputStream->writePacket(pPacket);


    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// disconnect ( close socket )
//
//////////////////////////////////////////////////////////////////////
void GameServerPlayer::disconnect(bool bDisconnected) noexcept(false) {
    __BEGIN_TRY

    try {
        // On a proper logout the output buffer can be
        // flushed. But if the other side has already closed the
        // connection, flushing gets a SIG_PIPE.
        if (bDisconnected == UNDISCONNECTED) {
            m_pOutputStream->flush();
        }

        m_pSocket->close();
    } catch (InvalidProtocolException& t) {
        cerr << "GameServerPlayer::disconnect Exception Check!!" << endl;
        cerr << t.toString() << endl;
        m_pSocket->close();
        // throw Error("error...");
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// set socket
//////////////////////////////////////////////////////////////////////
void GameServerPlayer::setSocket(Socket* pSocket) noexcept {
    m_pSocket = pSocket;

    if (m_pInputStream != NULL) {
        delete m_pInputStream;
        m_pInputStream = new SocketInputStream(m_pSocket);
    }

    if (m_pOutputStream != NULL) {
        delete m_pOutputStream;
        m_pOutputStream = new SocketOutputStream(m_pSocket);
    }
}


//////////////////////////////////////////////////////////////////////
//
// get debug string
//
//////////////////////////////////////////////////////////////////////
string GameServerPlayer::toString() const noexcept(false) {
    __BEGIN_TRY

    StringStream msg;

    msg << "GameServerPlayer(" << "SocketID:" << m_pSocket->getSOCKET() << ",Host:" << m_pSocket->getHost()
        << ",ID:" << m_ID << ")";

    return msg.toString();

    __END_CATCH
}
