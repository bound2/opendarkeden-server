//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_guild_test.cpp
// Description : Golden byte fixtures, loopback round trips and size
//               pins for the guild protocol: every CG and GC packet a
//               client and a game server exchange to found a guild,
//               join or leave one, browse its roster and talk on its
//               channel. Twenty-five packets, each with the reason it
//               is here:
//
//               CGRegistGuild    the founding request: a guild name
//                                and an introduction.
//               CGRequestGuildList  the client asking for the guilds it
//                                may join, either the ones still
//                                waiting for registration or the
//                                ordinary ones.
//               GCActiveGuildList   that list: one record per guild,
//                                behind a WORD count.
//               CGSelectGuild    the client picking one guild out of
//                                the list.
//               GCShowGuildInfo  the guild's own page: name, state,
//                                master, member count, introduction and
//                                the fee it charges to join.
//               CGTryJoinGuild   the client asking what joining that
//                                guild at a given rank would cost.
//               GCShowGuildJoin  the answer: the guild, the rank and
//                                the fee.
//               CGJoinGuild      the client joining, with the
//                                introduction it wants recorded.
//               GCGuildResponse  the WORD result code, with a
//                                parameter, that answers the guild
//                                requests the server does not answer
//                                with a dialogue of its own.
//               GCNPCResponse    the NPC dialogue code that opens and
//                                closes the guild windows, and carries
//                                a parameter for the codes that need
//                                one.
//               CGRequestGuildMemberList  the client asking for the
//                                roster.
//               GCGuildMemberList   that roster: one record per member,
//                                each with a rank, a log-on flag and
//                                the server it is on.
//               CGSelectGuildMember the client picking one member.
//               GCShowGuildMemberInfo the member's page: rank and
//                                introduction.
//               CGModifyGuildMember the master changing a member's
//                                rank.
//               GCModifyGuildMemberInfo what the changed member is
//                                told: its guild, that guild's name and
//                                its new rank.
//               CGModifyGuildIntro  the master rewriting the guild's
//                                introduction.
//               CGModifyGuildMemberIntro a member rewriting its own.
//               CGExpelGuild     the master dissolving the guild.
//               CGGuildChat      a line of guild chat with the colour
//                                the sender chose.
//               GCGuildChat      that line forwarded to the members,
//                                with the sender in front of it and,
//                                for a union channel, the sending
//                                guild's name in front of that.
//               GCOtherGuildName the guild badge another character
//                                carries, sent when it comes into view.
//               GCUnionOfferList the guilds that have offered to join
//                                or leave the union, each with the date
//                                of its offer.
//               GCWaitGuildList  the same table for the guilds still
//                                waiting for registration.
//               GCShowWaitGuildInfo  one such guild's page, with the
//                                founding members it has gathered.
//
//               Deliberately excluded, because no server registers a
//               factory for them (tests/ratchet/factory_registrations.txt
//               lists every registration): CGExpelGuildMember,
//               CGQuitGuild and GCShowGuildRegist. A packet in no
//               registry can neither be dispatched nor sized by a
//               receiver, so there is no wire contract to pin.
//
//               No packet in this file calls readEncrypt/writeEncrypt,
//               so the goldens are recorded at encrypt code 0 only, and
//               every golden test also asserts the bytes do not vary
//               with the code, so adopting the encrypter fails loudly
//               instead of silently voiding the pin.
//
//               Each packet gets three pins (GUILD_PACKET_TESTS):
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
//               Extra goldens cover the branches one fixture cannot.
//               The four packets with an optional introduction
//               (CGJoinGuild, CGModifyGuildIntro,
//               CGModifyGuildMemberIntro, CGRegistGuild) and the two
//               that carry one in the middle of their body
//               (GCShowGuildInfo, GCShowGuildMemberInfo) get a second
//               golden with the string absent. GCGuildChat.plain is the
//               guild-channel shape, whose type byte suppresses the
//               sending guild's name that the canonical union-channel
//               fixture carries. GCNPCResponse.noparam is a code
//               outside the fifteen that carry a parameter.
//               GCActiveGuildList's canonical fixture carries one guild
//               with an expiry date and one without, so both sides of
//               GuildInfo::write()'s date branch run, and
//               GCActiveGuildList.empty, GCGuildMemberList.empty and
//               GCUnionOfferList.empty take the zero-record branch of
//               their lists. GCModifyGuildMemberInfo.noname and
//               GCOtherGuildName.noname take the empty-name branch of
//               their write().
//
//               Fixture values are distinct per field and >= 128 in
//               every byte the width allows. GuildState_t and
//               GuildMemberRank_t are BYTE typedefs rather than enums,
//               so they carry high bytes too. Three groups cannot
//               follow the rule and say so at the point of use: the
//               dialogue codes, which are enumerators; the log-on
//               flags, whose domain is 0 and 1; and the names,
//               introductions, dates and chat messages, which each
//               packet caps well below 128 characters.
//
//               Seven more pins cover the sizes, the counts and the
//               bounds every writer holds:
//               packetSizeCountsEveryGuildsExpireDate,
//               listNumIsTheWordCountOnTheWire,
//               aRosterPastTheFactoryBudgetIsRefused,
//               aFullListFitsTheFactoryMaxAndOneMoreIsRefused,
//               introsAreCutToTheWidthTheLengthByteAndTheFactoryMaxAllow,
//               theSendingGuildNameIsBounded and
//               theCodeGettersReturnTheWordTheWireCarries. A valid
//               packet is untouched by any of them.
//
//               Not expressible as a test: every CG packet in this file,
//               and GCGuildChat, GCModifyGuildMemberInfo,
//               GCOtherGuildName, GCShowGuildInfo, GCShowGuildJoin and
//               GCShowGuildMemberInfo, leave every scalar member
//               uninitialised. A packet that is written without every
//               setter being called puts whatever the allocation held on
//               the wire, and no test can pin that value.
//
//////////////////////////////////////////////////////////////////////

