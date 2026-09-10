//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_combat_test.cpp
// Description : Golden byte fixtures, loopback round trips and size
//               pins for the packets a game server sends as the result
//               of an attack or a skill use — to the actor, to the
//               target and to the observers — plus the status packets
//               that ride along.
//
//               The set is taken from the code that produces those
//               results: CGAttackHandler, skill/AttackMelee.cpp,
//               skill/AttackArms.cpp, the object/self/tile/inventory
//               skill handlers under skill/ and the helpers in
//               skill/SkillUtil.cpp they share, CGThrowBombHandler and
//               ZoneUtil.cpp's mine blast, and the two death paths in
//               PCManager.cpp and MonsterManager.cpp. Thirty-eight
//               packets, each with the reason it is here:
//
//               GCAttack         what CGAttackHandler broadcasts for the
//                                swing itself: who attacked, from where
//                                and facing which way.
//               GCGetDamage      the damage number the same handler, and
//                                the blood-drain and absorb-soul
//                                handlers, put over the creature that
//                                was hit.
//               GCAttackMeleeOK1 what AttackMelee sends the attacker: the
//                                target's id plus the attacker's own
//                                stat changes.
//               GCAttackMeleeOK2 the same result sent to the target
//                                player, carrying the attacker's id and
//                                the target's stat changes.
//               GCAttackMeleeOK3 the observers' copy — attacker and
//                                target ids and nothing else.
//               GCAttackArmsOK1  what AttackArms sends the shooter: the
//                                skill, the target, the rounds left in
//                                the gun, whether the shot landed, and
//                                the shooter's stat changes.
//               GCAttackArmsOK2  the target player's copy of the same
//                                shot.
//               GCAttackArmsOK3  the copy for players who can see the
//                                shooter: the shot's aim point rather
//                                than its target.
//               GCAttackArmsOK4  the copy for players who can see the
//                                target only.
//               GCAttackArmsOK5  the copy for players who can see both
//                                ends, which is why it carries both ids
//                                and the hit flag.
//               GCSkillToObjectOK1  the same five-way split for a skill
//               GCSkillToObjectOK2  aimed at a creature: OK1 to the
//               GCSkillToObjectOK3  caster, OK2 to the target, OK3 to
//               GCSkillToObjectOK4  the players who see the caster, OK4
//               GCSkillToObjectOK5  to those who see the target, OK5 to
//                                those who see both.
//               GCSkillToObjectOK6  the target's copy when the target
//                                cannot see the caster: the caster's
//                                position stands in for the caster's id.
//               GCSkillToSelfOK1 what a self-targeted skill sends its
//                                caster.
//               GCSkillToSelfOK2 the observers' copy of it.
//               GCSkillToSelfOK3 the observers' copy that carries the
//                                caster's position instead of the id,
//                                used where the caster leaves the view
//                                as the skill lands.
//               GCSkillToTileOK1 the same split for a skill aimed at a
//               GCSkillToTileOK2 tile, which additionally carries the
//               GCSkillToTileOK3 list of creature ids the area caught:
//               GCSkillToTileOK4 OK1 to the caster, OK2 to a caught
//               GCSkillToTileOK5 target, OK3/OK4/OK5 to the three
//               GCSkillToTileOK6 observer groups, OK6 to a caught target
//                                that cannot see the caster.
//               GCSkillToInventoryOK1  what a skill whose result lands in
//                                the caster's inventory (bomb making,
//                                blood-bottle filling) sends the caster.
//               GCSkillToInventoryOK2  the observers' copy of it.
//               GCSkillFailed1   what executeSkillFailNormal sends the
//                                caster when the skill does not run, and
//                                what CGAttackHandler sends when the
//                                swing is refused.
//               GCSkillFailed2   the broadcast half of the same refusal.
//               GCStatusCurrentHP  the health bar SkillUtil broadcasts
//                                for every creature whose HP the hit
//                                changed.
//               GCModifyInformation  the standalone carrier of the same
//                                stat-change record the OK packets embed,
//                                sent when a change has no packet of its
//                                own to ride on.
//               GCOtherModifyInfo  the copy of that record broadcast for
//                                a creature other than the receiver.
//               GCCreatureDied   what PCManager and MonsterManager
//                                broadcast when the hit was the last one.
//               GCThrowBombOK1   the thrown bomb's three-way split:
//               GCThrowBombOK2   OK1 to the thrower, OK2 to each caught
//               GCThrowBombOK3   target, OK3 to the observers. All three
//                                carry the list of creature ids the blast
//                                caught.
//               GCMineExplosionOK1  the same blast when the bomb is a
//               GCMineExplosionOK2  mine somebody stepped on: OK1 to each
//                                caught target, OK2 to the observers.
//                                Nobody threw it, so there is no thrower
//                                to answer.
//
//               Deliberately excluded:
//
//               GCAddEffect, which a skill sends after the OK packets to
//               put the effect marker on the target, is already pinned by
//               tests/packet_zone_scan_test.cpp. GCRemoveEffect belongs to
//               effect expiry, not to an attack result; AttackMelee.cpp
//               includes both headers and constructs neither.
//
//               No packet here calls readEncrypt/writeEncrypt, so the
//               goldens are recorded at code 0 only, and every golden
//               test also asserts the bytes do not vary with the code, so
//               adopting the encrypter fails loudly instead of silently
//               voiding the pin.
//
//               There is no GCStatusCurrentMP: the current-MP change
//               travels as a MODIFY_CURRENT_MP entry in the stat record.
//
//               Each packet gets three pins (COMBAT_PACKET_TESTS):
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
//               Fifteen of the packets carry the ModifyInfo stat record.
//               It is not a flag word: it is a count byte, that many
//               type/ushort pairs, a second count byte and that many
//               type/DWORD pairs, so a field's presence costs the same
//               bytes whichever ModifyType tags it. Every one of the 73
//               ModifyType values is therefore covered once as a short
//               entry and once as a long entry, in the
//               GCModifyInformation.allTypes golden and its round trip,
//               and the canonical fixture every embedding packet shares
//               carries a mixed three-short/two-long record so the
//               embedding offsets are pinned too.
//
//               Fixture values are distinct per field and >= 128 in every
//               byte the width allows. Three groups cannot follow that
//               rule and say so at the point of use: the ModifyType tag
//               bytes, which must be enumerators; the two hit flags,
//               which are bool and reach the wire as 0 or 1; and the
//               per-type values in the all-types record, whose low byte
//               counts the type it belongs to.
//
//               The findings this set produced are fixed, and the tests
//               that stated them are the positive pins below:
//
//               - ModifyInfo derives each list's count from the list and
//                 refuses the entry past the 255 the count byte carries,
//                 in the adders and in write().
//               - The five tile packets that carry a creature list do the
//                 same with theirs, popCListElement() takes the id off the
//                 count with the list, and each factory max budgets a full
//                 255-id list. The three bomb and two mine packets carry
//                 the same list and keep the same bounds.
//               - ModifyInfo::read() and the tile read()s replace the list
//                 the packet holds instead of appending to it.
//               - A type tag past the last ModifyType is refused in the
//                 adders and in read(), and toString() prints an unknown
//                 tag as its number.
//               - GCSkillToTileOK3::getObjectID() and
//                 GCSkillToInventoryOK2::getObjectID() return the
//                 ObjectID_t they hold.
//               - Every packet in the set initialises every member its
//                 write() emits, pinned by constructing each over poisoned
//                 storage.
//               - The two hit flags are read as a byte and narrowed, so a
//                 wire byte other than 0 or 1 is a hit rather than a bool
//                 no load may touch.
//
//////////////////////////////////////////////////////////////////////

