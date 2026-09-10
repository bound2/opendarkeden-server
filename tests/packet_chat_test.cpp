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
//               GCModifyNickname gets the golden and the size pin from
//               the macro but a round trip written out, because its
//               read() writes through a record pointer the receiving
//               packet has to be given first - the second finding
//               below.
//
//               Extra goldens cover the branches one fixture cannot:
//               the nickname written with no text (.noname on
//               CGModifyNickname); the three shapes NicknameInfo takes,
//               a bare id, an index and a string (.none and .index on
//               GCAddNickname, with all three side by side in
//               GCNicknameList's canonical listing); the counted lists
//               written empty (.empty on GCNicknameList, CGSMSSend and
//               GCSMSAddressList); and the address book entry written
//               with all three fields empty (.bare on CGAddSMSAddress,
//               golden and size only, because read() refuses what
//               write() emits there).
//
//               Fixture values are distinct per field and >= 128 in
//               every byte the width allows. Three groups cannot follow
//               that rule and say so at the point of use: the chat
//               messages, character names, nicknames and phone numbers,
//               which are text; the system message type, the kick type,
//               the friend command, the nickname type, the quit method
//               and the four verify codes, which are enumerators; and
//               CGModifyNickname's item id, whose setter takes a WORD.
//
//               Findings. Each is stated as a test that fails once the
//               packet is fixed, except where noted:
//
//               - GCSystemMessage carries a Race_t with a getter and a
//                 setter that neither read() nor write() touches, so a
//                 sender's setRace never leaves the process.
//               - GCModifyNickname leaves uninitialised the record
//                 pointer that getPacketSize(), write() and read() all
//                 dereference, so a sender that skips setNicknameInfo
//                 follows an indeterminate value and a receiver writes
//                 through one.
//               - GCRequestFailed::setCode takes a WORD and stores a
//                 BYTE, so a code past 255 loses its high half before
//                 it reaches the wire.
//               - GCRequestFailed derives its name's length byte from
//                 the name with no bound, while its factory max budgets
//                 ten bytes for it: an eleven-byte name already
//                 outgrows the read buffer.
//               - NicknameInfo::write admits an empty custom nickname
//                 and read() refuses one, so the record write() emits
//                 for a NICK_CUSTOM with no text cannot be read back.
//               - GCNicknameList derives its record count into a BYTE
//                 it caps nowhere, so the 256th record wraps the count
//                 to zero while write() still emits every one. Its
//                 factory max budgets MAX_NICKNAME_NUM (500) records,
//                 more than that byte can ever describe.
//               - GCSMSAddressList wraps its own count byte at 256 the
//                 same way; its max budgets thirty entries.
//               - CGSMSSend's factory max budgets MAX_RECEVIER_NUM (5)
//                 bytes for the caller number where read() accepts
//                 MAX_NUMBER_LENGTH (11), so a packet built at the
//                 lengths read() admits outgrows the read buffer.
//               - CGSMSSend::write derives every length byte from the
//                 string with no bound, so a message past
//                 MAX_MESSAGE_LENGTH is emitted whole and a message of
//                 256 wraps its length byte to zero.
//               - CGSMSSend's receiver count byte is capped nowhere
//                 either, so the 256th number wraps it to zero.
//               - CGModifyNickname::setItemObjectID takes a WORD while
//                 the member it writes and the wire both carry an
//                 ObjectID_t, so an item id past 65535 cannot be set.
//
//               Seven findings are recorded here rather than tested,
//               because reaching them is undefined behaviour, has no
//               observable wire effect, or is a leak:
//
//               - GCSystemMessage::read and GCKickMessage::read cast
//                 the type byte straight to SystemMessageType and
//                 KickMessageType before storing it, and
//                 GCKickMessage::setType casts on the way in too. Both
//                 enums declare fewer values than a byte carries, so a
//                 byte past their range is an out-of-range enum load.
//                 Every fixture here writes a real enumerator for that
//                 reason.
//               - NicknameInfo's getSize(), read() and write() end
//                 their switch on the nickname type with Assert(false),
//                 which appends to assertion_failed.log in the working
//                 directory before it throws. CGSMSSend::read bounds
//                 all four of its lengths through Assert() as well.
//               - ~GCNicknameList and ~GCSMSAddressList free no record,
//                 and both read()s clear the vector without freeing
//                 what it held, so every record a read() allocated
//                 leaks.
//               - NicknameInfo::write caps a custom nickname at the
//                 255 its length byte carries while read(), the setter
//                 and the record's own max all stop at
//                 MAX_NICKNAME_SIZE (22). The setter truncates, so no
//                 sender can reach the gap.
//               - GCRequestFailed's read() and write() raise their
//                 zero-length refusal as a ProtocolException carrying
//                 an empty message, so the log line names no field.
//               - GCFriendChatting caps its message at 128 on read and
//                 512 on write, and admits on write the empty name and
//                 message read() refuses; CGAddSMSAddress admits all
//                 three of its empty fields on write and refuses them
//                 on read. Both are already stated as tests in
//                 tests/packet_roundtrip_test.cpp.
//               - CGSMSSend's max budgets MAX_MESSAGE_LENGTH (40) for
//                 the message, the cap the client's own write()
//                 asserts, while the server once accepted 80.
//
//               Fifteen of the thirty-one leave at least one member the
//               default constructor never sets, so a packet sent
//               without every setter called puts indeterminate bytes on
//               the wire. The poisoned-storage pin at the end of the
//               file asserts today's split. Nine of the thirty-one
//               refuse a body with an empty required string, so those
//               are built over poisoned storage with only that string
//               set - the refusal comes after the scalar fields are
//               already in the buffer, so it proves nothing about them
//               on its own. GCModifyNickname is in neither list because
//               writing one over poisoned storage would follow the
//               indeterminate record pointer, so it gets a pin on the
//               pointer itself instead.
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
using wiretest::roundTrip;
using wiretest::writeBody;