#include <list>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "CGExpelGuild.h"
#include "CGGuildChat.h"
#include "CGJoinGuild.h"
#include "CGModifyGuildIntro.h"
#include "CGModifyGuildMember.h"
#include "CGModifyGuildMemberIntro.h"
#include "CGRegistGuild.h"
#include "CGRequestGuildList.h"
#include "CGRequestGuildMemberList.h"
#include "CGSelectGuild.h"
#include "CGSelectGuildMember.h"
#include "CGTryJoinGuild.h"
#include "GCActiveGuildList.h"
#include "GCGuildChat.h"
#include "GCGuildMemberList.h"
#include "GCGuildResponse.h"
#include "GCModifyGuildMemberInfo.h"
#include "GCNPCResponse.h"
#include "GCOtherGuildName.h"
#include "GCShowGuildInfo.h"
#include "GCShowGuildJoin.h"
#include "GCShowGuildMemberInfo.h"
#include "GCShowWaitGuildInfo.h"
#include "GCUnionOfferList.h"
#include "GCWaitGuildList.h"
#include "GuildInfo.h"
#include "GuildMemberInfo.h"
#include "TestStreams.h"

using wiretest::expectGolden;
using wiretest::kEncryptCodeCount;
using wiretest::kEncryptCodes;
using wiretest::roundTrip;
using wiretest::writeBody;

namespace {

// The unencrypted branch. Every packet in this file reads and writes its
// body with plain read()/write() calls, so this is the only code whose
// bytes differ from any other.
const uchar kPlainCode = 0;

//////////////////////////////////////////////////////////////////////
// The three pins every packet gets. fill() / expectEqual() are
// overloaded per packet, as in packet_login_test.cpp, so the same
// canonical instance feeds all three.
//////////////////////////////////////////////////////////////////////

#define GUILD_PACKET_TESTS(Name)                  \
    TEST(Name##Test, roundTripsThroughLoopback) { \
        Name src;                                 \
        fill(src);                                \
        Name dst;                                 \
        roundTrip(src, dst, kPlainCode);          \
        expectEqual(src, dst);                    \
    }                                             \
    GUILD_PACKET_WRITE_TESTS(Name)

// The golden and the size pins on their own, for a packet whose read()
// cannot reconstruct what write() sent.
#define GUILD_PACKET_WRITE_TESTS(Name)                                                               \
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

// A second fixture for a packet whose write() has a branch the canonical
// one does not take.
#define GUILD_PACKET_VARIANT(Name, Variant, fillVariant)                       \
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

// The same for a packet that cannot be read back.
#define GUILD_PACKET_WRITE_VARIANT(Name, Variant, fillVariant)                 \
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
// Founding, browsing and joining.
//////////////////////////////////////////////////////////////////////

void fill(CGRegistGuild& packet) {
    packet.setGuildName("GuildRegName");
    packet.setGuildIntro("new guild introduction");
}

void fillNointro(CGRegistGuild& packet) {
    packet.setGuildName("GuildRegName");
    packet.setGuildIntro("");
}

void expectEqual(const CGRegistGuild& a, const CGRegistGuild& b) {
    EXPECT_EQ(a.getGuildName(), b.getGuildName());
    EXPECT_EQ(a.getGuildIntro(), b.getGuildIntro());
}

GUILD_PACKET_TESTS(CGRegistGuild)
GUILD_PACKET_VARIANT(CGRegistGuild, nointro, fillNointro)

void fill(CGRequestGuildList& packet) {
    // The packet's own enum has two enumerators, so the type byte
    // carries a valid one rather than a high byte.
    packet.setGuildType(CGRequestGuildList::GUILDTYPE_NORMAL);
}

void expectEqual(const CGRequestGuildList& a, const CGRequestGuildList& b) {
    EXPECT_EQ((int)a.getGuildType(), (int)b.getGuildType());
}

GUILD_PACKET_TESTS(CGRequestGuildList)

void fill(CGSelectGuild& packet) {
    packet.setGuildID(0x8D0B);
}

void expectEqual(const CGSelectGuild& a, const CGSelectGuild& b) {
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
}

GUILD_PACKET_TESTS(CGSelectGuild)

void fill(CGTryJoinGuild& packet) {
    packet.setGuildID(0x8F2D);
    packet.setGuildMemberRank(0x90);
}

void expectEqual(const CGTryJoinGuild& a, const CGTryJoinGuild& b) {
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ((int)a.getGuildMemberRank(), (int)b.getGuildMemberRank());
}

GUILD_PACKET_TESTS(CGTryJoinGuild)

void fill(CGJoinGuild& packet) {
    packet.setGuildID(0x85B6);
    packet.setGuildMemberRank(0x87);
    packet.setGuildMemberIntro("join guild introduction");
}

void fillNointro(CGJoinGuild& packet) {
    packet.setGuildID(0x85B6);
    packet.setGuildMemberRank(0x87);
    packet.setGuildMemberIntro("");
}

void expectEqual(const CGJoinGuild& a, const CGJoinGuild& b) {
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ((int)a.getGuildMemberRank(), (int)b.getGuildMemberRank());
    EXPECT_EQ(a.getGuildMemberIntro(), b.getGuildMemberIntro());
}

GUILD_PACKET_TESTS(CGJoinGuild)
GUILD_PACKET_VARIANT(CGJoinGuild, nointro, fillNointro)

void fill(CGExpelGuild& packet) {
    packet.setGuildID(0x819C);
}

void expectEqual(const CGExpelGuild& a, const CGExpelGuild& b) {
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
}

GUILD_PACKET_TESTS(CGExpelGuild)

//////////////////////////////////////////////////////////////////////
// The roster and the member pages.
//////////////////////////////////////////////////////////////////////

void fill(CGRequestGuildMemberList& packet) {
    packet.setGuildID(0x8CFA);
}

void expectEqual(const CGRequestGuildMemberList& a, const CGRequestGuildMemberList& b) {
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
}

GUILD_PACKET_TESTS(CGRequestGuildMemberList)

void fill(CGSelectGuildMember& packet) {
    packet.setGuildID(0x8E1C);
    packet.setName("GuildSelectNm");
}

void expectEqual(const CGSelectGuildMember& a, const CGSelectGuildMember& b) {
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getName(), b.getName());
}

