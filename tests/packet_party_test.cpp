//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_party_test.cpp
// Description : Golden byte fixtures, loopback round trips and size
//               pins for the party protocol: every CG and GC packet a
//               client and a game server exchange to form a party,
//               leave one, and keep the members' chat and map markers
//               up to date. Ten packets, each with the reason it is
//               here:
//
//               CGPartyInvite    the whole invitation exchange in one
//                                packet: request, cancel, accept,
//                                reject and busy are the five codes it
//                                carries next to the other player's
//                                object id.
//               GCPartyInvite    the server's half of the same
//                                exchange, with two codes the client
//                                cannot send: the target is already in
//                                a party, and the party is full.
//               GCPartyError     the refusal that answers an invite or
//                                an expulsion the server will not act
//                                on: a missing target, a different
//                                race, an unsafe zone, a bat or wolf
//                                form, a double invitation, a reply
//                                with no invitation outstanding, and a
//                                missing expulsion right.
//               GCPartyJoined    what every member receives once the
//                                party changes: the whole roster, one
//                                record per member.
//               CGPartyLeave     the client asking to leave, or to
//                                expel the member it names.
//               GCPartyLeave     the answer both the leaver and the
//                                rest receive, naming who left and,
//                                when it was an expulsion, who did it.
//               CGPartyPosition  the position and health a member
//                                reports so the others can draw it.
//               GCPartyPosition  the same reading forwarded to the
//                                other members, with the member's name
//                                in front of it.
//               CGPartySay       a line of party chat with the colour
//                                the sender chose.
//               GCPartySay       that line forwarded to the members,
//                                with the sender's name in front.
//
//               No packet in this file calls readEncrypt/writeEncrypt,
//               so the goldens are recorded at encrypt code 0 only, and
//               every golden test also asserts the bytes do not vary
//               with the code, so adopting the encrypter fails loudly
//               instead of silently voiding the pin.
//
//               Each packet gets three pins (PARTY_PACKET_TESTS):
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
//               Three packets have a branch one fixture cannot reach,
//               and take a second golden: GCPartyJoined.empty is the
//               zero-member roster, GCPartyJoined's canonical fixture
//               carries a member with an empty name so both sides of
//               the name-length branch run, and GCPartyLeave.left is
//               the empty-expeller shape that says the member left on
//               its own rather than being expelled.
//
//               Fixture values are distinct per field and >= 128 in
//               every byte the width allows. Two groups cannot follow
//               that rule and say so at the point of use: the code
//               bytes, which hold a handful of small enumerators, and
//               the names, which each packet caps well below 128
//               characters.
//
//               Open write/read disagreements are stated as tests that
//               fail once they are fixed:
//
//               - no name or message in this protocol is bounded. The
//                 six strings each go on the wire behind a BYTE length
//                 derived from the string itself, so past the width the
//                 factory max budgets the body outgrows the receiver's
//                 read buffer, and past 255 characters the length byte
//                 wraps and the reader stops mid-text.
//               - GCPartyJoined does not cap its roster at the six
//                 members its factory max budgets, and its member count
//                 is a BYTE that wraps at 256.
//
//               Not expressible as a test: CGPartyInvite,
//               CGPartyPosition, CGPartySay, GCPartyError,
//               GCPartyInvite, GCPartyPosition and GCPartySay leave
//               every scalar member uninitialised. A packet that is
//               written without every setter being called puts whatever
//               the allocation held on the wire, and no test can pin
//               that value.
//
//////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "CGPartyInvite.h"
#include "CGPartyLeave.h"
#include "CGPartyPosition.h"
#include "CGPartySay.h"
#include "GCPartyError.h"
#include "GCPartyInvite.h"
#include "GCPartyJoined.h"
#include "GCPartyLeave.h"
#include "GCPartyPosition.h"
#include "GCPartySay.h"
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

#define PARTY_PACKET_TESTS(Name)                                                                     \
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

// A second fixture for a packet whose write() has a branch the canonical
// one does not take.
#define PARTY_PACKET_VARIANT(Name, Variant, fillVariant)                       \
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
// The invitation exchange.
//////////////////////////////////////////////////////////////////////

void fill(CGPartyInvite& packet) {
    packet.setTargetObjectID(0x8A9BACBD);
    // The code is an enum byte with five enumerators, so it carries a
    // valid one rather than a high byte.
    packet.setCode(CG_PARTY_INVITE_ACCEPT);
}

void expectEqual(const CGPartyInvite& a, const CGPartyInvite& b) {
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getCode(), b.getCode());
}

PARTY_PACKET_TESTS(CGPartyInvite)

void fill(GCPartyInvite& packet) {
    packet.setTargetObjectID(0x8C9DAEBF);
    // Seven enumerators here: the server adds "already in another
    // party" and "party full" to the five the client can send.
    packet.setCode(GC_PARTY_INVITE_MEMBER_FULL);
}

