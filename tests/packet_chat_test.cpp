//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_chat_test.cpp
// Description : Golden byte fixtures, loopback round trips and size
//               pins for the packets a client and a game server
//               exchange to talk: zone chat, whispers, the world
//               channel, the friend channel, system notices, the
//               nickname a character wears, the guild union offers and
//               the SMS address book.
//
//               The set is taken from the code that sends them: the
//               handlers under src/server/gameserver/handler
//               (CGSayHandler, CGWhisperHandler, CGGlobalChatHandler,
//               CGRangerSayHandler, GCFriendChattingHandler,
//               GGServerChatHandler, CGSelectNicknameHandler,
//               CGModifyNicknameHandler, the seven union handlers,
//               CGAppointSubmasterHandler, CGAddSMSAddressHandler,
//               CGDeleteSMSAddressHandler, CGSMSAddressListHandler,
//               CGSMSSendHandler, CGRequestIPHandler) and the senders
//               outside handler/: GamePlayer.cpp, ZonePlayerManager.cpp,
//               PlayerCreature.cpp, NicknameBook.cpp,
//               SMSAddressBook.cpp, skill/SkillUtil.cpp and
//               GDRLairAbstractStates.cpp. Thirty-one packets, each
//               with the reason it is here:
//
//               GCSystemMessage  the game's own voice: the coloured,
//                                typed notice 138 server sources send.
//               GCSay            one player's zone chat, broadcast to
//                                everyone who can see them.
//               GCWhisper        a private line delivered, on this
//                                server or relayed from another, and
//               GCWhisperFailed  the same line when nobody answers to
//                                the name.
//               CGGlobalChat     the client's world-wide shout, and
//               GCGlobalChat     that shout delivered.
//               CGRangerSay      the zone chat a Dragon Eye ranger
//                                sends, which comes back as a
//                                GCSystemMessage.
//               GCFriendChatting the friend list's own channel - the
//                                one GC packet the game server also
//                                handles, so both halves are its own.
//               GCKickMessage    the countdown a session sees before
//                                it is closed.
//               GCShowMessageBox a bare message box.
//               GCRequestFailed  the refusal of a look-up-a-player
//                                request.
//
//               CGSelectNickname the player picks one of the nicknames
//                                the character has earned, and
//               CGModifyNickname writes a free one with an item.
//               GCNicknameVerify answers both.
//               GCNicknameList   the nicknames a character holds, sent
//                                once as it loads;
//               GCAddNickname    one being granted, and
//               GCModifyNickname one changing, broadcast to the zone.
//
//               CGRequestUnion   a guild master offers an alliance,
//               CGAcceptUnion    the other master takes it,
//               CGDenyUnion      or turns it down.
//               CGQuitUnion      one side leaves the alliance,
//               CGQuitUnionAccept    the other agrees, and
//               CGQuitUnionDeny  refuses.
//               CGRequestUnionInfo   the player asks what alliances
//                                the guild holds.
//               CGAppointSubmaster   the master names a submaster.
//
//               CGAddSMSAddress  the player puts a number in the
//                                address book,
//               CGDeleteSMSAddress   takes one out,
//               CGSMSAddressList asks for the book, and
//               CGSMSSend        sends a text to up to five of them.
//               GCSMSAddressList is the book, and
//               GCAddressListVerify  the answer to each of the other
//                                three.
//
//               Deliberately excluded:
//
//               No packet in this family reads or writes the
//               encrypter. The nineteen src/Core sources that call
//               readEncrypt or writeEncrypt are CGAddMouseToZone,
//               CGAddZoneToInventory, CGAddZoneToMouse, CGAttack,
//               CGDissectionCorpse, CGDropMoney, CGMove,
//               CGNPCAskAnswer, CGPickupMoney, CGSkillToInventory,
//               CGSkillToObject, CGSkillToSelf, CGSkillToTile,
//               CGUseItemFromGear, CGUseItemFromInventory,
//               CGUsePotionFromInventory, GCAddItemToZone,
//               GCMoveError and GCMoveOK, all pinned at encrypt codes
//               0..5 by tests/packet_encrypter_test.cpp. None of the
//               thirty-one derives from one of them: every one extends
//               Packet directly. So each golden is recorded at code 0
//               and its test also asserts the bytes do not vary with
//               the code - adopting the encrypter fails loudly instead
//               of silently voiding the pin.
//
//               CGSay and CGWhisper, the client halves of the two chat
//               packets above, already have code-0 goldens
//               (tests/golden/CGSay.code0.hex,
//               tests/golden/CGWhisper.code0.hex) from
//               tests/packet_roundtrip_test.cpp. The chat these
//               handlers relay to a channel is pinned with its channel:
//               CGGuildChat, GCGuildChat (packet_guild_test.cpp),
//               CGPartySay, GCPartySay (packet_party_test.cpp),
//               GGServerChat, GGGuildChat
//               (packet_interserver_test.cpp). The answers these
//               handlers share with other families are not repeated
//               either: GCGuildResponse (packet_guild_test.cpp),
//               GCUseOK, GCCreateItem (packet_inventory_test.cpp),
//               GCModifyInformation (packet_combat_test.cpp),
//               GCNoticeEvent, GCNotifyWin
//               (packet_quest_war_test.cpp), GCPetStashList
//               (packet_store_test.cpp), GCAddSlayer, GCAddVampire,
//               GCAddOusters, GCAddEffect, GCRemoveEffect
//               (packet_zone_scan_test.cpp), GSModifyGuildMember,
//               GGCommand (packet_interserver_test.cpp).
//
//               Four of the thirty-one had a single-aspect width pin
//               in tests/packet_roundtrip_test.cpp and no golden -
//               CGAddSMSAddress, CGModifyNickname, GCFriendChatting
//               and GCShowMessageBox - as GCSystemMessage did; all
//               five are pinned in full here, and the width tests stay
//               where they are.
//
//               The whole phone family is excluded because no server
//               registers a factory for it: CGDialUp, CGPhoneSay,
//               CGPhoneDisconnect, GCPhoneConnected,
//               GCPhoneConnectionFailed, GCPhoneDisconnected and
//               GCPhoneSay appear in no line of
//               tests/ratchet/factory_registrations.txt, so no
//               PacketFactoryManager list can build one off the wire.
//               The three CG handlers are still registered on the
//               gameserver's dispatch table, and CGDialUpHandler,
//               CGPhoneSayHandler and CGPhoneDisconnectHandler are the
//               only sources that construct the four GC answers, so
//               the whole exchange is unreachable. GCShowUnionInfo is
//               excluded for the same reason: its factory is in no
//               registration list either, though
//               CGRequestUnionInfoHandler sends one.
//
//               GCShowMessageBox has no sender at all - no source
//               outside src/Core names it. It is pinned anyway,
//               because its factory IS registered: a registered
//               factory is the wire contract the client's own copy has
//               to match, and a GC packet the client can only receive
//               needs no server sender to be part of it. GCAddNickname
//               is here on the same rule.
//
//               Every one of the thirty-one has a registered factory
//               in tests/ratchet/factory_registrations.txt.
//
//               Each packet gets three pins (CHAT_PACKET_TESTS):
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
//               Extra goldens cover the branches one fixture cannot:
//               the nickname written with no text (.noname on
//               CGModifyNickname); the three shapes NicknameInfo takes,
//               a bare id, an index and a string (.none and .index on
//               GCAddNickname, with all three side by side in
//               GCNicknameList's canonical listing); the counted lists
//               written empty (.empty on GCNicknameList, CGSMSSend and
//               GCSMSAddressList); and the address book entry written
//               with all three fields empty (.bare on CGAddSMSAddress).
//
//               Fixture values are distinct per field and >= 128 in
//               every byte the width allows. Three groups cannot follow
//               that rule and say so at the point of use: the chat
//               messages, character names, nicknames and phone numbers,
//               which are text; the system message type, the kick type,
//               the friend command, the nickname type, the quit method
//               and the four verify codes, which are enumerators; and
//               CGModifyNickname's item id, which the golden records as
//               a WORD-wide value.
//
//               The write/read disagreements this set found are fixed
//               and pinned as the behaviour the packets now produce.
//               These packets are client-facing, so write() and
//               getPacketSize() are the contract: every fix is a
//               refusal, a cap at the width the factory max budgets, a
//               size-accounting correction, a read-side correction or
//               an initialisation, and no golden moved.
//
//               - GCSystemMessage carries no race: the client's reader
//                 takes the message, the colour and the type byte and
//                 nothing else, so the getter and setter no sender
//                 could reach the wire through are gone.
//               - GCModifyNickname's record pointer starts empty,
//                 getPacketSize() and write() refuse on it, and read()
//                 allocates the record it fills. Every sender hands
//                 over a record it keeps - the character's
//                 NicknameBook's, or one on the stack - so the packet
//                 frees only what read() allocated.
//               - GCRequestFailed::setCode takes the BYTE the wire
//                 carries, its name stops at the ten the factory max
//                 budgets in the setter and travels through de::wire in
//                 write() and read(), and the zero-length refusal names
//                 the field.
//               - NicknameInfo's custom nickname is bounded at
//                 MAX_NICKNAME_SIZE in the setter, in write() and in
//                 read(), all three admitting the empty value a
//                 NICK_CUSTOM slot can hold; a type no record shape
//                 names is an InvalidProtocolException in each of
//                 getSize(), read() and write(), in place of the
//                 Assert() that wrote assertion_failed.log first.
//               - GCNicknameList's listing, GCSMSAddressList's and
//                 CGSMSSend's receiver list are each held to what their
//                 own maximum budgets - MAX_NICKNAME_NUM 255 records,
//                 MAX_ADDRESS_NUM 30 entries and MAX_RECEVIER_NUM 5
//                 numbers - in write() and in read(), so no count byte
//                 can wrap while write() emits every entry.
//                 SMSAddressBook stops the book at the thirty the
//                 listing carries.
//               - CGSMSSend's factory max budgets MAX_NUMBER_LENGTH for
//                 the caller number, which is what read() accepts, so a
//                 packet built at every read cap is exactly the max;
//                 its four strings travel through de::wire, so no
//                 length byte can wrap and the message stops at
//                 MAX_MESSAGE_LENGTH.
//               - CGModifyNickname::setItemObjectID takes the full
//                 ObjectID_t the member and the wire both carry.
//               - GCSystemMessage::read, GCKickMessage::read and
//                 GCKickMessage::setType test the raw byte against the
//                 enum's range before it reaches an enum that declares
//                 fewer values than a byte carries.
//               - ~GCNicknameList and ~GCSMSAddressList free what the
//                 packet owns, and both read()s replace the listing
//                 they hold instead of clearing it and dropping the
//                 records. A GCSMSAddressList entry is the packet's own
//                 either way: its one sender builds a fresh record per
//                 address.
//               - GCFriendChatting's name and message are admitted
//                 empty and stop at 32 and 512 on both sides, and
//                 CGAddSMSAddress admits all three of its empty fields
//                 on both sides. Both are also pinned in
//                 tests/packet_roundtrip_test.cpp.
//
//               Every one of the thirty-one initialises each member
//               write() emits, which the poisoned-storage pin at the
//               end of the file asserts. Nine of them refuse a body
//               with an empty required string, so those are built over
//               poisoned storage with only that string set - the
//               refusal comes after the scalar fields are already in
//               the buffer, so it proves nothing about them on its own.
//
//////////////////////////////////////////////////////////////////////