GUILD_PACKET_TESTS(CGSelectGuildMember)

void fill(CGModifyGuildMember& packet) {
    packet.setGuildID(0x89D8);
    packet.setName("GuildMemberNm");
    packet.setGuildMemberRank(0x8A);
}

void expectEqual(const CGModifyGuildMember& a, const CGModifyGuildMember& b) {
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ((int)a.getGuildMemberRank(), (int)b.getGuildMemberRank());
}

GUILD_PACKET_TESTS(CGModifyGuildMember)

void fill(CGModifyGuildIntro& packet) {
    packet.setGuildID(0x88C7);
    packet.setGuildIntro("modified guild introduction");
}

void fillNointro(CGModifyGuildIntro& packet) {
    packet.setGuildID(0x88C7);
    packet.setGuildIntro("");
}

void expectEqual(const CGModifyGuildIntro& a, const CGModifyGuildIntro& b) {
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getGuildIntro(), b.getGuildIntro());
}

GUILD_PACKET_TESTS(CGModifyGuildIntro)
GUILD_PACKET_VARIANT(CGModifyGuildIntro, nointro, fillNointro)

void fill(CGModifyGuildMemberIntro& packet) {
    packet.setGuildID(0x8BE9);
    packet.setGuildMemberIntro("modified member introduction");
}

void fillNointro(CGModifyGuildMemberIntro& packet) {
    packet.setGuildID(0x8BE9);
    packet.setGuildMemberIntro("");
}

void expectEqual(const CGModifyGuildMemberIntro& a, const CGModifyGuildMemberIntro& b) {
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getGuildMemberIntro(), b.getGuildMemberIntro());
}

GUILD_PACKET_TESTS(CGModifyGuildMemberIntro)
GUILD_PACKET_VARIANT(CGModifyGuildMemberIntro, nointro, fillNointro)

//////////////////////////////////////////////////////////////////////
// Guild chat.
//////////////////////////////////////////////////////////////////////

void fill(CGGuildChat& packet) {
    // The type byte selects the channel the sender is talking on; the
    // wire layer carries it without an enumeration of its own.
    packet.setType(0x83);
    packet.setColor(0x84A5B6C7);
    packet.setMessage("guild chat from the client");
}

void expectEqual(const CGGuildChat& a, const CGGuildChat& b) {
    EXPECT_EQ((int)a.getType(), (int)b.getType());
    EXPECT_EQ(a.getColor(), b.getColor());
    EXPECT_EQ(a.getMessage(), b.getMessage());
}

GUILD_PACKET_TESTS(CGGuildChat)

void fill(GCGuildChat& packet) {
    packet.setType(0x91);
    packet.setSendGuildName("GuildChatNm");
    packet.setSender("ChatSender");
    packet.setColor(0x92A3B4C5);
    packet.setMessage("guild chat forwarded to the members");
}

// The guild channel rather than the union channel: a zero type byte
// suppresses the sending guild's name entirely.
void fillPlain(GCGuildChat& packet) {
    packet.setType(0);
    packet.setSendGuildName("GuildChatNm");
    packet.setSender("ChatSender");
    packet.setColor(0x92A3B4C5);
    packet.setMessage("guild chat forwarded to the members");
}

void expectEqual(const GCGuildChat& a, const GCGuildChat& b) {
    EXPECT_EQ((int)a.getType(), (int)b.getType());
    if (a.getType() != 0)
        EXPECT_EQ(a.getSendGuildName(), b.getSendGuildName());
    EXPECT_EQ(a.getSender(), b.getSender());
    EXPECT_EQ(a.getColor(), b.getColor());
    EXPECT_EQ(a.getMessage(), b.getMessage());
}

GUILD_PACKET_TESTS(GCGuildChat)

