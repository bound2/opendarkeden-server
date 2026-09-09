//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_guild_test.cpp
// Description : Golden byte fixtures, loopback round trips and size
//               pins for the guild protocol: every CG and GC packet a
//               client and a game server exchange to found a guild,
//               join or leave one, browse its roster and talk on its
//               channel. Twenty-three packets, each with the reason it
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
//               Two packets get the golden and the size pins without
//               the round trip: GCModifyGuildMemberInfo and
//               GCOtherGuildName cannot be read back at all, for the
//               reason recorded under the findings below. write() is
//               the pinned contract for both.
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
//               Open write/read disagreements are stated as tests that
//               fail once they are fixed:
//
//               - GuildInfo::getSize() counts neither the length byte
//                 its write() emits for the expiry date nor the date
//                 itself, so GCActiveGuildList declares a body shorter
//                 than the one it sends, by one byte plus the date for
//                 every guild in the table.
//               - GCActiveGuildList and GCGuildMemberList read their
//                 lists front-first against a write() that walks front
//                 to back, so both tables arrive reversed.
//               - GCActiveGuildList::getListNum() returns a BYTE while
//                 the count on the wire is a WORD, so a table past 255
//                 guilds reports a truncated count to its own caller.
//               - GCGuildMemberList caps its roster nowhere, and
//                 GuildMemberInfo::getMaxSize() budgets 220 members
//                 without counting the ServerID each of them carries,
//                 so 203 full-width members already outgrow the read
//                 buffer the receiver sizes from the factory max.
//               - GCUnionOfferList's factory max is exactly twenty
//                 entries' worth with no room for the count byte in
//                 front of them, so a full list outgrows it by one
//                 byte, and neither side caps the list at twenty.
//               - none of the six guild or member introductions is
//                 bounded. Each writer derives a BYTE length from the
//                 string and emits the string whole; the guards that
//                 look like a cap compare a BYTE against 255, which no
//                 BYTE can exceed.
//               - GCGuildChat bounds its sender and its message but
//                 not the sending guild's name.
//               - GCGuildResponse::getCode() and
//                 GCNPCResponse::getCode() return a BYTE while the
//                 member and the wire field are a WORD, so a code past
//                 255 reaches the wire whole and the accessor whole
//                 halved.
//
//               Not expressible as a test: GCModifyGuildMemberInfo and
//               GCOtherGuildName declare an uninitialised BYTE for the
//               guild-name length, test it against 30 and against 0,
//               and only then read the length byte the stream carries.
//               So read() consumes a length that was never checked when
//               the uninitialised value happens to be non-zero, and
//               reads no length byte at all when it happens to be zero,
//               leaving that byte in the stream and taking the rank
//               from it. The outcome depends on what the allocation
//               held, so no round trip can be stated for either packet.
//               Every CG packet in this file, and GCGuildChat,
//               GCModifyGuildMemberInfo, GCOtherGuildName,
//               GCShowGuildInfo, GCShowGuildJoin and
//               GCShowGuildMemberInfo, also leave every scalar member
//               uninitialised.
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
#include "GCUnionOfferList.h"
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
// The two packets that cannot be read back.
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

GUILD_PACKET_WRITE_TESTS(GCModifyGuildMemberInfo)
GUILD_PACKET_WRITE_VARIANT(GCModifyGuildMemberInfo, noname, fillNoname)

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

GUILD_PACKET_WRITE_TESTS(GCOtherGuildName)
GUILD_PACKET_WRITE_VARIANT(GCOtherGuildName, noname, fillNoname)

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
    std::vector<GuildInfo*> left, right;
    for (int i = 0; i < guilds; i++) {
        left.push_back(a.popFrontGuildInfoList());
        right.push_back(b.popFrontGuildInfoList());
    }

    // The table comes back reversed — see guildListReversesOnARoundTrip.
    for (int i = 0; i < guilds; i++) {
        ASSERT_TRUE(left[i] != NULL);
        ASSERT_TRUE(right[guilds - 1 - i] != NULL);
        expectGuildInfoEqual(*left[i], *right[guilds - 1 - i], i);
    }

    for (int i = 0; i < guilds; i++) {
        delete left[i];
        delete right[i];
    }
}