#include <cstring>
#include <new>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Exception.h"
#include "GCAttack.h"
#include "GCAttackArmsOK1.h"
#include "GCAttackArmsOK2.h"
#include "GCAttackArmsOK3.h"
#include "GCAttackArmsOK4.h"
#include "GCAttackArmsOK5.h"
#include "GCAttackMeleeOK1.h"
#include "GCAttackMeleeOK2.h"
#include "GCAttackMeleeOK3.h"
#include "GCCreatureDied.h"
#include "GCGetDamage.h"
#include "GCMineExplosionOK1.h"
#include "GCMineExplosionOK2.h"
#include "GCModifyInformation.h"
#include "GCOtherModifyInfo.h"
#include "GCSkillFailed1.h"
#include "GCSkillFailed2.h"
#include "GCSkillToInventoryOK1.h"
#include "GCSkillToInventoryOK2.h"
#include "GCSkillToObjectOK1.h"
#include "GCSkillToObjectOK2.h"
#include "GCSkillToObjectOK3.h"
#include "GCSkillToObjectOK4.h"
#include "GCSkillToObjectOK5.h"
#include "GCSkillToObjectOK6.h"
#include "GCSkillToSelfOK1.h"
#include "GCSkillToSelfOK2.h"
#include "GCSkillToSelfOK3.h"
#include "GCSkillToTileOK1.h"
#include "GCSkillToTileOK2.h"
#include "GCSkillToTileOK3.h"
#include "GCSkillToTileOK4.h"
#include "GCSkillToTileOK5.h"
#include "GCSkillToTileOK6.h"
#include "GCStatusCurrentHP.h"
#include "GCThrowBombOK1.h"
#include "GCThrowBombOK2.h"
#include "GCThrowBombOK3.h"
#include "ModifyInfo.h"
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
// comparing the embedded stat record consumes it.
//////////////////////////////////////////////////////////////////////

#define COMBAT_PACKET_GOLDEN_AND_SIZE(Name)                                                          \
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

#define COMBAT_PACKET_TESTS(Name)                 \
    TEST(Name##Test, roundTripsThroughLoopback) { \
        Name src;                                 \
        fill(src);                                \
        Name dst;                                 \
        roundTrip(src, dst, kPlainCode);          \
        expectEqual(src, dst);                    \
    }                                             \
    COMBAT_PACKET_GOLDEN_AND_SIZE(Name)

// A second fixture for a packet whose write() has a branch the canonical
// one does not take.
#define COMBAT_PACKET_VARIANT(Name, Variant, fillVariant)                      \
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
// The stat record thirteen of the packets embed.
//////////////////////////////////////////////////////////////////////

// The type tags are enumerators, so they carry ModifyType values rather
// than high bytes. The values do not: each is a full-width ushort or
// DWORD with every byte >= 128.
void fillModifyInfo(ModifyInfo& info) {
    info.addShortData(MODIFY_CURRENT_HP, 0x81A2);
    info.addShortData(MODIFY_CURRENT_MP, 0x83A4);
    info.addShortData(MODIFY_ATTACK_SPEED, 0x85A6);
    info.addLongData(MODIFY_GOLD, 0x87A8B9CA);
    info.addLongData(MODIFY_ALIGNMENT, 0x8BACBDCE);
}

// ModifyInfo exposes its lists only through the destructive popShortData
// / popLongData, so comparing two records empties both. Nothing reads
// them afterwards.
void expectModifyInfoEqual(ModifyInfo& a, ModifyInfo& b) {
    ASSERT_EQ(a.getShortCount(), b.getShortCount());
    ASSERT_EQ(a.getLongCount(), b.getLongCount());

    const int shorts = (int)a.getShortCount();
    for (int i = 0; i < shorts; i++) {
        SHORTDATA left, right;
        a.popShortData(left);
        b.popShortData(right);
        EXPECT_EQ((int)left.type, (int)right.type) << "short entry " << i;
        EXPECT_EQ(left.value, right.value) << "short entry " << i;
    }

    const int longs = (int)a.getLongCount();
    for (int i = 0; i < longs; i++) {
        LONGDATA left, right;
        a.popLongData(left);
        b.popLongData(right);
        EXPECT_EQ((int)left.type, (int)right.type) << "long entry " << i;
        EXPECT_EQ(left.value, right.value) << "long entry " << i;
    }
}

// The creature list the tile and blast packets carry. The count is the
// list, so popping shortens it: it is read once and then drives both
// walks.
template <typename TilePacket> void fillCList(TilePacket& packet, int count, ObjectID_t base) {
    for (int i = 0; i < count; i++)
        packet.addCListElement((ObjectID_t)(base + (ObjectID_t)i * 0x01010101u));
}

template <typename TilePacket> void expectCListEqual(TilePacket& a, TilePacket& b) {
    ASSERT_EQ(a.getCListNum(), b.getCListNum());
    const int entries = (int)a.getCListNum();
    for (int i = 0; i < entries; i++)
        EXPECT_EQ(a.popCListElement(), b.popCListElement()) << "creature list entry " << i;
}

//////////////////////////////////////////////////////////////////////
// The swing and the damage number.
//////////////////////////////////////////////////////////////////////

void fill(GCAttack& packet) {
    packet.setObjectID(0x81A2B3C4);
    packet.setX(0x85);
    packet.setY(0x96);
    packet.setDir(0xA7);
}

void expectEqual(GCAttack& a, GCAttack& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
}

COMBAT_PACKET_TESTS(GCAttack)

void fill(GCGetDamage& packet) {
    packet.setObjectID(0x82A3B4C5);
    packet.setDamage(0x86D7);
}

void expectEqual(GCGetDamage& a, GCGetDamage& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getDamage(), b.getDamage());
}