// The plain variant cannot use GUILD_PACKET_VARIANT: the fixture keeps a
// guild name that write() drops, so the round trip is compared against
// the shape the wire actually carries.
TEST(GCGuildChatTest, plainBodyBytesMatchGolden) {
    GCGuildChat packet;
    fillPlain(packet);

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    expectGolden("GCGuildChat.plain", kPlainCode, body);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCGuildChatFactory factory;
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize());

    GCGuildChat dst;
    roundTrip(packet, dst, kPlainCode);
    expectEqual(packet, dst);
    EXPECT_EQ("", dst.getSendGuildName()) << "a zero type byte carries no guild name";
}

//////////////////////////////////////////////////////////////////////
// The guild and member pages the server sends.
//////////////////////////////////////////////////////////////////////

void fill(GCShowGuildInfo& packet) {
    packet.setGuildID(0x9BFC);
    packet.setGuildName("GuildShowName");
    packet.setGuildState(0x9C);
    packet.setGuildMaster("GuildMaster");
    packet.setGuildMemberCount(0x9D);
    packet.setGuildIntro("guild introduction shown to the client");
    packet.setJoinFee(0x9EAFC0D1);
}

void fillNointro(GCShowGuildInfo& packet) {
    fill(packet);
    packet.setGuildIntro("");
}

void expectEqual(const GCShowGuildInfo& a, const GCShowGuildInfo& b) {
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getGuildName(), b.getGuildName());
    EXPECT_EQ((int)a.getGuildState(), (int)b.getGuildState());
    EXPECT_EQ(a.getGuildMaster(), b.getGuildMaster());
    EXPECT_EQ((int)a.getGuildMemberCount(), (int)b.getGuildMemberCount());
    EXPECT_EQ(a.getGuildIntro(), b.getGuildIntro());
    EXPECT_EQ(a.getJoinFee(), b.getJoinFee());
}

GUILD_PACKET_TESTS(GCShowGuildInfo)
GUILD_PACKET_VARIANT(GCShowGuildInfo, nointro, fillNointro)

void fill(GCShowGuildJoin& packet) {
    packet.setGuildID(0x9F0D);
    packet.setGuildName("GuildJoinName");
    packet.setGuildMemberRank(0xA0);
    packet.setJoinFee(0xA1B2C3D4);
}

void expectEqual(const GCShowGuildJoin& a, const GCShowGuildJoin& b) {
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getGuildName(), b.getGuildName());
    EXPECT_EQ((int)a.getGuildMemberRank(), (int)b.getGuildMemberRank());
    EXPECT_EQ(a.getJoinFee(), b.getJoinFee());
}

GUILD_PACKET_TESTS(GCShowGuildJoin)

void fill(GCShowGuildMemberInfo& packet) {
    packet.setGuildID(0xA21E);
    packet.setName("GuildMbrName");
    packet.setGuildMemberRank(0xA3);
    packet.setGuildMemberIntro("member introduction");
}

void fillNointro(GCShowGuildMemberInfo& packet) {
    fill(packet);
    packet.setGuildMemberIntro("");
}

void expectEqual(const GCShowGuildMemberInfo& a, const GCShowGuildMemberInfo& b) {
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ((int)a.getGuildMemberRank(), (int)b.getGuildMemberRank());
    EXPECT_EQ(a.getGuildMemberIntro(), b.getGuildMemberIntro());
}

GUILD_PACKET_TESTS(GCShowGuildMemberInfo)
GUILD_PACKET_VARIANT(GCShowGuildMemberInfo, nointro, fillNointro)

//////////////////////////////////////////////////////////////////////
// The two result codes.
//////////////////////////////////////////////////////////////////////

void fill(GCGuildResponse& packet) {
    // The code is a WORD on the wire but a BYTE through getCode(), so
    // the canonical value stays inside a byte and the truncation gets
    // its own test below.
    packet.setCode(0x94);
    packet.setParameter(0x95A6B7C8);
}

void expectEqual(const GCGuildResponse& a, const GCGuildResponse& b) {
    EXPECT_EQ((int)a.getCode(), (int)b.getCode());
    EXPECT_EQ(a.getParameter(), b.getParameter());
}

GUILD_PACKET_TESTS(GCGuildResponse)

void fill(GCNPCResponse& packet) {
    // One of the fifteen codes whose write() carries a parameter after
    // it; the code is an enumerator, not a high byte.
    packet.setCode(NPC_RESPONSE_QUEST);
    packet.setParameter(0x98E9FA0B);
}

// A code outside those fifteen: the body is the code alone, and the
// parameter keeps the zero the constructor gave it.
void fillNoparam(GCNPCResponse& packet) {
    packet.setCode(NPC_RESPONSE_HEAL);
}

void expectEqual(const GCNPCResponse& a, const GCNPCResponse& b) {
    EXPECT_EQ((int)a.getCode(), (int)b.getCode());
    EXPECT_EQ(a.getParameter(), b.getParameter());
}

GUILD_PACKET_TESTS(GCNPCResponse)
GUILD_PACKET_VARIANT(GCNPCResponse, noparam, fillNoparam)

//////////////////////////////////////////////////////////////////////
// The two guild badges, whose name is absent for a guildless character.
//////////////////////////////////////////////////////////////////////

void fill(GCModifyGuildMemberInfo& packet) {
    packet.setGuildID(0x96D9);
    packet.setGuildName("GuildModName");
    packet.setGuildMemberRank(0x97);
}

void fillNoname(GCModifyGuildMemberInfo& packet) {
    packet.setGuildID(0x96D9);
    packet.setGuildName("");
    packet.setGuildMemberRank(0x97);
}

