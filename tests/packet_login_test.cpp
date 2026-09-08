//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_login_test.cpp
// Description : Golden byte fixtures, loopback round trips and size
//               pins for every CL and LC packet — the whole login
//               phase, from the version check to the game server
//               handoff.
//
//               These 33 packets are the only thing the client and the
//               login server say to each other before a character is
//               in a zone, and the client repo carries its own
//               hand-written copy of each. A byte that moves here
//               breaks live clients with no compile error anywhere, so
//               a failing golden is a protocol change to review, not a
//               test to silence.
//
//               None of the 33 references the encrypter — they all take
//               the plain read()/write() path — so goldens are recorded
//               at encrypt code 0 only, as CGSay and CGWhisper are in
//               packet_roundtrip_test.cpp. Recording six identical
//               files would advertise coverage that does not exist. The
//               golden test of every packet also asserts that its bytes
//               do not vary with the code, so adopting the encrypter
//               fails loudly instead of silently voiding five sixths of
//               the pin.
//
//               Each packet gets three pins (LOGIN_PACKET_TESTS):
//
//               - a loopback round trip through the real socket and
//                 stream classes, comparing every getter;
//               - the body bytes against tests/golden/<Name>.code0.hex;
//               - getPacketSize() against the byte count write()
//                 actually emits, and against the factory's
//                 getPacketMaxSize(). writePacket() puts
//                 getPacketSize() on the wire BEFORE calling write(),
//                 so a disagreement is not a wrong length, it is a
//                 stream that never resynchronises; and the receiving
//                 side sizes its read buffer from the factory max, so a
//                 body that outgrows it is a truncated packet, not a
//                 caught error.
//
//               Fixture values are distinct per field and >= 128 in
//               every byte the width allows, so a transposed pair of
//               fields or a signedness flip changes the golden. Two
//               groups of fields cannot follow that rule and say so at
//               the point of use: enum-valued bytes, whose domain is a
//               handful of small values, and the PC attributes, whose
//               getters reject anything above 2000.
//
//               Range checks on the enum bytes CLCreatePC and
//               CLSelectPC read off an unauthenticated socket live in
//               packet_field_bounds_test.cpp; what is pinned here is
//               the string framing those two share with the rest of the
//               login phase.
//
//               Three findings are stated below as tests that FAIL when
//               the underlying packet is fixed, which is the signal to
//               retire them:
//               emptySlayerNameUnderflowsTheBody,
//               groupNameLengthPrefixIsUnbounded and
//               listCapacityIsThirtySevenEntries.
//
//////////////////////////////////////////////////////////////////////

#include <cstring>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "CLChangeServer.h"
#include "CLCreatePC.h"
#include "CLDeletePC.h"
#include "CLGetPCList.h"
#include "CLGetServerList.h"
#include "CLGetWorldList.h"
#include "CLLogin.h"
#include "CLLogout.h"
#include "CLQueryCharacterName.h"
#include "CLQueryPlayerID.h"
#include "CLReconnectLogin.h"
#include "CLRegisterPlayer.h"
#include "CLSelectPC.h"
#include "CLSelectServer.h"
#include "CLSelectWorld.h"
#include "CLVersionCheck.h"
#include "Exception.h"
#include "LCCreatePCError.h"
#include "LCCreatePCOK.h"
#include "LCDeletePCError.h"
#include "LCDeletePCOK.h"
#include "LCLoginError.h"
#include "LCLoginOK.h"
#include "LCPCList.h"
#include "LCQueryResultCharacterName.h"
#include "LCQueryResultPlayerID.h"
#include "LCReconnect.h"
#include "LCRegisterPlayerError.h"
#include "LCRegisterPlayerOK.h"
#include "LCSelectPCError.h"
#include "LCServerList.h"
#include "LCVersionCheckError.h"
#include "LCVersionCheckOK.h"
#include "LCWorldList.h"
#include "TestStreams.h"

using wiretest::expectGolden;
using wiretest::kEncryptCodeCount;
using wiretest::kEncryptCodes;
using wiretest::Loopback;
using wiretest::roundTrip;
using wiretest::writeBody;

namespace {

// The unencrypted branch. Every packet in this file reads and writes
// its body with plain read()/write() calls, so this is the only code
// whose bytes differ from any other.
const uchar kPlainCode = 0;

// PC attributes are WORDs but their getters reject anything above
// maxSlayerAttr / maxVampireAttr / maxOustersAttr (2000), so the high
// byte of an attribute fixture cannot be >= 128. The low byte still is,
// and the three values stay distinct.
const Attr_t kSTR = 0x0781; // 1921
const Attr_t kDEX = 0x0792; // 1938
const Attr_t kINT = 0x07A3; // 1955

// Push a raw byte image through a real loopback connection and let the
// packet read it, exactly as the login server reads a client's bytes.
void readImage(Packet& packet, const std::vector<unsigned char>& bytes) {
    Loopback loopback;
    loopback.setCodes(kPlainCode);
    loopback.out().write(reinterpret_cast<const char*>(&bytes[0]), (uint)bytes.size());
    loopback.pump((uint)bytes.size());
    packet.read(loopback.in());
}

// A BYTE length prefix followed by the string's raw bytes — the framing
// every string field in the login phase uses.
void appendString(std::vector<unsigned char>& image, const std::string& value) {
    image.push_back((unsigned char)value.size());
    for (size_t i = 0; i < value.size(); i++)
        image.push_back((unsigned char)value[i]);
}

//////////////////////////////////////////////////////////////////////
// The three pins every packet gets. fill() / expectEqual() are
// overloaded per packet, as in packet_encrypter_test.cpp, so the same
// canonical instance feeds all three.
//////////////////////////////////////////////////////////////////////

#define LOGIN_PACKET_TESTS(Name)                                                                     \
    TEST(Name##Test, roundTripsThroughLoopback) {                                                    \
        Name src;                                                                                    \
        fill(src);                                                                                   \
        Name dst;                                                                                    \
        roundTrip(src, dst, kPlainCode);                                                             \
        expectEqual(src, dst);                                                                       \
    }                                                                                                \
    TEST(Name##Test, bodyBytesMatchGolden) {                                                         \
        Name packet;                                                                                 \
        fill(packet);                                                                                \
        const std::vector<unsigned char> body = writeBody(packet, kPlainCode);                       \
        expectGolden(#Name, kPlainCode, body);                                                       \
        for (size_t i = 1; i < kEncryptCodeCount; i++)                                               \
            EXPECT_EQ(body, writeBody(packet, kEncryptCodes[i]))                                     \
                << #Name " now varies with the encrypt code — add per-code goldens";                 \
    }                                                                                                \
    TEST(Name##Test, sizeMatchesTheBytesWrittenAndFitsTheFactoryMax) {                               \
        Name packet;                                                                                 \
        fill(packet);                                                                                \
        Name##Factory factory;                                                                       \
        EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size())              \
            << #Name ": getPacketSize() disagrees with the bytes write() emits; writePacket() puts " \
                     "the former on the wire, so the stream never resynchronises";                   \
        EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize())                                \
            << #Name ": the body outgrows the read buffer the receiver sizes from the factory max";  \
        EXPECT_EQ(factory.getPacketID(), packet.getPacketID());                                      \
        EXPECT_EQ(factory.getPacketName(), packet.getPacketName());                                  \
    }

