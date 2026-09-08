//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_gameserver_handshake_test.cpp
// Description : Golden byte fixtures, loopback round trips and size
//               pins for every packet that crosses the gameserver's
//               client socket between the TCP connect and the moment
//               the player reaches GPS_NORMAL — on the fresh-login
//               path and on the reconnect / zone-transfer path.
//
//               The set is taken from the dispatcher's own gates
//               (PacketValidator.cpp, __GAME_SERVER__ branch) and from
//               the code that runs in that window. Eleven packets, each
//               with the reason it is here:
//
//               CGConnectSetKey  one of exactly two ids the
//                                GPS_BEGIN_SESSION gate admits; it
//                                installs the session's encrypt and
//                                hash keys before anything else.
//               CGConnect        the other; carries the login server's
//                                key, the PC name and the MAC address,
//                                and drives the whole character load.
//               CGReady          the id the GPS_WAITING_FOR_CG_READY
//                                gate is named for; its handler moves
//                                the player from the incoming manager
//                                to the zone pipeline and sets
//                                GPS_NORMAL.
//               CGSetSlayerHotKey   the two other ids that gate
//               CGSetVampireHotKey  whitelists, so both legally cross
//                                the socket before the player is in a
//                                zone.
//               CGVerifyTime     the client's clock beacon.
//                                GamePlayer::verifySpeed() arms its
//                                speed-hack window on the first one, so
//                                the first one a session sends decides
//                                whether that session is ever checked.
//               GCSystemAvailabilities  what SEND_SYSTEM_AVAILABILITIES
//                                sends, first on both paths — from
//                                CGConnectHandler after authentication
//                                and again from IncomingPlayerManager
//                                on a zone transfer.
//               GCUpdateInfo     the payload of the phase: the whole
//                                character, its inventory, gear, extra
//                                slots, effects, motorcycle, nickname,
//                                blood bible signs and zone. Sent on
//                                both paths, one golden per race.
//               GCPetInfo        sendPetInfo() immediately after
//                                GCUpdateInfo in CGConnectHandler, for
//                                every player, pet or no pet.
//               GCDisconnect     the two refusal paths in
//                                CGConnectHandler (no ConnectionInfo
//                                for the client IP; a key, name or
//                                expiry mismatch) — the last bytes a
//                                rejected session sees.
//               GCReconnectLogin built by LGIncomingConnectionOKHandler,
//                                held on the GamePlayer and written by
//                                GamePlayer::disconnect(): the handoff
//                                that sends a relogging client back to
//                                the login server.
//
//               GCSetPosition and GCReconnect are deliberately absent:
//               no server source constructs either, so neither can
//               cross this socket. GCSystemMessage and GCNoticeEvent
//               are in-game notices — nothing in this window builds one
//               (CGConnectHandler includes GCSystemMessage.h and never
//               uses it).
//
//               None of the eleven calls readEncrypt/writeEncrypt, so
//               goldens are recorded at encrypt code 0 only, as the
//               login phase is. The golden test of every packet also
//               asserts that its bytes do not vary with the code, so
//               adopting the encrypter fails loudly instead of silently
//               voiding five sixths of the pin.
//
//               Each packet gets three pins (HANDSHAKE_PACKET_TESTS):
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
//               every byte the width allows. Two groups cannot follow
//               that rule and say so at the point of use: enum-valued
//               bytes, whose domain is a handful of small values, and
//               the PC attributes, whose getters reject anything above
//               2000.
//
//               Five findings are stated below as tests that FAIL when
//               the underlying code is fixed, which is the signal to
//               retire them:
//               emptySlayerNameIsSwallowedAndUnderflowsTheBody,
//               oversizedSlayerGuildNameIsSwallowedAndUnderflowsTheBody,
//               namelessNPCInfoOverstatesTheUpdateInfoSize,
//               maxSizeUnderstatesAFullEffectList and
//               listNumIsIndependentOfTheListAndTruncatesTheReader.
//
//////////////////////////////////////////////////////////////////////

#include <cstring>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "BloodBibleSignInfo.h"
#include "CGConnect.h"
#include "CGConnectSetKey.h"
#include "CGReady.h"
#include "CGSetSlayerHotKey.h"
#include "CGSetVampireHotKey.h"
#include "CGVerifyTime.h"
#include "EffectInfo.h"
#include "Exception.h"
#include "GCDisconnect.h"
#include "GCPetInfo.h"
#include "GCReconnectLogin.h"
#include "GCSystemAvailabilities.h"
#include "GCUpdateInfo.h"
#include "NPCInfo.h"
#include "NicknameInfo.h"
#include "PCOustersInfo2.h"
#include "PCSlayerInfo2.h"
#include "PCVampireInfo2.h"
#include "PetInfo.h"
#include "TestStreams.h"

using wiretest::expectGolden;
using wiretest::kEncryptCodeCount;
using wiretest::kEncryptCodes;
using wiretest::Loopback;
using wiretest::roundTrip;
using wiretest::writeBody;

