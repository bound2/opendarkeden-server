//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_roundtrip_test.cpp
// Description : Round-trip and golden-byte pins for representative
//               packets, through the real stream + socket classes.
//
//               The golden .hex files under tests/golden/ ARE the wire
//               contract: the client repo carries its own hand-written
//               copy of every packet, so a byte that moves here breaks
//               live clients with no compile error anywhere. A failing
//               golden is a protocol change to be reviewed, not a test
//               to be silenced.
//
//               Encrypt codes 0..3 cover both branches of the packet
//               read/write pattern: 0 takes the plain branch, 1..3 take
//               the __USE_ENCRYPTER__ branch through all three
//               SHUFFLE_STATEMENT field orders (code % 3).
//
//////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>

#include "CGMove.h"
#include "CGSay.h"
#include "CGWhisper.h"
#include "GCMoveOK.h"
#include "TestStreams.h"

using wiretest::expectGolden;
using wiretest::Loopback;
using wiretest::roundTrip;
using wiretest::writeBody;

namespace {

const uchar kEncryptCodes[] = {0, 1, 2, 3};

//////////////////////////////////////////////////////////////////////
// GCMoveOK — fixed-width fields, uses the encrypter shuffle
//////////////////////////////////////////////////////////////////////

TEST(GCMoveOKTest, roundTripsThroughLoopbackForEveryEncryptCode) {
    for (size_t i = 0; i < sizeof(kEncryptCodes); i++) {
        GCMoveOK src(11, 22, 3);
        GCMoveOK dst;
        roundTrip(src, dst, kEncryptCodes[i]);
        EXPECT_EQ(src.getX(), dst.getX()) << "code " << (int)kEncryptCodes[i];
        EXPECT_EQ(src.getY(), dst.getY()) << "code " << (int)kEncryptCodes[i];
        EXPECT_EQ(src.getDir(), dst.getDir()) << "code " << (int)kEncryptCodes[i];
    }
}

TEST(GCMoveOKTest, bodyBytesMatchGolden) {
    GCMoveOK packet(11, 22, 3);
    for (size_t i = 0; i < sizeof(kEncryptCodes); i++)
        expectGolden("GCMoveOK", kEncryptCodes[i], writeBody(packet, kEncryptCodes[i]));
}

//////////////////////////////////////////////////////////////////////
// CGMove — the client->server twin
//////////////////////////////////////////////////////////////////////

TEST(CGMoveTest, roundTripsThroughLoopbackForEveryEncryptCode) {
    for (size_t i = 0; i < sizeof(kEncryptCodes); i++) {
        CGMove src;
        src.setX(101);
        src.setY(57);
        src.setDir(6);
        CGMove dst;
        roundTrip(src, dst, kEncryptCodes[i]);
        EXPECT_EQ(src.getX(), dst.getX()) << "code " << (int)kEncryptCodes[i];
        EXPECT_EQ(src.getY(), dst.getY()) << "code " << (int)kEncryptCodes[i];
        EXPECT_EQ(src.getDir(), dst.getDir()) << "code " << (int)kEncryptCodes[i];
    }
}

TEST(CGMoveTest, bodyBytesMatchGolden) {
    CGMove packet;
    packet.setX(101);
    packet.setY(57);
    packet.setDir(6);
    for (size_t i = 0; i < sizeof(kEncryptCodes); i++)
        expectGolden("CGMove", kEncryptCodes[i], writeBody(packet, kEncryptCodes[i]));
}

//////////////////////////////////////////////////////////////////////
// CGSay — BYTE-length-prefixed string
//////////////////////////////////////////////////////////////////////

TEST(CGSayTest, roundTripsThroughLoopback) {
    for (size_t i = 0; i < sizeof(kEncryptCodes); i++) {
        CGSay src;
        src.setColor(0x11223344);
        src.setMessage("hello darkeden");
        CGSay dst;
        roundTrip(src, dst, kEncryptCodes[i]);
        EXPECT_EQ(src.getColor(), dst.getColor()) << "code " << (int)kEncryptCodes[i];
        EXPECT_EQ(src.getMessage(), dst.getMessage()) << "code " << (int)kEncryptCodes[i];
    }
}

TEST(CGSayTest, bodyBytesMatchGolden) {
    CGSay packet;
    packet.setColor(0x11223344);
    packet.setMessage("hello darkeden");
    for (size_t i = 0; i < sizeof(kEncryptCodes); i++)
        expectGolden("CGSay", kEncryptCodes[i], writeBody(packet, kEncryptCodes[i]));
}

TEST(CGSayTest, refusesOversizedMessage) {
    CGSay packet;
    packet.setColor(0);
    packet.setMessage(std::string(129, 'x'));
    SocketEncryptOutputStream oStream(NULL);
    EXPECT_THROW(packet.write(oStream), InvalidProtocolException);
}

//////////////////////////////////////////////////////////////////////
// CGWhisper — two length-prefixed strings
//////////////////////////////////////////////////////////////////////

TEST(CGWhisperTest, roundTripsThroughLoopback) {
    for (size_t i = 0; i < sizeof(kEncryptCodes); i++) {
        CGWhisper src;
        src.setName("Reiot");
        src.setColor(0xCAFEBABE);
        src.setMessage("wire pin test");
        CGWhisper dst;
        roundTrip(src, dst, kEncryptCodes[i]);
        EXPECT_EQ(src.getName(), dst.getName()) << "code " << (int)kEncryptCodes[i];
        EXPECT_EQ(src.getColor(), dst.getColor()) << "code " << (int)kEncryptCodes[i];
        EXPECT_EQ(src.getMessage(), dst.getMessage()) << "code " << (int)kEncryptCodes[i];
    }
}

TEST(CGWhisperTest, bodyBytesMatchGolden) {
    CGWhisper packet;
    packet.setName("Reiot");
    packet.setColor(0xCAFEBABE);
    packet.setMessage("wire pin test");
    for (size_t i = 0; i < sizeof(kEncryptCodes); i++)
        expectGolden("CGWhisper", kEncryptCodes[i], writeBody(packet, kEncryptCodes[i]));
}

//////////////////////////////////////////////////////////////////////
// Frame layout — writePacket's header is part of the contract too
//////////////////////////////////////////////////////////////////////

TEST(PacketFramingTest, headerIsIdSizeSequenceThenBody) {
    Loopback loopback;
    loopback.setCodes(0);

    GCMoveOK packet(11, 22, 3);
    loopback.out().writePacket(&packet);
    loopback.pump(szPacketHeader + packet.getPacketSize());

    PacketID_t id = 0;
    PacketSize_t size = 0;
    SequenceSize_t sequence = 0xFF;
    loopback.in().read(id);
    loopback.in().read(size);
    loopback.in().read(sequence);

    EXPECT_EQ(packet.getPacketID(), id);
    EXPECT_EQ(packet.getPacketSize(), size);
    EXPECT_EQ(0, (int)sequence) << "a fresh stream starts at sequence 0";

    GCMoveOK dst;
    dst.read(loopback.in());
    EXPECT_EQ(packet.getX(), dst.getX());
    EXPECT_EQ(packet.getY(), dst.getY());
    EXPECT_EQ(packet.getDir(), dst.getDir());
}

TEST(PacketFramingTest, sequenceIncrementsPerPacket) {
    Loopback loopback;
    loopback.setCodes(0);

    GCMoveOK packet(1, 2, 3);
    loopback.out().writePacket(&packet);
    loopback.out().writePacket(&packet);
    loopback.pump(2 * (szPacketHeader + packet.getPacketSize()));

    for (int expectedSequence = 0; expectedSequence < 2; expectedSequence++) {
        PacketID_t id = 0;
        PacketSize_t size = 0;
        SequenceSize_t sequence = 0xFF;
        loopback.in().read(id);
        loopback.in().read(size);
        loopback.in().read(sequence);
        EXPECT_EQ(expectedSequence, (int)sequence);
        GCMoveOK dst;
        dst.read(loopback.in());
    }
}

} // namespace
