//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_movement_test.cpp
// Description : Golden byte fixtures, loopback round trips and size
//               pins for three families that share one shape: a client
//               asks the game server to move or to talk, and the server
//               answers the asker and tells the observers. The packets
//               that carry a step, the packets that end an effect, and
//               the packets that run an NPC conversation.
//
//               The set is taken from the code that sends them:
//               Zone::movePCBroadcast and Zone::moveCreatureBroadcast,
//               Zone::sendNPCInfo, the burrow/unburrow and
//               transform/untransform paths in ZoneUtil.cpp with their
//               two client requests (CGUnburrowHandler,
//               CGUntransformHandler), skill/DuplicateSelf.cpp, the
//               EffectManager tick and the Effect*::unaffect bodies
//               under src/server/gameserver, and the dialogue handlers
//               CGNPCTalkHandler and CGNPCAskAnswerHandler with the
//               quest actions that drive them. Twenty-six packets, each
//               with the reason it is here:
//
//               GCMove           the step itself, broadcast by both
//                                move-broadcast helpers to every player
//                                who can see the creature.
//               GCKnockBack      the same two helpers' packet for a step
//                                the creature did not choose: it carries
//                                both ends of the shove.
//               GCFakeMove       the decoy's step, sent by the
//                                DuplicateSelf skill.
//               CGUnburrow       the client's request to come out of a
//                                burrow, with the tile it wants.
//               GCUnburrowOK     ZoneUtil's answer to the unburrowing
//                                player: the tile it actually got.
//               GCUnburrowFail   the refusal, from the handler, from
//                                ZoneUtil and from the Unburrow skill.
//               CGUntransform    the client's request to drop a
//                                transformation; it carries no fields.
//               GCUntransformOK  ZoneUtil's answer, the tile the
//                                restored character stands on.
//               GCUntransformFail  the refusal.
//               GCAddMonsterFromBurrowing       what ZoneUtil
//               GCAddMonsterFromTransformation  broadcasts to the
//               GCAddVampireFromBurrowing       players who see the
//               GCAddVampireFromTransformation  creature reappear: the
//                                whole creature record again, because
//                                a mound or a bat is not the creature
//                                the client was drawing.
//               GCRemoveEffect   the effect lifecycle's other half: the
//                                markers ZoneUtil, the skill cures and
//                                every Effect*::unaffect take back off a
//                                creature already in view.
//               GCRemoveInjuriousCreature  what the trap and turret
//                                effects send when the thing hurting
//                                the player expires.
//               GCHPRecoveryEndToSelf   what EffectHPRecovery sends its
//                                carrier when the regeneration stops.
//               GCHPRecoveryEndToOthers the observers' copy of it.
//               GCMPRecoveryEnd  the same end-of-regeneration packet for
//                                mana, which has only the self shape.
//               GCCannotUse      what EffectLoveChain sends when the
//                                effect refuses the item.
//               CGNPCTalk        the client's request to open a
//                                conversation with an NPC.
//               GCNPCAsk         the handler's answer for a scripted
//                                question: the script the client should
//                                run.
//               GCNPCAskDynamic  the same question built at run time,
//                                carrying its subject and its choices as
//                                strings.
//               GCNPCAskVariable the scripted question plus the named
//                                values the script substitutes into it.
//               GCNPCSay         a scripted line, named by script and
//                                subject id.
//               GCNPCSayDynamic  a line built at run time; the talk
//                                handler alone sends seven of them.
//               GCNPCInfo        what Zone::sendNPCInfo sends a player:
//                                the name and tile of every NPC in the
//                                zone, so the client can label them.
//
//               Deliberately excluded:
//
//               CGMove, GCMoveOK and GCMoveError, the request half of a
//               step and its two direct answers, use the encrypter and
//               are pinned at encrypt codes 0..5 by
//               tests/packet_encrypter_test.cpp; so is CGNPCAskAnswer,
//               the client's reply to a question. GCFastMove (the
//               viewport reframe), GCSetPosition, GCAddEffect,
//               GCAddEffectToTile, GCDeleteEffectFromTile,
//               GCDeleteObject, GCAddBurrowingCreature and GCAddNPC are
//               already pinned by tests/packet_zone_scan_test.cpp and
//               tests/packet_gameserver_handshake_test.cpp; GCNPCResponse,
//               the dialogue's result code, by tests/packet_guild_test.cpp.
//               GCKnocksTargetBackOK1/2/4/5 have registered factories but
//               no sender: no source outside src/Core constructs one.
//               The store dialogue is not part of this family - the talk
//               handler builds only GCNPCAsk, GCNPCSayDynamic and
//               GCNPCResponse.
//
//               No packet here calls readEncrypt/writeEncrypt, so the
//               goldens are recorded at code 0 only, and every golden
//               test also asserts the bytes do not vary with the code, so
//               adopting the encrypter fails loudly instead of silently
//               voiding the pin.
//
//               Each packet gets three pins (MOVEMENT_PACKET_TESTS):
//
//               - a loopback round trip through the real socket and
//                 stream classes, comparing every getter;
//               - the body bytes against tests/golden/<Name>.code0.hex;
//               - getPacketSize() against the byte count write() actually
//                 emits, and against the factory's getPacketMaxSize().
//                 writePacket() puts getPacketSize() on the wire BEFORE
//                 calling write(), so a disagreement is not a wrong
//                 length, it is a stream that never resynchronises; and
//                 the receiving side sizes its read buffer from the
//                 factory max, so a body that outgrows it is a truncated
//                 packet, not a caught error.
//
//               GCNPCInfo gets the same three pins written out by hand,
//               because its records are pointers the test has to drain.
//
//               Extra goldens cover the branches one fixture cannot:
//               the two monster transition packets take their empty-name
//               and absent-effect-record branches together in .noname,
//               the two vampire ones take the absent-effect-record branch
//               in .noeffects, GCRemoveEffect and GCNPCAskVariable send
//               an empty list in .empty, GCNPCAskDynamic a question with
//               no choices in .nocontents, and GCNPCInfo a record whose
//               name is empty - which is the whole record - in
//               .emptyname.
//
//               Fixture values are distinct per field and >= 128 in every
//               byte the width allows. Three groups cannot follow that
//               rule and say so at the point of use: the direction bytes,
//               which index an eight-entry table; the names and messages,
//               which are text and which two of the packets cap well
//               below 128 characters; and the vampire record's sex, coat
//               type and outlook fields, which hold enumerators.
//
//               Beyond the three pins, the bounds each list and string
//               now keeps, one test each:
//
//               - GCNPCAskDynamic's choice count is the list and stops at
//                 fifteen, the width the factory max budgets and the most
//                 choices a script holds; an empty choice is a bare length
//                 word both sides keep.
//               - GCNPCSayDynamic's message and GCNPCAskVariable's
//                 parameters stop at the 255 their length byte can
//                 describe, and are measured whole.
//               - GCRemoveEffect's effect list is its count, stops at the
//                 255 the count byte carries and the max budgets, and
//                 read() replaces the list rather than appending to it.
//               - The two monster transition packets hold the name to the
//                 32 bytes their max budgets, the width GCAddMonster
//                 holds the same record to.
//               - DIR_NONE reaches the client through GCMove,
//                 GCUnburrowOK and GCUntransformOK, because MonsterAI
//                 computes it for a monster already on its destination;
//                 the two client requests refuse it.
//               - Every packet initialises every member it writes, pinned
//                 by constructing each over storage poisoned with two
//                 different bytes and requiring the same body.
//
//////////////////////////////////////////////////////////////////////