#include <cstring>
#include <list>
#include <new>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "CGAcceptUnion.h"
#include "CGAddSMSAddress.h"
#include "CGAppointSubmaster.h"
#include "CGDeleteSMSAddress.h"
#include "CGDenyUnion.h"
#include "CGGlobalChat.h"
#include "CGModifyNickname.h"
#include "CGQuitUnion.h"
#include "CGQuitUnionAccept.h"
#include "CGQuitUnionDeny.h"
#include "CGRangerSay.h"
#include "CGRequestUnion.h"
#include "CGRequestUnionInfo.h"
#include "CGSMSAddressList.h"
#include "CGSMSSend.h"
#include "CGSelectNickname.h"
#include "Exception.h"
#include "GCAddNickname.h"
#include "GCAddressListVerify.h"
#include "GCFriendChatting.h"
#include "GCGlobalChat.h"
#include "GCKickMessage.h"
#include "GCModifyNickname.h"
#include "GCNicknameList.h"
#include "GCNicknameVerify.h"
#include "GCRequestFailed.h"
#include "GCSMSAddressList.h"
#include "GCSay.h"
#include "GCShowMessageBox.h"
#include "GCSystemMessage.h"
#include "GCWhisper.h"
#include "GCWhisperFailed.h"
#include "NicknameInfo.h"
#include "TestStreams.h"

using wiretest::expectGolden;
using wiretest::kEncryptCodeCount;
using wiretest::kEncryptCodes;
using wiretest::Loopback;
using wiretest::roundTrip;
using wiretest::writeBody;