namespace {

// The unencrypted branch. Every packet in this file reads and writes its
// body with plain read()/write() calls, so this is the only code whose
// bytes differ from any other.
const uchar kPlainCode = 0;

// PC attributes are WORDs but their getters reject anything above
// maxSlayerAttr / maxVampireAttr / maxOustersAttr (2000), so the high
// byte of an attribute fixture cannot be >= 128. The low byte still is,
// and the nine values of a race stay distinct.
const Attr_t kAttrs[9] = {0x0781, 0x0792, 0x07A3, 0x0784, 0x0795, 0x07A6, 0x0787, 0x0798, 0x07A9};

// A BYTE length prefix followed by the string's raw bytes.
void appendString(std::vector<unsigned char>& image, const std::string& value) {
    image.push_back((unsigned char)value.size());
    for (size_t i = 0; i < value.size(); i++)
        image.push_back((unsigned char)value[i]);
}

// Push a raw byte image through a real loopback connection and let the
// packet read it, exactly as the gameserver reads a client's bytes.
void readImage(Packet& packet, const std::vector<unsigned char>& bytes) {
    Loopback loopback;
    loopback.setCodes(kPlainCode);
    loopback.out().write(reinterpret_cast<const char*>(&bytes[0]), (uint)bytes.size());
    loopback.pump((uint)bytes.size());
    packet.read(loopback.in());
}

//////////////////////////////////////////////////////////////////////
// The three pins every packet gets. fill() / expectEqual() are
// overloaded per packet, as in packet_login_test.cpp, so the same
// canonical instance feeds all three.
//////////////////////////////////////////////////////////////////////

#define HANDSHAKE_PACKET_TESTS(Name)                                                                 \
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
// Bodyless packets. A golden of zero bytes is not a formality: these
// ride the same framed header as everything else, so a field added to
// one of them desynchronises a client that still expects an empty body.
//////////////////////////////////////////////////////////////////////

#define EMPTY_HANDSHAKE_PACKET(Name)                 \
    void fill(Name&) {}                              \
    void expectEqual(const Name& a, const Name& b) { \
        EXPECT_EQ(0u, a.getPacketSize());            \
        EXPECT_EQ(0u, b.getPacketSize());            \
    }                                                \
    HANDSHAKE_PACKET_TESTS(Name)

EMPTY_HANDSHAKE_PACKET(CGReady)
EMPTY_HANDSHAKE_PACKET(CGVerifyTime)

//////////////////////////////////////////////////////////////////////
// CG — client to gameserver
//////////////////////////////////////////////////////////////////////

// CGConnect has no setter for its six MAC bytes and its constructor
// leaves them indeterminate, so a default-constructed instance would
// write six bytes of whatever the allocation happened to hold — no
// golden could be recorded from it. The canonical instance is built by
// reading a crafted image instead, which is also what the gameserver
// does with every CGConnect it ever sees.
const unsigned char kConnectMac[6] = {0x8A, 0x9B, 0xAC, 0xBD, 0xCE, 0xDF};
const char* const kConnectPCName = "GoldConnectPC";

void fill(CGConnect& p) {
    std::vector<unsigned char> image;
    // authentication key, little-endian DWORD
    image.push_back(0x84);
    image.push_back(0x95);
    image.push_back(0xA6);
    image.push_back(0xB7);
    // PC type is an enum byte with three enumerators, so it carries its
    // highest valid value rather than a high byte.
    image.push_back((unsigned char)PC_OUSTERS);
    appendString(image, kConnectPCName);
    for (size_t i = 0; i < 6; i++)
        image.push_back(kConnectMac[i]);
    readImage(p, image);
}
void expectEqual(const CGConnect& a, const CGConnect& b) {
    EXPECT_EQ(a.getKey(), b.getKey());
    EXPECT_EQ(a.getPCType(), b.getPCType());
    EXPECT_EQ(a.getPCName(), b.getPCName());
    EXPECT_EQ(0, memcmp(a.getMacAddress(), b.getMacAddress(), 6));
    EXPECT_EQ(0, memcmp(a.getMacAddress(), kConnectMac, 6));
}
HANDSHAKE_PACKET_TESTS(CGConnect)

TEST(CGConnectTest, refusesNamesOutsideOneToTwenty) {
    CGConnect packet;
    fill(packet);
    SocketEncryptOutputStream oStream(NULL);

    packet.setPCName("");
    EXPECT_THROW(packet.write(oStream), InvalidProtocolException);

    packet.setPCName(std::string(21, 'x'));
    EXPECT_THROW(packet.write(oStream), InvalidProtocolException);
}

void fill(CGConnectSetKey& p) {
    p.setEncryptKey(0x8B9C);
    p.setHashKey(0xADBE);
}
void expectEqual(const CGConnectSetKey& a, const CGConnectSetKey& b) {
    EXPECT_EQ(a.getEncryptKey(), b.getEncryptKey());
    EXPECT_EQ(a.getHashKey(), b.getHashKey());
}
HANDSHAKE_PACKET_TESTS(CGConnectSetKey)

void fill(CGSetSlayerHotKey& p) {
    p.setHotKey(0, 0x81C2);
    p.setHotKey(1, 0x83C4);
    p.setHotKey(2, 0x85C6);
    p.setHotKey(3, 0x87C8);
}
void expectEqual(const CGSetSlayerHotKey& a, const CGSetSlayerHotKey& b) {
    for (BYTE i = 0; i < 4; i++)
        EXPECT_EQ(a.getHotKey(i), b.getHotKey(i)) << "hot key " << (int)i;
}
HANDSHAKE_PACKET_TESTS(CGSetSlayerHotKey)

void fill(CGSetVampireHotKey& p) {
    for (BYTE i = 0; i < 8; i++)
        p.setHotKey(i, (SkillType_t)(0x91D0 + i * 0x0101));
}
void expectEqual(const CGSetVampireHotKey& a, const CGSetVampireHotKey& b) {
    for (BYTE i = 0; i < 8; i++)
        EXPECT_EQ(a.getHotKey(i), b.getHotKey(i)) << "hot key " << (int)i;
}
HANDSHAKE_PACKET_TESTS(CGSetVampireHotKey)

//////////////////////////////////////////////////////////////////////
// GC — gameserver to client
//////////////////////////////////////////////////////////////////////

void fill(GCDisconnect& p) {
    p.setMessage("gameserver refused the connect: session already expired");
}
void expectEqual(const GCDisconnect& a, const GCDisconnect& b) {
    EXPECT_EQ(a.getMessage(), b.getMessage());
}
HANDSHAKE_PACKET_TESTS(GCDisconnect)

// The message caps at 128 on both sides. CGConnectHandler builds the
// refusal from an exception's toString(), which is not bounded by
// anything, so the packet's own refusal is the only check there is.
TEST(GCDisconnectTest, refusesEmptyOrOversizedMessages) {
    SocketEncryptOutputStream oStream(NULL);

    GCDisconnect empty;
    EXPECT_THROW(empty.write(oStream), InvalidProtocolException);

    GCDisconnect tooLong;
    tooLong.setMessage(std::string(129, 'x'));
    EXPECT_THROW(tooLong.write(oStream), InvalidProtocolException);
}

void fill(GCReconnectLogin& p) {
    p.setLoginServerIP("218.145.190.201");
    p.setLoginServerPort(0x82939AA5);
    p.setKey(0xB6C7D8E9);
}
void expectEqual(const GCReconnectLogin& a, const GCReconnectLogin& b) {
    EXPECT_EQ(a.getLoginServerIP(), b.getLoginServerIP());
    EXPECT_EQ(a.getLoginServerPort(), b.getLoginServerPort());
    EXPECT_EQ(a.getKey(), b.getKey());
}
HANDSHAKE_PACKET_TESTS(GCReconnectLogin)

TEST(GCReconnectLoginTest, refusesEmptyOrOversizedServerIP) {
    SocketEncryptOutputStream oStream(NULL);

    GCReconnectLogin empty;
    fill(empty);
    empty.setLoginServerIP("");
    EXPECT_THROW(empty.write(oStream), InvalidProtocolException);

    GCReconnectLogin tooLong;
    fill(tooLong);
    tooLong.setLoginServerIP(std::string(16, '9'));
    EXPECT_THROW(tooLong.write(oStream), InvalidProtocolException);
}

void fill(GCSystemAvailabilities& p) {
    p.setFlag(0x8A9BACBD);
    p.setOpenDegree(0xCE);
    p.setSkillLimit(0xDF);
}
void expectEqual(const GCSystemAvailabilities& a, const GCSystemAvailabilities& b) {
    EXPECT_EQ(a.getFlag(), b.getFlag());
    EXPECT_EQ(a.getOpenDegree(), b.getOpenDegree());
    EXPECT_EQ(a.getSkillLimit(), b.getSkillLimit());
}
HANDSHAKE_PACKET_TESTS(GCSystemAvailabilities)

//////////////////////////////////////////////////////////////////////
// GCPetInfo. Two shapes: a player with a pet and a player without one,
// which is the branch every pet-less character takes on connect.
//
// GCPetInfo does not own the PetInfo it is handed, so the fixture holds
// it and outlives the packet.
//////////////////////////////////////////////////////////////////////

struct PetFixture {
    PetInfo info;
    GCPetInfo packet;
};

void fillPet(PetFixture& f) {
    // Pet type is an enum byte with six enumerators, so it carries a
    // valid enumerator rather than a high byte.
    f.info.setPetType(PET_PIXIE);
    f.info.setPetCreatureType(0x8394);
    f.info.setPetLevel(0x85);
    f.info.setPetExp(0x8697A8B9);
    f.info.setPetHP(0x8CAD);
    f.info.setPetAttr(0x8E);
    f.info.setPetAttrLevel(0x8F);
    f.info.setPetOption(0x90);
    f.info.setFoodType(0x91A2);
    f.info.setGamble(0x93);
    f.info.setCutHead(0x94);
    f.info.setAttack(0x95);
    f.info.setNickname("GoldPetNick");

    f.packet.setPetInfo(&f.info);
    f.packet.setObjectID(0x96A7B8C9);
    f.packet.setSummonInfo(0x9A);
}

// The packet's own summon flag is write-only: read() restores it into
// the PetInfo, never back into the packet, so a received GCPetInfo is
// compared through its PetInfo.
void expectPetEqual(GCPetInfo& a, GCPetInfo& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    ASSERT_TRUE(a.getPetInfo() != NULL);
    ASSERT_TRUE(b.getPetInfo() != NULL);
    PetInfo* x = a.getPetInfo();
    PetInfo* y = b.getPetInfo();
    EXPECT_EQ(x->getPetType(), y->getPetType());
    if (x->getPetType() == PET_NONE)
        return;
    EXPECT_EQ(x->getPetCreatureType(), y->getPetCreatureType());
    EXPECT_EQ(x->getPetLevel(), y->getPetLevel());
    EXPECT_EQ(x->getPetExp(), y->getPetExp());
    EXPECT_EQ(x->getPetHP(), y->getPetHP());
    EXPECT_EQ(x->getPetAttr(), y->getPetAttr());
    EXPECT_EQ(x->getPetAttrLevel(), y->getPetAttrLevel());
    EXPECT_EQ(x->getPetOption(), y->getPetOption());
    EXPECT_EQ(x->getFoodType(), y->getFoodType());
    EXPECT_EQ(x->canGamble(), y->canGamble());
    EXPECT_EQ(x->canCutHead(), y->canCutHead());
    EXPECT_EQ(x->canAttack(), y->canAttack());
    EXPECT_EQ(x->getNickname(), y->getNickname());
    EXPECT_EQ(a.isSummonInfo(), y->isSummonInfo());
    EXPECT_EQ(x->getItemObjectID(), y->getItemObjectID());
}

void expectPetPins(GCPetInfo& packet, const char* goldenName) {
    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    expectGolden(goldenName, kPlainCode, body);
    for (size_t i = 1; i < kEncryptCodeCount; i++)
        EXPECT_EQ(body, writeBody(packet, kEncryptCodes[i]))
            << goldenName << " now varies with the encrypt code — add per-code goldens";

    GCPetInfoFactory factory;
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size())
        << goldenName
        << ": getPacketSize() disagrees with the bytes write() emits; writePacket() puts the former on "
           "the wire, so the stream never resynchronises";
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize())
        << goldenName << ": the body outgrows the read buffer the receiver sizes from the factory max";
    EXPECT_EQ(factory.getPacketID(), packet.getPacketID());
    EXPECT_EQ(factory.getPacketName(), packet.getPacketName());
}