void expectEqual(const GCModifyGuildMemberInfo& a, const GCModifyGuildMemberInfo& b) {
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getGuildName(), b.getGuildName());
    EXPECT_EQ((int)a.getGuildMemberRank(), (int)b.getGuildMemberRank());
}

GUILD_PACKET_TESTS(GCModifyGuildMemberInfo)
GUILD_PACKET_VARIANT(GCModifyGuildMemberInfo, noname, fillNoname)

void fill(GCOtherGuildName& packet) {
    packet.setObjectID(0x99AABBCC);
    packet.setGuildID(0x9AEB);
    packet.setGuildName("GuildOtherNm");
}

void fillNoname(GCOtherGuildName& packet) {
    packet.setObjectID(0x99AABBCC);
    packet.setGuildID(0x9AEB);
    packet.setGuildName("");
}

void expectEqual(const GCOtherGuildName& a, const GCOtherGuildName& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getGuildName(), b.getGuildName());
}

GUILD_PACKET_TESTS(GCOtherGuildName)
GUILD_PACKET_VARIANT(GCOtherGuildName, noname, fillNoname)

//////////////////////////////////////////////////////////////////////
// The three lists.
//////////////////////////////////////////////////////////////////////

// Every list packet here takes ownership of the records it is handed, so
// each fixture allocates fresh ones.
GuildInfo* makeGuildInfo(GuildID_t id, const std::string& name, const std::string& master, BYTE memberCount,
                         const std::string& expireDate) {
    GuildInfo* pInfo = new GuildInfo();
    pInfo->setGuildID(id);
    pInfo->setGuildName(name);
    pInfo->setGuildMaster(master);
    pInfo->setGuildMemberCount(memberCount);
    pInfo->setGuildExpireDate(expireDate);
    return pInfo;
}

void expectGuildInfoEqual(GuildInfo& a, GuildInfo& b, int index) {
    EXPECT_EQ(a.getGuildID(), b.getGuildID()) << "guild " << index;
    EXPECT_EQ(a.getGuildName(), b.getGuildName()) << "guild " << index;
    EXPECT_EQ(a.getGuildMaster(), b.getGuildMaster()) << "guild " << index;
    EXPECT_EQ((int)a.getGuildMemberCount(), (int)b.getGuildMemberCount()) << "guild " << index;
    EXPECT_EQ(a.getGuildExpireDate(), b.getGuildExpireDate()) << "guild " << index;
}

void fill(GCActiveGuildList& packet) {
    // The second guild has no expiry date, so both sides of the date
    // branch of GuildInfo::write() run.
    packet.addGuildInfo(makeGuildInfo(0xA4BF, "GuildListNameA", "GuildMasterA", 0xA5, "2026.09.09"));
    packet.addGuildInfo(makeGuildInfo(0xA6D1, "GuildListNameB", "GuildMasterB", 0xA7, ""));
}

void fillEmpty(GCActiveGuildList&) {}

void expectEqual(GCActiveGuildList& a, GCActiveGuildList& b) {
    ASSERT_EQ((int)a.getListNum(), (int)b.getListNum());

    const int guilds = (int)a.getListNum();
    for (int i = 0; i < guilds; i++) {
        GuildInfo* pLeft = a.popFrontGuildInfoList();
        GuildInfo* pRight = b.popFrontGuildInfoList();
        ASSERT_TRUE(pLeft != NULL);
        ASSERT_TRUE(pRight != NULL);
        expectGuildInfoEqual(*pLeft, *pRight, i);
        delete pLeft;
        delete pRight;
    }
}

GUILD_PACKET_TESTS(GCActiveGuildList)
GUILD_PACKET_VARIANT(GCActiveGuildList, empty, fillEmpty)

// The same record on the other guild table: the guilds still waiting for
// registration rather than the active ones.
void fill(GCWaitGuildList& packet) {
    packet.addGuildInfo(makeGuildInfo(0xA8E3, "GuildWaitNameA", "GuildWaitMsA", 0xA9, "2026.09.09"));
    packet.addGuildInfo(makeGuildInfo(0xAAF5, "GuildWaitNameB", "GuildWaitMsB", 0xAB, ""));
}

void expectEqual(GCWaitGuildList& a, GCWaitGuildList& b) {
    ASSERT_EQ((int)a.getListNum(), (int)b.getListNum());

    const int guilds = (int)a.getListNum();
    for (int i = 0; i < guilds; i++) {
        GuildInfo* pLeft = a.popFrontGuildInfoList();
        GuildInfo* pRight = b.popFrontGuildInfoList();
        ASSERT_TRUE(pLeft != NULL);
        ASSERT_TRUE(pRight != NULL);
        expectGuildInfoEqual(*pLeft, *pRight, i);
        delete pLeft;
        delete pRight;
    }
}

GUILD_PACKET_TESTS(GCWaitGuildList)

GuildMemberInfo* makeGuildMemberInfo(const std::string& name, GuildMemberRank_t rank, bool logOn, ServerID_t serverID) {
    GuildMemberInfo* pInfo = new GuildMemberInfo();
    pInfo->setName(name);
    pInfo->setRank(rank);
    pInfo->setLogOn(logOn);
    pInfo->setServerID(serverID);
    return pInfo;
}

void fill(GCGuildMemberList& packet) {
    packet.setType(0x93);
    // The log-on flag is a bool, so it carries 0 and 1 rather than a
    // high byte.
    packet.addGuildMemberInfo(makeGuildMemberInfo("GuildRosterA", 0xA8, true, 0xA9BA));
    packet.addGuildMemberInfo(makeGuildMemberInfo("GuildRosterB", 0xAB, false, 0xACBD));
}