#include <cstring>
#include <new>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "CGMove.h"
#include "CGNPCTalk.h"
#include "CGUnburrow.h"
#include "CGUntransform.h"
#include "EffectInfo.h"
#include "Exception.h"
#include "GCAddMonsterFromBurrowing.h"
#include "GCAddMonsterFromTransformation.h"
#include "GCAddVampireFromBurrowing.h"
#include "GCAddVampireFromTransformation.h"
#include "GCCannotUse.h"
#include "GCFakeMove.h"
#include "GCHPRecoveryEndToOthers.h"
#include "GCHPRecoveryEndToSelf.h"
#include "GCKnockBack.h"
#include "GCMPRecoveryEnd.h"
#include "GCMove.h"
#include "GCNPCAsk.h"
#include "GCNPCAskDynamic.h"
#include "GCNPCAskVariable.h"
#include "GCNPCInfo.h"
#include "GCNPCSay.h"
#include "GCNPCSayDynamic.h"
#include "GCRemoveEffect.h"
#include "GCRemoveInjuriousCreature.h"
#include "GCUnburrowFail.h"
#include "GCUnburrowOK.h"
#include "GCUntransformFail.h"
#include "GCUntransformOK.h"
#include "NPCInfo.h"
#include "PCVampireInfo3.h"
#include "ScriptParameter.h"
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

//////////////////////////////////////////////////////////////////////
// The three pins every packet gets. fill() / expectEqual() are
// overloaded per packet, so the same canonical instance feeds all three.
// Both take a non-const reference: several getters are not const, and
// comparing a list consumes it.
//////////////////////////////////////////////////////////////////////

#define MOVEMENT_PACKET_GOLDEN_AND_SIZE(Name)                                                        \
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

#define MOVEMENT_PACKET_TESTS(Name)               \
    TEST(Name##Test, roundTripsThroughLoopback) { \
        Name src;                                 \
        fill(src);                                \
        Name dst;                                 \
        roundTrip(src, dst, kPlainCode);          \
        expectEqual(src, dst);                    \
    }                                             \
    MOVEMENT_PACKET_GOLDEN_AND_SIZE(Name)

// A second fixture for a packet whose write() has a branch the canonical
// one does not take.
#define MOVEMENT_PACKET_VARIANT(Name, Variant, fillVariant)                    \
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
// Shared records.
//////////////////////////////////////////////////////////////////////

// The transition packets take ownership of the EffectInfo they are
// handed, so every fixture allocates a fresh one.
EffectInfo* makeEffectInfo(int count) {
    EffectInfo* pInfo = new EffectInfo();
    for (int i = 0; i < count; i++)
        pInfo->addListElement((EffectID_t)(0x82A3 + i * 0x0101), (WORD)(0x94B5 + i * 0x0101));
    return pInfo;
}

// EffectInfo exposes its list only through the destructive
// popFrontListElement(), so comparing two of them empties both. Nothing
// reads them afterwards.
void expectEffectInfoEqual(EffectInfo* a, EffectInfo* b) {
    ASSERT_TRUE(a != NULL);
    ASSERT_TRUE(b != NULL);
    ASSERT_EQ(a->getListNum(), b->getListNum());
    EXPECT_EQ(a->getSize(), b->getSize());
    const int words = (int)a->getListNum() * 2;
    for (int i = 0; i < words; i++)
        EXPECT_EQ(a->popFrontListElement(), b->popFrontListElement()) << "effect word " << i;
}

// Sex is an enum byte with two enumerators, the coat type travels in a
// single byte and the direction indexes an eight-entry table, so none of
// the three carries a high byte.
void fillVampireInfo(PCVampireInfo3& info) {
    info.setObjectID(0x87A8B9CA);
    info.setName("MoveScanVamp");
    info.setX(0x8B);
    info.setY(0x9C);
    info.setDir(RIGHTUP);
    info.setSex(MALE);
    info.setCoatType(VAMPIRE_COAT4);
    info.setBatColor(0x83D4);
    info.setSkinColor(0x85D6);
    info.setCoatColor(0x87D8, MAIN_COLOR);
    info.setCoatColor(0x89DA, SUB_COLOR);
    info.setMasterEffectColor(0x8D);
    info.setCurrentHP(0x8EDF);
    info.setMaxHP(0x90E1);
    info.setAttackSpeed(0x92);
    info.setAlignment((Alignment_t)0x93A4B5C6);
    info.setShape(0x97);
    info.setCompetence(0x98);
    info.setGuildID(0x99AA);
    info.setUnionID(0x9BACBDCE);
    info.setRank(0x9F);
    info.setAdvancementLevel(0xA0);
}