COMBAT_PACKET_TESTS(GCGetDamage)

//////////////////////////////////////////////////////////////////////
// The melee result.
//////////////////////////////////////////////////////////////////////

void fill(GCAttackMeleeOK1& packet) {
    packet.setObjectID(0x83A4B5C6);
    fillModifyInfo(packet);
}

void expectEqual(GCAttackMeleeOK1& a, GCAttackMeleeOK1& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCAttackMeleeOK1)

void fill(GCAttackMeleeOK2& packet) {
    packet.setObjectID(0x84A5B6C7);
    fillModifyInfo(packet);
}

void expectEqual(GCAttackMeleeOK2& a, GCAttackMeleeOK2& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCAttackMeleeOK2)

void fill(GCAttackMeleeOK3& packet) {
    packet.setObjectID(0x85A6B7C8);
    packet.setTargetObjectID(0x89AABBCC);
}

void expectEqual(GCAttackMeleeOK3& a, GCAttackMeleeOK3& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
}

COMBAT_PACKET_TESTS(GCAttackMeleeOK3)

//////////////////////////////////////////////////////////////////////
// The ranged result. The hit flag is a bool and reaches the wire as 0
// or 1, so it is the one field in these fixtures below 128.
//////////////////////////////////////////////////////////////////////

void fill(GCAttackArmsOK1& packet) {
    packet.setSkillType(0x86B7);
    packet.setObjectID(0x88A9BACB);
    packet.setBulletNum(0x8C);
    packet.setSkillSuccess(true);
    fillModifyInfo(packet);
}

void expectEqual(GCAttackArmsOK1& a, GCAttackArmsOK1& b) {
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getBullet(), b.getBullet());
    EXPECT_EQ(a.getSkillSuccess(), b.getSkillSuccess());
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCAttackArmsOK1)

void fill(GCAttackArmsOK2& packet) {
    packet.setSkillType(0x87B8);
    packet.setObjectID(0x89AABBCC);
    fillModifyInfo(packet);
}

void expectEqual(GCAttackArmsOK2& a, GCAttackArmsOK2& b) {
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCAttackArmsOK2)

void fill(GCAttackArmsOK3& packet) {
    packet.setSkillType(0x88B9);
    packet.setObjectID(0x8AABBCCD);
    packet.setTargetXY(0x8E, 0x9F);
}

void expectEqual(GCAttackArmsOK3& a, GCAttackArmsOK3& b) {
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getTargetX(), b.getTargetX());
    EXPECT_EQ(a.getTargetY(), b.getTargetY());
}

COMBAT_PACKET_TESTS(GCAttackArmsOK3)

void fill(GCAttackArmsOK4& packet) {
    packet.setSkillType(0x89BA);
    packet.setTargetObjectID(0x8BACBDCE);
}

void expectEqual(GCAttackArmsOK4& a, GCAttackArmsOK4& b) {
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
}

COMBAT_PACKET_TESTS(GCAttackArmsOK4)

void fill(GCAttackArmsOK5& packet) {
    packet.setSkillType(0x8ABB);
    packet.setObjectID(0x8CADBECF);
    packet.setTargetObjectID(0x90D1E2F3);
    packet.setSkillSuccess(true);
}

void expectEqual(GCAttackArmsOK5& a, GCAttackArmsOK5& b) {
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getSkillSuccess(), b.getSkillSuccess());
}

COMBAT_PACKET_TESTS(GCAttackArmsOK5)

//////////////////////////////////////////////////////////////////////
// A skill aimed at a creature.
//////////////////////////////////////////////////////////////////////

void fill(GCSkillToObjectOK1& packet) {
    packet.setSkillType(0x8BBC);
    packet.setCEffectID(0x8DBE);
    packet.setTargetObjectID(0x8FC0D1E2);
    packet.setDuration(0x93C4);
    packet.setGrade(0x95);
    fillModifyInfo(packet);
}

void expectEqual(GCSkillToObjectOK1& a, GCSkillToObjectOK1& b) {
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getCEffectID(), b.getCEffectID());
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getDuration(), b.getDuration());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCSkillToObjectOK1)

void fill(GCSkillToObjectOK2& packet) {
    packet.setObjectID(0x8CBDCEDF);
    packet.setSkillType(0x90C1);
    packet.setDuration(0x92C3);
    packet.setGrade(0x94);
    fillModifyInfo(packet);
}

void expectEqual(GCSkillToObjectOK2& a, GCSkillToObjectOK2& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getDuration(), b.getDuration());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCSkillToObjectOK2)

void fill(GCSkillToObjectOK3& packet) {
    packet.setObjectID(0x8DBECFD0);
    packet.setSkillType(0x91C2);
    packet.setTargetXY(0x93, 0xA4);
    packet.setGrade(0x95);
}

void expectEqual(GCSkillToObjectOK3& a, GCSkillToObjectOK3& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getTargetX(), b.getTargetX());
    EXPECT_EQ(a.getTargetY(), b.getTargetY());
    EXPECT_EQ(a.getGrade(), b.getGrade());
}

COMBAT_PACKET_TESTS(GCSkillToObjectOK3)

void fill(GCSkillToObjectOK4& packet) {
    packet.setTargetObjectID(0x8EBFD0E1);
    packet.setSkillType(0x92C3);
    packet.setDuration(0x94C5);
    packet.setGrade(0x96);
}

void expectEqual(GCSkillToObjectOK4& a, GCSkillToObjectOK4& b) {
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getDuration(), b.getDuration());
    EXPECT_EQ(a.getGrade(), b.getGrade());
}

COMBAT_PACKET_TESTS(GCSkillToObjectOK4)

void fill(GCSkillToObjectOK5& packet) {
    packet.setObjectID(0x8FC0D1E2);
    packet.setTargetObjectID(0x93C4D5E6);
    packet.setSkillType(0x97C8);
    packet.setDuration(0x99CA);
    packet.setGrade(0x9B);
}

void expectEqual(GCSkillToObjectOK5& a, GCSkillToObjectOK5& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getDuration(), b.getDuration());
    EXPECT_EQ(a.getGrade(), b.getGrade());
}

COMBAT_PACKET_TESTS(GCSkillToObjectOK5)

void fill(GCSkillToObjectOK6& packet) {
    packet.setXY(0x90, 0xA1);
    packet.setSkillType(0x92C3);
    packet.setDuration(0x94C5);
    packet.setGrade(0x96);
    fillModifyInfo(packet);
}

void expectEqual(GCSkillToObjectOK6& a, GCSkillToObjectOK6& b) {
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getDuration(), b.getDuration());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCSkillToObjectOK6)