void fillEmpty(GCGuildMemberList& packet) {
    packet.setType(0x93);
}

void expectEqual(GCGuildMemberList& a, GCGuildMemberList& b) {
    EXPECT_EQ((int)a.getType(), (int)b.getType());
    ASSERT_EQ((int)a.getListNum(), (int)b.getListNum());

    const int members = (int)a.getListNum();
    for (int i = 0; i < members; i++) {
        GuildMemberInfo* pLeft = a.popFrontGuildMemberInfoList();
        GuildMemberInfo* pRight = b.popFrontGuildMemberInfoList();
        ASSERT_TRUE(pLeft != NULL);
        ASSERT_TRUE(pRight != NULL);
        EXPECT_EQ(pLeft->getName(), pRight->getName()) << "member " << i;
        EXPECT_EQ((int)pLeft->getRank(), (int)pRight->getRank()) << "member " << i;
        EXPECT_EQ(pLeft->getLogOn(), pRight->getLogOn()) << "member " << i;
        EXPECT_EQ(pLeft->getServerID(), pRight->getServerID()) << "member " << i;
        delete pLeft;
        delete pRight;
    }
}

GUILD_PACKET_TESTS(GCGuildMemberList)
GUILD_PACKET_VARIANT(GCGuildMemberList, empty, fillEmpty)

SingleGuildUnionOffer* makeUnionOffer(GuildID_t id, BYTE type, const std::string& name, const std::string& master,
                                      DWORD date) {
    SingleGuildUnionOffer* pOffer = new SingleGuildUnionOffer();
    pOffer->setGuildID(id);
    pOffer->setGuildType(type);
    pOffer->setGuildName(name);
    pOffer->setGuildMaster(master);
    pOffer->setDate(date);
    return pOffer;
}

void fill(GCUnionOfferList& packet) {
    // The offer type is the JOIN/QUIT enumerator the record declares.
    packet.addUnionOfferList(
        makeUnionOffer(0xAECF, SingleGuildUnionOffer::JOIN, "UnionGuildNmA", "UnionMasterA", 0xB0C1D2E3));
    packet.addUnionOfferList(
        makeUnionOffer(0xB2E5, SingleGuildUnionOffer::QUIT, "UnionGuildNmB", "UnionMasterB", 0xB4F6A8B9));
}

void fillEmpty(GCUnionOfferList&) {}

void expectEqual(const GCUnionOfferList& a, const GCUnionOfferList& b) {
    const std::list<SingleGuildUnionOffer*> left = a.getUnionOfferList();
    const std::list<SingleGuildUnionOffer*> right = b.getUnionOfferList();
    ASSERT_EQ(left.size(), right.size());

    std::list<SingleGuildUnionOffer*>::const_iterator iLeft = left.begin();
    std::list<SingleGuildUnionOffer*>::const_iterator iRight = right.begin();
    for (int i = 0; iLeft != left.end(); ++iLeft, ++iRight, i++) {
        ASSERT_TRUE(*iLeft != NULL);
        ASSERT_TRUE(*iRight != NULL);
        EXPECT_EQ((*iLeft)->getGuildID(), (*iRight)->getGuildID()) << "offer " << i;
        EXPECT_EQ((int)(*iLeft)->getGuildType(), (int)(*iRight)->getGuildType()) << "offer " << i;
        EXPECT_EQ((*iLeft)->getGuildName(), (*iRight)->getGuildName()) << "offer " << i;
        EXPECT_EQ((*iLeft)->getGuildMaster(), (*iRight)->getGuildMaster()) << "offer " << i;
        EXPECT_EQ((*iLeft)->getDate(), (*iRight)->getDate()) << "offer " << i;
    }
}

GUILD_PACKET_TESTS(GCUnionOfferList)
GUILD_PACKET_VARIANT(GCUnionOfferList, empty, fillEmpty)

//////////////////////////////////////////////////////////////////////
// The registration page, whose founding members are a list of names.
//////////////////////////////////////////////////////////////////////

void fill(GCShowWaitGuildInfo& packet) {
    packet.setGuildID(0xB6A7);
    packet.setGuildName("GuildWaitPage");
    packet.setGuildState(0xB8);
    packet.setGuildMaster("GuildWaitMst");
    packet.setGuildMemberCount(0xB9);
    packet.setGuildIntro("guild waiting for registration");
    packet.setJoinFee(0xBACBDCED);
    packet.addMember("WaitMemberA");
    packet.addMember("WaitMemberB");
}

void expectEqual(GCShowWaitGuildInfo& a, GCShowWaitGuildInfo& b) {
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getGuildName(), b.getGuildName());
    EXPECT_EQ((int)a.getGuildState(), (int)b.getGuildState());
    EXPECT_EQ(a.getGuildMaster(), b.getGuildMaster());
    EXPECT_EQ((int)a.getGuildMemberCount(), (int)b.getGuildMemberCount());
    EXPECT_EQ(a.getGuildIntro(), b.getGuildIntro());
    EXPECT_EQ(a.getJoinFee(), b.getJoinFee());
    ASSERT_EQ((int)a.getMemberNum(), (int)b.getMemberNum());

    const int members = (int)a.getMemberNum();
    for (int i = 0; i < members; i++)
        EXPECT_EQ(a.popMember(), b.popMember()) << "member " << i;
}