//////////////////////////////////////////////////////////////////////
// Bodyless packets — the request/acknowledgement half of the login
// phase. A golden of zero bytes is not a formality: these ride the same
// framed header as everything else, so a field added to one of them
// desynchronises a client that still expects an empty body.
//////////////////////////////////////////////////////////////////////

#define EMPTY_LOGIN_PACKET(Name)                     \
    void fill(Name&) {}                              \
    void expectEqual(const Name& a, const Name& b) { \
        EXPECT_EQ(0u, a.getPacketSize());            \
        EXPECT_EQ(0u, b.getPacketSize());            \
    }                                                \
    LOGIN_PACKET_TESTS(Name)

EMPTY_LOGIN_PACKET(CLGetPCList)
EMPTY_LOGIN_PACKET(CLGetServerList)
EMPTY_LOGIN_PACKET(CLGetWorldList)
EMPTY_LOGIN_PACKET(CLLogout)
EMPTY_LOGIN_PACKET(LCCreatePCOK)
EMPTY_LOGIN_PACKET(LCDeletePCOK)
EMPTY_LOGIN_PACKET(LCVersionCheckError)
EMPTY_LOGIN_PACKET(LCVersionCheckOK)

//////////////////////////////////////////////////////////////////////
// CL — client to login server
//////////////////////////////////////////////////////////////////////

void fill(CLChangeServer& p) {
    p.setServerGroupID(0x8B);
}
void expectEqual(const CLChangeServer& a, const CLChangeServer& b) {
    EXPECT_EQ(a.getServerGroupID(), b.getServerGroupID());
}
LOGIN_PACKET_TESTS(CLChangeServer)

// Slot, sex, hair style and race are enum-valued bytes with a handful
// of enumerators each, so they carry their highest valid value rather
// than a high byte. read() rejects anything past it — pinned in
// packet_field_bounds_test.cpp.
void fill(CLCreatePC& p) {
    p.setName("CreatePCFixture");
    p.setSlot(SLOT2);
    p.setSex(MALE);
    p.setHairStyle(HAIR_STYLE3);
    p.setHairColor(0x8A11);
    p.setSkinColor(0x8B22);
    p.setShirtColor(0x8C33);
    p.setShirtColor(0x8D44, SUB_COLOR);
    p.setJeansColor(0x8E55);
    p.setJeansColor(0x8F66, SUB_COLOR);
    p.setSTR(0x9A1B);
    p.setDEX(0x9B2C);
    p.setINT(0x9C3D);
    p.setRace(RACE_OUSTERS);
}
void expectEqual(const CLCreatePC& a, const CLCreatePC& b) {
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getSlot(), b.getSlot());
    EXPECT_EQ(a.getSex(), b.getSex());
    EXPECT_EQ(a.getHairStyle(), b.getHairStyle());
    EXPECT_EQ(a.getHairColor(), b.getHairColor());
    EXPECT_EQ(a.getSkinColor(), b.getSkinColor());
    EXPECT_EQ(a.getShirtColor(), b.getShirtColor());
    EXPECT_EQ(a.getShirtColor(SUB_COLOR), b.getShirtColor(SUB_COLOR));
    EXPECT_EQ(a.getJeansColor(), b.getJeansColor());
    EXPECT_EQ(a.getJeansColor(SUB_COLOR), b.getJeansColor(SUB_COLOR));
    EXPECT_EQ(a.getSTR(), b.getSTR());
    EXPECT_EQ(a.getDEX(), b.getDEX());
    EXPECT_EQ(a.getINT(), b.getINT());
    EXPECT_EQ(a.getRace(), b.getRace());
}
LOGIN_PACKET_TESTS(CLCreatePC)

TEST(CLCreatePCTest, refusesNamesOutsideOneToTwenty) {
    CLCreatePC packet;
    fill(packet);
    SocketEncryptOutputStream oStream(NULL);

    packet.setName("");
    EXPECT_THROW(packet.write(oStream), InvalidProtocolException);

    packet.setName(std::string(21, 'x'));
    EXPECT_THROW(packet.write(oStream), InvalidProtocolException);
}

void fill(CLDeletePC& p) {
    p.setName("DeleteMe");
    p.setSlot(SLOT3);
    p.setSSN("8801011234567");
}
void expectEqual(const CLDeletePC& a, const CLDeletePC& b) {
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getSlot(), b.getSlot());
    EXPECT_EQ(a.getSSN(), b.getSSN());
}
LOGIN_PACKET_TESTS(CLDeletePC)

// The SSN caps at 14 on both sides even though the factory's max size
// budgets 20 bytes for it, so the slack is unreachable.
TEST(CLDeletePCTest, refusesEmptyOrOversizedNameAndSSN) {
    SocketEncryptOutputStream oStream(NULL);

    CLDeletePC emptyName;
    fill(emptyName);
    emptyName.setName("");
    EXPECT_THROW(emptyName.write(oStream), InvalidProtocolException);

    CLDeletePC longName;
    fill(longName);
    longName.setName(std::string(21, 'x'));
    EXPECT_THROW(longName.write(oStream), InvalidProtocolException);

    CLDeletePC emptySSN;
    fill(emptySSN);
    emptySSN.setSSN("");
    EXPECT_THROW(emptySSN.write(oStream), InvalidProtocolException);

    CLDeletePC longSSN;
    fill(longSSN);
    longSSN.setSSN(std::string(15, '9'));
    EXPECT_THROW(longSSN.write(oStream), InvalidProtocolException);
}