void expectVampireInfoEqual(const PCVampireInfo3& a, const PCVampireInfo3& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
    EXPECT_EQ(a.getSex(), b.getSex());
    EXPECT_EQ(a.getCoatType(), b.getCoatType());
    EXPECT_EQ(a.getBatColor(), b.getBatColor());
    EXPECT_EQ(a.getSkinColor(), b.getSkinColor());
    EXPECT_EQ(a.getCoatColor(MAIN_COLOR), b.getCoatColor(MAIN_COLOR));
    EXPECT_EQ(a.getCoatColor(SUB_COLOR), b.getCoatColor(SUB_COLOR));
    EXPECT_EQ(a.getMasterEffectColor(), b.getMasterEffectColor());
    EXPECT_EQ(a.getCurrentHP(), b.getCurrentHP());
    EXPECT_EQ(a.getMaxHP(), b.getMaxHP());
    EXPECT_EQ(a.getAttackSpeed(), b.getAttackSpeed());
    EXPECT_EQ(a.getAlignment(), b.getAlignment());
    EXPECT_EQ(a.getShape(), b.getShape());
    EXPECT_EQ(a.getCompetence(), b.getCompetence());
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getUnionID(), b.getUnionID());
    EXPECT_EQ(a.getRank(), b.getRank());
    EXPECT_EQ(a.getAdvancementLevel(), b.getAdvancementLevel());
}

//////////////////////////////////////////////////////////////////////
// The step. The direction is an index into an eight-entry table, so it
// carries one of the eight rather than a high byte.
//////////////////////////////////////////////////////////////////////

void fill(GCMove& packet) {
    packet.setObjectID(0x81A2B3C4);
    packet.setX(0x85);
    packet.setY(0x96);
    packet.setDir(LEFTUP);
}

void expectEqual(GCMove& a, GCMove& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
}

MOVEMENT_PACKET_TESTS(GCMove)

void fill(GCKnockBack& packet) {
    packet.setObjectID(0x82A3B4C5);
    packet.setOrigin(0x83D4, 0x85D6);
    packet.setTarget(0x87D8, 0x89DA);
}

void expectEqual(GCKnockBack& a, GCKnockBack& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getOriginX(), b.getOriginX());
    EXPECT_EQ(a.getOriginY(), b.getOriginY());
    EXPECT_EQ(a.getTargetX(), b.getTargetX());
    EXPECT_EQ(a.getTargetY(), b.getTargetY());
}

MOVEMENT_PACKET_TESTS(GCKnockBack)

// The decoy's destination only: the five-argument constructor takes an
// origin the packet has no field for and drops it.
void fill(GCFakeMove& packet) {
    packet.setObjectID(0x83A4B5C6);
    packet.setXY(0x87, 0x98);
}

void expectEqual(GCFakeMove& a, GCFakeMove& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getToX(), b.getToX());
    EXPECT_EQ(a.getToY(), b.getToY());
}

MOVEMENT_PACKET_TESTS(GCFakeMove)

//////////////////////////////////////////////////////////////////////
// Coming out of a burrow and dropping a transformation. Four of the six
// packets carry nothing at all, so their round trip compares the only
// thing they have.
//////////////////////////////////////////////////////////////////////

void fill(CGUnburrow& packet) {
    packet.setX(0x88);
    packet.setY(0x99);
    packet.setDir(RIGHTUP);
}

void expectEqual(CGUnburrow& a, CGUnburrow& b) {
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
}

MOVEMENT_PACKET_TESTS(CGUnburrow)

void fill(GCUnburrowOK& packet) {
    packet.setX(0x8A);
    packet.setY(0x9B);
    packet.setDir(DOWN);
}

void expectEqual(GCUnburrowOK& a, GCUnburrowOK& b) {
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
}

MOVEMENT_PACKET_TESTS(GCUnburrowOK)

void fill(GCUntransformOK& packet) {
    packet.setX(0x8C);
    packet.setY(0x9D);
    packet.setDir(RIGHT);
}

void expectEqual(GCUntransformOK& a, GCUntransformOK& b) {
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
}

MOVEMENT_PACKET_TESTS(GCUntransformOK)

// The three fieldless packets. There is nothing to set and nothing to
// compare but the identity the receiver dispatches on.
void fill(CGUntransform&) {}

void expectEqual(CGUntransform& a, CGUntransform& b) {
    EXPECT_EQ(a.getPacketID(), b.getPacketID());
}

MOVEMENT_PACKET_TESTS(CGUntransform)

void fill(GCUnburrowFail&) {}

void expectEqual(GCUnburrowFail& a, GCUnburrowFail& b) {
    EXPECT_EQ(a.getPacketID(), b.getPacketID());
}

MOVEMENT_PACKET_TESTS(GCUnburrowFail)

void fill(GCUntransformFail&) {}

void expectEqual(GCUntransformFail& a, GCUntransformFail& b) {
    EXPECT_EQ(a.getPacketID(), b.getPacketID());
}

MOVEMENT_PACKET_TESTS(GCUntransformFail)

//////////////////////////////////////////////////////////////////////
// The creature that reappears. The monster name is text, so it does not
// follow the high-byte rule.
//////////////////////////////////////////////////////////////////////

void fill(GCAddMonsterFromBurrowing& packet) {
    packet.setObjectID(0x84A5B6C7);
    packet.setMonsterType(0x88D9);
    packet.setMonsterName("BurrowMonster");
    packet.setMainColor(0x8ADB);
    packet.setSubColor(0x8CDD);
    packet.setX(0x8E);
    packet.setY(0x9F);
    packet.setDir(RIGHTDOWN);
    packet.setEffectInfo(makeEffectInfo(2));
    packet.setCurrentHP(0x90E1);
    packet.setMaxHP(0x92E3);
}

// The empty-name branch, which also takes the absent-effect-record
// branch of the same write().
void fillNoname(GCAddMonsterFromBurrowing& packet) {
    fill(packet);
    delete packet.getEffectInfo();
    packet.setEffectInfo(NULL);
    packet.setMonsterName("");
}

void expectEqual(GCAddMonsterFromBurrowing& a, GCAddMonsterFromBurrowing& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getMonsterType(), b.getMonsterType());
    EXPECT_EQ(a.getMonsterName(), b.getMonsterName());
    EXPECT_EQ(a.getMainColor(), b.getMainColor());
    EXPECT_EQ(a.getSubColor(), b.getSubColor());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
    EXPECT_EQ(a.getCurrentHP(), b.getCurrentHP());
    EXPECT_EQ(a.getMaxHP(), b.getMaxHP());
    if (a.getEffectInfo() == NULL) {
        // A packet holding no record still puts an empty list on the
        // wire, so the reader always has one.
        ASSERT_TRUE(b.getEffectInfo() != NULL);
        EXPECT_EQ(0, (int)b.getEffectInfo()->getListNum());
    } else {
        expectEffectInfoEqual(a.getEffectInfo(), b.getEffectInfo());
    }
}