// GCActiveGuildList does not get the shared size pin: GuildInfo::getSize()
// under-reports every guild, which packetSizeOmitsEveryGuildsExpireDate
// states below.
TEST(GCActiveGuildListTest, roundTripsThroughLoopback) {
    GCActiveGuildList src;
    fill(src);
    GCActiveGuildList dst;
    roundTrip(src, dst, kPlainCode);
    expectEqual(src, dst);
}

TEST(GCActiveGuildListTest, bodyBytesMatchGolden) {
    GCActiveGuildList packet;
    fill(packet);
    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    expectGolden("GCActiveGuildList", kPlainCode, body);
    for (size_t i = 1; i < kEncryptCodeCount; i++)
        EXPECT_EQ(body, writeBody(packet, kEncryptCodes[i]))
            << "GCActiveGuildList now varies with the encrypt code — add per-code goldens";

    GCActiveGuildListFactory factory;
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize());
    EXPECT_EQ(factory.getPacketID(), packet.getPacketID());
    EXPECT_EQ(factory.getPacketName(), packet.getPacketName());
}

GUILD_PACKET_VARIANT(GCActiveGuildList, empty, fillEmpty)

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
    std::vector<GuildMemberInfo*> left, right;
    for (int i = 0; i < members; i++) {
        left.push_back(a.popFrontGuildMemberInfoList());
        right.push_back(b.popFrontGuildMemberInfoList());
    }

    // The roster comes back reversed — see memberListReversesOnARoundTrip.
    for (int i = 0; i < members; i++) {
        GuildMemberInfo* pLeft = left[i];
        GuildMemberInfo* pRight = right[members - 1 - i];
        ASSERT_TRUE(pLeft != NULL);
        ASSERT_TRUE(pRight != NULL);
        EXPECT_EQ(pLeft->getName(), pRight->getName()) << "member " << i;
        EXPECT_EQ((int)pLeft->getRank(), (int)pRight->getRank()) << "member " << i;
        EXPECT_EQ(pLeft->getLogOn(), pRight->getLogOn()) << "member " << i;
        EXPECT_EQ(pLeft->getServerID(), pRight->getServerID()) << "member " << i;
    }

    for (int i = 0; i < members; i++) {
        delete left[i];
        delete right[i];
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
// The open disagreements.
//////////////////////////////////////////////////////////////////////

// FINDING, stated as a test that fails once it is fixed.
// GuildInfo::write() emits a length byte for the expiry date and then
// the date itself; GuildInfo::getSize() counts neither. Every guild in
// the table therefore shortens the size GCActiveGuildList declares by a
// byte plus its date, and writePacket() puts that short length on the
// wire ahead of the longer body.
TEST(GCActiveGuildListTest, packetSizeOmitsEveryGuildsExpireDate) {
    GCActiveGuildList packet;
    fill(packet);

    // The canonical table holds one ten-character date and one empty
    // one, behind a length byte each.
    const size_t omitted = (szBYTE + 10) + (szBYTE + 0);

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)packet.getPacketSize() + omitted, body.size())
        << "GuildInfo::getSize() now counts the expiry date write() emits — delete this test";
}

// FINDING, stated as a test that fails once it is fixed.
// GCActiveGuildList::write() walks its list front to back while read()
// pushes every guild it parses to the front, so the table arrives
// reversed.
TEST(GCActiveGuildListTest, guildListReversesOnARoundTrip) {
    GCActiveGuildList src;
    fill(src);

    GCActiveGuildList dst;
    roundTrip(src, dst, kPlainCode);

    GuildInfo* pSentFirst = src.popFrontGuildInfoList();
    GuildInfo* pReceivedFirst = dst.popFrontGuildInfoList();
    ASSERT_TRUE(pSentFirst != NULL);
    ASSERT_TRUE(pReceivedFirst != NULL);

    EXPECT_NE(pSentFirst->getGuildID(), pReceivedFirst->getGuildID())
        << "GCActiveGuildList::read() now preserves the order write() sent — delete this test";

    delete pSentFirst;
    delete pReceivedFirst;
}

// FINDING, stated as a test that fails once it is fixed.
// The count GCActiveGuildList puts on the wire is a WORD, and the table
// the factory max budgets is 5000 guilds, but getListNum() narrows the
// list size to a BYTE, so the caller that asks how many guilds it holds
// is told zero at 256.
TEST(GCActiveGuildListTest, listNumTruncatesTheWordCountToAByte) {
    const int kGuilds = 256;

    GCActiveGuildList packet;
    for (int i = 0; i < kGuilds; i++)
        packet.addGuildInfo(
            makeGuildInfo((GuildID_t)(0x8000 + i), "GuildListNm", "GuildMastr", (BYTE)(0x81 + i % 0x7F), ""));

    EXPECT_EQ(0, (int)packet.getListNum())
        << "GCActiveGuildList::getListNum() now returns the WORD count write() emits — delete this test";
}