namespace {

// The unencrypted branch. No packet in this file rides the encrypter, so
// this is the only code whose bytes could differ from any other.
const uchar kPlainCode = 0;

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

// The same, for a branch the reader refuses: write() emits a body read()
// will not take back, so only the bytes and the size are pinned.
#define CHAT_PACKET_WRITE_ONLY_VARIANT(Name, Variant, fillVariant)             \
    TEST(Name##Test, Variant##BodyBytesMatchGolden) {                          \
        Name packet;                                                           \
        fillVariant(packet);                                                   \
        const std::vector<unsigned char> body = writeBody(packet, kPlainCode); \
        expectGolden(#Name "." #Variant, kPlainCode, body);                    \
        EXPECT_EQ((size_t)packet.getPacketSize(), body.size());                \
        Name##Factory factory;                                                 \
        EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize());         \
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
// call site, so the narrowing the setter does is the packet's and not a
// constant the compiler folds.
ObjectID_t fullItemObjectID() {
    return 0x81A2B3C4;
}

// The item id cannot follow the >= 128 rule across a full ObjectID_t:
// the setter takes a WORD, which is the finding below.
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

CHAT_PACKET_GOLDEN_AND_SIZE(GCModifyNickname)

// read() writes through the record pointer rather than allocating one,
// so the receiving packet is given a record first.
TEST(GCModifyNicknameTest, roundTripsThroughLoopback) {
    GCModifyNickname src;
    fill(src);

    NicknameInfo target;
    GCModifyNickname dst;
    dst.setNicknameInfo(&target);
    roundTrip(src, dst, kPlainCode);
    expectEqual(src, dst);
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

// write() admits all three empty; read() refuses each.
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
CHAT_PACKET_WRITE_ONLY_VARIANT(CGAddSMSAddress, bare, fillBareAddress)

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
// Findings.
//////////////////////////////////////////////////////////////////////

// FINDING, stated as a test that fails once it is fixed.
// GCSystemMessage carries a Race_t with a getter and a setter that
// neither read() nor write() touches, so a sender's setRace never
// leaves the process.
TEST(GCSystemMessageTest, theRaceItHoldsNeverReachesTheWire) {
    GCSystemMessage slayer;
    fill(slayer);
    slayer.setRace(0x81);

    GCSystemMessage vampire;
    fill(vampire);
    vampire.setRace(0x92);

    EXPECT_EQ(writeBody(slayer, kPlainCode), writeBody(vampire, kPlainCode))
        << "GCSystemMessage now puts the race on the wire - drop this test and record the byte in the golden";
}

// FINDING, stated as a test that fails once it is fixed.
// GCRequestFailed::setCode takes a WORD and stores a BYTE, so a code
// past 255 loses its high half before it reaches the wire.
TEST(GCRequestFailedTest, theCodeSetterDropsTheHighHalfOfItsWord) {
    GCRequestFailed packet;
    packet.setName("Duskwarden");
    packet.setCode(0x0181);

    EXPECT_EQ(0x81, (int)packet.getCode()) << "GCRequestFailed::setCode now keeps the WORD it takes - drop this test";
}

// FINDING, stated as a test that fails once it is fixed.
// GCRequestFailed derives its name's length byte from the name with no
// bound, while its factory max budgets ten bytes for it.
TEST(GCRequestFailedTest, theNameIsNotHeldToWhatTheMaxBudgets) {
    GCRequestFailed packet;
    packet.setCode(REQUEST_FAILED_IP);
    packet.setName(std::string(11, 'n'));

    GCRequestFailedFactory factory;
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCRequestFailed: the name is now held to the ten its max budgets - drop this test";
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());
}

// FINDING, stated as a test that fails once it is fixed.
// CGModifyNickname::setItemObjectID takes a WORD while the member it
// writes and the wire both carry an ObjectID_t.
TEST(CGModifyNicknameTest, theItemIdSetterDropsTheHighHalfOfTheObjectId) {
    const ObjectID_t full = fullItemObjectID();

    CGModifyNickname packet;
    packet.setNickname("Ashen Duelist");
    packet.setItemObjectID(full);

    EXPECT_EQ((ObjectID_t)0xB3C4, packet.getItemObjectID())
        << "CGModifyNickname::setItemObjectID now takes the full ObjectID - drop this test";
}

// FINDING, stated as a test that fails once it is fixed.
// NicknameInfo::write admits an empty custom nickname and read()
// refuses one, so the record write() emits cannot be read back.
TEST(GCAddNicknameTest, theEmptyCustomNicknameWriteEmitsIsRefusedOnRead) {
    GCAddNickname packet;
    fillNicknameInfo(packet.getNicknameInfo(), 0x81A2, NicknameInfo::NICK_CUSTOM, 0, "");

    // The record id, the type byte and a zero length byte.
    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)(szWORD + szBYTE + szBYTE), body.size());