namespace {

// The unencrypted branch. No packet in this file rides the encrypter, so
// this is the only code whose bytes could differ from any other.
const uchar kPlainCode = 0;

// Emit a body field by field and hand it to a reader, so a refusal on
// the read side can be pinned without a sender that could produce those
// bytes.
template <typename Emit, typename Consume> void throughLoopback(Emit emit, Consume consume) {
    Loopback link;
    link.setCodes(kPlainCode);
    emit(link.out());
    const uint length = link.out().length();
    link.pump(length);
    consume(link.in());
}

//////////////////////////////////////////////////////////////////////
// The three pins every packet gets. fill() / expectEqual() are
// overloaded per packet, so the same canonical instance feeds all three.
// Both take a non-const reference: several getters are not const, and
// reaching a list's records goes through a non-const accessor.
//////////////////////////////////////////////////////////////////////

#define CHAT_PACKET_GOLDEN_AND_SIZE(Name)                                                            \
    TEST(Name##Test, bodyBytesMatchGolden) {                                                         \
        Name packet;                                                                                 \
        fill(packet);                                                                                \
        const std::vector<unsigned char> body = writeBody(packet, kPlainCode);                       \
        expectGolden(#Name, kPlainCode, body);                                                       \
        for (size_t i = 1; i < kEncryptCodeCount; i++)                                               \
            EXPECT_EQ(body, writeBody(packet, kEncryptCodes[i]))                                     \
                << #Name " now varies with the encrypt code - add per-code goldens";                 \
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

#define CHAT_PACKET_TESTS(Name)                   \
    TEST(Name##Test, roundTripsThroughLoopback) { \
        Name src;                                 \
        fill(src);                                \
        Name dst;                                 \
        roundTrip(src, dst, kPlainCode);          \
        expectEqual(src, dst);                    \
    }                                             \
    CHAT_PACKET_GOLDEN_AND_SIZE(Name)

// A second fixture for a packet whose write() has a branch the canonical
// one does not take.
#define CHAT_PACKET_VARIANT(Name, Variant, fillVariant)                        \
    TEST(Name##Test, Variant##BodyBytesMatchGolden) {                          \
        Name packet;                                                           \
        fillVariant(packet);                                                   \
        const std::vector<unsigned char> body = writeBody(packet, kPlainCode); \
        expectGolden(#Name "." #Variant, kPlainCode, body);                    \
        EXPECT_EQ((size_t)packet.getPacketSize(), body.size());                \
        Name##Factory factory;                                                 \
        EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize());         \
        Name dst;                                                              \
        roundTrip(packet, dst, kPlainCode);                                    \
        expectEqual(packet, dst);                                              \
    }

//////////////////////////////////////////////////////////////////////
// The game's own voice, and one player's talking.
//////////////////////////////////////////////////////////////////////

// The message is text, and the type is an enumerator: read() casts the
// wire byte straight to it, so a fixture may only use a real one.
void fill(GCSystemMessage& packet) {
    packet.setMessage("The holy land gate closes at dusk.");
    packet.setColor(0x81A2B3C4);
    packet.setType(SYSTEM_MESSAGE_HOLY_LAND);
}

void expectEqual(GCSystemMessage& a, GCSystemMessage& b) {
    EXPECT_EQ(a.getMessage(), b.getMessage());
    EXPECT_EQ(a.getColor(), b.getColor());
    EXPECT_EQ((int)a.getType(), (int)b.getType());
}

CHAT_PACKET_TESTS(GCSystemMessage)

void fill(GCSay& packet) {
    packet.setObjectID(0x85A6B7C8);
    packet.setColor(0x89AABBCC);
    packet.setMessage("Anyone selling a bat wing?");
}

void expectEqual(GCSay& a, GCSay& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getColor(), b.getColor());
    EXPECT_EQ(a.getMessage(), b.getMessage());
}

CHAT_PACKET_TESTS(GCSay)

// The name is text and stops at the ten the factory max budgets.
void fill(GCWhisper& packet) {
    packet.setName("Duskwarden");
    packet.setColor(0x8DAEBFC0);
    packet.setMessage("Meet me by the north gate.");
    packet.setRace(0x91);
}

void expectEqual(GCWhisper& a, GCWhisper& b) {
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getColor(), b.getColor());
    EXPECT_EQ(a.getMessage(), b.getMessage());
    EXPECT_EQ((int)a.getRace(), (int)b.getRace());
}

CHAT_PACKET_TESTS(GCWhisper)

// An empty body: the refusal carries no reason.
void fill(GCWhisperFailed&) {}

void expectEqual(GCWhisperFailed&, GCWhisperFailed&) {}

CHAT_PACKET_TESTS(GCWhisperFailed)

void fill(CGGlobalChat& packet) {
    packet.setColor(0x95A6B7C8);
    packet.setMessage("Selling a full set of ashen plate.");
}

void expectEqual(CGGlobalChat& a, CGGlobalChat& b) {
    EXPECT_EQ(a.getColor(), b.getColor());
    EXPECT_EQ(a.getMessage(), b.getMessage());
}

CHAT_PACKET_TESTS(CGGlobalChat)

void fill(GCGlobalChat& packet) {
    packet.setColor(0x99AABBCC);
    packet.setMessage("Trading a dragon eye for a bat wing.");
    packet.setRace(0x9D);
}

void expectEqual(GCGlobalChat& a, GCGlobalChat& b) {
    EXPECT_EQ(a.getColor(), b.getColor());
    EXPECT_EQ(a.getMessage(), b.getMessage());
    EXPECT_EQ((int)a.getRace(), (int)b.getRace());
}

CHAT_PACKET_TESTS(GCGlobalChat)

void fill(CGRangerSay& packet) {
    packet.setMessage("Ranger channel: the lair is open.");
}

void expectEqual(CGRangerSay& a, CGRangerSay& b) {
    EXPECT_EQ(a.getMessage(), b.getMessage());
}

CHAT_PACKET_TESTS(CGRangerSay)

// The command is an enumerator; the name and the message are text and
// stop at the 32 and 128 read() takes. The two flags are plain wire
// bytes, so they follow the >= 128 rule even though a sender writes 0
// or 1.
void fill(GCFriendChatting& packet) {
    packet.setCommand(GC_MESSAGE);
    packet.setPlayerName("Duskwarden");
    packet.setMessage("Are you online tonight?");
    packet.setIsBlack(0xA1);
    packet.setIsOnLine(0xA2);
}

void expectEqual(GCFriendChatting& a, GCFriendChatting& b) {
    EXPECT_EQ(a.getCommand(), b.getCommand());
    EXPECT_EQ(a.getPlayerName(), b.getPlayerName());
    EXPECT_EQ(a.getMessage(), b.getMessage());
    EXPECT_EQ((int)a.getIsBlack(), (int)b.getIsBlack());
    EXPECT_EQ((int)a.getIsOnLine(), (int)b.getIsOnLine());
}

CHAT_PACKET_TESTS(GCFriendChatting)

// The type is an enumerator on both sides: the setter casts to
// KickMessageType and read() casts the wire byte to it too.
void fill(GCKickMessage& packet) {
    packet.setType(KICK_MESSAGE_EXPIRE_FREEPLAY);
    packet.setSeconds(0xA3B4C5D6);
}

void expectEqual(GCKickMessage& a, GCKickMessage& b) {
    EXPECT_EQ((int)a.getType(), (int)b.getType());
    EXPECT_EQ(a.getSeconds(), b.getSeconds());
}

CHAT_PACKET_TESTS(GCKickMessage)

void fill(GCShowMessageBox& packet) {
    packet.setMessage("Your guild registration is pending.");
}

void expectEqual(GCShowMessageBox& a, GCShowMessageBox& b) {
    EXPECT_EQ(a.getMessage(), b.getMessage());
}

CHAT_PACKET_TESTS(GCShowMessageBox)

// The code is an enumerator and the name is text, held here to the ten
// the factory max budgets.
void fill(GCRequestFailed& packet) {
    packet.setCode(REQUEST_FAILED_IP);
    packet.setName("Duskwarden");
}

void expectEqual(GCRequestFailed& a, GCRequestFailed& b) {
    EXPECT_EQ((int)a.getCode(), (int)b.getCode());
    EXPECT_EQ(a.getName(), b.getName());
}

CHAT_PACKET_TESTS(GCRequestFailed)

//////////////////////////////////////////////////////////////////////
// The nickname a character wears.
//////////////////////////////////////////////////////////////////////

void fill(CGSelectNickname& packet) {
    packet.setNicknameID(0xA5B6);
}

void expectEqual(CGSelectNickname& a, CGSelectNickname& b) {
    EXPECT_EQ((int)a.getNicknameID(), (int)b.getNicknameID());
}

CHAT_PACKET_TESTS(CGSelectNickname)

// An item id a WORD cannot hold. Returned rather than written at the
// call site, so what the setter does with it is the packet's and not a
// constant the compiler folds.
ObjectID_t fullItemObjectID() {
    return 0x81A2B3C4;
}

// The golden records the item id as a WORD-wide value, so the fixture
// cannot follow the >= 128 rule across the whole ObjectID_t.
void fill(CGModifyNickname& packet) {
    packet.setItemObjectID(0xA9BA);
    packet.setNickname("Ashen Duelist");
}

// write() admits the empty nickname read() takes back as an empty one.
void fillNoNickname(CGModifyNickname& packet) {
    packet.setItemObjectID(0xADBE);
    packet.setNickname("");
}

void expectEqual(CGModifyNickname& a, CGModifyNickname& b) {
    EXPECT_EQ(a.getItemObjectID(), b.getItemObjectID());
    EXPECT_EQ(a.getNickname(), b.getNickname());
}

CHAT_PACKET_TESTS(CGModifyNickname)
CHAT_PACKET_VARIANT(CGModifyNickname, noname, fillNoNickname)

// The nickname type picks one of three record shapes, so it is an
// enumerator; the nickname itself is text.
void fillNicknameInfo(NicknameInfo& info, WORD id, BYTE type, WORD index, const std::string& text) {
    info.setNicknameID(id);
    info.setNicknameType(type);
    info.setNicknameIndex(index);
    info.setNickname(text);
}

void expectEqualNicknameInfo(NicknameInfo& a, NicknameInfo& b) {
    EXPECT_EQ((int)a.getNicknameID(), (int)b.getNicknameID());
    EXPECT_EQ((int)a.getNicknameType(), (int)b.getNicknameType());
    if (a.getNicknameType() == NicknameInfo::NICK_BUILT_IN || a.getNicknameType() == NicknameInfo::NICK_QUEST ||
        a.getNicknameType() == NicknameInfo::NICK_FORCED)
        EXPECT_EQ((int)a.getNicknameIndex(), (int)b.getNicknameIndex());
    if (a.getNicknameType() == NicknameInfo::NICK_CUSTOM || a.getNicknameType() == NicknameInfo::NICK_CUSTOM_FORCED)
        EXPECT_EQ(a.getNickname(), b.getNickname());
}

void fill(GCAddNickname& packet) {
    fillNicknameInfo(packet.getNicknameInfo(), 0xB1C2, NicknameInfo::NICK_CUSTOM, 0, "Gloom Sapper");
}

// The record shape that carries neither an index nor a string.
void fillBareNickname(GCAddNickname& packet) {
    fillNicknameInfo(packet.getNicknameInfo(), 0xB3C4, NicknameInfo::NICK_NONE, 0, "");
}

// The record shape that carries an index instead of a string.
void fillIndexedNickname(GCAddNickname& packet) {
    fillNicknameInfo(packet.getNicknameInfo(), 0xB5C6, NicknameInfo::NICK_QUEST, 0xB7C8, "");
}

void expectEqual(GCAddNickname& a, GCAddNickname& b) {
    expectEqualNicknameInfo(a.getNicknameInfo(), b.getNicknameInfo());
}

CHAT_PACKET_TESTS(GCAddNickname)
CHAT_PACKET_VARIANT(GCAddNickname, none, fillBareNickname)
CHAT_PACKET_VARIANT(GCAddNickname, index, fillIndexedNickname)

// The record is a function-local static because the packet neither owns
// nor frees the one a sender hands it.
void fill(GCModifyNickname& packet) {
    static NicknameInfo* pInfo = new NicknameInfo;
    fillNicknameInfo(*pInfo, 0xBDCE, NicknameInfo::NICK_CUSTOM_FORCED, 0, "Ember Warden");
    packet.setObjectID(0xB9CADBEC);
    packet.setNicknameInfo(pInfo);
}

void expectEqual(GCModifyNickname& a, GCModifyNickname& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    ASSERT_TRUE(b.getNicknameInfo() != NULL);
    expectEqualNicknameInfo(*a.getNicknameInfo(), *b.getNicknameInfo());
}

CHAT_PACKET_TESTS(GCModifyNickname)

// The record a sender hands over outlives the packet; the one read()
// allocates does not, and a second read replaces it.
TEST(GCModifyNicknameTest, thePacketFreesOnlyTheRecordItRead) {
    NicknameInfo sender;
    fillNicknameInfo(sender, 0xC1D2, NicknameInfo::NICK_CUSTOM, 0, "Ashen Herald");

    {
        GCModifyNickname src;
        src.setObjectID(0x81A2B3C4);
        src.setNicknameInfo(&sender);

        GCModifyNickname dst;
        roundTrip(src, dst, kPlainCode);
        ASSERT_TRUE(dst.getNicknameInfo() != NULL);
        EXPECT_NE(&sender, dst.getNicknameInfo());
        EXPECT_EQ(sender.getNickname(), dst.getNicknameInfo()->getNickname());

        roundTrip(src, dst, kPlainCode);
        ASSERT_TRUE(dst.getNicknameInfo() != NULL);
        EXPECT_EQ(sender.getNickname(), dst.getNicknameInfo()->getNickname());
    }

    EXPECT_EQ("Ashen Herald", sender.getNickname());
}

NicknameInfo* makeNickname(WORD id, BYTE type, WORD index, const std::string& text) {
    NicknameInfo* pInfo = new NicknameInfo;
    fillNicknameInfo(*pInfo, id, type, index, text);
    return pInfo;
}

// All three record shapes side by side, so one listing covers every
// branch of NicknameInfo::write.
void fill(GCNicknameList& packet) {
    packet.getNicknames().push_back(makeNickname(0xC1D2, NicknameInfo::NICK_NONE, 0, ""));
    packet.getNicknames().push_back(makeNickname(0xC3D4, NicknameInfo::NICK_BUILT_IN, 0xC5D6, ""));
    packet.getNicknames().push_back(makeNickname(0xC7D8, NicknameInfo::NICK_CUSTOM, 0, "Ashen Herald"));
}

void fillNoNicknames(GCNicknameList&) {}

void expectEqual(GCNicknameList& a, GCNicknameList& b) {
    ASSERT_EQ(a.getNicknames().size(), b.getNicknames().size());
    for (size_t i = 0; i < a.getNicknames().size(); i++)
        expectEqualNicknameInfo(*a.getNicknames()[i], *b.getNicknames()[i]);
}

CHAT_PACKET_TESTS(GCNicknameList)
CHAT_PACKET_VARIANT(GCNicknameList, empty, fillNoNicknames)

// The code is an enumerator.
void fill(GCNicknameVerify& packet) {
    packet.setCode(NICKNAME_SELECT_FAIL_FORCED);
    packet.setParameter(0xC9DAEBFC);
}

void expectEqual(GCNicknameVerify& a, GCNicknameVerify& b) {
    EXPECT_EQ((int)a.getCode(), (int)b.getCode());
    EXPECT_EQ(a.getParameter(), b.getParameter());
}

CHAT_PACKET_TESTS(GCNicknameVerify)

//////////////////////////////////////////////////////////////////////
// The alliances a guild offers, joins and leaves.
//////////////////////////////////////////////////////////////////////

void fill(CGRequestUnion& packet) {
    packet.setGuildID(0xCDDE);
}

void expectEqual(CGRequestUnion& a, CGRequestUnion& b) {
    EXPECT_EQ((int)a.getGuildID(), (int)b.getGuildID());
}

CHAT_PACKET_TESTS(CGRequestUnion)

void fill(CGAcceptUnion& packet) {
    packet.setGuildID(0xD1E2);
}

void expectEqual(CGAcceptUnion& a, CGAcceptUnion& b) {
    EXPECT_EQ((int)a.getGuildID(), (int)b.getGuildID());
}

CHAT_PACKET_TESTS(CGAcceptUnion)

void fill(CGDenyUnion& packet) {
    packet.setGuildID(0xD3E4);
}

void expectEqual(CGDenyUnion& a, CGDenyUnion& b) {
    EXPECT_EQ((int)a.getGuildID(), (int)b.getGuildID());
}

CHAT_PACKET_TESTS(CGDenyUnion)

// The quit method is an enumerator.
void fill(CGQuitUnion& packet) {
    packet.setGuildID(0xD5E6);
    packet.setQuitMethod(CGQuitUnion::QUIT_QUICK);
}

void expectEqual(CGQuitUnion& a, CGQuitUnion& b) {
    EXPECT_EQ((int)a.getGuildID(), (int)b.getGuildID());
    EXPECT_EQ((int)a.getQuitMethod(), (int)b.getQuitMethod());
}

CHAT_PACKET_TESTS(CGQuitUnion)

void fill(CGQuitUnionAccept& packet) {
    packet.setGuildID(0xD7E8);
}

void expectEqual(CGQuitUnionAccept& a, CGQuitUnionAccept& b) {
    EXPECT_EQ((int)a.getGuildID(), (int)b.getGuildID());
}

CHAT_PACKET_TESTS(CGQuitUnionAccept)

void fill(CGQuitUnionDeny& packet) {
    packet.setGuildID(0xD9EA);
}

void expectEqual(CGQuitUnionDeny& a, CGQuitUnionDeny& b) {
    EXPECT_EQ((int)a.getGuildID(), (int)b.getGuildID());
}

CHAT_PACKET_TESTS(CGQuitUnionDeny)

// An empty body: the request carries nothing but its id.
void fill(CGRequestUnionInfo&) {}

void expectEqual(CGRequestUnionInfo&, CGRequestUnionInfo&) {}

CHAT_PACKET_TESTS(CGRequestUnionInfo)

// The name is text and stops at the twenty the factory max budgets.
void fill(CGAppointSubmaster& packet) {
    packet.setGuildID(0xDBEC);
    packet.setName("Ember Warden");
}

void expectEqual(CGAppointSubmaster& a, CGAppointSubmaster& b) {
    EXPECT_EQ((int)a.getGuildID(), (int)b.getGuildID());
    EXPECT_EQ(a.getName(), b.getName());
}

CHAT_PACKET_TESTS(CGAppointSubmaster)

//////////////////////////////////////////////////////////////////////
// The SMS address book and the texts sent from it.
//////////////////////////////////////////////////////////////////////

// The three fields are text and stop at the 20, 40 and 11 the factory
// max budgets.
void fill(CGAddSMSAddress& packet) {
    packet.setCharacterName("Duskwarden");
    packet.setCustomName("Night shift contact");
    packet.setNumber("01098765432");
}

// All three fields empty, which both sides admit.
void fillBareAddress(CGAddSMSAddress& packet) {
    packet.setCharacterName("");
    packet.setCustomName("");
    packet.setNumber("");
}

void expectEqual(CGAddSMSAddress& a, CGAddSMSAddress& b) {
    EXPECT_EQ(a.getCharacterName(), b.getCharacterName());
    EXPECT_EQ(a.getCustomName(), b.getCustomName());
    EXPECT_EQ(a.getNumber(), b.getNumber());
}

CHAT_PACKET_TESTS(CGAddSMSAddress)
CHAT_PACKET_VARIANT(CGAddSMSAddress, bare, fillBareAddress)

void fill(CGDeleteSMSAddress& packet) {
    packet.setElementID(0xDDEEF0A1);
}

void expectEqual(CGDeleteSMSAddress& a, CGDeleteSMSAddress& b) {
    EXPECT_EQ(a.getElementID(), b.getElementID());
}

CHAT_PACKET_TESTS(CGDeleteSMSAddress)

// An empty body: the request carries nothing but its id.
void fill(CGSMSAddressList&) {}

void expectEqual(CGSMSAddressList&, CGSMSAddressList&) {}

CHAT_PACKET_TESTS(CGSMSAddressList)

// The numbers and the message are text and stop at the 11 and 40
// read() asserts against.
void fill(CGSMSSend& packet) {
    packet.getNumbersList().push_back("01098765432");
    packet.getNumbersList().push_back("01087654321");
    packet.getNumbersList().push_back("01076543210");
    packet.setCallerNumber("01011112222");
    packet.setMessage("See you at the lair at nine.");
}

void fillNoNumbers(CGSMSSend& packet) {
    packet.setCallerNumber("01033334444");
    packet.setMessage("Alone tonight.");
}

void expectEqual(CGSMSSend& a, CGSMSSend& b) {
    EXPECT_EQ(a.getCallerNumber(), b.getCallerNumber());
    EXPECT_EQ(a.getMessage(), b.getMessage());
    ASSERT_EQ(a.getNumbersList().size(), b.getNumbersList().size());
    std::list<std::string>::const_iterator left = a.getNumbersList().begin();
    std::list<std::string>::const_iterator right = b.getNumbersList().begin();
    for (; left != a.getNumbersList().end(); ++left, ++right)
        EXPECT_EQ(*left, *right);
}

CHAT_PACKET_TESTS(CGSMSSend)
CHAT_PACKET_VARIANT(CGSMSSend, empty, fillNoNumbers)

AddressUnit* makeAddress(DWORD id, const std::string& character, const std::string& custom, const std::string& number) {
    AddressUnit* pUnit = new AddressUnit;
    pUnit->ElementID = id;
    pUnit->CharacterName = character;
    pUnit->CustomName = custom;
    pUnit->Number = number;
    return pUnit;
}

// The three text fields stop at the 20, 40 and 11 the record's max
// budgets.
void fill(GCSMSAddressList& packet) {
    packet.getAddresses().push_back(makeAddress(0xE1F283A4, "Duskwarden", "Night shift contact", "01098765432"));
    packet.getAddresses().push_back(makeAddress(0xE5F687A8, "Emberward", "Day shift contact", "01087654321"));
}

void fillNoAddresses(GCSMSAddressList&) {}

void expectEqual(GCSMSAddressList& a, GCSMSAddressList& b) {
    ASSERT_EQ(a.getAddresses().size(), b.getAddresses().size());
    for (size_t i = 0; i < a.getAddresses().size(); i++) {
        EXPECT_EQ(a.getAddresses()[i]->ElementID, b.getAddresses()[i]->ElementID);
        EXPECT_EQ(a.getAddresses()[i]->CharacterName, b.getAddresses()[i]->CharacterName);
        EXPECT_EQ(a.getAddresses()[i]->CustomName, b.getAddresses()[i]->CustomName);
        EXPECT_EQ(a.getAddresses()[i]->Number, b.getAddresses()[i]->Number);
    }
}

CHAT_PACKET_TESTS(GCSMSAddressList)
CHAT_PACKET_VARIANT(GCSMSAddressList, empty, fillNoAddresses)

// The code is an enumerator.
void fill(GCAddressListVerify& packet) {
    packet.setCode(GCAddressListVerify::SMS_SEND_FAIL);
    packet.setParameter(0xE9FA8BAC);
}

void expectEqual(GCAddressListVerify& a, GCAddressListVerify& b) {
    EXPECT_EQ((int)a.getCode(), (int)b.getCode());
    EXPECT_EQ(a.getParameter(), b.getParameter());
}

CHAT_PACKET_TESTS(GCAddressListVerify)

//////////////////////////////////////////////////////////////////////
// The refusals the text fields already produce.
//////////////////////////////////////////////////////////////////////

// Nine of the thirty-one require at least one non-empty string, and the
// senders that leave it empty get an exception instead of a short body.
TEST(ChatTextTest, theTextPacketsRefuseAnEmptyRequiredField) {
    GCSystemMessage systemMessage;
    fill(systemMessage);
    systemMessage.setMessage("");
    EXPECT_THROW(writeBody(systemMessage, kPlainCode), ProtocolException) << "GCSystemMessage";

    GCSay say;
    fill(say);
    say.setMessage("");
    EXPECT_THROW(writeBody(say, kPlainCode), ProtocolException) << "GCSay";

    GCWhisper whisper;
    fill(whisper);
    whisper.setName("");
    EXPECT_THROW(writeBody(whisper, kPlainCode), ProtocolException) << "GCWhisper name";
    fill(whisper);
    whisper.setMessage("");
    EXPECT_THROW(writeBody(whisper, kPlainCode), ProtocolException) << "GCWhisper message";

    CGGlobalChat clientGlobal;
    fill(clientGlobal);
    clientGlobal.setMessage("");
    EXPECT_THROW(writeBody(clientGlobal, kPlainCode), ProtocolException) << "CGGlobalChat";

    GCGlobalChat serverGlobal;
    fill(serverGlobal);
    serverGlobal.setMessage("");
    EXPECT_THROW(writeBody(serverGlobal, kPlainCode), ProtocolException) << "GCGlobalChat";

    CGRangerSay rangerSay;
    fill(rangerSay);
    rangerSay.setMessage("");
    EXPECT_THROW(writeBody(rangerSay, kPlainCode), ProtocolException) << "CGRangerSay";

    GCShowMessageBox messageBox;
    fill(messageBox);
    messageBox.setMessage("");
    EXPECT_THROW(writeBody(messageBox, kPlainCode), ProtocolException) << "GCShowMessageBox";

    GCRequestFailed requestFailed;
    fill(requestFailed);
    requestFailed.setName("");
    EXPECT_THROW(writeBody(requestFailed, kPlainCode), ProtocolException) << "GCRequestFailed";

    CGAppointSubmaster submaster;
    fill(submaster);
    submaster.setName("");
    EXPECT_THROW(writeBody(submaster, kPlainCode), ProtocolException) << "CGAppointSubmaster";
}

//////////////////////////////////////////////////////////////////////
// What the two halves now agree on.
//////////////////////////////////////////////////////////////////////

// The type byte is tested raw, before it reaches an enum that declares
// fewer values than a byte carries.
TEST(GCSystemMessageTest, aTypePastTheLastOneIsRefused) {
    GCSystemMessage dst;
    EXPECT_THROW(throughLoopback(
                     [](SocketEncryptOutputStream& out) {
                         out.write((BYTE)6);
                         out.write(std::string("notice"));
                         out.write((uint)0x81A2B3C4);
                         out.write((BYTE)SYSTEM_MESSAGE_MAX);
                     },
                     [&dst](SocketEncryptInputStream& in) { dst.read(in); }),
                 InvalidProtocolException);
}

// The same for the kick countdown, on the way in through the setter as
// well as off the wire.
TEST(GCKickMessageTest, aTypePastTheLastOneIsRefused) {
    GCKickMessage packet;
    EXPECT_THROW(packet.setType((BYTE)KICK_MESSAGE_MAX), InvalidProtocolException);

    GCKickMessage dst;
    EXPECT_THROW(throughLoopback(
                     [](SocketEncryptOutputStream& out) {
                         out.write((BYTE)KICK_MESSAGE_MAX);
                         out.write((uint)0x81A2B3C4);
                     },
                     [&dst](SocketEncryptInputStream& in) { dst.read(in); }),
                 InvalidProtocolException);
}

// The code setter takes the BYTE the wire carries, so a caller sees
// every value that reaches it.
TEST(GCRequestFailedTest, theCodeIsTheByteTheWireCarries) {
    GCRequestFailed packet;
    packet.setName("Duskwarden");
    packet.setCode(0x81);
    EXPECT_EQ(0x81, (int)packet.getCode());

    GCRequestFailed dst;
    roundTrip(packet, dst, kPlainCode);
    EXPECT_EQ(0x81, (int)dst.getCode());
    EXPECT_EQ(packet.getName(), dst.getName());
}

// The name is held to the ten the factory max budgets, in the setter and
// in read() alike, and the zero-length refusal names the field.
TEST(GCRequestFailedTest, theNameStopsAtWhatTheMaxBudgets) {
    GCRequestFailed packet;
    packet.setCode(REQUEST_FAILED_IP);
    packet.setName(std::string(GCRequestFailed::kMaxNameLength + 1, 'n'));
    EXPECT_EQ((size_t)GCRequestFailed::kMaxNameLength, packet.getName().size());

    GCRequestFailedFactory factory;
    EXPECT_EQ(packet.getPacketSize(), factory.getPacketMaxSize());
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());