TEST(GCPetInfoTest, roundTripsThroughLoopback) {
    PetFixture src;
    fillPet(src);
    GCPetInfo dst;
    roundTrip(src.packet, dst, kPlainCode);
    expectPetEqual(src.packet, dst);
}

TEST(GCPetInfoTest, bodyBytesMatchGoldenAndSizeMatchesTheBytesWritten) {
    PetFixture f;
    fillPet(f);
    expectPetPins(f.packet, "GCPetInfo");
}

TEST(GCPetInfoTest, noPetBodyBytesMatchGoldenAndSizeMatchesTheBytesWritten) {
    GCPetInfo packet;
    packet.setObjectID(0x9BACBDCE);
    packet.setSummonInfo(0);
    expectPetPins(packet, "GCPetInfo.nopet");
}

TEST(GCPetInfoTest, noPetRoundTripsAsAPetNoneRecord) {
    GCPetInfo src;
    src.setObjectID(0x9BACBDCE);
    src.setSummonInfo(0);

    GCPetInfo dst;
    roundTrip(src, dst, kPlainCode);

    EXPECT_EQ(src.getObjectID(), dst.getObjectID());
    ASSERT_TRUE(src.getPetInfo() == NULL);
    ASSERT_TRUE(dst.getPetInfo() != NULL);
    EXPECT_EQ(PET_NONE, dst.getPetInfo()->getPetType());
}

//////////////////////////////////////////////////////////////////////
// GCUpdateInfo — the payload of the phase, one golden per race.
//
// GCUpdateInfo deletes the PC, inventory, gear, extra, effect and
// motorcycle records it is handed, so those are allocated with new. It
// does NOT delete the nickname, the blood bible signs or the NPC
// records, so those live in the fixture and outlive the packet: members
// are destroyed in reverse declaration order, so `packet` goes first.
//
// It also never initialises m_pBloodBibleSign, and read() writes
// through it without allocating one, so both ends of a round trip have
// to install one before they touch the packet.
//////////////////////////////////////////////////////////////////////

struct UpdateInfoFixture {
    BloodBibleSignInfo sign;
    NicknameInfo nickname;
    NPCInfo npcs[2];
    GCUpdateInfo packet;

    UpdateInfoFixture() {
        packet.setBloodBibleSignInfo(&sign);
    }
};

// InventorySlotInfo and RideMotorcycleSlotInfo are separate classes with
// the same shape: PCItemInfo plus a pair of inventory coordinates.
template <typename SlotInfo>
SlotInfo* makeItemSlot(ObjectID_t objectID, CoordInven_t x, CoordInven_t y, int subItems, int optionTypes) {
    SlotInfo* pSlot = new SlotInfo();
    pSlot->setObjectID(objectID);
    pSlot->setItemClass(0x81);
    pSlot->setItemType(0x8293);
    for (int i = 0; i < optionTypes; i++)
        pSlot->addOptionType((OptionType_t)(0x84 + i));
    pSlot->setDurability(0x8798A9BA);
    pSlot->setSilver(0x8BAC);
    pSlot->setGrade(0x8DBECFD0);
    pSlot->setEnchantLevel((EnchantLevel_t)0x91);
    pSlot->setItemNum(0x92);
    pSlot->setMainColor(0x93A4);
    for (int i = 0; i < subItems; i++) {
        SubItemInfo* pSub = new SubItemInfo();
        pSub->setObjectID((ObjectID_t)(0x95A6B7C8 + i));
        pSub->setItemClass((BYTE)(0x99 + i));
        pSub->setItemType((ItemType_t)(0x9AAB + i));
        pSub->setItemNum((ItemNum_t)(0x9C + i));
        pSub->setSlotID((SlotID_t)(0x9D + i));
        // addListElement is the only place PCItemInfo's own list count is
        // maintained, so the sub-item count is always derived here.
        pSlot->addListElement(pSub);
    }
    pSlot->setInvenX(x);
    pSlot->setInvenY(y);
    return pSlot;
}

InventorySlotInfo* makeInventorySlot(ObjectID_t objectID, CoordInven_t x, CoordInven_t y, int subItems,
                                     int optionTypes) {
    return makeItemSlot<InventorySlotInfo>(objectID, x, y, subItems, optionTypes);
}

RideMotorcycleSlotInfo* makeMotorcycleSlot(ObjectID_t objectID, CoordInven_t x, CoordInven_t y, int subItems,
                                           int optionTypes) {
    return makeItemSlot<RideMotorcycleSlotInfo>(objectID, x, y, subItems, optionTypes);
}

GearSlotInfo* makeGearSlot(ObjectID_t objectID, SlotID_t slotID) {
    GearSlotInfo* pSlot = new GearSlotInfo();
    pSlot->setObjectID(objectID);
    pSlot->setItemClass(0xA1);
    pSlot->setItemType(0xA2B3);
    pSlot->addOptionType(0xA4);
    pSlot->addOptionType(0xA5);
    pSlot->setDurability(0xA6B7C8D9);
    pSlot->setSilver(0xAABB);
    pSlot->setGrade(0xACBDCEDF);
    pSlot->setEnchantLevel((EnchantLevel_t)0xB0);
    pSlot->setItemNum(0xB1);
    pSlot->setMainColor(0xB2C3);
    pSlot->setSlotID(slotID);
    return pSlot;
}

ExtraSlotInfo* makeExtraSlot(ObjectID_t objectID) {
    ExtraSlotInfo* pSlot = new ExtraSlotInfo();
    pSlot->setObjectID(objectID);
    pSlot->setItemClass(0xC1);
    pSlot->setItemType(0xC2D3);
    pSlot->addOptionType(0xC4);
    pSlot->setDurability(0xC5D6E7F8);
    pSlot->setSilver(0xC9DA);
    pSlot->setGrade(0xCBDCEDFE);
    pSlot->setEnchantLevel((EnchantLevel_t)0xCF);
    pSlot->setItemNum(0xD0);
    pSlot->setMainColor(0xD1E2);
    return pSlot;
}

// The zone half every race's fixture shares, plus the parts whose shape
// is varied per race by the caller.
void fillZone(GCUpdateInfo& p, ZoneID_t zoneID, BYTE npcTypes, BYTE monsterTypes) {
    p.setZoneID(zoneID);
    // Zone coordinates are BYTEs.
    p.setZoneX(0x8C);
    p.setZoneY(0x9D);

    GameTime gameTime;
    gameTime.setYear(0x8ABC);
    gameTime.setMonth(0x8D);
    gameTime.setDay(0x9E);
    gameTime.setHour(0xAF);
    gameTime.setMinute(0xB0);
    gameTime.setSecond(0xC1);
    p.setGameTime(gameTime);

    // Weather is an enum byte with three enumerators, so it carries its
    // highest valid value rather than a high byte.
    p.setWeather(WEATHER_SNOWY);
    p.setWeatherLevel(0xD2);
    p.setDarkLevel(0xE3);
    p.setLightLevel(0xF4);

    p.setNPCCount(npcTypes);
    for (uint i = 0; i < npcTypes; i++)
        p.setNPCType(i, (NPCType_t)(0x81A2 + i * 0x0101));

    p.setMonsterCount(monsterTypes);
    for (uint i = 0; i < monsterTypes; i++)
        p.setMonsterType(i, (MonsterType_t)(0x93B4 + i * 0x0101));

    p.setServerStat(0x85);
    p.setPremiumZone();
    p.setPremiumPlay();
    p.setSMSCharge(0x86979AAB);
    p.setNonPK(0x8B);
    p.setGuildUnionID(0x8C9DAEBF);
    p.setGuildUnionUserType(GCUpdateInfo::UNION_NOTHING);
    p.setPowerPoint((int)0xC0D1E2F3);
}