//////////////////////////////////////////////////////////////////////
// A skill aimed at the caster.
//////////////////////////////////////////////////////////////////////

void fill(GCSkillToSelfOK1& packet) {
    packet.setSkillType(0x91C2);
    packet.setCEffectID(0x93C4);
    packet.setDuration(0x95C6);
    packet.setGrade(0x97);
    fillModifyInfo(packet);
}

void expectEqual(GCSkillToSelfOK1& a, GCSkillToSelfOK1& b) {
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getCEffectID(), b.getCEffectID());
    EXPECT_EQ(a.getDuration(), b.getDuration());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCSkillToSelfOK1)

void fill(GCSkillToSelfOK2& packet) {
    packet.setObjectID(0x92C3D4E5);
    packet.setSkillType(0x96C7);
    packet.setDuration(0x98C9);
    packet.setGrade(0x9A);
}

void expectEqual(GCSkillToSelfOK2& a, GCSkillToSelfOK2& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getDuration(), b.getDuration());
    EXPECT_EQ(a.getGrade(), b.getGrade());
}

COMBAT_PACKET_TESTS(GCSkillToSelfOK2)

void fill(GCSkillToSelfOK3& packet) {
    packet.setXY(0x93, 0xA4);
    packet.setSkillType(0x95C6);
    packet.setDuration(0x97C8);
    packet.setGrade(0x99);
}

void expectEqual(GCSkillToSelfOK3& a, GCSkillToSelfOK3& b) {
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getDuration(), b.getDuration());
    EXPECT_EQ(a.getGrade(), b.getGrade());
}

COMBAT_PACKET_TESTS(GCSkillToSelfOK3)

//////////////////////////////////////////////////////////////////////
// A skill aimed at a tile. Five of the six carry the list of creature
// ids the area caught.
//////////////////////////////////////////////////////////////////////

void fill(GCSkillToTileOK1& packet) {
    packet.setSkillType(0x94C5);
    packet.setCEffectID(0x96C7);
    packet.setX(0x98);
    packet.setY(0xA9);
    packet.setDuration(0x9ACB);
    packet.setRange(0x9C);
    packet.setGrade(0x9D);
    fillCList(packet, 3, 0x9ECFE0F1);
    fillModifyInfo(packet);
}

void expectEqual(GCSkillToTileOK1& a, GCSkillToTileOK1& b) {
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getCEffectID(), b.getCEffectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDuration(), b.getDuration());
    EXPECT_EQ(a.getRange(), b.getRange());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    expectCListEqual(a, b);
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCSkillToTileOK1)

// The empty-area branch: the skill landed but caught nobody, which is
// what every tile skill sends when its sweep finds no creature.
void fillEmptyArea(GCSkillToTileOK1& packet) {
    packet.setSkillType(0x94C5);
    packet.setCEffectID(0x96C7);
    packet.setX(0x98);
    packet.setY(0xA9);
    packet.setDuration(0x9ACB);
    packet.setRange(0x9C);
    packet.setGrade(0x9D);
}

COMBAT_PACKET_VARIANT(GCSkillToTileOK1, emptyArea, fillEmptyArea)

void fill(GCSkillToTileOK2& packet) {
    packet.setObjectID(0x95C6D7E8);
    packet.setSkillType(0x99CA);
    packet.setX(0x9B);
    packet.setY(0xAC);
    packet.setRange(0x9D);
    packet.setDuration(0x9ECF);
    packet.setGrade(0xA0);
    fillCList(packet, 3, 0xA1D2E3F4);
    fillModifyInfo(packet);
}

void expectEqual(GCSkillToTileOK2& a, GCSkillToTileOK2& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getRange(), b.getRange());
    EXPECT_EQ(a.getDuration(), b.getDuration());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    expectCListEqual(a, b);
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCSkillToTileOK2)

void fill(GCSkillToTileOK3& packet) {
    packet.setObjectID(0x96C7D8E9);
    packet.setSkillType(0x9ACB);
    packet.setX(0x9C);
    packet.setY(0xAD);
    packet.setGrade(0x9E);
}

void expectEqual(GCSkillToTileOK3& a, GCSkillToTileOK3& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getGrade(), b.getGrade());
}

COMBAT_PACKET_TESTS(GCSkillToTileOK3)

void fill(GCSkillToTileOK4& packet) {
    packet.setSkillType(0x97C8);
    packet.setX(0x99);
    packet.setY(0xAA);
    packet.setRange(0x9B);
    packet.setDuration(0x9CCD);
    packet.setGrade(0x9E);
    fillCList(packet, 3, 0x9FD0E1F2);
}

void expectEqual(GCSkillToTileOK4& a, GCSkillToTileOK4& b) {
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getRange(), b.getRange());
    EXPECT_EQ(a.getDuration(), b.getDuration());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    expectCListEqual(a, b);
}

COMBAT_PACKET_TESTS(GCSkillToTileOK4)

void fill(GCSkillToTileOK5& packet) {
    packet.setObjectID(0x98C9DAEB);
    packet.setSkillType(0x9CCD);
    packet.setX(0x9E);
    packet.setY(0xAF);
    packet.setRange(0xA0);
    packet.setDuration(0xA1D2);
    packet.setGrade(0xA3);
    fillCList(packet, 3, 0xA4D5E6F7);
}

void expectEqual(GCSkillToTileOK5& a, GCSkillToTileOK5& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getRange(), b.getRange());
    EXPECT_EQ(a.getDuration(), b.getDuration());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    expectCListEqual(a, b);
}

COMBAT_PACKET_TESTS(GCSkillToTileOK5)

void fill(GCSkillToTileOK6& packet) {
    packet.setOrgXY(0x99, 0xAA);
    packet.setSkillType(0x9BCC);
    packet.setX(0x9D);
    packet.setY(0xAE);
    packet.setRange(0x9F);
    packet.setDuration(0xA0D1);
    packet.setGrade(0xA2);
    fillCList(packet, 3, 0xA3D4E5F6);
    fillModifyInfo(packet);
}

void expectEqual(GCSkillToTileOK6& a, GCSkillToTileOK6& b) {
    EXPECT_EQ(a.getOrgX(), b.getOrgX());
    EXPECT_EQ(a.getOrgY(), b.getOrgY());
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getRange(), b.getRange());
    EXPECT_EQ(a.getDuration(), b.getDuration());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    expectCListEqual(a, b);
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCSkillToTileOK6)

//////////////////////////////////////////////////////////////////////
// A skill whose result lands in the caster's inventory.
//////////////////////////////////////////////////////////////////////