    GCRequestFailed wide;
    EXPECT_THROW(throughLoopback(
                     [](SocketEncryptOutputStream& out) {
                         out.write((BYTE)REQUEST_FAILED_IP);
                         out.write((BYTE)(GCRequestFailed::kMaxNameLength + 1));
                         out.write(std::string(GCRequestFailed::kMaxNameLength + 1, 'n'));
                     },
                     [&wide](SocketEncryptInputStream& in) { wide.read(in); }),
                 InvalidProtocolException);

    packet.setName("");
    EXPECT_THROW(writeBody(packet, kPlainCode), InvalidProtocolException);
}

// The item id setter takes the whole ObjectID_t the member and the wire
// both carry.
TEST(CGModifyNicknameTest, theItemIdSetterTakesTheFullObjectId) {
    const ObjectID_t full = fullItemObjectID();

    CGModifyNickname packet;
    packet.setNickname("Ashen Duelist");
    packet.setItemObjectID(full);
    EXPECT_EQ(full, packet.getItemObjectID());

    CGModifyNickname dst;
    roundTrip(packet, dst, kPlainCode);
    EXPECT_EQ(full, dst.getItemObjectID());
}

// A NICK_CUSTOM slot with no text is a record both halves accept.
TEST(GCAddNicknameTest, theEmptyCustomNicknameRoundTrips) {
    GCAddNickname packet;
    fillNicknameInfo(packet.getNicknameInfo(), 0x81A2, NicknameInfo::NICK_CUSTOM, 0, "");

    // The record id, the type byte and a zero length byte.
    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)(szWORD + szBYTE + szBYTE), body.size());
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCAddNickname dst;
    roundTrip(packet, dst, kPlainCode);
    expectEqual(packet, dst);
    EXPECT_TRUE(dst.getNicknameInfo().getNickname().empty());
}