void fillNPCInfo(NPCInfo& info, const std::string& name, NPCID_t id) {
    info.setName(name);
    info.setNPCID(id);
    info.setX(0x81A2);
    info.setY(0x93B4);
}

//////////////////////////////////////////////////////////////////////
// Slayer: a two-slot inventory (one slot with sub-items and several
// option types), gear, an extra slot, effects, a motorcycle, a custom
// nickname string and two named NPCs.
//////////////////////////////////////////////////////////////////////
void fillSlayer(UpdateInfoFixture& f) {
    PCSlayerInfo2* pInfo = new PCSlayerInfo2();
    pInfo->setObjectID(0x8A9BACBD);
    pInfo->setName("GoldSlayer");
    // Sex and hair style are enum bytes with two and three enumerators,
    // so they carry their highest valid value rather than a high byte.
    pInfo->setSex(MALE);
    pInfo->setHairStyle(HAIR_STYLE3);
    pInfo->setHairColor(0x81C2);
    pInfo->setSkinColor(0x83C4);
    pInfo->setMasterEffectColor(0x85);
    pInfo->setAlignment((Alignment_t)0x8E9FA0B1);
    pInfo->setSTR(kAttrs[0], ATTR_CURRENT);
    pInfo->setSTR(kAttrs[1], ATTR_MAX);
    pInfo->setSTR(kAttrs[2], ATTR_BASIC);
    pInfo->setDEX(kAttrs[3], ATTR_CURRENT);
    pInfo->setDEX(kAttrs[4], ATTR_MAX);
    pInfo->setDEX(kAttrs[5], ATTR_BASIC);
    pInfo->setINT(kAttrs[6], ATTR_CURRENT);
    pInfo->setINT(kAttrs[7], ATTR_MAX);
    pInfo->setINT(kAttrs[8], ATTR_BASIC);
    pInfo->setRank(0x86);
    pInfo->setRankExp(0x8798A9BA);
    pInfo->setSTRExp(0x8BACBDCE);
    pInfo->setDEXExp(0x8FA0B1C2);
    pInfo->setINTExp(0x93A4B5C6);
    pInfo->setHP(0x97D8, ATTR_CURRENT);
    pInfo->setHP(0x99DA, ATTR_MAX);
    pInfo->setMP(0x9BDC, ATTR_CURRENT);
    pInfo->setMP(0x9DDE, ATTR_MAX);
    pInfo->setFame(0x9FE0F1A2);
    pInfo->setGold(0xA3B4C5D6);
    for (uint i = 0; i < SKILL_DOMAIN_VAMPIRE; i++) {
        pInfo->setSkillDomainLevel((SkillDomain)i, (SkillLevel_t)(0xB0 + i));
        pInfo->setSkillDomainExp((SkillDomain)i, (SkillExp_t)(0xC0D1E2F3 + i));
    }
    pInfo->setSight(0xB8);
    for (BYTE i = 0; i < 4; i++)
        pInfo->setHotKey(i, (SkillType_t)(0xD1E2 + i * 0x0101));
    pInfo->setCompetence(0xB9);
    pInfo->setGuildID(0xBACB);
    pInfo->setGuildName("GoldSlayerGuild");
    pInfo->setGuildMemberRank(0xBC);
    pInfo->setUnionID(0xBDCEDFE0);
    pInfo->setAdvancementLevel(0xBE);
    pInfo->setAdvancementGoalExp(0xBFD0E1F2);
    pInfo->setAttrBonus(0xC1D2);
    f.packet.setPCInfo(pInfo);

    InventoryInfo* pInventory = new InventoryInfo();
    pInventory->addListElement(makeInventorySlot(0x81929394, 0x95, 0xA6, 2, 3));
    pInventory->addListElement(makeInventorySlot(0xA7B8C9DA, 0xEB, 0xFC, 0, 0));
    // addListElement does not maintain the count that goes on the wire.
    pInventory->setListNum(2);
    f.packet.setInventoryInfo(pInventory);

    GearInfo* pGear = new GearInfo();
    pGear->addListElement(makeGearSlot(0x8DBECFD0, 0xE1));
    pGear->setListNum(1);
    f.packet.setGearInfo(pGear);

    ExtraInfo* pExtra = new ExtraInfo();
    pExtra->addListElement(makeExtraSlot(0xD2E3F4A5));
    pExtra->setListNum(1);
    f.packet.setExtraInfo(pExtra);

    EffectInfo* pEffect = new EffectInfo();
    pEffect->addListElement(0x81A2, 0x93B4);
    pEffect->addListElement(0xA5C6, 0xB7D8);
    pEffect->addListElement(0xC9EA, 0xDBFC);
    f.packet.setEffectInfo(pEffect);

    RideMotorcycleInfo* pMotorcycle = new RideMotorcycleInfo();
    pMotorcycle->setObjectID(0x8394A5B6);
    pMotorcycle->setItemType(0x87C8);
    pMotorcycle->addOptionType(0x89);
    pMotorcycle->addOptionType(0x8A);
    pMotorcycle->addListElement(makeMotorcycleSlot(0x8BCCDDEE, 0x8F, 0x90, 1, 1));
    pMotorcycle->setListNum(1);
    f.packet.setRideMotorcycleInfo(pMotorcycle);

    fillZone(f.packet, 0x8AFB, 3, 2);

    // The nickname type is an enum byte with six enumerators; all three
    // getSize() branches are covered across the three races, this one
    // being the string branch.
    f.nickname.setNicknameID(0x81E2);
    f.nickname.setNicknameType(NicknameInfo::NICK_CUSTOM);
    f.nickname.setNickname("GoldNickname");
    f.nickname.setNicknameIndex(0x93F4);
    f.packet.setNicknameInfo(&f.nickname);

    fillNPCInfo(f.npcs[0], "GoldNPCOne", 0x85C6);
    fillNPCInfo(f.npcs[1], "GoldNPCTwo", 0x97D8);
    f.packet.addNPCInfo(&f.npcs[0]);
    f.packet.addNPCInfo(&f.npcs[1]);

    f.sign.setOpenNum(0x82A3C4E5);
    f.sign.getList().push_back(0x86A7);
    f.sign.getList().push_back(0x98B9);
    f.sign.getList().push_back(0xCADB);
}

