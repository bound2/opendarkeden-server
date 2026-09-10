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
//               kEncryptCodes (TestStreams.h) covers both branches of the
//               packet read/write pattern FOR PACKETS THAT USE THE
//               ENCRYPTER: 0 takes the plain branch, 1..5 take the
//               __USE_ENCRYPTER__ branch through every SHUFFLE_STATEMENT_N
//               field order. GCMoveOK and CGMove are such packets; the
//               other 17 encrypter packets are pinned the same way in
//               packet_encrypter_test.cpp.
//
//               CGSay and CGWhisper do NOT reference the encrypter — they
//               always take the plain path — so they are pinned at code 0
//               only; recording six identical files would advertise
//               coverage that does not exist. encrypterFreePacketsAreStill
//               EncrypterFree below fails if that ever changes, which is
//               the signal to add per-code goldens for them.
//
//               The last section holds the length-prefixed string
//               fields of packets and records that belong to no packet
//               family with a file of its own: what each field's bounds
//               admit, and what they refuse.
//
//////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>

#include "CGAddSMSAddress.h"
#include "CGCrashReport.h"
#include "CGModifyNickname.h"
#include "CGMove.h"
#include "CGSay.h"
#include "CGWhisper.h"
#include "GCBloodBibleStatus.h"
#include "GCFriendChatting.h"
#include "GCMoveOK.h"
#include "GCNotifyWin.h"
#include "GCShopList.h"
#include "GCShowMessageBox.h"
#include "GCSystemMessage.h"
#include "GuildWarInfo.h"
#include "ItemNameInfo.h"
#include "QuestStatusInfo.h"
#include "Resource.h"
#include "TestStreams.h"
#include "WireString.h"

using wiretest::expectGolden;
using wiretest::Loopback;
using wiretest::roundTrip;
using wiretest::writeBody;
using wiretest::writeFramed;

