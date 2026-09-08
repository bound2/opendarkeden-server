//////////////////////////////////////////////////////////////////////
//
// Filename    : datagram_frame_test.cpp
// Description : Pins the UDP framing contract of Datagram: the size
//               field carries the number of bytes the body actually
//               wrote, the datagram's length matches it, and a body
//               that outgrows the buffer its declared size bought is
//               still sent whole.
//
// The server-to-server UDP link has no goldens, because a datagram
// packet cannot be written to a socket stream at all. These tests are
// its equivalent: the frame is checked byte for byte, and one packet
// makes a real loopback trip through the receive path that
// reconstructs it.
//
//////////////////////////////////////////////////////////////////////

#include <cstring>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "CGPortCheck.h"
#include "Datagram.h"
#include "DatagramPacket.h"
#include "DatagramSocket.h"
#include "Exception.h"
#include "Packet.h"
#include "PacketFactoryManager.h"
#include "Types.h"

namespace {

// A packet whose declared size can disagree with what its write()
// produces, in either direction -- the datagram twin of the TCP
// stream's DriftingPacket.
class DriftingDatagramPacket : public DatagramPacket {
public:
    DriftingDatagramPacket(uint bodySize, int declaredDrift) : m_BodySize(bodySize), m_DeclaredDrift(declaredDrift) {}

    void read(Datagram& iDatagram) {
        throw UnsupportedError();
    }

    void write(Datagram& oDatagram) const {
        // One field at a time: a body that overruns its buffer must grow
        // it through the same path every packet writes through.
        for (uint i = 0; i < m_BodySize; i++)
            oDatagram.write((BYTE)(0xA0 + i));
    }

    PacketID_t getPacketID() const {
        return 0x4321;
    }
    PacketSize_t getPacketSize() const {
        return (PacketSize_t)((int)m_BodySize + m_DeclaredDrift);
    }
    string getPacketName() const {
        return "DriftingDatagramPacket";
    }
    string toString() const {
        return "DriftingDatagramPacket";
    }

private:
    uint m_BodySize;
    int m_DeclaredDrift;
};

// The size field as the receiver reads it: little-endian, at the
// datagram's own header offset.
unsigned int sizeField(Datagram& datagram) {
    const unsigned char* pBuffer = (const unsigned char*)datagram.getData();
    unsigned int size = 0;
    for (uint i = 0; i < szPacketSize; i++)
        size |= (unsigned int)pBuffer[szPacketID + i] << (8 * i);
    return size;
}

// The receive path builds packets through the global factory manager,
// which no other test in this binary installs.
void installPortCheckFactory() {
    if (g_pPacketFactoryManager == NULL) {
        g_pPacketFactoryManager = new PacketFactoryManager();
        g_pPacketFactoryManager->addFactory(new CGPortCheckFactory());
    }
}

} // namespace

// A lie in either direction still frames the body that was written.
TEST(DatagramFrameSize, TheHeaderCarriesTheMeasuredBodyLength) {
    const uint kBodySize = 9;

    for (int drift : {-4, 0, 7}) {
        DriftingDatagramPacket packet(kBodySize, drift);
        Datagram datagram;
        datagram.write(&packet);

        EXPECT_EQ(kBodySize, sizeField(datagram)) << "drift " << drift;
        ASSERT_EQ(szPacketHeader + kBodySize, datagram.getLength()) << "drift " << drift;

        const unsigned char* pBuffer = (const unsigned char*)datagram.getData();
        for (uint i = 0; i < kBodySize; i++)
            EXPECT_EQ(0xA0 + i, pBuffer[szPacketID + szPacketSize + i]) << "drift " << drift << ", body byte " << i;
    }
}

// An honest packet's datagram is byte for byte what it always was: id,
// size, body, and the one trailing pad byte both peers count into the
// datagram's length but neither reads.
TEST(DatagramFrameSize, AnHonestPacketsDatagramIsUnchanged) {
    ASSERT_EQ(2u, szPacketID);
    ASSERT_EQ(4u, szPacketSize);
    ASSERT_EQ(7u, szPacketHeader);

    DriftingDatagramPacket packet(4, 0);
    Datagram datagram;
    datagram.write(&packet);

    const unsigned char expected[] = {0x21, 0x43, 0x04, 0x00, 0x00, 0x00, 0xA0, 0xA1, 0xA2, 0xA3, 0x00};
    ASSERT_EQ(sizeof(expected), datagram.getLength());
    EXPECT_EQ(0, memcmp(expected, datagram.getData(), sizeof(expected)));
}