//////////////////////////////////////////////////////////////////////
// Vampire: no motorcycle (the other branch of the hasMotorcycle byte),
// an index nickname, one NPC, no extra slots.
//////////////////////////////////////////////////////////////////////
void fillVampire(UpdateInfoFixture& f) {
    PCVampireInfo2* pInfo = new PCVampireInfo2();
    pInfo->setObjectID(0x8B9CADBE);
    pInfo->setName("GoldVampire");
    pInfo->setLevel(0x82);
    pInfo->setSex(FEMALE);
    pInfo->setBatColor(0x83D4);
    pInfo->setSkinColor(0x85D6);
    pInfo->setMasterEffectColor(0x87);
    pInfo->setAlignment((Alignment_t)0x8899AABB);
    pInfo->setSTR(kAttrs[0], ATTR_CURRENT);
    pInfo->setSTR(kAttrs[1], ATTR_MAX);
    pInfo->setSTR(kAttrs[2], ATTR_BASIC);
    pInfo->setDEX(kAttrs[3], ATTR_CURRENT);
    pInfo->setDEX(kAttrs[4], ATTR_MAX);
    pInfo->setDEX(kAttrs[5], ATTR_BASIC);
    pInfo->setINT(kAttrs[6], ATTR_CURRENT);
    pInfo->setINT(kAttrs[7], ATTR_MAX);
    pInfo->setINT(kAttrs[8], ATTR_BASIC);
    pInfo->setHP(0x8CDD, ATTR_CURRENT);
    pInfo->setHP(0x8EDF, ATTR_MAX);
    pInfo->setRank(0x90);
    pInfo->setRankExp(0x9192A3B4);
    pInfo->setExp(0x95A6B7C8);
    pInfo->setGold(0x99AABBCC);
    pInfo->setFame(0x9DAEBFD0);
    pInfo->setSight(0xA1);
    pInfo->setBonus(0xA2B3);
    for (BYTE i = 0; i < 8; i++)
        pInfo->setHotKey(i, (SkillType_t)(0xA4C5 + i * 0x0101));
    pInfo->setSilverDamage(0xB4D5);
    pInfo->setCompetence(0xB6);
    pInfo->setGuildID(0xB7C8);
    pInfo->setGuildName("GoldVampireGuild");
    pInfo->setGuildMemberRank(0xB9);
    pInfo->setUnionID(0xBACBDCED);
    pInfo->setAdvancementLevel(0xBB);
    pInfo->setAdvancementGoalExp(0xBCCDDEEF);
    f.packet.setPCInfo(pInfo);

    InventoryInfo* pInventory = new InventoryInfo();
    pInventory->addListElement(makeInventorySlot(0x82939495, 0x96, 0xA7, 1, 2));
    pInventory->setListNum(1);
    f.packet.setInventoryInfo(pInventory);

    GearInfo* pGear = new GearInfo();
    pGear->addListElement(makeGearSlot(0x8EBFD0E1, 0xE2));
    pGear->addListElement(makeGearSlot(0x9FC0D1E2, 0xE3));
    pGear->setListNum(2);
    f.packet.setGearInfo(pGear);

    // An empty extra list: the count byte alone, which is what a
    // character with no extra slot sends.
    ExtraInfo* pExtra = new ExtraInfo();
    f.packet.setExtraInfo(pExtra);

    EffectInfo* pEffect = new EffectInfo();
    pEffect->addListElement(0x82A3, 0x94B5);
    f.packet.setEffectInfo(pEffect);

    // No setRideMotorcycleInfo call: the hasMotorcycle byte is false and
    // no motorcycle record follows it.

    fillZone(f.packet, 0x8BFC, 1, 4);

    f.nickname.setNicknameID(0x82E3);
    f.nickname.setNicknameType(NicknameInfo::NICK_BUILT_IN);
    f.nickname.setNicknameIndex(0x94F5);
    f.packet.setNicknameInfo(&f.nickname);

    fillNPCInfo(f.npcs[0], "GoldNPCVampire", 0x86C7);
    f.packet.addNPCInfo(&f.npcs[0]);

    f.sign.setOpenNum(0x83A4C5E6);
    f.sign.getList().push_back(0x87A8);
}

//////////////////////////////////////////////////////////////////////
// Ousters: no motorcycle, no NPC records, an empty blood bible sign
// list, and the NICK_NONE nickname branch — every "nothing here" case
// the packet has.
//////////////////////////////////////////////////////////////////////
void fillOusters(UpdateInfoFixture& f) {
    PCOustersInfo2* pInfo = new PCOustersInfo2();
    pInfo->setObjectID(0x8C9DAEBF);
    pInfo->setName("GoldOusters");
    pInfo->setLevel(0x83);
    pInfo->setSex(MALE);
    pInfo->setHairColor(0x84D5);
    pInfo->setMasterEffectColor(0x86);
    pInfo->setAlignment((Alignment_t)0x8797A7B7);
    pInfo->setSTR(kAttrs[0], ATTR_CURRENT);
    pInfo->setSTR(kAttrs[1], ATTR_MAX);
    pInfo->setSTR(kAttrs[2], ATTR_BASIC);
    pInfo->setDEX(kAttrs[3], ATTR_CURRENT);
    pInfo->setDEX(kAttrs[4], ATTR_MAX);
    pInfo->setDEX(kAttrs[5], ATTR_BASIC);
    pInfo->setINT(kAttrs[6], ATTR_CURRENT);
    pInfo->setINT(kAttrs[7], ATTR_MAX);
    pInfo->setINT(kAttrs[8], ATTR_BASIC);
    pInfo->setHP(0x8DDE, ATTR_CURRENT);
    pInfo->setHP(0x8FE0, ATTR_MAX);
    pInfo->setMP(0x91E2, ATTR_CURRENT);
    pInfo->setMP(0x93E4, ATTR_MAX);
    pInfo->setRank(0x95);
    pInfo->setRankExp(0x9697A8B9);
    pInfo->setExp(0x9AABBCCD);
    pInfo->setGold(0x9EAFC0D1);
    pInfo->setFame(0xA2B3C4D5);
    pInfo->setSight(0xA6);
    pInfo->setBonus(0xA7B8);
    pInfo->setSkillBonus(0xA9BA);
    pInfo->setSilverDamage(0xABBC);
    pInfo->setCompetence(0xAD);
    pInfo->setGuildID(0xAEBF);
    pInfo->setGuildName("GoldOustersGuild");
    pInfo->setGuildMemberRank(0xC0);
    pInfo->setUnionID(0xC1D2E3F4);
    pInfo->setAdvancementLevel(0xC5);
    pInfo->setAdvancementGoalExp(0xC6D7E8F9);
    f.packet.setPCInfo(pInfo);

    // Empty inventory and gear: the count byte alone in both, the shape
    // a freshly created character connects with.
    f.packet.setInventoryInfo(new InventoryInfo());
    f.packet.setGearInfo(new GearInfo());

    ExtraInfo* pExtra = new ExtraInfo();
    pExtra->addListElement(makeExtraSlot(0xD3E4F5A6));
    pExtra->setListNum(1);
    f.packet.setExtraInfo(pExtra);

    // An empty effect list, the third "nothing here" case.
    f.packet.setEffectInfo(new EffectInfo());

    fillZone(f.packet, 0x8CFD, 2, 1);

    f.nickname.setNicknameID(0x83E4);
    f.nickname.setNicknameType(NicknameInfo::NICK_NONE);
    f.packet.setNicknameInfo(&f.nickname);

    f.sign.setOpenNum(0x84A5C6E7);
}

void expectItemEqual(PCItemInfo& a, PCItemInfo& b, const char* what) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID()) << what;
    EXPECT_EQ(a.getItemClass(), b.getItemClass()) << what;
    EXPECT_EQ(a.getItemType(), b.getItemType()) << what;
    EXPECT_EQ(a.getOptionType(), b.getOptionType()) << what;
    EXPECT_EQ(a.getDurability(), b.getDurability()) << what;
    EXPECT_EQ(a.getSilver(), b.getSilver()) << what;
    EXPECT_EQ(a.getGrade(), b.getGrade()) << what;
    EXPECT_EQ(a.getEnchantLevel(), b.getEnchantLevel()) << what;
    EXPECT_EQ(a.getItemNum(), b.getItemNum()) << what;
    EXPECT_EQ(a.getMainColor(), b.getMainColor()) << what;
    ASSERT_EQ(a.getListNum(), b.getListNum()) << what;
    for (BYTE i = 0; i < a.getListNum(); i++) {
        SubItemInfo* x = a.popFrontListElement();
        SubItemInfo* y = b.popFrontListElement();
        EXPECT_EQ(x->getObjectID(), y->getObjectID()) << what;
        EXPECT_EQ(x->getItemClass(), y->getItemClass()) << what;
        EXPECT_EQ(x->getItemType(), y->getItemType()) << what;
        EXPECT_EQ(x->getItemNum(), y->getItemNum()) << what;
        EXPECT_EQ(x->getSlotID(), y->getSlotID()) << what;
        delete x;
        delete y;
    }
}