// The nickname is held to MAX_NICKNAME_SIZE in the setter, in write() and
// in read(), and a type no record shape names is refused rather than
// asserted.
TEST(GCAddNicknameTest, theNicknameStopsAtItsMaximumAndAnUnknownTypeIsRefused) {
    GCAddNickname packet;
    fillNicknameInfo(packet.getNicknameInfo(), 0x83A4, NicknameInfo::NICK_CUSTOM, 0,
                     std::string(MAX_NICKNAME_SIZE + 1, 'n'));
    EXPECT_EQ((size_t)MAX_NICKNAME_SIZE, packet.getNicknameInfo().getNickname().size());

    GCAddNicknameFactory factory;
    EXPECT_EQ(packet.getPacketSize(), factory.getPacketMaxSize());
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());

    GCAddNickname wide;
    EXPECT_THROW(throughLoopback(
                     [](SocketEncryptOutputStream& out) {
                         out.write((WORD)0x85A6);
                         out.write((BYTE)NicknameInfo::NICK_CUSTOM);
                         out.write((BYTE)(MAX_NICKNAME_SIZE + 1));
                         out.write(std::string(MAX_NICKNAME_SIZE + 1, 'n'));
                     },
                     [&wide](SocketEncryptInputStream& in) { wide.read(in); }),
                 InvalidProtocolException);

    GCAddNickname unknown;
    EXPECT_THROW(throughLoopback(
                     [](SocketEncryptOutputStream& out) {
                         out.write((WORD)0x87A8);
                         out.write((BYTE)(NicknameInfo::NICK_CUSTOM + 1));
                     },
                     [&unknown](SocketEncryptInputStream& in) { unknown.read(in); }),
                 InvalidProtocolException);

    GCAddNickname emitting;
    fillNicknameInfo(emitting.getNicknameInfo(), 0x89AA, (BYTE)(NicknameInfo::NICK_CUSTOM + 1), 0, "");
    EXPECT_THROW(writeBody(emitting, kPlainCode), InvalidProtocolException);
}