// FINDING, stated as a test that fails once it is fixed.
// The same front-push in GCGuildMemberList::read() reverses the roster.
TEST(GCGuildMemberListTest, memberListReversesOnARoundTrip) {
    GCGuildMemberList src;
    fill(src);

    GCGuildMemberList dst;
    roundTrip(src, dst, kPlainCode);

    GuildMemberInfo* pSentFirst = src.popFrontGuildMemberInfoList();
    GuildMemberInfo* pReceivedFirst = dst.popFrontGuildMemberInfoList();
    ASSERT_TRUE(pSentFirst != NULL);
    ASSERT_TRUE(pReceivedFirst != NULL);

    EXPECT_NE(pSentFirst->getName(), pReceivedFirst->getName())
        << "GCGuildMemberList::read() now preserves the order write() sent — delete this test";

    delete pSentFirst;
    delete pReceivedFirst;
}

// FINDING, stated as a test that fails once it is fixed.
// GCGuildMemberListFactory::kMaxSize is one GuildMemberInfo::getMaxSize()
// plus the type byte, and that maximum budgets 220 members of a name,
// a rank and a log-on flag each — it counts one ServerID for the whole
// table rather than one per member, and leaves out the roster's own
// count byte. Nothing caps the list either, so 203 full-width members
// are written and the body outgrows the read buffer the receiver sizes
// from that maximum.
TEST(GCGuildMemberListTest, aRosterPastTheFactoryBudgetIsWrittenRatherThanRefused) {
    const int kMembers = 203;
    const std::string name(20, 'm');

    GCGuildMemberList packet;
    packet.setType(0x93);
    for (int i = 0; i < kMembers; i++)
        packet.addGuildMemberInfo(
            makeGuildMemberInfo(name, (BYTE)(0x81 + i % 0x7F), (i % 2) == 0, (ServerID_t)(0x8000 + i)));

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCGuildMemberListFactory factory;
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCGuildMemberList now fits the roster its factory max budgets — delete this test";
}

// FINDING, stated as a test that fails once it is fixed.
// GCUnionOfferListFactory::kMaxSize is exactly twenty offers' worth and
// budgets nothing for the count byte write() puts in front of them, so
// a full list of twenty already outgrows it. Nothing caps the list at
// twenty either.
TEST(GCUnionOfferListTest, aFullListOutgrowsTheFactoryMax) {
    const int kOffers = 20;
    const std::string name(30, 'u');
    const std::string master(20, 'M');

    GCUnionOfferList packet;
    for (int i = 0; i < kOffers; i++)
        packet.addUnionOfferList(makeUnionOffer((GuildID_t)(0x8000 + i), SingleGuildUnionOffer::JOIN, name, master,
                                                (DWORD)(0x81A2B3C4 + i)));

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCUnionOfferListFactory factory;
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCUnionOfferList's factory max now budgets its count byte — delete this test";
}

// Each introduction goes on the wire behind a BYTE length that write()
// derives from the string and emits unchecked. `offset` names the byte
// that length lands on, so the wrap is read from the body rather than
// inferred.
void expectUnboundedIntro(const Packet& packet, PacketSize_t maxSize, size_t length, size_t offset, const char* what) {
    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size()) << what;
    EXPECT_GT(packet.getPacketSize(), maxSize) << what << " is now bounded — delete this case";
    ASSERT_LT(offset, body.size()) << what;
    EXPECT_EQ((int)(BYTE)length, (int)body[offset]) << what << ": the length byte no longer wraps";
}