void expectEqual(const GCPartyInvite& a, const GCPartyInvite& b) {
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getCode(), b.getCode());
}

PARTY_PACKET_TESTS(GCPartyInvite)

void fill(GCPartyError& packet) {
    packet.setTargetObjectID(0x8D9EAFC0);
    packet.setCode(GC_PARTY_ERROR_NO_AUTHORITY);
}

void expectEqual(const GCPartyError& a, const GCPartyError& b) {
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getCode(), b.getCode());
}

PARTY_PACKET_TESTS(GCPartyError)

//////////////////////////////////////////////////////////////////////
// The roster.
//////////////////////////////////////////////////////////////////////

// GCPartyJoined takes ownership of every record it is handed, so each
// fixture allocates fresh ones.
PARTY_MEMBER_INFO* makeMemberInfo(const std::string& name, BYTE sex, BYTE hairStyle, IP_t ip) {
    PARTY_MEMBER_INFO* pInfo = new PARTY_MEMBER_INFO;
    pInfo->name = name;
    pInfo->sex = sex;
    pInfo->hair_style = hairStyle;
    pInfo->ip = ip;
    return pInfo;
}

void fill(GCPartyJoined& packet) {
    // The sex byte holds the creature's own two-valued enumerator, so it
    // carries one of those rather than a high byte. The third record
    // takes the empty-name branch of write(), which the other two do
    // not.
    packet.addMemberInfo(makeMemberInfo("PtyMemberA", 0, 0x91, 0x92A3B4C5));
    packet.addMemberInfo(makeMemberInfo("PtyMemberB", 1, 0x93, 0x94A5B6C7));
    packet.addMemberInfo(makeMemberInfo("", 1, 0x95, 0x96A7B8C9));
}

void fillEmpty(GCPartyJoined&) {}

void expectEqual(GCPartyJoined& a, GCPartyJoined& b) {
    ASSERT_EQ(a.getMemberInfoCount(), b.getMemberInfoCount());

    // The roster is reachable only through the destructive
    // popMemberInfo(), which hands the record over; nothing reads the
    // packets afterwards.
    const int members = (int)a.getMemberInfoCount();
    for (int i = 0; i < members; i++) {
        PARTY_MEMBER_INFO* pLeft = a.popMemberInfo();
        PARTY_MEMBER_INFO* pRight = b.popMemberInfo();
        ASSERT_TRUE(pLeft != NULL);
        ASSERT_TRUE(pRight != NULL);
        EXPECT_EQ(pLeft->name, pRight->name) << "member " << i;
        EXPECT_EQ(pLeft->sex, pRight->sex) << "member " << i;
        EXPECT_EQ(pLeft->hair_style, pRight->hair_style) << "member " << i;
        EXPECT_EQ(pLeft->ip, pRight->ip) << "member " << i;
        delete pLeft;
        delete pRight;
    }
}

PARTY_PACKET_TESTS(GCPartyJoined)
PARTY_PACKET_VARIANT(GCPartyJoined, empty, fillEmpty)

//////////////////////////////////////////////////////////////////////
// Leaving and expelling.
//////////////////////////////////////////////////////////////////////

void fill(CGPartyLeave& packet) {
    packet.setTargetName("PtyLeaveTg");
}

void expectEqual(const CGPartyLeave& a, const CGPartyLeave& b) {
    EXPECT_EQ(a.getTargetName(), b.getTargetName());
}

PARTY_PACKET_TESTS(CGPartyLeave)

void fill(GCPartyLeave& packet) {
    packet.setExpeller("PtyExpellr");
    packet.setExpellee("PtyExpelle");
}

// The member left on its own: the expeller is empty, which is the other
// side of both length branches of write().
void fillLeft(GCPartyLeave& packet) {
    packet.setExpeller("");
    packet.setExpellee("PtyQuitter");
}

void expectEqual(const GCPartyLeave& a, const GCPartyLeave& b) {
    EXPECT_EQ(a.getExpeller(), b.getExpeller());
    EXPECT_EQ(a.getExpellee(), b.getExpellee());
}

PARTY_PACKET_TESTS(GCPartyLeave)
PARTY_PACKET_VARIANT(GCPartyLeave, left, fillLeft)

//////////////////////////////////////////////////////////////////////
// Position and chat.
//////////////////////////////////////////////////////////////////////

void fill(CGPartyPosition& packet) {
    packet.setZoneID(0x819C);
    packet.setXY(0x83A4, 0x85B6);
    packet.setMaxHP(0x87C8);
    packet.setHP(0x89DA);
}

void expectEqual(const CGPartyPosition& a, const CGPartyPosition& b) {
    EXPECT_EQ(a.getZoneID(), b.getZoneID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getMaxHP(), b.getMaxHP());
    EXPECT_EQ(a.getHP(), b.getHP());
}

PARTY_PACKET_TESTS(CGPartyPosition)