void fill(GCSkillToInventoryOK1& packet) {
    packet.setSkillType(0x9ACB);
    packet.setObjectID(0x9CCDDEEF);
    packet.setItemType(0xA0D1);
    packet.setCEffectID(0xA2D3);
    packet.setX(0xA4);
    packet.setY(0xB5);
    packet.setDuration(0xA6D7);
    fillModifyInfo(packet);
}

void expectEqual(GCSkillToInventoryOK1& a, GCSkillToInventoryOK1& b) {
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getItemType(), b.getItemType());
    EXPECT_EQ(a.getCEffectID(), b.getCEffectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDuration(), b.getDuration());
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCSkillToInventoryOK1)

void fill(GCSkillToInventoryOK2& packet) {
    packet.setObjectID(0x9BCCDDEE);
    packet.setSkillType(0x9FD0);
    packet.setDuration(0xA1D2);
}

void expectEqual(GCSkillToInventoryOK2& a, GCSkillToInventoryOK2& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getDuration(), b.getDuration());
}

COMBAT_PACKET_TESTS(GCSkillToInventoryOK2)

//////////////////////////////////////////////////////////////////////
// The refusal, the health bar, the stat record and the death.
//////////////////////////////////////////////////////////////////////

void fill(GCSkillFailed1& packet) {
    packet.setSkillType(0x9CCD);
    packet.setGrade(0x9E);
    fillModifyInfo(packet);
}

void expectEqual(GCSkillFailed1& a, GCSkillFailed1& b) {
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCSkillFailed1)

// The refusal a caster gets when nothing changed: executeSkillFailNormal
// sends the packet with an empty stat record.
void fillNoChanges(GCSkillFailed1& packet) {
    packet.setSkillType(0x9CCD);
    packet.setGrade(0x9E);
}

COMBAT_PACKET_VARIANT(GCSkillFailed1, noChanges, fillNoChanges)

void fill(GCSkillFailed2& packet) {
    packet.setObjectID(0x9DCEDFE0);
    packet.setTargetObjectID(0xA1D2E3F4);
    packet.setSkillType(0xA5D6);
    packet.setGrade(0xA7);
}

void expectEqual(const GCSkillFailed2& a, const GCSkillFailed2& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getGrade(), b.getGrade());
}
COMBAT_PACKET_TESTS(GCSkillFailed2)

void fill(GCStatusCurrentHP& packet) {
    packet.setObjectID(0x9ECFE0F1);
    packet.setCurrentHP(0xA2D3);
}

void expectEqual(GCStatusCurrentHP& a, GCStatusCurrentHP& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getCurrentHP(), b.getCurrentHP());
}

COMBAT_PACKET_TESTS(GCStatusCurrentHP)

void fill(GCModifyInformation& packet) {
    fillModifyInfo(packet);
}

void expectEqual(GCModifyInformation& a, GCModifyInformation& b) {
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCModifyInformation)

// Every ModifyType the record can tag, once as a short entry and once as
// a long one. The low byte of each value is the type it belongs to, so
// the golden shows which entry moved if the enum is ever reordered; that
// is the one place these fixtures go below 128.
void fillAllTypes(GCModifyInformation& packet) {
    for (int type = 0; type < MODIFY_MAX; type++)
        packet.addShortData((ModifyType)type, (ushort)(0x8080 + type));
    for (int type = 0; type < MODIFY_MAX; type++)
        packet.addLongData((ModifyType)type, (DWORD)(0x81828380u + (unsigned)type));
}

COMBAT_PACKET_VARIANT(GCModifyInformation, allTypes, fillAllTypes)

// An empty record: the two count bytes and nothing else, which is what a
// packet whose result changed no stat puts on the wire.
void fillEmpty(GCModifyInformation& packet) {
    packet.clearList();
}

COMBAT_PACKET_VARIANT(GCModifyInformation, empty, fillEmpty)

void fill(GCOtherModifyInfo& packet) {
    packet.setObjectID(0x9FD0E1F2);
    fillModifyInfo(packet);
}

void expectEqual(GCOtherModifyInfo& a, GCOtherModifyInfo& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCOtherModifyInfo)

void fill(GCCreatureDied& packet) {
    packet.setObjectID(0xA0D1E2F3);
}

void expectEqual(const GCCreatureDied& a, const GCCreatureDied& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
}
COMBAT_PACKET_TESTS(GCCreatureDied)

//////////////////////////////////////////////////////////////////////
// The thrown bomb and the mine it leaves. Both carry the same creature
// list the tile packets do, in the same three-way split.
//////////////////////////////////////////////////////////////////////

void fill(GCThrowBombOK1& packet) {
    packet.setXYDir(0xA1, 0xB2, 0xC3);
    packet.setItemType(0xA4D5);
    fillCList(packet, 3, 0xA6D7E8F9);
    fillModifyInfo(packet);
}

void expectEqual(GCThrowBombOK1& a, GCThrowBombOK1& b) {
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
    EXPECT_EQ(a.getItemType(), b.getItemType());
    expectCListEqual(a, b);
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCThrowBombOK1)

void fill(GCThrowBombOK2& packet) {
    packet.setObjectID(0xA2D3E4F5);
    packet.setXYDir(0xA7, 0xB8, 0xC9);
    packet.setItemType(0xAAD1);
    fillCList(packet, 3, 0xACD3E4F5);
    fillModifyInfo(packet);
}

void expectEqual(GCThrowBombOK2& a, GCThrowBombOK2& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
    EXPECT_EQ(a.getItemType(), b.getItemType());
    expectCListEqual(a, b);
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCThrowBombOK2)

void fill(GCThrowBombOK3& packet) {
    packet.setObjectID(0xAED5E6F7);
    packet.setXYDir(0xB1, 0xC2, 0xD3);
    packet.setItemType(0xB4E5);
    fillCList(packet, 3, 0xB6E7F8D9);
}

void expectEqual(GCThrowBombOK3& a, GCThrowBombOK3& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
    EXPECT_EQ(a.getItemType(), b.getItemType());
    expectCListEqual(a, b);
}

COMBAT_PACKET_TESTS(GCThrowBombOK3)

void fill(GCMineExplosionOK1& packet) {
    packet.setXYDir(0xB7, 0xC8, 0xD9);
    packet.setItemType(0xBAE1);
    fillCList(packet, 3, 0xBCE3F4D5);
    fillModifyInfo(packet);
}

void expectEqual(GCMineExplosionOK1& a, GCMineExplosionOK1& b) {
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
    EXPECT_EQ(a.getItemType(), b.getItemType());
    expectCListEqual(a, b);
    expectModifyInfoEqual(a, b);
}

COMBAT_PACKET_TESTS(GCMineExplosionOK1)