// The listing is held to the records the factory max budgets, which is
// what the count byte in front of it can describe.
TEST(GCNicknameListTest, theListingStopsAtWhatTheFactoryMaxBudgets) {
    EXPECT_EQ(255, MAX_NICKNAME_NUM);

    std::vector<NicknameInfo*> owned;
    GCNicknameList packet;
    for (int i = 0; i < MAX_NICKNAME_NUM; i++) {
        owned.push_back(makeNickname((WORD)(0x8100 + i), NicknameInfo::NICK_NONE, 0, ""));
        packet.getNicknames().push_back(owned.back());
    }

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ(MAX_NICKNAME_NUM, (int)body[0]);
    EXPECT_EQ((size_t)(szBYTE + MAX_NICKNAME_NUM * (szWORD + szBYTE)), body.size());
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCNicknameListFactory factory;
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize());

    owned.push_back(makeNickname(0x9100, NicknameInfo::NICK_NONE, 0, ""));
    packet.getNicknames().push_back(owned.back());
    EXPECT_THROW(writeBody(packet, kPlainCode), InvalidProtocolException);

    for (size_t i = 0; i < owned.size(); i++)
        delete owned[i];
}

// A sender keeps every record it fills the listing with; only the ones
// read() allocated are the packet's to free.
TEST(GCNicknameListTest, thePacketFreesOnlyTheRecordsItRead) {
    GCNicknameList reference;
    fill(reference);
    const std::vector<unsigned char> body = writeBody(reference, kPlainCode);

    {
        GCNicknameList src;
        fill(src);

        GCNicknameList dst;
        roundTrip(src, dst, kPlainCode);
        EXPECT_EQ((size_t)3, dst.getNicknames().size());

        // A second read replaces the records the first one allocated.
        roundTrip(src, dst, kPlainCode);
        EXPECT_EQ((size_t)3, dst.getNicknames().size());
    }

    GCNicknameList again;
    fill(again);
    EXPECT_EQ(body, writeBody(again, kPlainCode));
}