// FINDING, stated as a test that fails once it is fixed.
// None of the six writers that carry a guild or member introduction
// bounds it. Each derives a BYTE length from the string and then emits
// the string whole, so past 255 characters the length byte wraps and
// the reader stops mid-text, and the body outgrows the read buffer the
// receiver sizes from the factory max. The guards that look like a cap
// (`> 255` in CGModifyGuildIntro and CGModifyGuildMemberIntro) compare
// a BYTE against a value no BYTE can exceed, so they never fire.
// CGRegistGuild, GCShowGuildInfo and GCShowGuildMemberInfo do bound
// their names; only the introduction is open.
TEST(GuildBoundsTest, unboundedIntrosWrapTheLengthByteAndOutgrowTheFactoryMax) {
    const size_t kLength = 300;
    const std::string intro(kLength, 'i');

    CGJoinGuild joinGuild;
    fill(joinGuild);
    joinGuild.setGuildMemberIntro(intro);
    expectUnboundedIntro(joinGuild, CGJoinGuildFactory::kMaxSize, kLength, szGuildID + szGuildMemberRank,
                         "CGJoinGuild");

    CGModifyGuildIntro modifyIntro;
    fill(modifyIntro);
    modifyIntro.setGuildIntro(intro);
    expectUnboundedIntro(modifyIntro, CGModifyGuildIntroFactory::kMaxSize, kLength, szGuildID, "CGModifyGuildIntro");

    CGModifyGuildMemberIntro modifyMemberIntro;
    fill(modifyMemberIntro);
    modifyMemberIntro.setGuildMemberIntro(intro);
    expectUnboundedIntro(modifyMemberIntro, CGModifyGuildMemberIntroFactory::kMaxSize, kLength, szGuildID,
                         "CGModifyGuildMemberIntro");

    CGRegistGuild registGuild;
    fill(registGuild);
    registGuild.setGuildIntro(intro);
    expectUnboundedIntro(registGuild, CGRegistGuildFactory::kMaxSize, kLength,
                         szBYTE + registGuild.getGuildName().size(), "CGRegistGuild");

    GCShowGuildInfo showGuildInfo;
    fill(showGuildInfo);
    showGuildInfo.setGuildIntro(intro);
    expectUnboundedIntro(showGuildInfo, GCShowGuildInfoFactory::kMaxSize, kLength,
                         szGuildID + szBYTE + showGuildInfo.getGuildName().size() + szGuildState + szBYTE +
                             showGuildInfo.getGuildMaster().size() + szBYTE,
                         "GCShowGuildInfo");

    GCShowGuildMemberInfo showMemberInfo;
    fill(showMemberInfo);
    showMemberInfo.setGuildMemberIntro(intro);
    expectUnboundedIntro(showMemberInfo, GCShowGuildMemberInfoFactory::kMaxSize, kLength,
                         szGuildID + szBYTE + showMemberInfo.getName().size() + szGuildMemberRank,
                         "GCShowGuildMemberInfo");
}

// FINDING, stated as a test that fails once it is fixed.
// GCGuildChat refuses a sender past 10 characters and a message past
// 128, on both sides, but writes the sending guild's name with no check
// at all — while the factory max budgets 20 for it.
TEST(GCGuildChatTest, theSendingGuildNameIsNotBounded) {
    const size_t kLength = 300;

    GCGuildChat packet;
    fill(packet);
    packet.setSendGuildName(std::string(kLength, 'g'));

    expectUnboundedIntro(packet, GCGuildChatFactory::kMaxSize, kLength, szBYTE, "GCGuildChat sending guild name");
}

// FINDING, stated as a test that fails once it is fixed.
// Both response packets keep their code in a WORD, put a WORD on the
// wire and hand the caller a BYTE, so a code past 255 is delivered
// whole and read back halved. GCNPCResponse already declares 139
// dialogue codes.
TEST(GuildResponseCodeTest, theCodeGettersTruncateTheWordTheWireCarries) {
    const WORD code = 0x0141;

    GCGuildResponse guildResponse;
    guildResponse.setCode(code);
    guildResponse.setParameter(0x95A6B7C8);
    EXPECT_EQ((int)(BYTE)code, (int)guildResponse.getCode())
        << "GCGuildResponse::getCode() now returns the WORD it holds — delete this case";

    GCNPCResponse npcResponse;
    npcResponse.setCode(code);
    EXPECT_EQ((int)(BYTE)code, (int)npcResponse.getCode())
        << "GCNPCResponse::getCode() now returns the WORD it holds — delete this case";

    // The wire is not the problem: the WORD survives the round trip, so
    // the loss is in the accessor alone.
    GCGuildResponse dst;
    roundTrip(guildResponse, dst, kPlainCode);
    EXPECT_EQ(guildResponse.getParameter(), dst.getParameter());
    EXPECT_EQ((size_t)(szWORD + szuint), writeBody(guildResponse, kPlainCode).size());
}

} // namespace