// CLLogin has no setter for its six MAC bytes and its constructor
// leaves them indeterminate, so a default-constructed instance would
// write six bytes of whatever the allocation happened to hold — no
// golden could be recorded from it. The canonical instance is built by
// reading a crafted image instead, which is also what the login server
// does with every CLLogin it ever sees.
const unsigned char kLoginMac[6] = {0x8A, 0x9B, 0xAC, 0xBD, 0xCE, 0xDF};
const char* const kLoginID = "gold-login";
const char* const kLoginPassword = "pw-fixture-01";

void fill(CLLogin& p) {
    std::vector<unsigned char> image;
    appendString(image, kLoginID);
    appendString(image, kLoginPassword);
    for (size_t i = 0; i < 6; i++)
        image.push_back(kLoginMac[i]);
    image.push_back((unsigned char)LOGIN_MODE_WEBLOGIN);
    readImage(p, image);
}
void expectEqual(const CLLogin& a, const CLLogin& b) {
    EXPECT_EQ(a.getID(), b.getID());
    EXPECT_EQ(a.getPassword(), b.getPassword());
    EXPECT_EQ(0, std::memcmp(a.getRareMacAddress(), b.getRareMacAddress(), 6));
    EXPECT_EQ(a.isWebLogin(), b.isWebLogin());
}
LOGIN_PACKET_TESTS(CLLogin)

TEST(CLLoginTest, theFixtureCarriesTheImageItWasBuiltFrom) {
    CLLogin packet;
    fill(packet);
    EXPECT_EQ(std::string(kLoginID), packet.getID());
    EXPECT_EQ(std::string(kLoginPassword), packet.getPassword());
    EXPECT_EQ(0, std::memcmp(packet.getRareMacAddress(), kLoginMac, 6));
    EXPECT_TRUE(packet.isWebLogin());
}

TEST(CLLoginTest, refusesEmptyOrOversizedIDAndPassword) {
    SocketEncryptOutputStream oStream(NULL);

    CLLogin emptyID;
    fill(emptyID);
    emptyID.setID("");
    EXPECT_THROW(emptyID.write(oStream), InvalidProtocolException);

    CLLogin longID;
    fill(longID);
    longID.setID(std::string(31, 'x'));
    EXPECT_THROW(longID.write(oStream), InvalidProtocolException);

    CLLogin emptyPassword;
    fill(emptyPassword);
    emptyPassword.setPassword("");
    EXPECT_THROW(emptyPassword.write(oStream), InvalidProtocolException);

    CLLogin longPassword;
    fill(longPassword);
    longPassword.setPassword(std::string(31, 'x'));
    EXPECT_THROW(longPassword.write(oStream), InvalidProtocolException);
}

void fill(CLQueryCharacterName& p) {
    p.setCharacterName("QueryCharName");
}
void expectEqual(const CLQueryCharacterName& a, const CLQueryCharacterName& b) {
    EXPECT_EQ(a.getCharacterName(), b.getCharacterName());
}
LOGIN_PACKET_TESTS(CLQueryCharacterName)

TEST(CLQueryCharacterNameTest, refusesNamesOutsideOneToTwenty) {
    SocketEncryptOutputStream oStream(NULL);

    CLQueryCharacterName empty;
    empty.setCharacterName("");
    EXPECT_THROW(empty.write(oStream), InvalidProtocolException);

    CLQueryCharacterName tooLong;
    tooLong.setCharacterName(std::string(21, 'x'));
    EXPECT_THROW(tooLong.write(oStream), InvalidProtocolException);
}

void fill(CLQueryPlayerID& p) {
    p.setPlayerID("queryplayer");
}
void expectEqual(const CLQueryPlayerID& a, const CLQueryPlayerID& b) {
    EXPECT_EQ(a.getPlayerID(), b.getPlayerID());
}
LOGIN_PACKET_TESTS(CLQueryPlayerID)

TEST(CLQueryPlayerIDTest, refusesIDsOutsideOneToTwenty) {
    SocketEncryptOutputStream oStream(NULL);

    CLQueryPlayerID empty;
    empty.setPlayerID("");
    EXPECT_THROW(empty.write(oStream), InvalidProtocolException);

    CLQueryPlayerID tooLong;
    tooLong.setPlayerID(std::string(21, 'x'));
    EXPECT_THROW(tooLong.write(oStream), InvalidProtocolException);
}

// The login mode byte has no general setter — setWebLogin() names the
// only value a constructed instance can put on the wire on purpose, and
// the constructor leaves the member indeterminate otherwise, so the
// fixture always calls it.
void fill(CLReconnectLogin& p) {
    p.setKey(0x8A9BACBD);
    p.setWebLogin();
}
void expectEqual(const CLReconnectLogin& a, const CLReconnectLogin& b) {
    EXPECT_EQ(a.getKey(), b.getKey());
    EXPECT_EQ(a.isWebLogin(), b.isWebLogin());
}
LOGIN_PACKET_TESTS(CLReconnectLogin)

// Every setter here truncates to the field's cap, so an over-long value
// cannot reach write(); the reachable refusals are the empty and the
// too-short ones.
void fill(CLRegisterPlayer& p) {
    p.setID("goldid");
    p.setPassword("goldpw1234");
    p.setName("RegisterFixture");
    p.setSex(FEMALE);
    p.setSSN("7001012345678");
    p.setTelephone("02-1234-5678");
    p.setCellular("010-9876-5432");
    p.setZipCode("1234567");
    p.setAddress("12 Gold Street, Elcastle");
    p.setNation(JAPAN);
    p.setEmail("gold@example.com");
    p.setHomepage("http://example.com/gold");
    p.setProfile("pinned by the login packet suite");
    p.setPublic(true);
}
void expectEqual(const CLRegisterPlayer& a, const CLRegisterPlayer& b) {
    EXPECT_EQ(a.getID(), b.getID());
    EXPECT_EQ(a.getPassword(), b.getPassword());
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getSex(), b.getSex());
    EXPECT_EQ(a.getSSN(), b.getSSN());
    EXPECT_EQ(a.getTelephone(), b.getTelephone());
    EXPECT_EQ(a.getCellular(), b.getCellular());
    EXPECT_EQ(a.getZipCode(), b.getZipCode());
    EXPECT_EQ(a.getAddress(), b.getAddress());
    EXPECT_EQ(a.getNation(), b.getNation());
    EXPECT_EQ(a.getEmail(), b.getEmail());
    EXPECT_EQ(a.getHomepage(), b.getHomepage());
    EXPECT_EQ(a.getProfile(), b.getProfile());
    EXPECT_EQ(a.getPublic(), b.getPublic());
}
LOGIN_PACKET_TESTS(CLRegisterPlayer)