GUILD_PACKET_TESTS(GCShowWaitGuildInfo)

//////////////////////////////////////////////////////////////////////
// The bounds and the counts.
//////////////////////////////////////////////////////////////////////

// The expiry date goes on the wire behind its own length byte, and
// GuildInfo::getSize() counts both, so the size GCActiveGuildList
// declares is the body it sends. The shared size pin covers the
// canonical table; this one isolates the date.
TEST(GCActiveGuildListTest, packetSizeCountsEveryGuildsExpireDate) {
    GCActiveGuildList withDates;
    fill(withDates);

    GCActiveGuildList withoutDates;
    withoutDates.addGuildInfo(makeGuildInfo(0xA4BF, "GuildListNameA", "GuildMasterA", 0xA5, ""));
    withoutDates.addGuildInfo(makeGuildInfo(0xA6D1, "GuildListNameB", "GuildMasterB", 0xA7, ""));

    // The canonical table holds one ten-character date and one empty one.
    EXPECT_EQ(withoutDates.getPacketSize() + 10, withDates.getPacketSize());
    EXPECT_EQ((size_t)withDates.getPacketSize(), writeBody(withDates, kPlainCode).size());
}

// The count GCActiveGuildList puts on the wire is a WORD, and so is the
// one getListNum() hands its caller; the table the factory max budgets
// is 5000 guilds.
TEST(GCActiveGuildListTest, listNumIsTheWordCountOnTheWire) {
    const int kGuilds = 256;

    GCActiveGuildList packet;
    for (int i = 0; i < kGuilds; i++)
        packet.addGuildInfo(
            makeGuildInfo((GuildID_t)(0x8000 + i), "GuildListNm", "GuildMastr", (BYTE)(0x81 + i % 0x7F), ""));

    EXPECT_EQ(kGuilds, (int)packet.getListNum());

    GCWaitGuildList waiting;
    for (int i = 0; i < kGuilds; i++)
        waiting.addGuildInfo(
            makeGuildInfo((GuildID_t)(0x8000 + i), "GuildWaitNm", "GuildMastr", (BYTE)(0x81 + i % 0x7F), ""));

    EXPECT_EQ(kGuilds, (int)waiting.getListNum());
}

// The roster stops where the factory max does: 220 members of a
// full-width name, a rank, a log-on flag and the server each is on,
// behind the list type and the count byte.
TEST(GCGuildMemberListTest, aRosterPastTheFactoryBudgetIsRefused) {
    const std::string name(20, 'm');

    GCGuildMemberListFactory factory;
    GCGuildMemberList exactFit;
    exactFit.setType(0x93);
    for (uint i = 0; i < GuildMemberInfo::kMaxCount; i++)
        exactFit.addGuildMemberInfo(
            makeGuildMemberInfo(name, (BYTE)(0x81 + i % 0x7F), (i % 2) == 0, (ServerID_t)(0x8000 + i)));

    EXPECT_EQ(factory.getPacketMaxSize(), exactFit.getPacketSize())
        << "a full roster of full-width members is exactly the factory max";
    EXPECT_EQ((size_t)exactFit.getPacketSize(), writeBody(exactFit, kPlainCode).size());

    EXPECT_THROW(exactFit.addGuildMemberInfo(makeGuildMemberInfo(name, 0x99, true, 0x9ABC)), InvalidProtocolException);
    EXPECT_EQ((int)GuildMemberInfo::kMaxCount, (int)exactFit.getListNum());
    EXPECT_EQ(factory.getPacketMaxSize(), exactFit.getPacketSize());
}

// The offer list stops at the twenty offers the factory max budgets,
// count byte included.
TEST(GCUnionOfferListTest, aFullListFitsTheFactoryMaxAndOneMoreIsRefused) {
    const std::string name(30, 'u');
    const std::string master(20, 'M');

    GCUnionOfferListFactory factory;
    GCUnionOfferList exactFit;
    for (uint i = 0; i < SingleGuildUnionOffer::kMaxCount; i++)
        exactFit.addUnionOfferList(makeUnionOffer((GuildID_t)(0x8000 + i), SingleGuildUnionOffer::JOIN, name, master,
                                                  (DWORD)(0x81A2B3C4 + i)));

    EXPECT_EQ(factory.getPacketMaxSize(), exactFit.getPacketSize())
        << "a full list of full-width offers is exactly the factory max";
    EXPECT_EQ((size_t)exactFit.getPacketSize(), writeBody(exactFit, kPlainCode).size());

    EXPECT_THROW(
        exactFit.addUnionOfferList(makeUnionOffer(0x9999, SingleGuildUnionOffer::QUIT, name, master, 0x9AABBCCD)),
        InvalidProtocolException);
    EXPECT_EQ((size_t)SingleGuildUnionOffer::kMaxCount, exactFit.getUnionOfferList().size());
    EXPECT_EQ(factory.getPacketMaxSize(), exactFit.getPacketSize());
}

// An introduction goes on the wire behind a BYTE length, so
// GUILD_INTRO_MAX_LENGTH is both what the length can express and what
// every carrier's factory max budgets. Each setter cuts to it and each
// write() refuses past it, and `size` is what the packet declares once
// the value is cut.
void expectIntroIsCut(Packet& packet, PacketSize_t size, const std::string& cut, const char* what) {
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size()) << what;
    EXPECT_EQ(size, packet.getPacketSize()) << what;
    EXPECT_EQ(GUILD_INTRO_MAX_LENGTH, cut.size()) << what << ": the setter cuts to the budgeted width";
}