void fill(GCMineExplosionOK2& packet) {
    packet.setXYDir(0xC1, 0xD2, 0xE3);
    packet.setItemType(0xC4F5);
    fillCList(packet, 3, 0xC6F7E8D9);
}

void expectEqual(GCMineExplosionOK2& a, GCMineExplosionOK2& b) {
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
    EXPECT_EQ(a.getItemType(), b.getItemType());
    expectCListEqual(a, b);
}

COMBAT_PACKET_TESTS(GCMineExplosionOK2)

// The blast lists are bounded the way the tile ones are: the count is
// the list, the adder refuses the id past the 255 the count byte carries,
// and the factory max budgets a full list.
template <typename BlastPacket, typename BlastFactory> void expectTheBlastListIsBounded(const char* what) {
    BlastPacket packet;
    fill(packet);
    packet.clearCList();
    for (uint i = 0; i < BlastPacket::kMaxCount; i++)
        packet.addCListElement((ObjectID_t)(0x81828384u + i));

    EXPECT_EQ(255, (int)packet.getCListNum()) << what;
    EXPECT_THROW(packet.addCListElement(0x81828384u), InvalidProtocolException) << what;

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size()) << what;

    BlastFactory factory;
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize()) << what;

    packet.popCListElement();
    EXPECT_EQ(254, (int)packet.getCListNum()) << what;
}

TEST(CombatBoundsTest, theBlastCreatureListsAreBounded) {
    expectTheBlastListIsBounded<GCThrowBombOK1, GCThrowBombOK1Factory>("GCThrowBombOK1");
    expectTheBlastListIsBounded<GCThrowBombOK2, GCThrowBombOK2Factory>("GCThrowBombOK2");
    expectTheBlastListIsBounded<GCThrowBombOK3, GCThrowBombOK3Factory>("GCThrowBombOK3");
    expectTheBlastListIsBounded<GCMineExplosionOK1, GCMineExplosionOK1Factory>("GCMineExplosionOK1");
    expectTheBlastListIsBounded<GCMineExplosionOK2, GCMineExplosionOK2Factory>("GCMineExplosionOK2");
}

// A packet read into twice declares and writes one blast.
TEST(CombatBoundsTest, aSecondReadReplacesTheBlastListItHolds) {
    GCThrowBombOK3 src;
    fill(src);

    GCThrowBombOK3 dst;
    roundTrip(src, dst, kPlainCode);
    roundTrip(src, dst, kPlainCode);

    EXPECT_EQ(3, (int)dst.getCListNum());
    EXPECT_EQ((size_t)dst.getPacketSize(), writeBody(dst, kPlainCode).size());
}

//////////////////////////////////////////////////////////////////////
// The shape of the stat record itself.
//////////////////////////////////////////////////////////////////////

// The tag byte, the two counts and the two value widths are the whole
// record. A record holding every type twice measures the maximum the
// count bytes allow, so the max size budgets exactly a full pair of
// 255-entry lists.
TEST(ModifyInfoTest, theMaximumBudgetsTwoFullLists) {
    const PacketSize_t counts = (PacketSize_t)(szBYTE * 2);
    const PacketSize_t shorts = (PacketSize_t)(255 * (szBYTE + szshort));
    const PacketSize_t longs = (PacketSize_t)(255 * (szBYTE + szDWORD));
    EXPECT_EQ(counts + shorts + longs, ModifyInfo::getPacketMaxSize());

    GCModifyInformation packet;
    for (int i = 0; i < 255; i++) {
        packet.addShortData(MODIFY_CURRENT_HP, (ushort)(0x8181 + i));
        packet.addLongData(MODIFY_GOLD, (DWORD)(0x81828384u + (unsigned)i));
    }
    EXPECT_EQ(255, (int)packet.getShortCount());
    EXPECT_EQ(255, (int)packet.getLongCount());

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());
    EXPECT_EQ((size_t)ModifyInfo::getPacketMaxSize(), body.size());

    GCModifyInformationFactory factory;
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize());
}

// The debug string indexes ModifyType2String with the tag byte, so the
// table has to hold one name per enumerator plus the sentinel.
TEST(ModifyInfoTest, theTypeNameTableCoversEveryEnumerator) {
    EXPECT_EQ((size_t)MODIFY_MAX + 1, sizeof(ModifyType2String) / sizeof(ModifyType2String[0]));
}

//////////////////////////////////////////////////////////////////////
// The bounds the counts and the factory maxima agree on.
//////////////////////////////////////////////////////////////////////

// Each list is counted in a BYTE derived from the list, so the record
// stops where the count byte and the maximum do and the entry past it is
// refused rather than wrapping the count to zero.
TEST(ModifyInfoTest, anEntryPastTheCountByteIsRefused) {
    GCModifyInformation packet;
    for (uint i = 0; i < ModifyInfo::kMaxCount; i++) {
        packet.addShortData(MODIFY_CURRENT_HP, (ushort)(0x8181 + i));
        packet.addLongData(MODIFY_GOLD, (DWORD)(0x81828384u + i));
    }

    EXPECT_THROW(packet.addShortData(MODIFY_CURRENT_HP, 0x8181), InvalidProtocolException);
    EXPECT_THROW(packet.addLongData(MODIFY_GOLD, 0x81828384u), InvalidProtocolException);

    EXPECT_EQ((int)ModifyInfo::kMaxCount, (int)packet.getShortCount());
    EXPECT_EQ((int)ModifyInfo::kMaxCount, (int)packet.getLongCount());

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());
    EXPECT_EQ((size_t)ModifyInfo::getPacketMaxSize(), body.size());
    EXPECT_EQ((int)ModifyInfo::kMaxCount, (int)body[0]);
}

// The tile packets bound their creature list the same way, and
// popCListElement() takes the id off the count with the list.
TEST(GCSkillToTileOK1Test, aCreatureListPastTheCountByteIsRefusedAndPoppingDecrementsIt) {
    GCSkillToTileOK1 packet;
    fillEmptyArea(packet);
    fillCList(packet, (int)GCSkillToTileOK1::kMaxCount, 0x81828384);

    EXPECT_EQ((int)GCSkillToTileOK1::kMaxCount, (int)packet.getCListNum());
    EXPECT_THROW(packet.addCListElement(0x91A2B3C4), InvalidProtocolException);
    EXPECT_EQ((int)GCSkillToTileOK1::kMaxCount, (int)packet.getCListNum());
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());

    GCSkillToTileOK4 popped;
    popped.addCListElement(0x91A2B3C4);
    popped.addCListElement(0x95A6B7C8);
    ASSERT_EQ(2, (int)popped.getCListNum());
    EXPECT_EQ((ObjectID_t)0x91A2B3C4, popped.popCListElement());
    EXPECT_EQ(1, (int)popped.getCListNum());
    EXPECT_EQ((size_t)popped.getPacketSize(), writeBody(popped, kPlainCode).size());
}