namespace {

using wiretest::kEncryptCodeCount;
using wiretest::kEncryptCodes;

//////////////////////////////////////////////////////////////////////
// GCMoveOK — fixed-width fields, uses the encrypter shuffle
//////////////////////////////////////////////////////////////////////

TEST(GCMoveOKTest, roundTripsThroughLoopbackForEveryEncryptCode) {
    for (size_t i = 0; i < kEncryptCodeCount; i++) {
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
    for (size_t i = 0; i < kEncryptCodeCount; i++)
        expectGolden("GCMoveOK", kEncryptCodes[i], writeBody(packet, kEncryptCodes[i]));
}

//////////////////////////////////////////////////////////////////////
// CGMove — the client->server twin
//////////////////////////////////////////////////////////////////////

TEST(CGMoveTest, roundTripsThroughLoopbackForEveryEncryptCode) {
    for (size_t i = 0; i < kEncryptCodeCount; i++) {
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
    for (size_t i = 0; i < kEncryptCodeCount; i++)
        expectGolden("CGMove", kEncryptCodes[i], writeBody(packet, kEncryptCodes[i]));
}

//////////////////////////////////////////////////////////////////////
// CGSay — BYTE-length-prefixed string
//////////////////////////////////////////////////////////////////////

TEST(CGSayTest, roundTripsThroughLoopback) {
    for (size_t i = 0; i < kEncryptCodeCount; i++) {
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
    for (size_t i = 0; i < kEncryptCodeCount; i++) {
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

    for (size_t i = 1; i < kEncryptCodeCount; i++) {
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

#include "GCSkillInfo.h"
#include "OustersSkillInfo.h"

TEST(GCSkillInfoTest, oustersLevelIsIncludedInFrameSize) {
    for (int count : {0, 1, 2, 120}) {
        GCSkillInfo packet;
        packet.setPCType(PC_OUSTERS);
        auto* skills = new OustersSkillInfo();
        for (int i = 0; i < count; ++i) {
            auto* skill = new SubOustersSkillInfo();
            skill->setSkillType(246 + i);
            skill->setExpLevel(1);
            skill->setSkillTurn(5);
            skill->setCastingTime(3);
            skills->addListElement(skill);
        }
        packet.addListElement(skills);
        const auto body = writeBody(packet, 0);
        const auto frame = writeFramed(packet, 0);
        const unsigned expectedSize = 4 + 12 * count;
        ASSERT_EQ(expectedSize, body.size());
        EXPECT_EQ(expectedSize, packet.getPacketSize());
        ASSERT_EQ(expectedSize + 7, frame.size());
        const unsigned declaredSize = frame[2] | (frame[3] << 8) | (frame[4] << 16) | (frame[5] << 24);
        EXPECT_EQ(expectedSize, declaredSize);
        EXPECT_EQ(12u, SubOustersSkillInfo::getMaxSize());
        EXPECT_GE(OustersSkillInfo::getMaxSize(), skills->getSize());
    }
}

TEST(GCShopListTest, shopTypeIsIncludedInFrameSize) {
    // Beginner-zone Dennis sends a full rack with one option per item.
    // Populate wire fields directly: live Item objects belong to gameserver.
    for (BYTE count : {0, 1, SHOP_RACK_INDEX_MAX}) {
        for (BYTE shopType : {0, 1}) {
            Loopback fixture;
            fixture.setCodes(0);
            auto& out = fixture.out();
            out.write(ObjectID_t(10194));
            out.write(ShopVersion_t(100));
            out.write(ShopRackType_t(1));
            out.write(count);
            for (BYTE i = 0; i < count; ++i) {
                out.write(i);
                out.write(ObjectID_t(10204 + i));
                out.write(BYTE(11));
                out.write(ItemType_t(0));
                out.write(BYTE(1));
                out.write(OptionType_t(0));
                out.write(Durability_t(1500));
                out.write(Silver_t(0));
                out.write(Grade_t(4));
                out.write(EnchantLevel_t(0));
            }
            out.write(MarketCond_t(25));
            out.write(MarketCond_t(100));
            out.write(shopType);
            const unsigned expectedSize = 15 + 21 * count;
            ASSERT_EQ(expectedSize, out.length());
            fixture.pump(expectedSize);
            GCShopList packet;
            packet.read(fixture.in());
            ASSERT_EQ(0u, fixture.in().length());
            EXPECT_EQ(shopType, packet.getNPCShopType());

            const auto body = writeBody(packet, 0);
            const auto frame = writeFramed(packet, 0);
            ASSERT_EQ(expectedSize, body.size());
            EXPECT_EQ(expectedSize, packet.getPacketSize());
            ASSERT_EQ(expectedSize + 7, frame.size());
            const unsigned declaredSize = frame[2] | (frame[3] << 8) | (frame[4] << 16) | (frame[5] << 24);
            EXPECT_EQ(expectedSize, declaredSize);
            EXPECT_EQ(shopType, frame.back());
        }
    }
}

#include "SubOustersSkillInfo.h"
#include "SubSlayerSkillInfo.h"
#include "SubVampireSkillInfo.h"

// A stream that stops short leaves a sub-skill record half-parsed and
// the caller reading the next record from the wrong offset, so read()
// lets the failure reach it instead of printing it.
TEST(SubSkillInfoTest, aShortStreamStopsTheRead) {
    for (int which = 0; which < 3; which++) {
        Loopback loopback;
        loopback.setCodes(0);
        loopback.out().write((BYTE)0x81);
        loopback.pump(1);

        if (which == 0) {
            SubSlayerSkillInfo info;
            EXPECT_THROW(info.read(loopback.in()), InsufficientDataException);
        } else if (which == 1) {
            SubVampireSkillInfo info;
            EXPECT_THROW(info.read(loopback.in()), InsufficientDataException);
        } else {
            SubOustersSkillInfo info;
            EXPECT_THROW(info.read(loopback.in()), InsufficientDataException);
        }
    }
}

//////////////////////////////////////////////////////////////////////
// Length-prefixed string fields, in packets with no family file
//
// Each of these fields states its bounds once, in the de::wire call
// that carries the prefix and the bytes. What is pinned per field is
// the value at the cap surviving the round trip and the refusals: a
// value past the cap, and -- where the two halves disagree about the
// empty value, because read() takes the field unconditionally and
// write() emits whatever it holds -- an empty field that write() sends
// and read() refuses.
//////////////////////////////////////////////////////////////////////

namespace {

// The unencrypted branch: none of these packets reference the
// encrypter, so this is the only code whose bytes differ from any other.
const uchar kStringFieldCode = 0;

// Write a record through a loopback and read it back, the way the
// packets that embed it do.
template <typename T> void recordRoundTrip(const T& src, T& dst, uint size) {
    Loopback link;
    link.setCodes(kStringFieldCode);
    src.write(link.out());
    link.pump(size);
    dst.read(link.in());
}

} // namespace

TEST(CGAddSMSAddressTest, theThreeFieldsStopAtWhatTheFactoryMaxBudgets) {
    CGAddSMSAddress src;
    src.setCharacterName(std::string(20, 'c'));
    src.setCustomName(std::string(40, 'u'));
    src.setNumber(std::string(11, '7'));

    CGAddSMSAddress dst;
    roundTrip(src, dst, kStringFieldCode);
    EXPECT_EQ(src.getCharacterName(), dst.getCharacterName());
    EXPECT_EQ(src.getCustomName(), dst.getCustomName());
    EXPECT_EQ(src.getNumber(), dst.getNumber());

    src.setNumber(std::string(12, '7'));
    EXPECT_THROW(writeBody(src, kStringFieldCode), InvalidProtocolException);

    CGAddSMSAddress noNumber;
    noNumber.setCharacterName("Name");
    noNumber.setCustomName("Custom");

    CGAddSMSAddress back;
    EXPECT_THROW(roundTrip(noNumber, back, kStringFieldCode), InvalidProtocolException);
}

TEST(CGCrashReportTest, theWordPrefixedFieldsStopAtTheirCaps) {
    CGCrashReport src;
    src.setExecutableTime("2026-01-02 03:04:05");
    src.setVersion(0x1234);
    src.setAddress("0x00401000");
    src.setOS(std::string(100, 'o'));
    src.setCallStack(std::string(1024, 'c'));
    src.setMessage(std::string(1024, 'm'));

    CGCrashReport dst;
    roundTrip(src, dst, kStringFieldCode);
    EXPECT_EQ(src.getOS(), dst.getOS());
    EXPECT_EQ(src.getCallStack(), dst.getCallStack());
    EXPECT_EQ(src.getMessage(), dst.getMessage());

    // The WORD prefix reaches past what a byte counts, which is why
    // these three carry one.
    EXPECT_EQ(szWORD + 1024u, de::wire::stringWireSize16(src.getMessage()));

    src.setOS(std::string(101, 'o'));
    EXPECT_THROW(writeBody(src, kStringFieldCode), InvalidProtocolException);

    CGCrashReport noOS;
    noOS.setExecutableTime("2026-01-02 03:04:05");
    noOS.setVersion(0x1234);
    noOS.setAddress("0x00401000");

    CGCrashReport back;
    EXPECT_THROW(roundTrip(noOS, back, kStringFieldCode), InvalidProtocolException);
}

TEST(CGModifyNicknameTest, theNicknameStopsAtWhatTheFactoryMaxBudgets) {
    CGModifyNickname src;
    src.setItemObjectID(0x4321);
    src.setNickname(std::string(MAX_NICKNAME_SIZE, 'n'));

    CGModifyNickname dst;
    roundTrip(src, dst, kStringFieldCode);
    EXPECT_EQ(src.getNickname(), dst.getNickname());

    CGModifyNickname absent;
    absent.setItemObjectID(0x4321);

    CGModifyNickname absentBack;
    absentBack.setNickname("left over from the last read");
    roundTrip(absent, absentBack, kStringFieldCode);
    EXPECT_TRUE(absentBack.getNickname().empty());

    src.setNickname(std::string(MAX_NICKNAME_SIZE + 1, 'n'));
    EXPECT_THROW(writeBody(src, kStringFieldCode), InvalidProtocolException);
}

TEST(GCBloodBibleStatusTest, theOwnerNameStopsAtWhatItsLengthByteCarries) {
    GCBloodBibleStatus src;
    src.setItemType(3);
    src.setZoneID(7);
    src.setStorage(2);
    src.setRace(1);
    src.setShrineRace(2);
    src.setX(11);
    src.setY(22);
    src.setOwnerName(std::string(de::wire::kMaxByteStringLength, 'o'));

    GCBloodBibleStatus dst;
    roundTrip(src, dst, kStringFieldCode);
    EXPECT_EQ(src.getOwnerName(), dst.getOwnerName());

    src.setOwnerName("");
    GCBloodBibleStatus absentBack;
    roundTrip(src, absentBack, kStringFieldCode);
    EXPECT_TRUE(absentBack.getOwnerName().empty());

    src.setOwnerName(std::string(de::wire::kMaxByteStringLength + 1, 'o'));
    EXPECT_THROW(writeBody(src, kStringFieldCode), InvalidProtocolException);
}

// The one packet here whose two halves cap the same field differently:
// the message is refused past 128 on read and past 512 on write.
TEST(GCFriendChattingTest, theNameAndTheMessageKeepTheirOwnCaps) {
    GCFriendChatting src;
    src.setCommand(GC_MESSAGE);
    src.setPlayerName(std::string(32, 'p'));
    src.setMessage(std::string(128, 'm'));
    src.setIsBlack(1);
    src.setIsOnLine(1);

    GCFriendChatting dst;
    roundTrip(src, dst, kStringFieldCode);
    EXPECT_EQ(src.getPlayerName(), dst.getPlayerName());
    EXPECT_EQ(src.getMessage(), dst.getMessage());

    src.setMessage(std::string(129, 'm'));
    GCFriendChatting refused;
    EXPECT_THROW(roundTrip(src, refused, kStringFieldCode), InvalidProtocolException);

    src.setMessage(std::string(513, 'm'));
    EXPECT_THROW(writeBody(src, kStringFieldCode), InvalidProtocolException);

    src.setPlayerName(std::string(33, 'p'));
    EXPECT_THROW(writeBody(src, kStringFieldCode), InvalidProtocolException);

    // Most of the sends leave both strings empty; read() refuses that.
    GCFriendChatting bare;
    bare.setCommand(GC_ADD_FRIEND_ERROR);

    GCFriendChatting bareBack;
    EXPECT_THROW(roundTrip(bare, bareBack, kStringFieldCode), InvalidProtocolException);
}

TEST(GCNotifyWinTest, theNameStopsAtWhatItsLengthByteCarries) {
    GCNotifyWin src;
    src.setGiftID(0x11223344);
    src.setName(std::string(de::wire::kMaxByteStringLength, 'w'));

    GCNotifyWin dst;
    roundTrip(src, dst, kStringFieldCode);
    EXPECT_EQ(src.getName(), dst.getName());

    src.setName(std::string(de::wire::kMaxByteStringLength + 1, 'w'));
    EXPECT_THROW(writeBody(src, kStringFieldCode), InvalidProtocolException);

    src.setName("");
    EXPECT_THROW(writeBody(src, kStringFieldCode), InvalidProtocolException);
}

// Before the bounds were stated once, both halves of this packet threw
// on every length, and write() emitted no length byte at all.
TEST(GCShowMessageBoxTest, theMessageStopsAtWhatItsLengthByteCarries) {
    GCShowMessageBox src;
    src.setMessage(std::string(de::wire::kMaxByteStringLength, 'b'));

    GCShowMessageBox dst;
    roundTrip(src, dst, kStringFieldCode);
    EXPECT_EQ(src.getMessage(), dst.getMessage());

    src.setMessage(std::string(de::wire::kMaxByteStringLength + 1, 'b'));
    EXPECT_THROW(writeBody(src, kStringFieldCode), InvalidProtocolException);

    src.setMessage("");
    EXPECT_THROW(writeBody(src, kStringFieldCode), InvalidProtocolException);
}

TEST(GCSystemMessageTest, theMessageStopsAtWhatItsLengthByteCarries) {
    GCSystemMessage src;
    src.setMessage(std::string(de::wire::kMaxByteStringLength, 's'));
    src.setColor(0x006040E8);
    src.setType(SYSTEM_MESSAGE_NORMAL);

    GCSystemMessage dst;
    roundTrip(src, dst, kStringFieldCode);
    EXPECT_EQ(src.getMessage(), dst.getMessage());
    EXPECT_EQ(src.getColor(), dst.getColor());

    src.setMessage(std::string(de::wire::kMaxByteStringLength + 1, 's'));
    EXPECT_THROW(writeBody(src, kStringFieldCode), InvalidProtocolException);

    src.setMessage("");
    EXPECT_THROW(writeBody(src, kStringFieldCode), InvalidProtocolException);
}

TEST(GuildWarInfoTest, theTwoGuildNamesStopAtWhatTheRecordMaxBudgets) {
    GuildWarInfo src;
    src.setStartTime(0x20260102);
    src.setRemainTime(3600);
    src.setCastleID(9);
    src.setAttackGuildName(std::string(40, 'a'));
    src.setDefenseGuildName(std::string(30, 'd'));
    src.addJoinGuild(0x1111);

    GuildWarInfo dst;
    recordRoundTrip(src, dst, src.getSize());
    EXPECT_EQ(src.getAttackGuildName(), dst.getAttackGuildName());
    EXPECT_EQ(src.getDefenseGuildName(), dst.getDefenseGuildName());

    GuildWarInfo unnamed;
    unnamed.setStartTime(0x20260102);
    unnamed.setRemainTime(3600);
    unnamed.setCastleID(9);

    GuildWarInfo unnamedBack;
    recordRoundTrip(unnamed, unnamedBack, unnamed.getSize());
    EXPECT_TRUE(unnamedBack.getAttackGuildName().empty());
    EXPECT_TRUE(unnamedBack.getDefenseGuildName().empty());

    src.setDefenseGuildName(std::string(31, 'd'));
    Loopback link;
    link.setCodes(kStringFieldCode);
    EXPECT_THROW(src.write(link.out()), InvalidProtocolException);
}

TEST(ItemNameInfoTest, theNameStopsAtWhatTheRecordMaxBudgets) {
    ItemNameInfo src(0x4455, std::string(20, 'i'));

    ItemNameInfo dst;
    recordRoundTrip(src, dst, src.getSize());
    EXPECT_EQ(src.getName(), dst.getName());

    ItemNameInfo tooLong(0x4455, std::string(21, 'i'));
    Loopback link;
    link.setCodes(kStringFieldCode);
    EXPECT_THROW(tooLong.write(link.out()), InvalidProtocolException);

    // write() emits an empty name; read() refuses it.
    ItemNameInfo unnamed(0x4455, "");
    ItemNameInfo back;
    EXPECT_THROW(recordRoundTrip(unnamed, back, unnamed.getSize()), InvalidProtocolException);
}

TEST(MissionInfoTest, theStringArgumentStopsAtWhatItsLengthByteCarries) {
    MissionInfo src;
    src.m_Condition = 1;
    src.m_Index = 2;
    src.m_Status = MissionInfo::CURRENT;
    src.m_StrArg = std::string(de::wire::kMaxByteStringLength, 'q');
    src.m_NumArg = 0x01020304;

    MissionInfo dst;
    recordRoundTrip(src, dst, src.getSize());
    EXPECT_EQ(src.m_StrArg, dst.m_StrArg);
    EXPECT_EQ(src.m_NumArg, dst.m_NumArg);

    src.m_StrArg = "";
    MissionInfo absentBack;
    absentBack.m_StrArg = "left over from the last read";
    recordRoundTrip(src, absentBack, src.getSize());
    EXPECT_TRUE(absentBack.m_StrArg.empty());

    src.m_StrArg = std::string(de::wire::kMaxByteStringLength + 1, 'q');
    Loopback link;
    link.setCodes(kStringFieldCode);
    EXPECT_THROW(src.write(link.out()), InvalidProtocolException);
}

// maxFilename is 256, one past what the length byte can describe, so the
// byte's own range is the cap.
TEST(ResourceTest, theFilenameStopsAtWhatItsLengthByteCarries) {
    Resource src;
    src.setVersion(0x0102);
    src.setFilename(std::string(de::wire::kMaxByteStringLength, 'f'));
    src.setFileSize(0x00112233);

    Resource dst;
    recordRoundTrip(src, dst, src.getSize());
    EXPECT_EQ(src.getFilename(), dst.getFilename());
    EXPECT_EQ(src.getFileSize(), dst.getFileSize());

    Loopback link;
    link.setCodes(kStringFieldCode);

    Resource tooLong;
    tooLong.setFilename(std::string(de::wire::kMaxByteStringLength + 1, 'f'));
    EXPECT_THROW(tooLong.write(link.out()), InvalidProtocolException);

    Resource unnamed;
    EXPECT_THROW(unnamed.write(link.out()), InvalidProtocolException);
}