void expectSlayerEqual(PCSlayerInfo2& a, PCSlayerInfo2& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getSex(), b.getSex());
    EXPECT_EQ(a.getHairStyle(), b.getHairStyle());
    EXPECT_EQ(a.getHairColor(), b.getHairColor());
    EXPECT_EQ(a.getSkinColor(), b.getSkinColor());
    EXPECT_EQ(a.getMasterEffectColor(), b.getMasterEffectColor());
    EXPECT_EQ(a.getAlignment(), b.getAlignment());
    EXPECT_EQ(a.getSTR(ATTR_CURRENT), b.getSTR(ATTR_CURRENT));
    EXPECT_EQ(a.getSTR(ATTR_MAX), b.getSTR(ATTR_MAX));
    EXPECT_EQ(a.getSTR(ATTR_BASIC), b.getSTR(ATTR_BASIC));
    EXPECT_EQ(a.getDEX(ATTR_CURRENT), b.getDEX(ATTR_CURRENT));
    EXPECT_EQ(a.getDEX(ATTR_MAX), b.getDEX(ATTR_MAX));
    EXPECT_EQ(a.getDEX(ATTR_BASIC), b.getDEX(ATTR_BASIC));
    EXPECT_EQ(a.getINT(ATTR_CURRENT), b.getINT(ATTR_CURRENT));
    EXPECT_EQ(a.getINT(ATTR_MAX), b.getINT(ATTR_MAX));
    EXPECT_EQ(a.getINT(ATTR_BASIC), b.getINT(ATTR_BASIC));
    EXPECT_EQ(a.getRank(), b.getRank());
    EXPECT_EQ(a.getRankExp(), b.getRankExp());
    EXPECT_EQ(a.getSTRExp(), b.getSTRExp());
    EXPECT_EQ(a.getDEXExp(), b.getDEXExp());
    EXPECT_EQ(a.getINTExp(), b.getINTExp());
    EXPECT_EQ(a.getHP(ATTR_CURRENT), b.getHP(ATTR_CURRENT));
    EXPECT_EQ(a.getHP(ATTR_MAX), b.getHP(ATTR_MAX));
    EXPECT_EQ(a.getMP(ATTR_CURRENT), b.getMP(ATTR_CURRENT));
    EXPECT_EQ(a.getMP(ATTR_MAX), b.getMP(ATTR_MAX));
    EXPECT_EQ(a.getFame(), b.getFame());
    EXPECT_EQ(a.getGold(), b.getGold());
    for (uint i = 0; i < SKILL_DOMAIN_VAMPIRE; i++) {
        EXPECT_EQ(a.getSkillDomainLevel((SkillDomain)i), b.getSkillDomainLevel((SkillDomain)i)) << "domain " << i;
        EXPECT_EQ(a.getSkillDomainExp((SkillDomain)i), b.getSkillDomainExp((SkillDomain)i)) << "domain " << i;
    }
    EXPECT_EQ(a.getSight(), b.getSight());
    for (BYTE i = 0; i < 4; i++)
        EXPECT_EQ(a.getHotKey(i), b.getHotKey(i)) << "hot key " << (int)i;
    EXPECT_EQ(a.getCompetence(), b.getCompetence());
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getGuildName(), b.getGuildName());
    EXPECT_EQ(a.getGuildMemberRank(), b.getGuildMemberRank());
    EXPECT_EQ(a.getUnionID(), b.getUnionID());
    EXPECT_EQ(a.getAdvancementLevel(), b.getAdvancementLevel());
    EXPECT_EQ(a.getAdvancementGoalExp(), b.getAdvancementGoalExp());
    EXPECT_EQ(a.getAttrBonus(), b.getAttrBonus());
}

void expectVampireEqual(PCVampireInfo2& a, PCVampireInfo2& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getLevel(), b.getLevel());
    EXPECT_EQ(a.getSex(), b.getSex());
    EXPECT_EQ(a.getBatColor(), b.getBatColor());
    EXPECT_EQ(a.getSkinColor(), b.getSkinColor());
    EXPECT_EQ(a.getMasterEffectColor(), b.getMasterEffectColor());
    EXPECT_EQ(a.getAlignment(), b.getAlignment());
    EXPECT_EQ(a.getSTR(ATTR_CURRENT), b.getSTR(ATTR_CURRENT));
    EXPECT_EQ(a.getSTR(ATTR_MAX), b.getSTR(ATTR_MAX));
    EXPECT_EQ(a.getSTR(ATTR_BASIC), b.getSTR(ATTR_BASIC));
    EXPECT_EQ(a.getDEX(ATTR_CURRENT), b.getDEX(ATTR_CURRENT));
    EXPECT_EQ(a.getDEX(ATTR_MAX), b.getDEX(ATTR_MAX));
    EXPECT_EQ(a.getDEX(ATTR_BASIC), b.getDEX(ATTR_BASIC));
    EXPECT_EQ(a.getINT(ATTR_CURRENT), b.getINT(ATTR_CURRENT));
    EXPECT_EQ(a.getINT(ATTR_MAX), b.getINT(ATTR_MAX));
    EXPECT_EQ(a.getINT(ATTR_BASIC), b.getINT(ATTR_BASIC));
    EXPECT_EQ(a.getHP(ATTR_CURRENT), b.getHP(ATTR_CURRENT));
    EXPECT_EQ(a.getHP(ATTR_MAX), b.getHP(ATTR_MAX));
    EXPECT_EQ(a.getRank(), b.getRank());
    EXPECT_EQ(a.getRankExp(), b.getRankExp());
    EXPECT_EQ(a.getExp(), b.getExp());
    EXPECT_EQ(a.getGold(), b.getGold());
    EXPECT_EQ(a.getFame(), b.getFame());
    EXPECT_EQ(a.getSight(), b.getSight());
    EXPECT_EQ(a.getBonus(), b.getBonus());
    for (BYTE i = 0; i < 8; i++)
        EXPECT_EQ(a.getHotKey(i), b.getHotKey(i)) << "hot key " << (int)i;
    EXPECT_EQ(a.getSilverDamage(), b.getSilverDamage());
    EXPECT_EQ(a.getCompetence(), b.getCompetence());
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getGuildName(), b.getGuildName());
    EXPECT_EQ(a.getGuildMemberRank(), b.getGuildMemberRank());
    EXPECT_EQ(a.getUnionID(), b.getUnionID());
    EXPECT_EQ(a.getAdvancementLevel(), b.getAdvancementLevel());
    EXPECT_EQ(a.getAdvancementGoalExp(), b.getAdvancementGoalExp());
}

void expectOustersEqual(PCOustersInfo2& a, PCOustersInfo2& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getLevel(), b.getLevel());
    EXPECT_EQ(a.getSex(), b.getSex());
    EXPECT_EQ(a.getHairColor(), b.getHairColor());
    EXPECT_EQ(a.getMasterEffectColor(), b.getMasterEffectColor());
    EXPECT_EQ(a.getAlignment(), b.getAlignment());
    EXPECT_EQ(a.getSTR(ATTR_CURRENT), b.getSTR(ATTR_CURRENT));
    EXPECT_EQ(a.getSTR(ATTR_MAX), b.getSTR(ATTR_MAX));
    EXPECT_EQ(a.getSTR(ATTR_BASIC), b.getSTR(ATTR_BASIC));
    EXPECT_EQ(a.getDEX(ATTR_CURRENT), b.getDEX(ATTR_CURRENT));
    EXPECT_EQ(a.getDEX(ATTR_MAX), b.getDEX(ATTR_MAX));
    EXPECT_EQ(a.getDEX(ATTR_BASIC), b.getDEX(ATTR_BASIC));
    EXPECT_EQ(a.getINT(ATTR_CURRENT), b.getINT(ATTR_CURRENT));
    EXPECT_EQ(a.getINT(ATTR_MAX), b.getINT(ATTR_MAX));
    EXPECT_EQ(a.getINT(ATTR_BASIC), b.getINT(ATTR_BASIC));
    EXPECT_EQ(a.getHP(ATTR_CURRENT), b.getHP(ATTR_CURRENT));
    EXPECT_EQ(a.getHP(ATTR_MAX), b.getHP(ATTR_MAX));
    EXPECT_EQ(a.getMP(ATTR_CURRENT), b.getMP(ATTR_CURRENT));
    EXPECT_EQ(a.getMP(ATTR_MAX), b.getMP(ATTR_MAX));
    EXPECT_EQ(a.getRank(), b.getRank());
    EXPECT_EQ(a.getRankExp(), b.getRankExp());
    EXPECT_EQ(a.getExp(), b.getExp());
    EXPECT_EQ(a.getGold(), b.getGold());
    EXPECT_EQ(a.getFame(), b.getFame());
    EXPECT_EQ(a.getSight(), b.getSight());
    EXPECT_EQ(a.getBonus(), b.getBonus());
    EXPECT_EQ(a.getSkillBonus(), b.getSkillBonus());
    EXPECT_EQ(a.getSilverDamage(), b.getSilverDamage());
    EXPECT_EQ(a.getCompetence(), b.getCompetence());
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getGuildName(), b.getGuildName());
    EXPECT_EQ(a.getGuildMemberRank(), b.getGuildMemberRank());
    EXPECT_EQ(a.getUnionID(), b.getUnionID());
    EXPECT_EQ(a.getAdvancementLevel(), b.getAdvancementLevel());
    EXPECT_EQ(a.getAdvancementGoalExp(), b.getAdvancementGoalExp());
}