void fill(GCPartyPosition& packet) {
    packet.setName("PtyPositionName");
    packet.setZoneID(0x8BEC);
    packet.setXY(0x8DFE, 0x8F10);
    packet.setMaxHP(0x9122);
    packet.setHP(0x9334);
}

void expectEqual(const GCPartyPosition& a, const GCPartyPosition& b) {
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getZoneID(), b.getZoneID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getMaxHP(), b.getMaxHP());
    EXPECT_EQ(a.getHP(), b.getHP());
}

PARTY_PACKET_TESTS(GCPartyPosition)

void fill(CGPartySay& packet) {
    packet.setColor(0x9546B7C8);
    packet.setMessage("party chat from the client");
}

void expectEqual(const CGPartySay& a, const CGPartySay& b) {
    EXPECT_EQ(a.getColor(), b.getColor());
    EXPECT_EQ(a.getMessage(), b.getMessage());
}

PARTY_PACKET_TESTS(CGPartySay)

void fill(GCPartySay& packet) {
    packet.setName("PtySayName");
    packet.setColor(0x97D8E9FA);
    packet.setMessage("party chat forwarded to the members");
}

void expectEqual(const GCPartySay& a, const GCPartySay& b) {
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getColor(), b.getColor());
    EXPECT_EQ(a.getMessage(), b.getMessage());
}

PARTY_PACKET_TESTS(GCPartySay)

//////////////////////////////////////////////////////////////////////
// The open disagreements.
//////////////////////////////////////////////////////////////////////

// Every string in this protocol goes on the wire behind a BYTE length
// that write() derives from the string and emits unchecked. `offset`
// names the byte that length lands on, so the wrap is read from the body
// rather than inferred.
void expectUnboundedString(const Packet& packet, PacketSize_t maxSize, size_t length, size_t offset, const char* what) {
    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size()) << what;
    EXPECT_GT(packet.getPacketSize(), maxSize) << what << " is now bounded — delete this case";
    ASSERT_LT(offset, body.size()) << what;
    EXPECT_EQ((int)(BYTE)length, (int)body[offset]) << what << ": the length byte no longer wraps";
}

// FINDING, stated as a test that fails once it is fixed.
// None of the six strings the party protocol carries is bounded on
// either side. Each writer derives a BYTE length from the string and
// then emits the string whole, so past 255 characters the length byte
// wraps and the reader stops mid-text, and well before that the body
// outgrows the read buffer the receiver sizes from the factory max: the
// names are budgeted at 10 or 20 characters and the chat messages at
// 128.
TEST(PartyBoundsTest, unboundedNamesAndMessagesWrapTheLengthByteAndOutgrowTheFactoryMax) {
    const size_t kLength = 300;
    const std::string oversized(kLength, 'p');

    CGPartyLeave leave;
    leave.setTargetName(oversized);
    expectUnboundedString(leave, CGPartyLeaveFactory::kMaxSize, kLength, 0, "CGPartyLeave target name");

    GCPartyLeave partyLeave;
    partyLeave.setExpeller(oversized);
    partyLeave.setExpellee("PtyExpelle");
    expectUnboundedString(partyLeave, GCPartyLeaveFactory::kMaxSize, kLength, 0, "GCPartyLeave expeller");

    GCPartyPosition position;
    fill(position);
    position.setName(oversized);
    expectUnboundedString(position, GCPartyPositionFactory::kMaxSize, kLength, 0, "GCPartyPosition name");

    CGPartySay clientSay;
    fill(clientSay);
    clientSay.setMessage(oversized);
    expectUnboundedString(clientSay, CGPartySayFactory::kMaxSize, kLength, szDWORD, "CGPartySay message");

    GCPartySay serverSay;
    fill(serverSay);
    serverSay.setMessage(oversized);
    expectUnboundedString(serverSay, GCPartySayFactory::kMaxSize, kLength,
                          szBYTE + serverSay.getName().size() + szDWORD, "GCPartySay message");
}

// FINDING, stated as a test that fails once it is fixed.
// GCPartyJoined's roster is a list neither addMemberInfo() nor write()
// caps. The factory max budgets six members, the size the client sees
// is the count of whatever the list holds, and the count itself is a
// BYTE that wraps at 256 while write() keeps emitting records.
TEST(GCPartyJoinedTest, aRosterPastTheFactoryBudgetIsWrittenRatherThanRefused) {
    const int kMembers = 7;

    GCPartyJoined packet;
    for (int i = 0; i < kMembers; i++)
        packet.addMemberInfo(makeMemberInfo("PtyMemberX", (BYTE)(i % 2), (BYTE)(0x81 + i), (IP_t)(0x82A3B4C5 + i)));

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());
    EXPECT_EQ(kMembers, (int)body[0]);

    GCPartyJoinedFactory factory;
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCPartyJoined now caps its roster at the six members its factory max budgets — delete this test";
}

} // namespace