TEST(CLRegisterPlayerTest, refusesEmptyAndTooShortCredentials) {
    SocketEncryptOutputStream oStream(NULL);

    CLRegisterPlayer emptyID;
    fill(emptyID);
    emptyID.setID("");
    EXPECT_THROW(emptyID.write(oStream), InvalidProtocolException);

    CLRegisterPlayer shortID;
    fill(shortID);
    shortID.setID("abc");
    EXPECT_THROW(shortID.write(oStream), InvalidProtocolException);

    CLRegisterPlayer shortPassword;
    fill(shortPassword);
    shortPassword.setPassword("abcde");
    EXPECT_THROW(shortPassword.write(oStream), InvalidProtocolException);

    CLRegisterPlayer emptyProfile;
    fill(emptyProfile);
    emptyProfile.setProfile("");
    EXPECT_THROW(emptyProfile.write(oStream), InvalidProtocolException);
}

void fill(CLSelectPC& p) {
    p.setPCName("SelectMeNow");
    p.setPCType(PC_OUSTERS);
}
void expectEqual(const CLSelectPC& a, const CLSelectPC& b) {
    EXPECT_EQ(a.getPCName(), b.getPCName());
    EXPECT_EQ(a.getPCType(), b.getPCType());
}
LOGIN_PACKET_TESTS(CLSelectPC)

TEST(CLSelectPCTest, refusesNamesOutsideOneToTwentyAndUnknownPCTypes) {
    SocketEncryptOutputStream oStream(NULL);

    CLSelectPC empty;
    fill(empty);
    empty.setPCName("");
    EXPECT_THROW(empty.write(oStream), InvalidProtocolException);

    CLSelectPC tooLong;
    fill(tooLong);
    tooLong.setPCName(std::string(21, 'x'));
    EXPECT_THROW(tooLong.write(oStream), InvalidProtocolException);

    // PC_OUSTERS + 1 is still inside the two bits PCType's enumerators
    // need, so the member holds a representable value and write() gets
    // to compare it — a larger byte would have to arrive through read(),
    // which is where packet_field_bounds_test.cpp pins it.
    CLSelectPC unknownType;
    fill(unknownType);
    unknownType.setPCType((PCType)(PC_OUSTERS + 1));
    EXPECT_THROW(unknownType.write(oStream), InvalidProtocolException);
}

void fill(CLSelectServer& p) {
    p.setServerGroupID(0x9C);
}
void expectEqual(const CLSelectServer& a, const CLSelectServer& b) {
    EXPECT_EQ(a.getServerGroupID(), b.getServerGroupID());
}
LOGIN_PACKET_TESTS(CLSelectServer)

void fill(CLSelectWorld& p) {
    p.setWorldID(0x8D);
}
void expectEqual(const CLSelectWorld& a, const CLSelectWorld& b) {
    EXPECT_EQ(a.getWorldID(), b.getWorldID());
}
LOGIN_PACKET_TESTS(CLSelectWorld)

void fill(CLVersionCheck& p) {
    p.setVersion(0x8A9BACBD);
}
void expectEqual(const CLVersionCheck& a, const CLVersionCheck& b) {
    EXPECT_EQ(a.getVersion(), b.getVersion());
}
LOGIN_PACKET_TESTS(CLVersionCheck)

//////////////////////////////////////////////////////////////////////
// LC — login server to client
//////////////////////////////////////////////////////////////////////

void fill(LCCreatePCError& p) {
    p.setErrorID(0x8A);
}
void expectEqual(const LCCreatePCError& a, const LCCreatePCError& b) {
    EXPECT_EQ(a.getErrorID(), b.getErrorID());
}
LOGIN_PACKET_TESTS(LCCreatePCError)

void fill(LCDeletePCError& p) {
    p.setErrorID(0x8B);
}
void expectEqual(const LCDeletePCError& a, const LCDeletePCError& b) {
    EXPECT_EQ(a.getErrorID(), b.getErrorID());
}
LOGIN_PACKET_TESTS(LCDeletePCError)

void fill(LCLoginError& p) {
    p.setErrorID(0x8C);
}
void expectEqual(const LCLoginError& a, const LCLoginError& b) {
    EXPECT_EQ(a.getErrorID(), b.getErrorID());
}
LOGIN_PACKET_TESTS(LCLoginError)

// The two flags carry opposite values so a swapped pair of bools moves
// the golden; a bool only ever puts 0 or 1 on the wire, so neither can
// follow the high-byte rule.
void fill(LCLoginOK& p) {
    p.setAdult(true);
    p.setFamily(false);
    p.setStat(0x8D);
    p.setLastDays(0x8E9F);
}
void expectEqual(const LCLoginOK& a, const LCLoginOK& b) {
    EXPECT_EQ(a.isAdult(), b.isAdult());
    EXPECT_EQ(a.isFamily(), b.isFamily());
    EXPECT_EQ(a.getStat(), b.getStat());
    EXPECT_EQ(a.getLastDays(), b.getLastDays());
}
LOGIN_PACKET_TESTS(LCLoginOK)

void fill(LCQueryResultCharacterName& p) {
    p.setCharacterName("ResultCharName");
    p.setExist(true);
}
void expectEqual(const LCQueryResultCharacterName& a, const LCQueryResultCharacterName& b) {
    EXPECT_EQ(a.getCharacterName(), b.getCharacterName());
    EXPECT_EQ(a.isExist(), b.isExist());
}
LOGIN_PACKET_TESTS(LCQueryResultCharacterName)