    GCAddNickname dst;
    EXPECT_THROW(roundTrip(packet, dst, kPlainCode), ProtocolException)
        << "NicknameInfo's two halves now agree on the empty custom nickname - drop this test";
}

// FINDING, stated as a test that fails once it is fixed.
// GCNicknameList derives its record count into a BYTE it caps nowhere,
// so the 256th record wraps the count to zero while write() emits every
// one. MAX_NICKNAME_NUM, which the factory max budgets, is 500.
TEST(GCNicknameListTest, theRecordCountWrapsAtTwoHundredAndFiftySix) {
    GCNicknameList packet;
    for (int i = 0; i < 256; i++)
        packet.getNicknames().push_back(makeNickname((WORD)(0x8100 + i), NicknameInfo::NICK_NONE, 0, ""));

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ(0, (int)body[0]) << "GCNicknameList: the listing is now capped - drop this test";
    EXPECT_EQ((size_t)(szBYTE + 256 * (szWORD + szBYTE)), body.size());
    EXPECT_GT(MAX_NICKNAME_NUM, 255) << "MAX_NICKNAME_NUM now fits the count byte - restate this test";
}

// FINDING, stated as a test that fails once it is fixed.
// GCSMSAddressList wraps its own count byte at 256 the same way; its
// max budgets MAX_ADDRESS_NUM entries.
TEST(GCSMSAddressListTest, theEntryCountWrapsAtTwoHundredAndFiftySix) {
    GCSMSAddressList packet;
    for (int i = 0; i < 256; i++)
        packet.getAddresses().push_back(makeAddress(0x81A2B300 + i, "", "", ""));

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ(0, (int)body[0]) << "GCSMSAddressList: the book is now capped - drop this test";
    EXPECT_EQ((size_t)(szBYTE + 256 * (szDWORD + szBYTE * 3)), body.size());
}

