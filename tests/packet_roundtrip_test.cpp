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
//               read/write pattern FOR PACKETS THAT USE THE ENCRYPTER:
//               0 takes the plain branch, 1..3 take the __USE_ENCRYPTER__
//               branch through all three SHUFFLE_STATEMENT_3 field orders
//               (code % 3). GCMoveOK and CGMove are such packets.
//
//               CGSay and CGWhisper do NOT reference the encrypter — they
//               always take the plain path — so they are pinned at code 0
//               only; recording four identical files would advertise
//               coverage that does not exist. encrypterFreePacketsAreStill
//               EncrypterFree below fails if that ever changes, which is
//               the signal to add per-code goldens for them.
//
//               Coverage is still narrow: 17 packets use the encrypter,
//               including SHUFFLE_STATEMENT_4/_5 whose case 3/4 orders are
//               non-rotations, and none of those are pinned yet. Tracked
//               as task 1.2 in docs/RESTRUCTURING.md.
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
using wiretest::writeFramed;

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
    // Encrypter-free: one golden, not four identical ones. See the header
    // comment and encrypterFreePacketsAreStillEncrypterFree.
    expectGolden("CGSay", 0, writeBody(packet, 0));
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
    expectGolden("CGWhisper", 0, writeBody(packet, 0));
}

// The two packets above are pinned at one encrypt code because their
// read/write ignore the encrypter. Prove that assumption rather than
// trusting it: if either starts encrypting, its bytes would vary by code
// and the single golden would silently stop covering three of them.
TEST(EncrypterCoverageTest, encrypterFreePacketsAreStillEncrypterFree) {
    CGSay say;
    say.setColor(0x11223344);
    say.setMessage("hello darkeden");

    CGWhisper whisper;
    whisper.setName("Reiot");
    whisper.setColor(0xCAFEBABE);
    whisper.setMessage("wire pin test");

    for (size_t i = 1; i < sizeof(kEncryptCodes); i++) {
        EXPECT_EQ(writeBody(say, 0), writeBody(say, kEncryptCodes[i]))
            << "CGSay now varies with the encrypt code — add per-code goldens";
        EXPECT_EQ(writeBody(whisper, 0), writeBody(whisper, kEncryptCodes[i]))
            << "CGWhisper now varies with the encrypt code — add per-code goldens";
    }
}

//////////////////////////////////////////////////////////////////////
// Frame layout — writePacket's header is part of the contract too
//////////////////////////////////////////////////////////////////////

// The header is written by writePacket(), not by any packet's write(), so
// no body golden covers it. Without this pin, widening PacketID_t,
// PacketSize_t or SequenceSize_t in Packet.h leaves all 16 body goldens
// and the whole inventory unchanged while desynchronising every client on
// the first byte of all 463 packets: the round-trip and header tests read
// back through the same typedefs and would still agree with themselves.
TEST(PacketFramingTest, framedBytesMatchGolden) {
    GCMoveOK packet(11, 22, 3);
    expectGolden("GCMoveOK.framed", 0, writeFramed(packet, 0));
}

// Belt and braces: state the on-wire header widths as a compile-time fact,
// so a typedef change fails at build time with a clear message instead of
// only as a hex diff.
TEST(PacketFramingTest, headerFieldWidthsAreUnchanged) {
    EXPECT_EQ(2u, szPacketID) << "PacketID_t width changed — every client desyncs";
    EXPECT_EQ(4u, szPacketSize) << "PacketSize_t width changed — every client desyncs";
    EXPECT_EQ(1u, szSequenceSize) << "SequenceSize_t width changed — every client desyncs";
    EXPECT_EQ(7u, szPacketHeader);
}

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