MOVEMENT_PACKET_TESTS(GCAddMonsterFromBurrowing)
MOVEMENT_PACKET_VARIANT(GCAddMonsterFromBurrowing, noname, fillNoname)

void fill(GCAddMonsterFromTransformation& packet) {
    packet.setObjectID(0x85A6B7C8);
    packet.setMonsterType(0x89DA);
    packet.setMonsterName("MorphMonster");
    packet.setMainColor(0x8BDC);
    packet.setSubColor(0x8DDE);
    packet.setX(0x8F);
    packet.setY(0xA0);
    packet.setDir(LEFTDOWN);
    packet.setEffectInfo(makeEffectInfo(3));
    packet.setCurrentHP(0x91E2);
    packet.setMaxHP(0x93E4);
}

void fillNoname(GCAddMonsterFromTransformation& packet) {
    fill(packet);
    delete packet.getEffectInfo();
    packet.setEffectInfo(NULL);
    packet.setMonsterName("");
}

void expectEqual(GCAddMonsterFromTransformation& a, GCAddMonsterFromTransformation& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getMonsterType(), b.getMonsterType());
    EXPECT_EQ(a.getMonsterName(), b.getMonsterName());
    EXPECT_EQ(a.getMainColor(), b.getMainColor());
    EXPECT_EQ(a.getSubColor(), b.getSubColor());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
    EXPECT_EQ(a.getCurrentHP(), b.getCurrentHP());
    EXPECT_EQ(a.getMaxHP(), b.getMaxHP());
    if (a.getEffectInfo() == NULL) {
        ASSERT_TRUE(b.getEffectInfo() != NULL);
        EXPECT_EQ(0, (int)b.getEffectInfo()->getListNum());
    } else {
        expectEffectInfoEqual(a.getEffectInfo(), b.getEffectInfo());
    }
}

MOVEMENT_PACKET_TESTS(GCAddMonsterFromTransformation)
MOVEMENT_PACKET_VARIANT(GCAddMonsterFromTransformation, noname, fillNoname)

void fill(GCAddVampireFromBurrowing& packet) {
    PCVampireInfo3 info;
    fillVampireInfo(info);
    packet.setVampireInfo(info);
    packet.setEffectInfo(makeEffectInfo(2));
}

void fillNoeffects(GCAddVampireFromBurrowing& packet) {
    fill(packet);
    delete packet.getEffectInfo();
    packet.setEffectInfo(NULL);
}

void expectEqual(GCAddVampireFromBurrowing& a, GCAddVampireFromBurrowing& b) {
    expectVampireInfoEqual(a.getVampireInfo(), b.getVampireInfo());
    if (a.getEffectInfo() == NULL) {
        ASSERT_TRUE(b.getEffectInfo() != NULL);
        EXPECT_EQ(0, (int)b.getEffectInfo()->getListNum());
    } else {
        expectEffectInfoEqual(a.getEffectInfo(), b.getEffectInfo());
    }
}

MOVEMENT_PACKET_TESTS(GCAddVampireFromBurrowing)
MOVEMENT_PACKET_VARIANT(GCAddVampireFromBurrowing, noeffects, fillNoeffects)

void fill(GCAddVampireFromTransformation& packet) {
    PCVampireInfo3 info;
    fillVampireInfo(info);
    packet.setVampireInfo(info);
    packet.setEffectInfo(makeEffectInfo(1));
}

void fillNoeffects(GCAddVampireFromTransformation& packet) {
    fill(packet);
    delete packet.getEffectInfo();
    packet.setEffectInfo(NULL);
}

void expectEqual(GCAddVampireFromTransformation& a, GCAddVampireFromTransformation& b) {
    expectVampireInfoEqual(a.getVampireInfo(), b.getVampireInfo());
    if (a.getEffectInfo() == NULL) {
        ASSERT_TRUE(b.getEffectInfo() != NULL);
        EXPECT_EQ(0, (int)b.getEffectInfo()->getListNum());
    } else {
        expectEffectInfoEqual(a.getEffectInfo(), b.getEffectInfo());
    }
}

MOVEMENT_PACKET_TESTS(GCAddVampireFromTransformation)
MOVEMENT_PACKET_VARIANT(GCAddVampireFromTransformation, noeffects, fillNoeffects)

//////////////////////////////////////////////////////////////////////
// The effect lifecycle's ending half.
//////////////////////////////////////////////////////////////////////

void fill(GCRemoveEffect& packet) {
    packet.setObjectID(0x86A7B8C9);
    packet.addEffectList(0x81E2);
    packet.addEffectList(0x83E4);
    packet.addEffectList(0x85E6);
}

// The empty-list branch: an effect sweep that removed nothing still
// names the creature.
void fillEmpty(GCRemoveEffect& packet) {
    packet.setObjectID(0x86A7B8C9);
}

// The count is the list, so popping shortens it: it is read once and
// then drives both walks.
void expectEqual(GCRemoveEffect& a, GCRemoveEffect& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    ASSERT_EQ(a.getListNum(), b.getListNum());
    const int entries = (int)a.getListNum();
    for (int i = 0; i < entries; i++)
        EXPECT_EQ(a.popFrontListElement(), b.popFrontListElement()) << "effect list entry " << i;
}

MOVEMENT_PACKET_TESTS(GCRemoveEffect)
MOVEMENT_PACKET_VARIANT(GCRemoveEffect, empty, fillEmpty)

// The name is text, and both read() and write() refuse one past ten
// characters, so it cannot follow the high-byte rule.
void fill(GCRemoveInjuriousCreature& packet) {
    packet.setName("TrapTurret");
}

void expectEqual(GCRemoveInjuriousCreature& a, GCRemoveInjuriousCreature& b) {
    EXPECT_EQ(a.getName(), b.getName());
}

MOVEMENT_PACKET_TESTS(GCRemoveInjuriousCreature)