// FINDING, stated as a test that fails once it is fixed.
// CGSMSSend's factory max budgets MAX_RECEVIER_NUM bytes for the caller
// number where read() accepts MAX_NUMBER_LENGTH, so a packet built at
// the lengths read() admits outgrows the read buffer.
TEST(CGSMSSendTest, aPacketAtTheReadCapsOutgrowsTheFactoryMax) {
    CGSMSSend packet;
    for (int i = 0; i < MAX_RECEVIER_NUM; i++)
        packet.getNumbersList().push_back(std::string(MAX_NUMBER_LENGTH, '8'));
    packet.setCallerNumber(std::string(MAX_NUMBER_LENGTH, '9'));
    packet.setMessage(std::string(MAX_MESSAGE_LENGTH, 'm'));

    CGSMSSendFactory factory;
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "CGSMSSend: the max now budgets the caller number read() accepts - drop this test";
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());
}

// FINDING, stated as a test that fails once it is fixed.
// CGSMSSend::write derives every length byte from the string with no
// bound, so a message past MAX_MESSAGE_LENGTH is emitted whole and a
// message of 256 wraps its length byte to zero.
TEST(CGSMSSendTest, theMessageLengthByteIsDerivedWithNoBound) {
    CGSMSSend packet;
    packet.setCallerNumber("01011112222");
    packet.setMessage(std::string(MAX_MESSAGE_LENGTH + 1, 'm'));

    // The receiver count, the caller number behind its length byte, then
    // the message behind its own.
    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)(szBYTE + szBYTE + 11 + szBYTE + MAX_MESSAGE_LENGTH + 1), body.size())
        << "CGSMSSend::write now holds the message to MAX_MESSAGE_LENGTH - drop this test";
    EXPECT_EQ(MAX_MESSAGE_LENGTH + 1, (int)body[szBYTE + szBYTE + 11]);

    packet.setMessage(std::string(256, 'm'));
    const std::vector<unsigned char> wrapped = writeBody(packet, kPlainCode);
    EXPECT_EQ(0, (int)wrapped[szBYTE + szBYTE + 11]) << "CGSMSSend: the message length byte no longer wraps";
    EXPECT_EQ((size_t)(szBYTE + szBYTE + 11 + szBYTE + 256), wrapped.size());
}

// FINDING, stated as a test that fails once it is fixed.
// CGSMSSend's receiver count byte is capped nowhere either.
TEST(CGSMSSendTest, theReceiverCountWrapsAtTwoHundredAndFiftySix) {
    CGSMSSend packet;
    for (int i = 0; i < 256; i++)
        packet.getNumbersList().push_back("8");
    packet.setCallerNumber("01011112222");
    packet.setMessage("wrapped");

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ(0, (int)body[0]) << "CGSMSSend: the receiver list is now capped - drop this test";
    EXPECT_EQ((size_t)(szBYTE + 256 * (szBYTE + 1) + szBYTE + 11 + szBYTE + 7), body.size());
}

//////////////////////////////////////////////////////////////////////
// What the default constructor leaves.
//////////////////////////////////////////////////////////////////////

// Each packet is built twice over storage poisoned with a different
// byte, so a member the constructor leaves alone reaches the wire as
// that byte and the two bodies differ. `prep` sets only the strings
// write() refuses to run without; it touches no scalar.
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

// FINDING, stated as a test that fails once it is fixed. Each of these
// puts an indeterminate byte on the wire when a sender skips a setter.
template <typename PacketType, typename Prep> void expectAMemberIsLeftUninitialised(const char* what, Prep prep) {
    EXPECT_NE(bodyOverPoison<PacketType>(0x00, prep), bodyOverPoison<PacketType>(0xFF, prep))
        << what
        << ": its default constructor now initialises every member write() emits - move it to the "
           "initialised list";
}

template <typename PacketType> void expectAMemberIsLeftUninitialised(const char* what) {
    expectAMemberIsLeftUninitialised<PacketType>(what, [](PacketType&) {});
}