// The address book is held to the entries its own maximum budgets, on
// both sides.
TEST(GCSMSAddressListTest, theListingStopsAtWhatTheFactoryMaxBudgets) {
    GCSMSAddressList packet;
    for (int i = 0; i < MAX_ADDRESS_NUM; i++)
        packet.getAddresses().push_back(makeAddress(0x81A2B300 + i, "", "", ""));

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ(MAX_ADDRESS_NUM, (int)body[0]);
    EXPECT_EQ((size_t)(szBYTE + MAX_ADDRESS_NUM * (szDWORD + szBYTE * 3)), body.size());
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCSMSAddressListFactory factory;
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize());

    packet.getAddresses().push_back(makeAddress(0x91A2B3C4, "", "", ""));
    EXPECT_THROW(writeBody(packet, kPlainCode), InvalidProtocolException);

    GCSMSAddressList dst;
    EXPECT_THROW(throughLoopback([](SocketEncryptOutputStream& out) { out.write((BYTE)(MAX_ADDRESS_NUM + 1)); },
                                 [&dst](SocketEncryptInputStream& in) { dst.read(in); }),
                 InvalidProtocolException);
}

// A packet built at every length read() accepts is exactly what the
// factory max budgets.
TEST(CGSMSSendTest, aPacketAtTheReadCapsIsTheFactoryMax) {
    CGSMSSend packet;
    for (int i = 0; i < MAX_RECEVIER_NUM; i++)
        packet.getNumbersList().push_back(std::string(MAX_NUMBER_LENGTH, '8'));
    packet.setCallerNumber(std::string(MAX_NUMBER_LENGTH, '9'));
    packet.setMessage(std::string(MAX_MESSAGE_LENGTH, 'm'));

    CGSMSSendFactory factory;
    EXPECT_EQ(packet.getPacketSize(), factory.getPacketMaxSize());
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());

    CGSMSSend dst;
    roundTrip(packet, dst, kPlainCode);
    expectEqual(packet, dst);
}

// Every string travels through de::wire, so a message past
// MAX_MESSAGE_LENGTH is refused instead of emitted whole behind a length
// byte that wrapped.
TEST(CGSMSSendTest, theMessageStopsAtWhatTheMaxBudgets) {
    CGSMSSend packet;
    packet.setCallerNumber("01011112222");
    packet.setMessage(std::string(MAX_MESSAGE_LENGTH + 1, 'm'));
    EXPECT_THROW(writeBody(packet, kPlainCode), InvalidProtocolException);

    packet.setMessage(std::string(256, 'm'));
    EXPECT_THROW(writeBody(packet, kPlainCode), InvalidProtocolException);

    packet.setCallerNumber(std::string(MAX_NUMBER_LENGTH + 1, '9'));
    packet.setMessage("refused");
    EXPECT_THROW(writeBody(packet, kPlainCode), InvalidProtocolException);

    CGSMSSend wide;
    EXPECT_THROW(throughLoopback(
                     [](SocketEncryptOutputStream& out) {
                         out.write((BYTE)0);
                         out.write((BYTE)11);
                         out.write(std::string("01011112222"));
                         out.write((BYTE)(MAX_MESSAGE_LENGTH + 1));
                         out.write(std::string(MAX_MESSAGE_LENGTH + 1, 'm'));
                     },
                     [&wide](SocketEncryptInputStream& in) { wide.read(in); }),
                 InvalidProtocolException);
}