TEST(LCQueryResultCharacterNameTest, refusesNamesOutsideOneToTwenty) {
    SocketEncryptOutputStream oStream(NULL);

    LCQueryResultCharacterName empty;
    empty.setCharacterName("");
    empty.setExist(true);
    EXPECT_THROW(empty.write(oStream), InvalidProtocolException);

    LCQueryResultCharacterName tooLong;
    tooLong.setCharacterName(std::string(21, 'x'));
    tooLong.setExist(true);
    EXPECT_THROW(tooLong.write(oStream), InvalidProtocolException);
}

// The false flag is the point: the answer to a name query is usually
// "no such player", and a copy that skips the byte when the flag is
// false round-trips against itself while shifting nothing but the
// client's answer.
void fill(LCQueryResultPlayerID& p) {
    p.setPlayerID("resultplayer");
    p.setExist(false);
}
void expectEqual(const LCQueryResultPlayerID& a, const LCQueryResultPlayerID& b) {
    EXPECT_EQ(a.getPlayerID(), b.getPlayerID());
    EXPECT_EQ(a.isExist(), b.isExist());
}
LOGIN_PACKET_TESTS(LCQueryResultPlayerID)

TEST(LCQueryResultPlayerIDTest, refusesIDsOutsideOneToTwenty) {
    SocketEncryptOutputStream oStream(NULL);

    LCQueryResultPlayerID empty;
    empty.setPlayerID("");
    empty.setExist(true);
    EXPECT_THROW(empty.write(oStream), InvalidProtocolException);

    LCQueryResultPlayerID tooLong;
    tooLong.setPlayerID(std::string(21, 'x'));
    tooLong.setExist(true);
    EXPECT_THROW(tooLong.write(oStream), InvalidProtocolException);
}

void fill(LCReconnect& p) {
    p.setGameServerIP("203.0.113.42");
    p.setGameServerPort(0x8A9BACBD);
    p.setKey(0xCEDFE0F1);
}
void expectEqual(const LCReconnect& a, const LCReconnect& b) {
    EXPECT_EQ(a.getGameServerIP(), b.getGameServerIP());
    EXPECT_EQ(a.getGameServerPort(), b.getGameServerPort());
    EXPECT_EQ(a.getKey(), b.getKey());
}
LOGIN_PACKET_TESTS(LCReconnect)

TEST(LCReconnectTest, refusesAddressesOutsideOneToFifteen) {
    SocketEncryptOutputStream oStream(NULL);

    LCReconnect empty;
    fill(empty);
    empty.setGameServerIP("");
    EXPECT_THROW(empty.write(oStream), InvalidProtocolException);

    LCReconnect tooLong;
    fill(tooLong);
    tooLong.setGameServerIP("255.255.255.2555");
    EXPECT_THROW(tooLong.write(oStream), InvalidProtocolException);
}

void fill(LCRegisterPlayerError& p) {
    p.setErrorID(0x8E);
}
void expectEqual(const LCRegisterPlayerError& a, const LCRegisterPlayerError& b) {
    EXPECT_EQ(a.getErrorID(), b.getErrorID());
}
LOGIN_PACKET_TESTS(LCRegisterPlayerError)

void fill(LCRegisterPlayerOK& p) {
    p.setGroupName("GoldGroup");
    p.setAdult(true);
}
void expectEqual(const LCRegisterPlayerOK& a, const LCRegisterPlayerOK& b) {
    EXPECT_EQ(a.getGroupName(), b.getGroupName());
    EXPECT_EQ(a.isAdult(), b.isAdult());
}
LOGIN_PACKET_TESTS(LCRegisterPlayerOK)

// FINDING, stated as a test that fails once it is fixed.
//
// LCRegisterPlayerOK carries the one string field in the login phase
// that neither side bounds: the setter takes any length, write()
// narrows it to a BYTE with no check, and read() takes whatever length
// byte arrives. Both ends of the range produce a packet the peer
// refuses — an empty name writes a zero prefix, and a 256-byte name
// wraps its prefix to zero — because the stream's own string read
// rejects a zero length. So the registration confirmation is
// undeliverable rather than misframed, and the 256-byte case also
// leaves the whole name unread behind the rejected packet.
//
// The fix is the cap every other string field in this file has, applied
// in write() and getPacketSize() together, plus write() refusing the
// empty name as its neighbours do; then both cases below stop
// round-tripping into a rejection and this test flips.
TEST(LCRegisterPlayerOKTest, groupNameLengthPrefixIsUnbounded) {
    LCRegisterPlayerOK empty;
    empty.setGroupName("");
    empty.setAdult(true);
    const std::vector<unsigned char> emptyBody = writeBody(empty, kPlainCode);
    ASSERT_EQ((size_t)empty.getPacketSize(), emptyBody.size());
    EXPECT_EQ(0u, emptyBody[0]) << "write() no longer emits a zero length prefix — retire this test";

    LCRegisterPlayerOK packet;
    packet.setGroupName(std::string(256, 'g'));
    packet.setAdult(true);
    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    ASSERT_EQ((size_t)packet.getPacketSize(), body.size()) << "the byte count still agrees; the framing does not";
    EXPECT_EQ(0u, body[0]) << "a 256-byte name no longer wraps its length prefix to zero — the cap has been "
                              "added, so retire this test and pin the truncated name instead";

    LCRegisterPlayerOK emptyDst;
    EXPECT_THROW(roundTrip(empty, emptyDst, kPlainCode), InvalidProtocolException);

    LCRegisterPlayerOK wrappedDst;
    EXPECT_THROW(roundTrip(packet, wrappedDst, kPlainCode), InvalidProtocolException);
}

void fill(LCSelectPCError& p) {
    p.setCode(0x8F);
}
void expectEqual(const LCSelectPCError& a, const LCSelectPCError& b) {
    EXPECT_EQ(a.getCode(), b.getCode());
}
LOGIN_PACKET_TESTS(LCSelectPCError)

//////////////////////////////////////////////////////////////////////
// The three list packets
//////////////////////////////////////////////////////////////////////

ServerGroupInfo* makeServerGroupInfo(ServerGroupID_t id, const std::string& name, BYTE stat) {
    ServerGroupInfo* pInfo = new ServerGroupInfo();
    pInfo->setGroupID(id);
    pInfo->setGroupName(name);
    pInfo->setStat(stat);
    return pInfo;
}