void expectUpdateInfoEqual(GCUpdateInfo& a, GCUpdateInfo& b) {
    ASSERT_TRUE(a.getPCInfo() != NULL);
    ASSERT_TRUE(b.getPCInfo() != NULL);
    ASSERT_EQ(a.getPCInfo()->getPCType(), b.getPCInfo()->getPCType());
    switch (a.getPCInfo()->getPCType()) {
    case PC_SLAYER:
        expectSlayerEqual(*dynamic_cast<PCSlayerInfo2*>(a.getPCInfo()), *dynamic_cast<PCSlayerInfo2*>(b.getPCInfo()));
        break;
    case PC_VAMPIRE:
        expectVampireEqual(*dynamic_cast<PCVampireInfo2*>(a.getPCInfo()),
                           *dynamic_cast<PCVampireInfo2*>(b.getPCInfo()));
        break;
    case PC_OUSTERS:
        expectOustersEqual(*dynamic_cast<PCOustersInfo2*>(a.getPCInfo()),
                           *dynamic_cast<PCOustersInfo2*>(b.getPCInfo()));
        break;
    }

    ASSERT_EQ(a.getInventoryInfo()->getListNum(), b.getInventoryInfo()->getListNum());
    for (BYTE i = 0; i < a.getInventoryInfo()->getListNum(); i++) {
        InventorySlotInfo* x = a.getInventoryInfo()->popFrontListElement();
        InventorySlotInfo* y = b.getInventoryInfo()->popFrontListElement();
        expectItemEqual(*x, *y, "inventory slot");
        EXPECT_EQ(x->getInvenX(), y->getInvenX());
        EXPECT_EQ(x->getInvenY(), y->getInvenY());
        delete x;
        delete y;
    }

    ASSERT_EQ(a.getGearInfo()->getListNum(), b.getGearInfo()->getListNum());
    for (BYTE i = 0; i < a.getGearInfo()->getListNum(); i++) {
        GearSlotInfo* x = a.getGearInfo()->popFrontListElement();
        GearSlotInfo* y = b.getGearInfo()->popFrontListElement();
        expectItemEqual(*x, *y, "gear slot");
        EXPECT_EQ(x->getSlotID(), y->getSlotID());
        delete x;
        delete y;
    }

    ASSERT_EQ(a.getExtraInfo()->getListNum(), b.getExtraInfo()->getListNum());
    for (BYTE i = 0; i < a.getExtraInfo()->getListNum(); i++) {
        ExtraSlotInfo* x = a.getExtraInfo()->popFrontListElement();
        ExtraSlotInfo* y = b.getExtraInfo()->popFrontListElement();
        expectItemEqual(*x, *y, "extra slot");
        delete x;
        delete y;
    }

    ASSERT_EQ(a.getEffectInfo()->getListNum(), b.getEffectInfo()->getListNum());
    for (int i = 0; i < a.getEffectInfo()->getListNum() * 2; i++)
        EXPECT_EQ(a.getEffectInfo()->popFrontListElement(), b.getEffectInfo()->popFrontListElement())
            << "effect word " << i;

    ASSERT_EQ(a.hasMotorcycle(), b.hasMotorcycle());
    if (a.hasMotorcycle()) {
        RideMotorcycleInfo* x = a.getRideMotorcycleInfo();
        RideMotorcycleInfo* y = b.getRideMotorcycleInfo();
        EXPECT_EQ(x->getObjectID(), y->getObjectID());
        EXPECT_EQ(x->getItemType(), y->getItemType());
        EXPECT_EQ(x->getOptionType(), y->getOptionType());
        ASSERT_EQ(x->getListNum(), y->getListNum());
        for (BYTE i = 0; i < x->getListNum(); i++) {
            RideMotorcycleSlotInfo* xs = x->popFrontListElement();
            RideMotorcycleSlotInfo* ys = y->popFrontListElement();
            expectItemEqual(*xs, *ys, "motorcycle slot");
            EXPECT_EQ(xs->getInvenX(), ys->getInvenX());
            EXPECT_EQ(xs->getInvenY(), ys->getInvenY());
            delete xs;
            delete ys;
        }
    }

    EXPECT_EQ(a.getZoneID(), b.getZoneID());
    EXPECT_EQ(a.getZoneX(), b.getZoneX());
    EXPECT_EQ(a.getZoneY(), b.getZoneY());
    EXPECT_EQ(a.getGameTime().getYear(), b.getGameTime().getYear());
    EXPECT_EQ(a.getGameTime().getMonth(), b.getGameTime().getMonth());
    EXPECT_EQ(a.getGameTime().getDay(), b.getGameTime().getDay());
    EXPECT_EQ(a.getGameTime().getHour(), b.getGameTime().getHour());
    EXPECT_EQ(a.getGameTime().getMinute(), b.getGameTime().getMinute());
    EXPECT_EQ(a.getGameTime().getSecond(), b.getGameTime().getSecond());
    EXPECT_EQ(a.getWeather(), b.getWeather());
    EXPECT_EQ(a.getWeatherLevel(), b.getWeatherLevel());
    EXPECT_EQ(a.getDarkLevel(), b.getDarkLevel());
    EXPECT_EQ(a.getLightLevel(), b.getLightLevel());

    ASSERT_EQ(a.getNPCCount(), b.getNPCCount());
    for (uint i = 0; i < a.getNPCCount(); i++)
        EXPECT_EQ(a.getNPCType(i), b.getNPCType(i)) << "npc type " << i;

    ASSERT_EQ(a.getMonsterCount(), b.getMonsterCount());
    for (uint i = 0; i < a.getMonsterCount(); i++)
        EXPECT_EQ(a.getMonsterType(i), b.getMonsterType(i)) << "monster type " << i;

    for (;;) {
        NPCInfo* x = a.popNPCInfo();
        NPCInfo* y = b.popNPCInfo();
        if (x == NULL) {
            EXPECT_TRUE(y == NULL) << "the receiver got more NPC records than were sent";
            break;
        }
        ASSERT_TRUE(y != NULL) << "the receiver got fewer NPC records than were sent";
        EXPECT_EQ(x->getName(), y->getName());
        EXPECT_EQ(x->getNPCID(), y->getNPCID());
        EXPECT_EQ(x->getX(), y->getX());
        EXPECT_EQ(x->getY(), y->getY());
    }

    EXPECT_EQ(a.getServerStat(), b.getServerStat());
    EXPECT_EQ(a.isPremiumZone(), b.isPremiumZone());
    EXPECT_EQ(a.isPremiumPlay(), b.isPremiumPlay());
    EXPECT_EQ(a.getSMSCharge(), b.getSMSCharge());

    ASSERT_TRUE(a.getNicknameInfo() != NULL);
    ASSERT_TRUE(b.getNicknameInfo() != NULL);
    EXPECT_EQ(a.getNicknameInfo()->getNicknameID(), b.getNicknameInfo()->getNicknameID());
    EXPECT_EQ(a.getNicknameInfo()->getNicknameType(), b.getNicknameInfo()->getNicknameType());
    switch (a.getNicknameInfo()->getNicknameType()) {
    case NicknameInfo::NICK_BUILT_IN:
    case NicknameInfo::NICK_QUEST:
    case NicknameInfo::NICK_FORCED:
        EXPECT_EQ(a.getNicknameInfo()->getNicknameIndex(), b.getNicknameInfo()->getNicknameIndex());
        break;
    case NicknameInfo::NICK_CUSTOM_FORCED:
    case NicknameInfo::NICK_CUSTOM:
        EXPECT_EQ(a.getNicknameInfo()->getNickname(), b.getNicknameInfo()->getNickname());
        break;
    default:
        break;
    }

    EXPECT_EQ(a.isNonPK(), b.isNonPK());
    EXPECT_EQ(a.getGuildUnionID(), b.getGuildUnionID());
    EXPECT_EQ(a.getGuildUnionUserType(), b.getGuildUnionUserType());
    EXPECT_EQ(a.getBloodBibleSignInfo()->getOpenNum(), b.getBloodBibleSignInfo()->getOpenNum());
    EXPECT_EQ(a.getBloodBibleSignInfo()->getList(), b.getBloodBibleSignInfo()->getList());
    EXPECT_EQ(a.getPowerPoint(), b.getPowerPoint());
}