// A body larger than the buffer its declared size bought grows the
// buffer and is sent whole, rather than aborting the write half way
// through a partly filled datagram.
TEST(DatagramFrameSize, ABodyLargerThanTheDeclaredSizeIsFramedWhole) {
    const uint kBodySize = 200;

    // Declared sizes from "this packet writes nothing" up to one byte
    // short, so the growth path runs from an empty buffer and from a
    // nearly full one.
    for (uint declared : {0u, 1u, 64u, kBodySize - 1}) {
        DriftingDatagramPacket packet(kBodySize, (int)declared - (int)kBodySize);
        Datagram datagram;
        datagram.write(&packet);

        EXPECT_EQ(kBodySize, sizeField(datagram)) << "declared " << declared;
        ASSERT_EQ(szPacketHeader + kBodySize, datagram.getLength()) << "declared " << declared;

        const unsigned char* pBuffer = (const unsigned char*)datagram.getData();
        EXPECT_EQ(0x21, pBuffer[0]);
        EXPECT_EQ(0x43, pBuffer[1]);
        for (uint i = 0; i < kBodySize; i++)
            EXPECT_EQ((unsigned char)(0xA0 + i), pBuffer[szPacketID + szPacketSize + i])
                << "declared " << declared << ", body byte " << i;
        // The pad the length counts is written, not left as whatever the
        // allocation happened to hold.
        EXPECT_EQ(0x00, pBuffer[datagram.getLength() - 1]) << "declared " << declared;
    }
}

// The whole path, over a real UDP socket pair: a packet is framed,
// sent, received, and rebuilt by the factory-backed read side.
TEST(DatagramFrameRoundTrip, LoopbackRebuildsThePacket) {
    installPortCheckFactory();

    // Port 0: the kernel picks a free port, so parallel runs of this
    // suite cannot collide.
    DatagramSocket receiver(0);

    SOCKADDR_IN bound;
    memset(&bound, 0, sizeof(bound));
    socklen_t boundLen = sizeof(bound);
    ASSERT_EQ(0, getsockname(receiver.getSOCKET(), (SOCKADDR*)&bound, &boundLen));

    // Without this a lost datagram would hang the case until ctest's
    // timeout instead of failing.
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    ASSERT_EQ(0, setsockopt(receiver.getSOCKET(), SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)));

    CGPortCheck sent;
    sent.setPCName("Cheiron");

    Datagram outgoing;
    outgoing.setHost("127.0.0.1");
    outgoing.setPort(ntohs(bound.sin_port));
    outgoing.write(&sent);

    ASSERT_EQ(szPacketHeader + (uint)sent.getPacketSize(), outgoing.getLength());

    DatagramSocket sender(0);
    ASSERT_EQ(outgoing.getLength(), sender.send(&outgoing));

    Datagram* pIncoming = receiver.receive();
    ASSERT_TRUE(pIncoming != NULL) << "no datagram arrived on the loopback";
    EXPECT_EQ(outgoing.getLength(), pIncoming->getLength());

    DatagramPacket* pReceived = NULL;
    pIncoming->read(pReceived);
    ASSERT_TRUE(pReceived != NULL);
    EXPECT_EQ((PacketID_t)Packet::PACKET_CG_PORT_CHECK, pReceived->getPacketID());
    EXPECT_EQ("Cheiron", ((CGPortCheck*)pReceived)->getPCName());

    delete pReceived;
    delete pIncoming;
}

// The receiver's length check is the other half of the measured size
// field: a datagram that does not hold exactly the packet its header
// claims is rejected rather than read past.
TEST(DatagramFrameRoundTrip, ALengthThatDisagreesWithTheSizeFieldIsRejected) {
    installPortCheckFactory();

    CGPortCheck packet;
    packet.setPCName("Cheiron");

    Datagram framed;
    framed.write(&packet);

    for (int lengthDrift : {-1, 1}) {
        std::vector<char> bytes(framed.getLength() + 1, 0);
        memcpy(bytes.data(), framed.getData(), framed.getLength());

        Datagram damaged;
        damaged.setData(bytes.data(), framed.getLength() + lengthDrift);

        DatagramPacket* pReceived = NULL;
        EXPECT_THROW(damaged.read(pReceived), Error) << "length drift " << lengthDrift;
        EXPECT_TRUE(pReceived == NULL) << "length drift " << lengthDrift;
    }
}