void fill(GCHPRecoveryEndToSelf& packet) {
    packet.setCurrentHP(0x87E8);
}

void expectEqual(GCHPRecoveryEndToSelf& a, GCHPRecoveryEndToSelf& b) {
    EXPECT_EQ(a.getCurrentHP(), b.getCurrentHP());
}

MOVEMENT_PACKET_TESTS(GCHPRecoveryEndToSelf)

void fill(GCHPRecoveryEndToOthers& packet) {
    packet.setObjectID(0x88A9BACB);
    packet.setCurrentHP(0x8CED);
}

void expectEqual(GCHPRecoveryEndToOthers& a, GCHPRecoveryEndToOthers& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getCurrentHP(), b.getCurrentHP());
}

MOVEMENT_PACKET_TESTS(GCHPRecoveryEndToOthers)

void fill(GCMPRecoveryEnd& packet) {
    packet.setCurrentMP(0x8DEE);
}

void expectEqual(GCMPRecoveryEnd& a, GCMPRecoveryEnd& b) {
    EXPECT_EQ(a.getCurrentMP(), b.getCurrentMP());
}

MOVEMENT_PACKET_TESTS(GCMPRecoveryEnd)

void fill(GCCannotUse& packet) {
    packet.setObjectID(0x89AABBCC);
}

void expectEqual(GCCannotUse& a, GCCannotUse& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
}

MOVEMENT_PACKET_TESTS(GCCannotUse)

//////////////////////////////////////////////////////////////////////
// The NPC conversation. Subjects, choices and messages are text.
//////////////////////////////////////////////////////////////////////

void fill(CGNPCTalk& packet) {
    packet.setObjectID(0x8AABBCCD);
}

void expectEqual(CGNPCTalk& a, CGNPCTalk& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
}

MOVEMENT_PACKET_TESTS(CGNPCTalk)

void fill(GCNPCAsk& packet) {
    packet.setObjectID(0x8BACBDCE);
    packet.setScriptID(0x8FD0E1F2);
    packet.setNPCID(0x93F4);
}

void expectEqual(GCNPCAsk& a, GCNPCAsk& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getScriptID(), b.getScriptID());
    EXPECT_EQ(a.getNPCID(), b.getNPCID());
}

MOVEMENT_PACKET_TESTS(GCNPCAsk)

void fill(GCNPCSay& packet) {
    packet.setObjectID(0x8CADBECF);
    packet.setScriptID(0x90D1E2F3);
    packet.setSubjectID(0x94);
}

void expectEqual(GCNPCSay& a, GCNPCSay& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getScriptID(), b.getScriptID());
    EXPECT_EQ(a.getSubjectID(), b.getSubjectID());
}

MOVEMENT_PACKET_TESTS(GCNPCSay)

void fill(GCNPCSayDynamic& packet) {
    packet.setObjectID(0x8DAEBFD0);
    packet.setMessage("NPCDialogueLine");
}

void expectEqual(GCNPCSayDynamic& a, GCNPCSayDynamic& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getMessage(), b.getMessage());
}

MOVEMENT_PACKET_TESTS(GCNPCSayDynamic)

// The packet owns the parameters it is handed and deletes them.
void addParameter(GCNPCAskVariable& packet, const string& name, const string& value) {
    ScriptParameter* pParam = new ScriptParameter();
    pParam->setName(name);
    pParam->setValue(value);
    packet.addScriptParameter(pParam);
}

// The parameters live in a map keyed by name, so the wire order is the
// names' order rather than the order they were added.
void fill(GCNPCAskVariable& packet) {
    packet.setObjectID(0x8EAFC0D1);
    packet.setScriptID(0x92D3E4F5);
    addParameter(packet, "gamma", "thirdValue");
    addParameter(packet, "alpha", "firstValue");
    addParameter(packet, "beta", "secondValue");
}

// The no-parameter branch: a script that substitutes nothing.
void fillEmpty(GCNPCAskVariable& packet) {
    packet.setObjectID(0x8EAFC0D1);
    packet.setScriptID(0x92D3E4F5);
}

void expectEqual(GCNPCAskVariable& a, GCNPCAskVariable& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getScriptID(), b.getScriptID());

    HashMapScriptParameter& left = a.getScriptParameters();
    HashMapScriptParameter& right = b.getScriptParameters();
    ASSERT_EQ(left.size(), right.size());
    for (HashMapScriptParameterItor itr = left.begin(); itr != left.end(); itr++) {
        HashMapScriptParameterItor found = right.find(itr->first);
        ASSERT_TRUE(found != right.end()) << "missing parameter " << itr->first;
        EXPECT_EQ(itr->second->getName(), found->second->getName());
        EXPECT_EQ(itr->second->getValue(), found->second->getValue());
    }
}

MOVEMENT_PACKET_TESTS(GCNPCAskVariable)
MOVEMENT_PACKET_VARIANT(GCNPCAskVariable, empty, fillEmpty)

//////////////////////////////////////////////////////////////////////
// The run-time question.
//////////////////////////////////////////////////////////////////////

void fill(GCNPCAskDynamic& packet) {
    packet.setObjectID(0x8FB0C1D2);
    packet.setScriptID(0x93D4E5F6);
    packet.setSubject("AskSubject");
    packet.addContent("ChoiceOne");
    packet.addContent("ChoiceTwo");
    packet.addContent("ChoiceThree");
}

// The no-choice branch: a question the client answers with a keypress.
void fillNocontents(GCNPCAskDynamic& packet) {
    packet.setObjectID(0x8FB0C1D2);
    packet.setScriptID(0x93D4E5F6);
    packet.setSubject("AskSubject");
}

// The count is the list, so popping shortens it: it is read once and
// then drives both walks.
void expectEqual(GCNPCAskDynamic& a, GCNPCAskDynamic& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getScriptID(), b.getScriptID());
    EXPECT_EQ(a.getSubject(), b.getSubject());
    ASSERT_EQ(a.getContentsCount(), b.getContentsCount());
    const int contents = (int)a.getContentsCount();
    for (int i = 0; i < contents; i++)
        EXPECT_EQ(a.popContent(), b.popContent()) << "content " << i;
}

MOVEMENT_PACKET_TESTS(GCNPCAskDynamic)
MOVEMENT_PACKET_VARIANT(GCNPCAskDynamic, nocontents, fillNocontents)