// The receiver list is held to what the max budgets on both sides.
TEST(CGSMSSendTest, theReceiverListStopsAtWhatTheMaxBudgets) {
    CGSMSSend packet;
    for (int i = 0; i < MAX_RECEVIER_NUM + 1; i++)
        packet.getNumbersList().push_back("8");
    packet.setCallerNumber("01011112222");
    packet.setMessage("refused");
    EXPECT_THROW(writeBody(packet, kPlainCode), InvalidProtocolException);

    CGSMSSend dst;
    EXPECT_THROW(throughLoopback([](SocketEncryptOutputStream& out) { out.write((BYTE)(MAX_RECEVIER_NUM + 1)); },
                                 [&dst](SocketEncryptInputStream& in) { dst.read(in); }),
                 InvalidProtocolException);
}

//////////////////////////////////////////////////////////////////////
// What the default constructor sets.
//////////////////////////////////////////////////////////////////////

// Each packet is built twice over storage poisoned with a different
// byte, so a member the constructor leaves alone reaches the wire as
// that byte and the two bodies differ. `prep` sets only what write()
// refuses to run without - the required strings, and
// GCModifyNickname's record; it touches no scalar.
template <typename PacketType, typename Prep>
std::vector<unsigned char> bodyOverPoison(unsigned char poison, Prep prep) {
    alignas(PacketType) unsigned char storage[sizeof(PacketType)];
    memset(storage, poison, sizeof(storage));
    PacketType* pPacket = new (storage) PacketType();
    prep(*pPacket);
    std::vector<unsigned char> body = writeBody(*pPacket, kPlainCode);
    pPacket->~PacketType();
    return body;
}

template <typename PacketType, typename Prep> void expectEveryMemberIsInitialised(const char* what, Prep prep) {
    EXPECT_EQ(bodyOverPoison<PacketType>(0x00, prep), bodyOverPoison<PacketType>(0xFF, prep))
        << what << ": its default constructor leaves a member write() emits uninitialised";
}

template <typename PacketType> void expectEveryMemberIsInitialised(const char* what) {
    expectEveryMemberIsInitialised<PacketType>(what, [](PacketType&) {});
}

// The record a poisoned GCModifyNickname is given, so write() reaches
// the scalar the test is about. The packet neither owns nor frees it.
NicknameInfo* poisonPrepNickname() {
    static NicknameInfo* pInfo = new NicknameInfo;
    pInfo->setNicknameType(NicknameInfo::NICK_NONE);
    return pInfo;
}

TEST(ChatConstructorTest, everyPacketInitialisesEveryMemberItWrites) {
    expectEveryMemberIsInitialised<GCSystemMessage>("GCSystemMessage",
                                                    [](GCSystemMessage& p) { p.setMessage("notice"); });
    expectEveryMemberIsInitialised<GCSay>("GCSay", [](GCSay& p) { p.setMessage("say"); });
    expectEveryMemberIsInitialised<GCWhisper>("GCWhisper", [](GCWhisper& p) {
        p.setName("who");
        p.setMessage("whisper");
    });
    expectEveryMemberIsInitialised<GCWhisperFailed>("GCWhisperFailed");
    expectEveryMemberIsInitialised<CGGlobalChat>("CGGlobalChat", [](CGGlobalChat& p) { p.setMessage("shout"); });
    expectEveryMemberIsInitialised<GCGlobalChat>("GCGlobalChat", [](GCGlobalChat& p) { p.setMessage("shout"); });
    expectEveryMemberIsInitialised<CGRangerSay>("CGRangerSay", [](CGRangerSay& p) { p.setMessage("ranger"); });
    expectEveryMemberIsInitialised<GCFriendChatting>("GCFriendChatting");
    expectEveryMemberIsInitialised<GCKickMessage>("GCKickMessage");
    expectEveryMemberIsInitialised<GCShowMessageBox>("GCShowMessageBox",
                                                     [](GCShowMessageBox& p) { p.setMessage("box"); });
    expectEveryMemberIsInitialised<GCRequestFailed>("GCRequestFailed", [](GCRequestFailed& p) { p.setName("who"); });
    expectEveryMemberIsInitialised<CGSelectNickname>("CGSelectNickname");
    expectEveryMemberIsInitialised<CGModifyNickname>("CGModifyNickname");
    expectEveryMemberIsInitialised<GCNicknameVerify>("GCNicknameVerify");
    expectEveryMemberIsInitialised<GCNicknameList>("GCNicknameList");
    expectEveryMemberIsInitialised<GCAddNickname>("GCAddNickname");
    expectEveryMemberIsInitialised<GCModifyNickname>(
        "GCModifyNickname", [](GCModifyNickname& p) { p.setNicknameInfo(poisonPrepNickname()); });
    expectEveryMemberIsInitialised<CGRequestUnion>("CGRequestUnion");
    expectEveryMemberIsInitialised<CGAcceptUnion>("CGAcceptUnion");
    expectEveryMemberIsInitialised<CGDenyUnion>("CGDenyUnion");
    expectEveryMemberIsInitialised<CGQuitUnion>("CGQuitUnion");
    expectEveryMemberIsInitialised<CGQuitUnionAccept>("CGQuitUnionAccept");
    expectEveryMemberIsInitialised<CGQuitUnionDeny>("CGQuitUnionDeny");
    expectEveryMemberIsInitialised<CGRequestUnionInfo>("CGRequestUnionInfo");
    expectEveryMemberIsInitialised<CGAppointSubmaster>("CGAppointSubmaster",
                                                       [](CGAppointSubmaster& p) { p.setName("who"); });
    expectEveryMemberIsInitialised<CGAddSMSAddress>("CGAddSMSAddress");
    expectEveryMemberIsInitialised<CGDeleteSMSAddress>("CGDeleteSMSAddress");
    expectEveryMemberIsInitialised<CGSMSAddressList>("CGSMSAddressList");
    expectEveryMemberIsInitialised<CGSMSSend>("CGSMSSend");
    expectEveryMemberIsInitialised<GCSMSAddressList>("GCSMSAddressList");
    expectEveryMemberIsInitialised<GCAddressListVerify>("GCAddressListVerify");
}

// The record pointer starts empty, and getPacketSize() and write()
// refuse on it instead of following what a sender that skipped
// setNicknameInfo left behind.
TEST(GCModifyNicknameTest, theRecordPointerStartsEmptyAndIsRefusedWhenMissing) {
    GCModifyNickname packet;
    EXPECT_TRUE(packet.getNicknameInfo() == NULL);
    EXPECT_THROW(packet.getPacketSize(), InvalidProtocolException);
    EXPECT_THROW(writeBody(packet, kPlainCode), InvalidProtocolException);
}

} // namespace