// Two entries of different name lengths, so a reader that assumed a
// fixed stride lands mid-record on the second one.
void fill(LCServerList& p) {
    p.setCurrentServerGroupID(0x9A);
    p.addListElement(makeServerGroupInfo(0x8A, "Server Alpha", 0x8B));
    p.addListElement(makeServerGroupInfo(0x8C, "Beta", 0x8D));
}
void expectEqual(LCServerList& a, LCServerList& b) {
    EXPECT_EQ(a.getCurrentServerGroupID(), b.getCurrentServerGroupID());
    ASSERT_EQ(a.getListNum(), b.getListNum());
    const BYTE count = a.getListNum();
    for (BYTE i = 0; i < count; i++) {
        ServerGroupInfo* pLeft = a.popFrontListElement();
        ServerGroupInfo* pRight = b.popFrontListElement();
        EXPECT_EQ(pLeft->getGroupID(), pRight->getGroupID());
        EXPECT_EQ(pLeft->getGroupName(), pRight->getGroupName());
        EXPECT_EQ(pLeft->getStat(), pRight->getStat());
        delete pLeft;
        delete pRight;
    }
}
LOGIN_PACKET_TESTS(LCServerList)

WorldInfo* makeWorldInfo(WorldID_t id, const std::string& name, BYTE stat) {
    WorldInfo* pInfo = new WorldInfo();
    pInfo->setID(id);
    pInfo->setName(name);
    pInfo->setStat(stat);
    return pInfo;
}

void fill(LCWorldList& p) {
    p.setCurrentWorldID(0x9B);
    p.addListElement(makeWorldInfo(0x8E, "World One", 0x8F));
    p.addListElement(makeWorldInfo(0x90, "Second World Name", 0x91));
}
void expectEqual(LCWorldList& a, LCWorldList& b) {
    EXPECT_EQ(a.getCurrentWorldID(), b.getCurrentWorldID());
    ASSERT_EQ(a.getListNum(), b.getListNum());
    const BYTE count = a.getListNum();
    for (BYTE i = 0; i < count; i++) {
        WorldInfo* pLeft = a.popFrontListElement();
        WorldInfo* pRight = b.popFrontListElement();
        EXPECT_EQ(pLeft->getID(), pRight->getID());
        EXPECT_EQ(pLeft->getName(), pRight->getName());
        EXPECT_EQ(pLeft->getStat(), pRight->getStat());
        delete pLeft;
        delete pRight;
    }
}
LOGIN_PACKET_TESTS(LCWorldList)

// FINDING, stated as a test that fails once it is fixed.
//
// Both list packets budget their factory max for exactly 37 entries of
// a 20-character name and nothing caps the list at fill time, so the
// 38th world or server group makes getPacketSize() exceed the max the
// receiver sizes its read buffer from. The entry count is the login
// server's configuration, not the client's, so nothing on the wire
// prevents it.
//
// The fix is a cap on the list — then the 38-entry packet stops
// exceeding the max and this test flips.
TEST(LCWorldListTest, listCapacityIsThirtySevenEntries) {
    LCWorldListFactory factory;
    const std::string maxName(20, 'w');

    LCWorldList exactFit;
    exactFit.setCurrentWorldID(1);
    for (int i = 0; i < 37; i++)
        exactFit.addListElement(makeWorldInfo((WorldID_t)(i + 1), maxName, 0x8F));
    EXPECT_EQ(factory.getPacketMaxSize(), exactFit.getPacketSize())
        << "37 full-width entries are exactly the factory max";

    LCWorldList oneTooMany;
    oneTooMany.setCurrentWorldID(1);
    for (int i = 0; i < 38; i++)
        oneTooMany.addListElement(makeWorldInfo((WorldID_t)(i + 1), maxName, 0x8F));
    EXPECT_GT(oneTooMany.getPacketSize(), factory.getPacketMaxSize())
        << "the 38th entry no longer outgrows the read buffer — the list is capped, so retire this test";
}

//////////////////////////////////////////////////////////////////////
// LCPCList — the character selection screen: one record per slot, of a
// different shape per race.
//////////////////////////////////////////////////////////////////////

PCSlayerInfo* makeSlayerInfo() {
    PCSlayerInfo* pInfo = new PCSlayerInfo();
    pInfo->setName("GoldSlayer");
    pInfo->setSlot(SLOT1);
    pInfo->setAlignment((Alignment_t)0x8A9BACBD);
    pInfo->setSTR(kSTR);
    pInfo->setDEX(kDEX);
    pInfo->setINT(kINT);
    pInfo->setRank(0x8B);
    pInfo->setSTRExp(0x8C9DAEBF);
    pInfo->setDEXExp(0x8D9EAFC0);
    pInfo->setINTExp(0x8E9FB0C1);
    pInfo->setHP(0x8A1B, 0x8B2C);
    pInfo->setMP(0x8C3D, 0x8D4E);
    pInfo->setFame(0x8FA0B1C2);
    for (int i = 0; i < SKILL_DOMAIN_VAMPIRE; i++)
        pInfo->setSkillDomainLevel((SkillDomain)i, (SkillLevel_t)(0x80 + i));
    pInfo->setSex(MALE);
    pInfo->setHairStyle(HAIR_STYLE3);
    pInfo->setHelmetType(HELMET3);
    pInfo->setJacketType(JACKET4);
    pInfo->setPantsType(PANTS4);
    pInfo->setWeaponType(WEAPON_MACE);
    pInfo->setShieldType(SHIELD2);
    pInfo->setHairColor(0x8A11);
    pInfo->setSkinColor(0x8B22);
    pInfo->setHelmetColor(0x8C33);
    pInfo->setJacketColor(0x8D44);
    pInfo->setPantsColor(0x8E55);
    pInfo->setWeaponColor(0x8F66);
    pInfo->setShieldColor(0x9077);
    pInfo->setAdvancementLevel(0x8D);
    return pInfo;
}