void expectUpdateInfoPins(GCUpdateInfo& packet, const char* goldenName) {
    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    expectGolden(goldenName, kPlainCode, body);
    for (size_t i = 1; i < kEncryptCodeCount; i++)
        EXPECT_EQ(body, writeBody(packet, kEncryptCodes[i]))
            << goldenName << " now varies with the encrypt code — add per-code goldens";

    GCUpdateInfoFactory factory;
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size())
        << goldenName
        << ": getPacketSize() disagrees with the bytes write() emits; writePacket() puts the former on "
           "the wire, so the stream never resynchronises";
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize())
        << goldenName << ": the body outgrows the read buffer the receiver sizes from the factory max";
    EXPECT_EQ(factory.getPacketID(), packet.getPacketID());
    EXPECT_EQ(factory.getPacketName(), packet.getPacketName());
}

#define UPDATE_INFO_TESTS(Race, GoldenName)                             \
    TEST(GCUpdateInfoTest, Race##RoundTripsThroughLoopback) {           \
        UpdateInfoFixture src;                                          \
        fill##Race(src);                                                \
        UpdateInfoFixture dst;                                          \
        roundTrip(src.packet, dst.packet, kPlainCode);                  \
        expectUpdateInfoEqual(src.packet, dst.packet);                  \
    }                                                                   \
    TEST(GCUpdateInfoTest, Race##BodyBytesMatchGoldenAndDeclaredSize) { \
        UpdateInfoFixture f;                                            \
        fill##Race(f);                                                  \
        expectUpdateInfoPins(f.packet, GoldenName);                     \
    }

UPDATE_INFO_TESTS(Slayer, "GCUpdateInfo.slayer")
UPDATE_INFO_TESTS(Vampire, "GCUpdateInfo.vampire")
UPDATE_INFO_TESTS(Ousters, "GCUpdateInfo.ousters")

//////////////////////////////////////////////////////////////////////
// Findings. Each of the tests below states a write/read disagreement
// that is real today; each FAILS once the underlying code is fixed,
// which is the signal to delete it.
//////////////////////////////////////////////////////////////////////

// FINDING, stated as a test that fails once it is fixed.
// PCSlayerInfo2::write() wraps its whole body in
// `catch (Throwable&) { cout << ... }`, so the refusal it raises on an
// empty name never leaves the function. The object id has already been
// written when the throw happens, so the PC record stops after four
// bytes while GCUpdateInfo::getPacketSize() — which writePacket() puts
// on the wire ahead of the body — still counts the whole record.
// PCVampireInfo2 and PCOustersInfo2 let the same refusal out.
TEST(GCUpdateInfoTest, emptySlayerNameIsSwallowedAndUnderflowsTheBody) {
    UpdateInfoFixture f;
    fillSlayer(f);
    dynamic_cast<PCSlayerInfo2*>(f.packet.getPCInfo())->setName("");

    const size_t written = writeBody(f.packet, kPlainCode).size();
    EXPECT_LT(written, (size_t)f.packet.getPacketSize())
        << "PCSlayerInfo2::write() no longer swallows its empty-name refusal — delete this test";
}

// FINDING, stated as a test that fails once it is fixed.
// The same swallow on the other refusal PCSlayerInfo2::write() raises:
// a guild name longer than 30. setGuildName() does not truncate, so a
// guild renamed past the cap silently shortens every GCUpdateInfo that
// carries one of its members.
TEST(GCUpdateInfoTest, oversizedSlayerGuildNameIsSwallowedAndUnderflowsTheBody) {
    UpdateInfoFixture f;
    fillSlayer(f);
    dynamic_cast<PCSlayerInfo2*>(f.packet.getPCInfo())->setGuildName(std::string(31, 'g'));

    const size_t written = writeBody(f.packet, kPlainCode).size();
    EXPECT_LT(written, (size_t)f.packet.getPacketSize())
        << "PCSlayerInfo2::write() no longer swallows its guild-name refusal — delete this test";
}

// FINDING, stated as a test that fails once it is fixed.
// NPCInfo::getSize() always adds the id and the two coordinates, but
// write() emits them only when the name is non-empty. One nameless NPC
// in a zone makes GCUpdateInfo declare six bytes it never sends.
TEST(GCUpdateInfoTest, namelessNPCInfoOverstatesTheUpdateInfoSize) {
    UpdateInfoFixture f;
    fillOusters(f);
    fillNPCInfo(f.npcs[0], "", 0x85C6);
    f.packet.addNPCInfo(&f.npcs[0]);

    const size_t written = writeBody(f.packet, kPlainCode).size();
    EXPECT_EQ(written + szNPCID + szZoneCoord * 2, (size_t)f.packet.getPacketSize())
        << "NPCInfo::getSize() no longer counts the fields a nameless record omits — delete this test";
}

// FINDING, stated as a test that fails once it is fixed.
// EffectInfo::getMaxSize() is a flat 255, but getSize() is
// szBYTE + 4 * ListNum and ListNum is a BYTE, so a character carrying
// the maximum 255 effects needs 1021 bytes. The shortfall is budgeted
// into GCUpdateInfoFactory::kMaxSize, which is what the receiver sizes
// its read buffer from.
TEST(EffectInfoTest, maxSizeUnderstatesAFullEffectList) {
    EffectInfo info;
    for (int i = 0; i < 255; i++)
        info.addListElement((EffectID_t)(0x81A2 + i), (WORD)(0x93B4 + i));

    EXPECT_EQ(255, (int)info.getListNum());
    EXPECT_GT(info.getSize(), EffectInfo::getMaxSize())
        << "EffectInfo::getMaxSize() now covers a full effect list — delete this test";
}

// FINDING, stated as a test that fails once it is fixed.
// InventoryInfo, GearInfo, ExtraInfo and RideMotorcycleInfo all carry a
// settable m_ListNum that addListElement() does not maintain. write()
// puts m_ListNum on the wire and then writes every element it holds, so
// a count that disagrees with the list leaves the reader parsing fewer
// records than the body carries — and getSize(), which counts the list
// rather than the field, still matches the byte count, so the packet
// size looks correct all the way to the receiver. PCItemInfo and
// EffectInfo derive their counts in addListElement() and do not have
// this shape.
TEST(InventoryInfoTest, listNumIsIndependentOfTheListAndTruncatesTheReader) {
    InventoryInfo src;
    src.addListElement(makeInventorySlot(0x81929394, 0x95, 0xA6, 1, 1));
    src.addListElement(makeInventorySlot(0xA7B8C9DA, 0xEB, 0xFC, 0, 0));
    src.setListNum(1);

    SocketEncryptOutputStream oStream(NULL);
    oStream.setEncryptCode(kPlainCode);
    src.write(oStream);
    const std::vector<unsigned char> body(oStream.getBuffer(), oStream.getBuffer() + oStream.length());
    EXPECT_EQ((size_t)src.getSize(), body.size()) << "the declared size counts the list, so it still matches";

    Loopback loopback;
    loopback.setCodes(kPlainCode);
    loopback.out().write(reinterpret_cast<const char*>(&body[0]), (uint)body.size());
    loopback.pump((uint)body.size());

    InventoryInfo dst;
    dst.read(loopback.in());
    EXPECT_EQ(1, (int)dst.getListNum())
        << "write() now derives the count from the list, so both slots arrive — delete this test";
}

} // namespace