//////////////////////////////////////////////////////////////////////
// The NPC roster. A filler hands the zone's own records and read()
// allocates its own, so the tests drain what they read: a popped record
// is the caller's.
//////////////////////////////////////////////////////////////////////

struct NPCInfoFixture {
    NPCInfo records[3];
    GCNPCInfo packet;
};

// The names are text; the ids and tiles follow the high-byte rule.
void fillNPCInfos(NPCInfoFixture& f) {
    const char* names[3] = {"GateGuard", "Blacksmith", "Priestess"};
    for (int i = 0; i < 3; i++) {
        f.records[i].setName(names[i]);
        f.records[i].setNPCID((NPCID_t)(0x81E2 + i * 0x0101));
        f.records[i].setX((ZoneCoord_t)(0x87E8 + i * 0x0101));
        f.records[i].setY((ZoneCoord_t)(0x8DEE + i * 0x0101));
        f.packet.addNPCInfo(&f.records[i]);
    }
}

// The empty-name branch, which is the whole record: a nameless entry is
// one length byte and nothing else.
void fillNPCInfosEmptyName(NPCInfoFixture& f) {
    f.records[0].setName("GateGuard");
    f.records[0].setNPCID(0x81E2);
    f.records[0].setX(0x87E8);
    f.records[0].setY(0x8DEE);
    f.packet.addNPCInfo(&f.records[0]);

    f.records[1].setName("");
    f.packet.addNPCInfo(&f.records[1]);
}

void drainNPCInfos(GCNPCInfo& packet) {
    while (NPCInfo* pInfo = packet.popNPCInfo())
        delete pInfo;
}

void expectNPCInfosEqual(GCNPCInfo& a, GCNPCInfo& b) {
    for (;;) {
        NPCInfo* pLeft = a.popNPCInfo();
        NPCInfo* pRight = b.popNPCInfo();
        if (pLeft == NULL || pRight == NULL) {
            EXPECT_TRUE(pLeft == NULL) << "the packet read back holds fewer records than it was sent";
            EXPECT_TRUE(pRight == NULL) << "the packet read back holds more records than it was sent";
            delete pRight;
            drainNPCInfos(b);
            return;
        }
        EXPECT_EQ(pLeft->getName(), pRight->getName());
        if (!pLeft->getName().empty()) {
            // A nameless record puts nothing but its length byte on the
            // wire, so the reader has no id or tile to compare.
            EXPECT_EQ(pLeft->getNPCID(), pRight->getNPCID());
            EXPECT_EQ(pLeft->getX(), pRight->getX());
            EXPECT_EQ(pLeft->getY(), pRight->getY());
        }
        delete pRight;
    }
}

TEST(GCNPCInfoTest, roundTripsThroughLoopback) {
    NPCInfoFixture f;
    fillNPCInfos(f);
    GCNPCInfo dst;
    roundTrip(f.packet, dst, kPlainCode);
    expectNPCInfosEqual(f.packet, dst);
}

TEST(GCNPCInfoTest, bodyBytesMatchGolden) {
    NPCInfoFixture f;
    fillNPCInfos(f);
    const std::vector<unsigned char> body = writeBody(f.packet, kPlainCode);
    expectGolden("GCNPCInfo", kPlainCode, body);
    for (size_t i = 1; i < kEncryptCodeCount; i++)
        EXPECT_EQ(body, writeBody(f.packet, kEncryptCodes[i]))
            << "GCNPCInfo now varies with the encrypt code - add per-code goldens";
}

TEST(GCNPCInfoTest, sizeMatchesTheBytesWrittenAndFitsTheFactoryMax) {
    NPCInfoFixture f;
    fillNPCInfos(f);
    GCNPCInfoFactory factory;
    EXPECT_EQ((size_t)f.packet.getPacketSize(), writeBody(f.packet, kPlainCode).size())
        << "GCNPCInfo: getPacketSize() disagrees with the bytes write() emits; writePacket() puts "
           "the former on the wire, so the stream never resynchronises";
    EXPECT_LE(f.packet.getPacketSize(), factory.getPacketMaxSize());
    EXPECT_EQ(factory.getPacketID(), f.packet.getPacketID());
    EXPECT_EQ(factory.getPacketName(), f.packet.getPacketName());
}

TEST(GCNPCInfoTest, emptynameBodyBytesMatchGolden) {
    NPCInfoFixture f;
    fillNPCInfosEmptyName(f);
    const std::vector<unsigned char> body = writeBody(f.packet, kPlainCode);
    expectGolden("GCNPCInfo.emptyname", kPlainCode, body);
    EXPECT_EQ((size_t)f.packet.getPacketSize(), body.size());

    GCNPCInfo dst;
    roundTrip(f.packet, dst, kPlainCode);
    expectNPCInfosEqual(f.packet, dst);
}

//////////////////////////////////////////////////////////////////////
// Bounds and refusals.
//////////////////////////////////////////////////////////////////////

// The choice count is the list, and the list stops at fifteen: the width
// the factory max budgets, and the most choices a script can hold.
TEST(GCNPCAskDynamicTest, theChoiceListStopsAtTheWidthTheFactoryMaxBudgets) {
    const std::string kSubject = "AskSubject";

    GCNPCAskDynamic packet;
    fillNocontents(packet);
    for (uint i = 0; i < GCNPCAskDynamic::kMaxCount; i++)
        packet.addContent(std::string(64, 'c'));

    EXPECT_EQ((int)GCNPCAskDynamic::kMaxCount, (int)packet.getContentsCount());
    EXPECT_THROW(packet.addContent("one choice too many"), InvalidProtocolException);

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    const size_t countOffset = szObjectID + szScriptID + szWORD + kSubject.size();
    ASSERT_LT(countOffset, body.size());
    EXPECT_EQ((int)GCNPCAskDynamic::kMaxCount, (int)body[countOffset])
        << "the count byte does not describe the choices behind it";
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCNPCAskDynamicFactory factory;
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize());
}