template <typename TilePacket, typename TileFactory>
void expectAFullSweepFitsTheFactoryMax(TilePacket& packet, const char* what) {
    fillCList(packet, (int)TilePacket::kMaxCount, 0x81828384);

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size()) << what;

    TileFactory factory;
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize())
        << what << "'s factory max no longer budgets the creature list its count byte carries";
}

// A tile packet's factory max budgets the full 255-id list the count byte
// carries, so the widest sweep still fits the read buffer the receiver
// sizes from that maximum.
TEST(CombatBoundsTest, aFullSweepFitsEveryTileFactoryMax) {
    // The three that also embed the stat record need it full as well:
    // their maximum budgets a whole record beside the whole list.
    GCSkillToTileOK1 tile1;
    fill(tile1);
    tile1.clearCList();
    for (int i = 3; i < 255; i++)
        tile1.addShortData(MODIFY_CURRENT_HP, (ushort)(0x8181 + i));
    for (int i = 2; i < 255; i++)
        tile1.addLongData(MODIFY_GOLD, (DWORD)(0x81828384u + (unsigned)i));
    expectAFullSweepFitsTheFactoryMax<GCSkillToTileOK1, GCSkillToTileOK1Factory>(tile1, "GCSkillToTileOK1");

    GCSkillToTileOK2 tile2;
    fill(tile2);
    tile2.clearCList();
    for (int i = 3; i < 255; i++)
        tile2.addShortData(MODIFY_CURRENT_HP, (ushort)(0x8181 + i));
    for (int i = 2; i < 255; i++)
        tile2.addLongData(MODIFY_GOLD, (DWORD)(0x81828384u + (unsigned)i));
    expectAFullSweepFitsTheFactoryMax<GCSkillToTileOK2, GCSkillToTileOK2Factory>(tile2, "GCSkillToTileOK2");

    GCSkillToTileOK6 tile6;
    fill(tile6);
    tile6.clearCList();
    for (int i = 3; i < 255; i++)
        tile6.addShortData(MODIFY_CURRENT_HP, (ushort)(0x8181 + i));
    for (int i = 2; i < 255; i++)
        tile6.addLongData(MODIFY_GOLD, (DWORD)(0x81828384u + (unsigned)i));
    expectAFullSweepFitsTheFactoryMax<GCSkillToTileOK6, GCSkillToTileOK6Factory>(tile6, "GCSkillToTileOK6");

    // The other two carry no stat record, so the list is the whole of it.
    GCSkillToTileOK4 tile4;
    fill(tile4);
    tile4.clearCList();
    expectAFullSweepFitsTheFactoryMax<GCSkillToTileOK4, GCSkillToTileOK4Factory>(tile4, "GCSkillToTileOK4");

    GCSkillToTileOK5 tile5;
    fill(tile5);
    tile5.clearCList();
    expectAFullSweepFitsTheFactoryMax<GCSkillToTileOK5, GCSkillToTileOK5Factory>(tile5, "GCSkillToTileOK5");
}

// A reader is reused across packets, so read() replaces the record it
// holds: two reads of the same record leave one record's worth of
// entries, and the bytes it writes back are the bytes it was sent.
TEST(ModifyInfoTest, aSecondReadReplacesTheRecordItAlreadyHolds) {
    GCModifyInformation src;
    fill(src);

    GCModifyInformation dst;
    roundTrip(src, dst, kPlainCode);
    roundTrip(src, dst, kPlainCode);

    EXPECT_EQ(3, (int)dst.getShortCount());
    EXPECT_EQ(2, (int)dst.getLongCount());

    const std::vector<unsigned char> body = writeBody(dst, kPlainCode);
    EXPECT_EQ((size_t)dst.getPacketSize(), body.size());
    EXPECT_EQ(writeBody(src, kPlainCode), body);
}

// The tile packets' read() replaces the creature list the same way.
TEST(GCSkillToTileOK4Test, aSecondReadReplacesTheCreatureListItAlreadyHolds) {
    GCSkillToTileOK4 src;
    fill(src);

    GCSkillToTileOK4 dst;
    roundTrip(src, dst, kPlainCode);
    roundTrip(src, dst, kPlainCode);

    EXPECT_EQ(3, (int)dst.getCListNum());

    const std::vector<unsigned char> body = writeBody(dst, kPlainCode);
    EXPECT_EQ((size_t)dst.getPacketSize(), body.size());
    EXPECT_EQ(writeBody(src, kPlainCode), body);
}

// The tag is checked against the enum on both sides: the adder refuses a
// value past the last ModifyType, and so does the reader, which is what
// keeps toString() off the end of ModifyType2String. 127 is the largest
// value the enum's own range admits, so the fixture can carry it without
// the load itself being the defect.
TEST(ModifyInfoTest, aTagOutsideTheEnumIsRefused) {
    const int kTagPastTheEnum = 127;
    ASSERT_GT(kTagPastTheEnum, (int)MODIFY_MAX);

    GCModifyInformation packet;
    EXPECT_THROW(packet.addShortData((ModifyType)kTagPastTheEnum, 0x81A2), InvalidProtocolException);
    EXPECT_THROW(packet.addLongData((ModifyType)kTagPastTheEnum, 0x81A2B3C4), InvalidProtocolException);
    EXPECT_EQ(0, (int)packet.getShortCount());
    EXPECT_EQ(0, (int)packet.getLongCount());

    // A peer sends bytes, not enumerators, so the reader carries the same
    // refusal. The record here is one short entry with the bad tag and an
    // empty long list.
    Loopback link;
    link.setCodes(kPlainCode);
    link.out().write((BYTE)1);
    link.out().write((BYTE)kTagPastTheEnum);
    link.out().write((ushort)0x81A2);
    link.out().write((BYTE)0);
    link.pump(szBYTE * 3 + szshort);

    GCModifyInformation dst;
    EXPECT_THROW(dst.read(link.in()), InvalidProtocolException);
}

// Both packets carry an ObjectID_t on the wire and hand back the member
// they hold, so two creatures whose ids differ only above bit 16 are two
// creatures to the caller.
TEST(CombatAccessorTest, theObjectIDGettersReturnTheFullId) {
    const ObjectID_t kID = 0x81A2B3C4;

    GCSkillToTileOK3 tile;
    tile.setObjectID(kID);
    EXPECT_EQ(kID, tile.getObjectID());

    GCSkillToInventoryOK2 inventory;
    inventory.setObjectID(kID);
    EXPECT_EQ(kID, inventory.getObjectID());
}