// The six client-facing introductions, plus the registration page's.
TEST(GuildBoundsTest, introsAreCutToTheWidthTheLengthByteAndTheFactoryMaxAllow) {
    const std::string intro(300, 'i');

    CGJoinGuild joinGuild;
    fill(joinGuild);
    joinGuild.setGuildMemberIntro(intro);
    expectIntroIsCut(joinGuild, szGuildID + szGuildMemberRank + szBYTE + GUILD_INTRO_MAX_LENGTH,
                     joinGuild.getGuildMemberIntro(), "CGJoinGuild");

    CGModifyGuildIntro modifyIntro;
    fill(modifyIntro);
    modifyIntro.setGuildIntro(intro);
    expectIntroIsCut(modifyIntro, szGuildID + szBYTE + GUILD_INTRO_MAX_LENGTH, modifyIntro.getGuildIntro(),
                     "CGModifyGuildIntro");

    CGModifyGuildMemberIntro modifyMemberIntro;
    fill(modifyMemberIntro);
    modifyMemberIntro.setGuildMemberIntro(intro);
    expectIntroIsCut(modifyMemberIntro, szGuildID + szBYTE + GUILD_INTRO_MAX_LENGTH,
                     modifyMemberIntro.getGuildMemberIntro(), "CGModifyGuildMemberIntro");

    CGRegistGuild registGuild;
    fill(registGuild);
    registGuild.setGuildIntro(intro);
    expectIntroIsCut(registGuild, szBYTE + registGuild.getGuildName().size() + szBYTE + GUILD_INTRO_MAX_LENGTH,
                     registGuild.getGuildIntro(), "CGRegistGuild");

    GCShowGuildInfo showGuildInfo;
    fill(showGuildInfo);
    showGuildInfo.setGuildIntro(intro);
    expectIntroIsCut(showGuildInfo,
                     szGuildID + szBYTE + showGuildInfo.getGuildName().size() + szGuildState + szBYTE +
                         showGuildInfo.getGuildMaster().size() + szBYTE + szBYTE + GUILD_INTRO_MAX_LENGTH + szGold,
                     showGuildInfo.getGuildIntro(), "GCShowGuildInfo");

    GCShowGuildMemberInfo showMemberInfo;
    fill(showMemberInfo);
    showMemberInfo.setGuildMemberIntro(intro);
    expectIntroIsCut(showMemberInfo,
                     szGuildID + szBYTE + showMemberInfo.getName().size() + szGuildMemberRank + szBYTE +
                         GUILD_INTRO_MAX_LENGTH,
                     showMemberInfo.getGuildMemberIntro(), "GCShowGuildMemberInfo");

    GCShowWaitGuildInfo showWaitInfo;
    fill(showWaitInfo);
    showWaitInfo.setGuildIntro(intro);
    EXPECT_EQ(GUILD_INTRO_MAX_LENGTH, showWaitInfo.getGuildIntro().size());
    EXPECT_EQ((size_t)showWaitInfo.getPacketSize(), writeBody(showWaitInfo, kPlainCode).size());

    GCShowWaitGuildInfoFactory factory;
    EXPECT_LE(showWaitInfo.getPacketSize(), factory.getPacketMaxSize());
}

// GCGuildChat refuses a sender past 10 characters and a message past
// 128; the sending guild's name is held to the 20 the factory max
// budgets the same way. It is a guild name, so it is refused rather than
// cut, and a union line always names the guild it came from.
TEST(GCGuildChatTest, theSendingGuildNameIsBounded) {
    GCGuildChat packet;
    fill(packet);

    packet.setSendGuildName(std::string(GUILD_NAME_MAX_LENGTH + 1, 'g'));
    EXPECT_THROW(writeBody(packet, kPlainCode), InvalidProtocolException);

    packet.setSendGuildName("");
    EXPECT_THROW(writeBody(packet, kPlainCode), InvalidProtocolException);

    packet.setSendGuildName(std::string(GUILD_NAME_MAX_LENGTH, 'g'));
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());

    // The guild channel carries no guild name at all, so the empty value
    // that the union channel refuses is what it writes.
    packet.setType(0);
    packet.setSendGuildName("");
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());
}

// Both response packets keep their code in a WORD, put a WORD on the
// wire and hand the caller the same WORD. GCNPCResponse declares 139
// dialogue codes, so the value can pass 255.
TEST(GuildResponseCodeTest, theCodeGettersReturnTheWordTheWireCarries) {
    const WORD code = 0x0141;

    GCGuildResponse guildResponse;
    guildResponse.setCode(code);
    guildResponse.setParameter(0x95A6B7C8);
    EXPECT_EQ((int)code, (int)guildResponse.getCode());

    GCNPCResponse npcResponse;
    npcResponse.setCode(code);
    EXPECT_EQ((int)code, (int)npcResponse.getCode());

    GCGuildResponse dst;
    roundTrip(guildResponse, dst, kPlainCode);
    EXPECT_EQ((int)code, (int)dst.getCode());
    EXPECT_EQ(guildResponse.getParameter(), dst.getParameter());
    EXPECT_EQ((size_t)(szWORD + szuint), writeBody(guildResponse, kPlainCode).size());
}

} // namespace