// An empty choice is its length word and nothing else. read() keeps it,
// so the packet read back declares and writes what it was sent.
TEST(GCNPCAskDynamicTest, anEmptyChoiceSurvivesTheRoundTrip) {
    GCNPCAskDynamic src;
    fillNocontents(src);
    src.addContent("ChoiceOne");
    src.addContent("");
    src.addContent("ChoiceThree");

    const std::vector<unsigned char> sent = writeBody(src, kPlainCode);

    GCNPCAskDynamic dst;
    roundTrip(src, dst, kPlainCode);

    EXPECT_EQ(3, (int)dst.getContentsCount());
    EXPECT_EQ(sent, writeBody(dst, kPlainCode));
}

// The length byte in front of the message describes it: a message the
// byte cannot count is refused rather than sent behind a wrapped length.
TEST(GCNPCSayDynamicTest, theMessageStopsAtTheWidthItsLengthByteCarries) {
    GCNPCSayDynamic packet;
    fill(packet);
    packet.setMessage(std::string(GCNPCSayDynamic::kMaxMessageSize, 'm'));

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    ASSERT_EQ(szObjectID + szBYTE + (size_t)GCNPCSayDynamic::kMaxMessageSize, body.size());
    EXPECT_EQ((int)GCNPCSayDynamic::kMaxMessageSize, (int)body[szObjectID]);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCNPCSayDynamicFactory factory;
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize());

    packet.setMessage(std::string(GCNPCSayDynamic::kMaxMessageSize + 1, 'm'));
    EXPECT_THROW(writeBody(packet, kPlainCode), InvalidProtocolException);
}

// A parameter is measured whole, and one longer than its length byte can
// count is refused.
TEST(GCNPCAskVariableTest, aParameterIsMeasuredWholeAndStopsAtItsLengthByte) {
    GCNPCAskVariable packet;
    fillEmpty(packet);
    addParameter(packet, "wide", std::string(ScriptParameter::kMaxStringSize, 'v'));

    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());

    GCNPCAskVariableFactory factory;
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize());

    addParameter(packet, "wider", std::string(ScriptParameter::kMaxStringSize + 1, 'v'));
    EXPECT_THROW(writeBody(packet, kPlainCode), InvalidProtocolException);
}

// The effect list count is the list, so popping an id shortens what the
// packet declares, and the adder refuses the id past the count byte.
TEST(GCRemoveEffectTest, theEffectListCountIsTheListAndPoppingShortensIt) {
    GCRemoveEffect packet;
    fillEmpty(packet);
    for (uint i = 0; i < GCRemoveEffect::kMaxCount; i++)
        packet.addEffectList((EffectID_t)(0x81E2 + i));

    EXPECT_EQ(255, (int)packet.getListNum());
    EXPECT_THROW(packet.addEffectList(0x81E2), InvalidProtocolException);

    packet.popFrontListElement();
    EXPECT_EQ(254, (int)packet.getListNum());
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());
}

// A full sweep fits the read buffer the receiver sizes from the factory
// max.
TEST(GCRemoveEffectTest, aFullListFitsTheFactoryMax) {
    GCRemoveEffect packet;
    fillEmpty(packet);
    for (uint i = 0; i < GCRemoveEffect::kMaxCount; i++)
        packet.addEffectList((EffectID_t)(0x81E2 + i));

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCRemoveEffectFactory factory;
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize());
    EXPECT_EQ(szObjectID + szBYTE + GCRemoveEffect::kMaxCount * szEffectID, (uint)factory.getPacketMaxSize());
}

// read() replaces the list the packet holds, so a packet read into twice
// declares and writes one sweep.
TEST(GCRemoveEffectTest, aSecondReadReplacesTheListItHolds) {
    GCRemoveEffect src;
    fill(src);

    GCRemoveEffect dst;
    roundTrip(src, dst, kPlainCode);
    roundTrip(src, dst, kPlainCode);

    EXPECT_EQ(3, (int)dst.getListNum());
    EXPECT_EQ((size_t)dst.getPacketSize(), writeBody(dst, kPlainCode).size());
}

// GCAddMonster holds its name to the 32 bytes its factory max budgets.
// The two transition packets that carry the same record hold it to the
// same width on both sides.
template <typename TransitionPacket, typename TransitionFactory>
void expectTheMonsterNameStopsAtTheBudget(TransitionPacket& packet, const char* what) {
    packet.setMonsterName(std::string(TransitionPacket::kMaxNameSize, 'n'));

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((int)TransitionPacket::kMaxNameSize, (int)body[szObjectID + szMonsterType]) << what;
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size()) << what;

    TransitionFactory factory;
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize()) << what;

    TransitionPacket dst;
    roundTrip(packet, dst, kPlainCode);
    EXPECT_EQ(packet.getMonsterName(), dst.getMonsterName()) << what;

    packet.setMonsterName(std::string(TransitionPacket::kMaxNameSize + 1, 'n'));
    EXPECT_THROW(writeBody(packet, kPlainCode), InvalidProtocolException) << what;
}

TEST(MovementBoundsTest, theTransitionMonsterNamesStopAtTheBudget) {
    GCAddMonsterFromBurrowing burrowing;
    fill(burrowing);
    expectTheMonsterNameStopsAtTheBudget<GCAddMonsterFromBurrowing, GCAddMonsterFromBurrowingFactory>(
        burrowing, "GCAddMonsterFromBurrowing");

    GCAddMonsterFromTransformation transforming;
    fill(transforming);
    expectTheMonsterNameStopsAtTheBudget<GCAddMonsterFromTransformation, GCAddMonsterFromTransformationFactory>(
        transforming, "GCAddMonsterFromTransformation");
}

// DIR_NONE is a direction the server holds and broadcasts: MonsterAI
// computes it for a monster already standing on its destination, and the
// move mask has an entry for it. So the packets that carry a creature's
// direction to the client carry that value too.
TEST(MovementDirectionTest, theBroadcastDirectionsCarryTheOneThatNamesNoDirection) {
    ASSERT_EQ(8, (int)DIR_MAX) << "Dir2String has one entry per direction below DIR_MAX";
    ASSERT_EQ((int)DIR_MAX, (int)DIR_NONE);

    GCMove move;
    fill(move);
    move.setDir(DIR_NONE);
    GCMove movedst;
    roundTrip(move, movedst, kPlainCode);
    EXPECT_EQ((Dir_t)DIR_NONE, movedst.getDir());

    GCUnburrowOK unburrowOK;
    fill(unburrowOK);
    unburrowOK.setDir(DIR_NONE);
    GCUnburrowOK unburrowOKdst;
    roundTrip(unburrowOK, unburrowOKdst, kPlainCode);
    EXPECT_EQ((Dir_t)DIR_NONE, unburrowOKdst.getDir());

    GCUntransformOK untransformOK;
    fill(untransformOK);
    untransformOK.setDir(DIR_NONE);
    GCUntransformOK untransformOKdst;
    roundTrip(untransformOK, untransformOKdst, kPlainCode);
    EXPECT_EQ((Dir_t)DIR_NONE, untransformOKdst.getDir());
}