//////////////////////////////////////////////////////////////////////
// What the default constructor leaves, and what the hit flag reads.
//////////////////////////////////////////////////////////////////////

// Each packet is built twice over storage poisoned with a different
// byte, so a member the constructor leaves alone reaches the wire as that
// byte and the two bodies differ.
template <typename PacketType> void expectEveryMemberIsInitialised(const char* what) {
    alignas(PacketType) unsigned char storage[sizeof(PacketType)];
    const unsigned char poison[2] = {0x00, 0xFF};
    std::vector<unsigned char> bodies[2];

    for (int i = 0; i < 2; i++) {
        memset(storage, poison[i], sizeof(storage));
        PacketType* pPacket = new (storage) PacketType();
        bodies[i] = writeBody(*pPacket, kPlainCode);
        pPacket->~PacketType();
    }

    EXPECT_EQ(bodies[0], bodies[1]) << what << ": its default constructor leaves a member write() emits uninitialised";
}

TEST(CombatConstructorTest, everyPacketInitialisesEveryMemberItWrites) {
    expectEveryMemberIsInitialised<GCAttack>("GCAttack");
    expectEveryMemberIsInitialised<GCGetDamage>("GCGetDamage");
    expectEveryMemberIsInitialised<GCAttackMeleeOK1>("GCAttackMeleeOK1");
    expectEveryMemberIsInitialised<GCAttackMeleeOK2>("GCAttackMeleeOK2");
    expectEveryMemberIsInitialised<GCAttackMeleeOK3>("GCAttackMeleeOK3");
    expectEveryMemberIsInitialised<GCAttackArmsOK1>("GCAttackArmsOK1");
    expectEveryMemberIsInitialised<GCAttackArmsOK2>("GCAttackArmsOK2");
    expectEveryMemberIsInitialised<GCAttackArmsOK3>("GCAttackArmsOK3");
    expectEveryMemberIsInitialised<GCAttackArmsOK4>("GCAttackArmsOK4");
    expectEveryMemberIsInitialised<GCAttackArmsOK5>("GCAttackArmsOK5");
    expectEveryMemberIsInitialised<GCSkillToObjectOK1>("GCSkillToObjectOK1");
    expectEveryMemberIsInitialised<GCSkillToObjectOK2>("GCSkillToObjectOK2");
    expectEveryMemberIsInitialised<GCSkillToObjectOK3>("GCSkillToObjectOK3");
    expectEveryMemberIsInitialised<GCSkillToObjectOK4>("GCSkillToObjectOK4");
    expectEveryMemberIsInitialised<GCSkillToObjectOK5>("GCSkillToObjectOK5");
    expectEveryMemberIsInitialised<GCSkillToObjectOK6>("GCSkillToObjectOK6");
    expectEveryMemberIsInitialised<GCSkillToSelfOK1>("GCSkillToSelfOK1");
    expectEveryMemberIsInitialised<GCSkillToSelfOK2>("GCSkillToSelfOK2");
    expectEveryMemberIsInitialised<GCSkillToSelfOK3>("GCSkillToSelfOK3");
    expectEveryMemberIsInitialised<GCSkillToTileOK1>("GCSkillToTileOK1");
    expectEveryMemberIsInitialised<GCSkillToTileOK2>("GCSkillToTileOK2");
    expectEveryMemberIsInitialised<GCSkillToTileOK3>("GCSkillToTileOK3");
    expectEveryMemberIsInitialised<GCSkillToTileOK4>("GCSkillToTileOK4");
    expectEveryMemberIsInitialised<GCSkillToTileOK5>("GCSkillToTileOK5");
    expectEveryMemberIsInitialised<GCSkillToTileOK6>("GCSkillToTileOK6");
    expectEveryMemberIsInitialised<GCSkillToInventoryOK1>("GCSkillToInventoryOK1");
    expectEveryMemberIsInitialised<GCSkillToInventoryOK2>("GCSkillToInventoryOK2");
    expectEveryMemberIsInitialised<GCSkillFailed1>("GCSkillFailed1");
    expectEveryMemberIsInitialised<GCSkillFailed2>("GCSkillFailed2");
    expectEveryMemberIsInitialised<GCStatusCurrentHP>("GCStatusCurrentHP");
    expectEveryMemberIsInitialised<GCModifyInformation>("GCModifyInformation");
    expectEveryMemberIsInitialised<GCOtherModifyInfo>("GCOtherModifyInfo");
    expectEveryMemberIsInitialised<GCCreatureDied>("GCCreatureDied");
    expectEveryMemberIsInitialised<GCThrowBombOK1>("GCThrowBombOK1");
    expectEveryMemberIsInitialised<GCThrowBombOK2>("GCThrowBombOK2");
    expectEveryMemberIsInitialised<GCThrowBombOK3>("GCThrowBombOK3");
    expectEveryMemberIsInitialised<GCMineExplosionOK1>("GCMineExplosionOK1");
    expectEveryMemberIsInitialised<GCMineExplosionOK2>("GCMineExplosionOK2");
}

// The hit flag is a byte on the wire. Both readers take a BYTE and
// narrow it, so a value the client's own bool would not admit is a hit
// rather than an object no load may touch.
TEST(GCAttackArmsOK1Test, aHitFlagByteOtherThanZeroOrOneReadsAsAHit) {
    Loopback link;
    link.setCodes(kPlainCode);
    link.out().write((SkillType_t)0x86B7);
    link.out().write((ObjectID_t)0x88A9BACB);
    link.out().write((Bullet_t)0x8C);
    link.out().write((BYTE)0x7F);
    link.out().write((BYTE)0);
    link.out().write((BYTE)0);
    link.pump(szSkillType + szObjectID + szBullet + szBYTE * 3);

    GCAttackArmsOK1 dst;
    dst.read(link.in());

    EXPECT_TRUE(dst.getSkillSuccess());
    EXPECT_EQ((ObjectID_t)0x88A9BACB, dst.getObjectID());
}

TEST(GCAttackArmsOK5Test, aHitFlagByteOtherThanZeroOrOneReadsAsAHit) {
    Loopback link;
    link.setCodes(kPlainCode);
    link.out().write((SkillType_t)0x8ABB);
    link.out().write((ObjectID_t)0x8CADBECF);
    link.out().write((ObjectID_t)0x90D1E2F3);
    link.out().write((BYTE)0x7F);
    link.pump(szSkillType + szObjectID * 2 + szBYTE);

    GCAttackArmsOK5 dst;
    dst.read(link.in());

    EXPECT_TRUE(dst.getSkillSuccess());
    EXPECT_EQ((ObjectID_t)0x90D1E2F3, dst.getTargetObjectID());
}

} // namespace