void expectEqual(const PCSlayerInfo& a, const PCSlayerInfo& b) {
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getSlot(), b.getSlot());
    EXPECT_EQ(a.getAlignment(), b.getAlignment());
    EXPECT_EQ(a.getSTR(), b.getSTR());
    EXPECT_EQ(a.getDEX(), b.getDEX());
    EXPECT_EQ(a.getINT(), b.getINT());
    EXPECT_EQ(a.getRank(), b.getRank());
    EXPECT_EQ(a.getSTRExp(), b.getSTRExp());
    EXPECT_EQ(a.getDEXExp(), b.getDEXExp());
    EXPECT_EQ(a.getINTExp(), b.getINTExp());
    EXPECT_EQ(a.getHP(ATTR_CURRENT), b.getHP(ATTR_CURRENT));
    EXPECT_EQ(a.getHP(ATTR_MAX), b.getHP(ATTR_MAX));
    EXPECT_EQ(a.getMP(ATTR_CURRENT), b.getMP(ATTR_CURRENT));
    EXPECT_EQ(a.getMP(ATTR_MAX), b.getMP(ATTR_MAX));
    EXPECT_EQ(a.getFame(), b.getFame());
    for (int i = 0; i < SKILL_DOMAIN_VAMPIRE; i++)
        EXPECT_EQ(a.getSkillDomainLevel((SkillDomain)i), b.getSkillDomainLevel((SkillDomain)i));
    EXPECT_EQ(a.getSex(), b.getSex());
    EXPECT_EQ(a.getHairStyle(), b.getHairStyle());
    EXPECT_EQ(a.getHelmetType(), b.getHelmetType());
    EXPECT_EQ(a.getJacketType(), b.getJacketType());
    EXPECT_EQ(a.getPantsType(), b.getPantsType());
    EXPECT_EQ(a.getWeaponType(), b.getWeaponType());
    EXPECT_EQ(a.getShieldType(), b.getShieldType());
    EXPECT_EQ(a.getHairColor(), b.getHairColor());
    EXPECT_EQ(a.getSkinColor(), b.getSkinColor());
    EXPECT_EQ(a.getHelmetColor(), b.getHelmetColor());
    EXPECT_EQ(a.getJacketColor(), b.getJacketColor());
    EXPECT_EQ(a.getPantsColor(), b.getPantsColor());
    EXPECT_EQ(a.getWeaponColor(), b.getWeaponColor());
    EXPECT_EQ(a.getShieldColor(), b.getShieldColor());
    EXPECT_EQ(a.getAdvancementLevel(), b.getAdvancementLevel());
}

PCVampireInfo* makeVampireInfo() {
    PCVampireInfo* pInfo = new PCVampireInfo();
    pInfo->setName("GoldVampireName");
    pInfo->setSlot(SLOT2);
    pInfo->setAlignment((Alignment_t)0x8F9EADBC);
    pInfo->setSex(FEMALE);
    pInfo->setBatColor(0x9188);
    pInfo->setSkinColor(0x9299);
    // The coat type is an ItemType_t (a WORD) that read()/write() carry
    // as a single byte, so anything above 255 is lost on the wire.
    pInfo->setCoatType(VAMPIRE_COAT4);
    pInfo->setCoatColor(0x93AA);
    pInfo->setSTR(kSTR);
    pInfo->setDEX(kDEX);
    pInfo->setINT(kINT);
    pInfo->setHP(0x94BB, 0x95CC);
    pInfo->setLevel(0x96);
    pInfo->setRank(0x97);
    pInfo->setExp(0x98A9BACB);
    pInfo->setFame(0x99AABBCC);
    pInfo->setBonus(0x9ADD);
    pInfo->setAdvancementLevel(0x9B);
    return pInfo;
}

void expectEqual(const PCVampireInfo& a, const PCVampireInfo& b) {
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getSlot(), b.getSlot());
    EXPECT_EQ(a.getAlignment(), b.getAlignment());
    EXPECT_EQ(a.getSex(), b.getSex());
    EXPECT_EQ(a.getBatColor(), b.getBatColor());
    EXPECT_EQ(a.getSkinColor(), b.getSkinColor());
    EXPECT_EQ(a.getCoatType(), b.getCoatType());
    EXPECT_EQ(a.getCoatColor(), b.getCoatColor());
    EXPECT_EQ(a.getSTR(), b.getSTR());
    EXPECT_EQ(a.getDEX(), b.getDEX());
    EXPECT_EQ(a.getINT(), b.getINT());
    EXPECT_EQ(a.getHP(ATTR_CURRENT), b.getHP(ATTR_CURRENT));
    EXPECT_EQ(a.getHP(ATTR_MAX), b.getHP(ATTR_MAX));
    EXPECT_EQ(a.getLevel(), b.getLevel());
    EXPECT_EQ(a.getRank(), b.getRank());
    EXPECT_EQ(a.getExp(), b.getExp());
    EXPECT_EQ(a.getFame(), b.getFame());
    EXPECT_EQ(a.getBonus(), b.getBonus());
    EXPECT_EQ(a.getAdvancementLevel(), b.getAdvancementLevel());
}

PCOustersInfo* makeOustersInfo() {
    PCOustersInfo* pInfo = new PCOustersInfo();
    pInfo->setName("GoldOusters");
    pInfo->setSlot(SLOT3);
    pInfo->setAlignment((Alignment_t)0x9C8DAEBF);
    pInfo->setSex(MALE);
    pInfo->setCoatColor(0x9D11);
    pInfo->setHairColor(0x9E22);
    pInfo->setArmColor(0x9F33);
    pInfo->setBootsColor(0xA044);
    // Coat and arm type share one byte: three bits and one bit.
    pInfo->setCoatType(OUSTERS_COAT4);
    pInfo->setArmType(OUSTERS_ARM_CHAKRAM);
    pInfo->setSTR(kSTR);
    pInfo->setDEX(kDEX);
    pInfo->setINT(kINT);
    pInfo->setHP(0xA155, 0xA266);
    pInfo->setMP(0xA377, 0xA488);
    pInfo->setLevel(0xA5);
    pInfo->setRank(0xA6);
    pInfo->setExp(0xA7B8C9DA);
    pInfo->setFame(0xA8B9CADB);
    pInfo->setBonus(0xA9EE);
    pInfo->setSkillBonus(0xAAFF);
    pInfo->setAdvancementLevel(0xAB);
    return pInfo;
}