TEST(ChatConstructorTest, thePacketsThatInitialiseEveryMemberTheyWrite) {
    expectEveryMemberIsInitialised<GCSystemMessage>("GCSystemMessage",
                                                    [](GCSystemMessage& p) { p.setMessage("notice"); });
    expectEveryMemberIsInitialised<GCWhisperFailed>("GCWhisperFailed");
    expectEveryMemberIsInitialised<CGRangerSay>("CGRangerSay", [](CGRangerSay& p) { p.setMessage("ranger"); });
    expectEveryMemberIsInitialised<GCFriendChatting>("GCFriendChatting");
    expectEveryMemberIsInitialised<GCShowMessageBox>("GCShowMessageBox",
                                                     [](GCShowMessageBox& p) { p.setMessage("box"); });
    expectEveryMemberIsInitialised<GCRequestFailed>("GCRequestFailed", [](GCRequestFailed& p) { p.setName("who"); });
    expectEveryMemberIsInitialised<GCAddNickname>("GCAddNickname");
    expectEveryMemberIsInitialised<GCNicknameList>("GCNicknameList");
    expectEveryMemberIsInitialised<GCNicknameVerify>("GCNicknameVerify");
    expectEveryMemberIsInitialised<CGRequestUnionInfo>("CGRequestUnionInfo");
    expectEveryMemberIsInitialised<CGAddSMSAddress>("CGAddSMSAddress");
    expectEveryMemberIsInitialised<CGSMSAddressList>("CGSMSAddressList");
    expectEveryMemberIsInitialised<CGSMSSend>("CGSMSSend");
    expectEveryMemberIsInitialised<GCSMSAddressList>("GCSMSAddressList");
    expectEveryMemberIsInitialised<GCAddressListVerify>("GCAddressListVerify");
}

TEST(ChatConstructorTest, thePacketsThatDoNot) {
    expectAMemberIsLeftUninitialised<GCSay>("GCSay", [](GCSay& p) { p.setMessage("say"); });
    expectAMemberIsLeftUninitialised<GCWhisper>("GCWhisper", [](GCWhisper& p) {
        p.setName("who");
        p.setMessage("whisper");
    });
    expectAMemberIsLeftUninitialised<CGGlobalChat>("CGGlobalChat", [](CGGlobalChat& p) { p.setMessage("shout"); });
    expectAMemberIsLeftUninitialised<GCGlobalChat>("GCGlobalChat", [](GCGlobalChat& p) { p.setMessage("shout"); });
    expectAMemberIsLeftUninitialised<GCKickMessage>("GCKickMessage");
    expectAMemberIsLeftUninitialised<CGSelectNickname>("CGSelectNickname");
    expectAMemberIsLeftUninitialised<CGModifyNickname>("CGModifyNickname");
    expectAMemberIsLeftUninitialised<CGRequestUnion>("CGRequestUnion");
    expectAMemberIsLeftUninitialised<CGAcceptUnion>("CGAcceptUnion");
    expectAMemberIsLeftUninitialised<CGDenyUnion>("CGDenyUnion");
    expectAMemberIsLeftUninitialised<CGQuitUnion>("CGQuitUnion");
    expectAMemberIsLeftUninitialised<CGQuitUnionAccept>("CGQuitUnionAccept");
    expectAMemberIsLeftUninitialised<CGQuitUnionDeny>("CGQuitUnionDeny");
    expectAMemberIsLeftUninitialised<CGAppointSubmaster>("CGAppointSubmaster",
                                                         [](CGAppointSubmaster& p) { p.setName("who"); });
    expectAMemberIsLeftUninitialised<CGDeleteSMSAddress>("CGDeleteSMSAddress");
}

// FINDING, stated as a test that fails once it is fixed.
// GCModifyNickname is in neither list because writing one over poisoned
// storage would follow the record pointer its constructor leaves alone.
// The pointer itself is pinned instead: getPacketSize(), write() and
// read() all dereference it, so a sender that skips setNicknameInfo
// follows an indeterminate value and a receiver writes through one.
TEST(ChatConstructorTest, theModifyNicknameRecordPointerIsLeftUninitialised) {
    alignas(GCModifyNickname) unsigned char zeroed[sizeof(GCModifyNickname)];
    memset(zeroed, 0x00, sizeof(zeroed));
    GCModifyNickname* pZeroed = new (zeroed) GCModifyNickname();
    NicknameInfo* pFromZero = pZeroed->getNicknameInfo();
    pZeroed->~GCModifyNickname();

    alignas(GCModifyNickname) unsigned char poisoned[sizeof(GCModifyNickname)];
    memset(poisoned, 0xFF, sizeof(poisoned));
    GCModifyNickname* pPoisoned = new (poisoned) GCModifyNickname();
    NicknameInfo* pFromPoison = pPoisoned->getNicknameInfo();
    pPoisoned->~GCModifyNickname();

    EXPECT_NE(pFromZero, pFromPoison)
        << "GCModifyNickname: its constructor now initialises the record pointer - move it to the "
           "initialised list";
}

} // namespace