// A client names one of the eight or nothing at all, so the two requests
// refuse anything else. CGMove reads through the encrypter; at code 0 it
// takes the plain branch these bytes are written on.
TEST(MovementDirectionTest, theRequestsRefuseADirectionOutsideTheEight) {
    Loopback unburrowLink;
    unburrowLink.setCodes(kPlainCode);
    unburrowLink.out().write((Coord_t)0x8A);
    unburrowLink.out().write((Coord_t)0x9B);
    unburrowLink.out().write((Dir_t)DIR_NONE);
    unburrowLink.pump(szCoord * 2 + szDir);

    CGUnburrow unburrow;
    EXPECT_THROW(unburrow.read(unburrowLink.in()), InvalidProtocolException);

    Loopback moveLink;
    moveLink.setCodes(kPlainCode);
    moveLink.out().write((Dir_t)DIR_NONE);
    moveLink.out().write((Coord_t)0x8A);
    moveLink.out().write((Coord_t)0x9B);
    moveLink.pump(szDir + szCoord * 2);

    CGMove move;
    EXPECT_THROW(move.read(moveLink.in()), InvalidProtocolException);
}

//////////////////////////////////////////////////////////////////////
// What the default constructor leaves.
//////////////////////////////////////////////////////////////////////

// Each packet is built twice over storage poisoned with a different
// byte, so a member the constructor leaves alone reaches the wire as that
// byte and the two bodies differ. `prepare` fills only the fields whose
// emptiness write() refuses.
template <typename PacketType, typename Prepare>
void expectEveryMemberIsInitialised(const char* what, Prepare prepare) {
    alignas(PacketType) unsigned char storage[sizeof(PacketType)];
    const unsigned char poison[2] = {0x00, 0xFF};
    std::vector<unsigned char> bodies[2];

    for (int i = 0; i < 2; i++) {
        memset(storage, poison[i], sizeof(storage));
        PacketType* pPacket = new (storage) PacketType();
        prepare(*pPacket);
        bodies[i] = writeBody(*pPacket, kPlainCode);
        pPacket->~PacketType();
    }

    EXPECT_EQ(bodies[0], bodies[1]) << what << ": its default constructor leaves a member write() emits uninitialised";
}

template <typename PacketType> void expectEveryMemberIsInitialised(const char* what) {
    expectEveryMemberIsInitialised<PacketType>(what, [](PacketType&) {});
}

TEST(MovementConstructorTest, everyPacketInitialisesEveryMemberItWrites) {
    expectEveryMemberIsInitialised<GCMove>("GCMove");
    expectEveryMemberIsInitialised<GCKnockBack>("GCKnockBack");
    expectEveryMemberIsInitialised<GCFakeMove>("GCFakeMove");
    expectEveryMemberIsInitialised<CGUnburrow>("CGUnburrow");
    expectEveryMemberIsInitialised<GCUnburrowOK>("GCUnburrowOK");
    expectEveryMemberIsInitialised<GCUnburrowFail>("GCUnburrowFail");
    expectEveryMemberIsInitialised<CGUntransform>("CGUntransform");
    expectEveryMemberIsInitialised<GCUntransformOK>("GCUntransformOK");
    expectEveryMemberIsInitialised<GCUntransformFail>("GCUntransformFail");
    expectEveryMemberIsInitialised<GCAddMonsterFromBurrowing>("GCAddMonsterFromBurrowing");
    expectEveryMemberIsInitialised<GCAddMonsterFromTransformation>("GCAddMonsterFromTransformation");
    expectEveryMemberIsInitialised<GCRemoveEffect>("GCRemoveEffect");
    expectEveryMemberIsInitialised<GCHPRecoveryEndToSelf>("GCHPRecoveryEndToSelf");
    expectEveryMemberIsInitialised<GCHPRecoveryEndToOthers>("GCHPRecoveryEndToOthers");
    expectEveryMemberIsInitialised<GCMPRecoveryEnd>("GCMPRecoveryEnd");
    expectEveryMemberIsInitialised<GCCannotUse>("GCCannotUse");
    expectEveryMemberIsInitialised<CGNPCTalk>("CGNPCTalk");
    expectEveryMemberIsInitialised<GCNPCAsk>("GCNPCAsk");
    expectEveryMemberIsInitialised<GCNPCAskVariable>("GCNPCAskVariable");
    expectEveryMemberIsInitialised<GCNPCSay>("GCNPCSay");
    expectEveryMemberIsInitialised<GCNPCInfo>("GCNPCInfo");

    // The five whose write() refuses an empty string get that one field.
    expectEveryMemberIsInitialised<GCRemoveInjuriousCreature>(
        "GCRemoveInjuriousCreature", [](GCRemoveInjuriousCreature& packet) { packet.setName("Turret"); });
    expectEveryMemberIsInitialised<GCNPCAskDynamic>("GCNPCAskDynamic",
                                                    [](GCNPCAskDynamic& packet) { packet.setSubject("Subject"); });
    expectEveryMemberIsInitialised<GCNPCSayDynamic>("GCNPCSayDynamic",
                                                    [](GCNPCSayDynamic& packet) { packet.setMessage("Message"); });
    expectEveryMemberIsInitialised<GCAddVampireFromBurrowing>(
        "GCAddVampireFromBurrowing", [](GCAddVampireFromBurrowing& packet) { packet.getVampireInfo().setName("V"); });
    expectEveryMemberIsInitialised<GCAddVampireFromTransformation>(
        "GCAddVampireFromTransformation",
        [](GCAddVampireFromTransformation& packet) { packet.getVampireInfo().setName("V"); });
}

} // namespace