void expectEqual(const PCOustersInfo& a, const PCOustersInfo& b) {
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getSlot(), b.getSlot());
    EXPECT_EQ(a.getAlignment(), b.getAlignment());
    EXPECT_EQ(a.getSex(), b.getSex());
    EXPECT_EQ(a.getCoatColor(), b.getCoatColor());
    EXPECT_EQ(a.getHairColor(), b.getHairColor());
    EXPECT_EQ(a.getArmColor(), b.getArmColor());
    EXPECT_EQ(a.getBootsColor(), b.getBootsColor());
    EXPECT_EQ(a.getCoatType(), b.getCoatType());
    EXPECT_EQ(a.getArmType(), b.getArmType());
    EXPECT_EQ(a.getSTR(), b.getSTR());
    EXPECT_EQ(a.getDEX(), b.getDEX());
    EXPECT_EQ(a.getINT(), b.getINT());
    EXPECT_EQ(a.getHP(ATTR_CURRENT), b.getHP(ATTR_CURRENT));
    EXPECT_EQ(a.getHP(ATTR_MAX), b.getHP(ATTR_MAX));
    EXPECT_EQ(a.getMP(ATTR_CURRENT), b.getMP(ATTR_CURRENT));
    EXPECT_EQ(a.getMP(ATTR_MAX), b.getMP(ATTR_MAX));
    EXPECT_EQ(a.getLevel(), b.getLevel());
    EXPECT_EQ(a.getRank(), b.getRank());
    EXPECT_EQ(a.getExp(), b.getExp());
    EXPECT_EQ(a.getFame(), b.getFame());
    EXPECT_EQ(a.getBonus(), b.getBonus());
    EXPECT_EQ(a.getSkillBonus(), b.getSkillBonus());
    EXPECT_EQ(a.getAdvancementLevel(), b.getAdvancementLevel());
}

// All three slots are filled, and with a different race each, so the
// type-tag prefix, the per-race record shapes and the slot each record
// claims are all exercised at once.
void fill(LCPCList& p) {
    p.setPCInfo(SLOT1, makeSlayerInfo());
    p.setPCInfo(SLOT2, makeVampireInfo());
    p.setPCInfo(SLOT3, makeOustersInfo());
}
void expectEqual(const LCPCList& a, const LCPCList& b) {
    for (uint slot = 0; slot < SLOT_MAX; slot++) {
        const PCInfo* pLeft = a.getPCInfo((Slot)slot);
        const PCInfo* pRight = b.getPCInfo((Slot)slot);
        ASSERT_EQ(pLeft->getPCType(), pRight->getPCType()) << "slot " << slot;
    }
    expectEqual(*dynamic_cast<const PCSlayerInfo*>(a.getPCInfo(SLOT1)),
                *dynamic_cast<const PCSlayerInfo*>(b.getPCInfo(SLOT1)));
    expectEqual(*dynamic_cast<const PCVampireInfo*>(a.getPCInfo(SLOT2)),
                *dynamic_cast<const PCVampireInfo*>(b.getPCInfo(SLOT2)));
    expectEqual(*dynamic_cast<const PCOustersInfo*>(a.getPCInfo(SLOT3)),
                *dynamic_cast<const PCOustersInfo*>(b.getPCInfo(SLOT3)));
}
LOGIN_PACKET_TESTS(LCPCList)

// The empty-slot tag is its own layout: the three type bytes are
// always written, and a slot with no character contributes nothing but
// its '0'.
TEST(LCPCListTest, emptySlotsStillOccupyTheTypePrefix) {
    LCPCList packet;
    packet.setPCInfo(SLOT2, makeVampireInfo());

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    ASSERT_GE(body.size(), (size_t)SLOT_MAX);
    EXPECT_EQ('0', body[SLOT1]);
    EXPECT_EQ('V', body[SLOT2]);
    EXPECT_EQ('0', body[SLOT3]);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    LCPCList dst;
    roundTrip(packet, dst, kPlainCode);
    EXPECT_THROW(dst.getPCInfo(SLOT1), NoSuchElementException);
    EXPECT_THROW(dst.getPCInfo(SLOT3), NoSuchElementException);
    expectEqual(*dynamic_cast<const PCVampireInfo*>(packet.getPCInfo(SLOT2)),
                *dynamic_cast<const PCVampireInfo*>(dst.getPCInfo(SLOT2)));
}

// A vampire or an ousters record refuses an empty name: the exception
// leaves write() and the packet is never sent.
TEST(LCPCListTest, vampireAndOustersRecordsRefuseAnEmptyName) {
    SocketEncryptOutputStream oStream(NULL);

    LCPCList vampire;
    PCVampireInfo* pVampire = makeVampireInfo();
    pVampire->setName("");
    vampire.setPCInfo(SLOT2, pVampire);
    EXPECT_THROW(vampire.write(oStream), InvalidProtocolException);

    LCPCList ousters;
    PCOustersInfo* pOusters = makeOustersInfo();
    pOusters->setName("");
    ousters.setPCInfo(SLOT3, pOusters);
    EXPECT_THROW(ousters.write(oStream), InvalidProtocolException);
}

// FINDING, stated as a test that fails once it is fixed.
//
// PCSlayerInfo::write() wraps its whole body in a try block that
// catches Throwable and prints it, so the empty-name refusal its two
// sibling records let escape is swallowed here: write() carries on and
// emits only the trailing advancement level. getPacketSize() still
// counts the full record, and writePacket() has already put that count
// on the wire, so the client reads a body that is dozens of bytes short
// and every packet after it is misframed.
//
// The fix is to let the exception leave write(), as PCVampireInfo and
// PCOustersInfo do; then no body is produced at all and this test
// flips.
TEST(LCPCListTest, emptySlayerNameUnderflowsTheBody) {
    LCPCList packet;
    PCSlayerInfo* pSlayer = makeSlayerInfo();
    pSlayer->setName("");
    packet.setPCInfo(SLOT1, pSlayer);

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_LT(body.size(), (size_t)packet.getPacketSize())
        << "PCSlayerInfo::write() no longer swallows the empty-name refusal — retire this test";
    EXPECT_EQ((size_t)(SLOT_MAX + szLevel), body.size()) << "only the type tags and the advancement level are emitted";
}

} // namespace
