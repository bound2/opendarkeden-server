//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_skill_test.cpp
// Description : Golden byte fixtures, loopback round trips and size
//               pins for the packets a client and a game server
//               exchange while a character advances: learning and
//               dropping a skill, casting one, spending a bonus or a
//               power point, picking a rank bonus and picking a blood
//               bible sign.
//
//               The set is taken from the code that sends them: the
//               progression handlers under
//               src/server/gameserver/handler (CGLearnSkill,
//               CGDownSkill, CGCastingSkill, CGSkillToNamed,
//               CGUseBonusPoint, CGUsePowerPoint,
//               CGRequestPowerPoint, CGSelectRankBonus,
//               CGSelectBloodBible), the skill book the three race
//               classes build (Slayer.cpp, Vampire.cpp, Ousters.cpp),
//               skill/SkillUtil.cpp, PlayerCreature.cpp,
//               SweeperBonusManager.cpp, BloodBibleBonusManager.cpp,
//               ShrineInfoManager.cpp, InitAllStat.cpp,
//               mofus/MPlayerManager.cpp and the three quest actions
//               that drive the same dialogue (ActionTeachSkill,
//               ActionSelectBloodBible, ActionClearBloodBible).
//               Twenty-nine packets, each with the reason it is here:
//
//               CGLearnSkill     the player asks to learn the next
//                                skill of a domain, and
//               CGDownSkill      to give one back.
//               CGCastingSkill   the client says a cast has begun.
//               CGSkillToNamed   and aims one at a player it names
//                                rather than at an object or a tile.
//
//               CGUseBonusPoint  the player spends one bonus point on
//                                a stat,
//               CGUsePowerPoint  redeems power points for an item, and
//               CGRequestPowerPoint  asks what the account has, by
//                                cell number.
//               CGSelectRankBonus    the player picks one rank bonus,
//               CGSelectBloodBible   and one blood bible sign.
//
//               GCSkillInfo      the whole skill book, sent once as
//                                the character loads: one record per
//                                domain for a slayer, one for a
//                                vampire and one for an ousters, each
//                                a different shape.
//               GCTeachSkillInfo the domain and level an NPC teaches.
//               GCLearnSkillOK   the skill was learned, was refused,
//               GCLearnSkillFailed  or is now within reach.
//               GCLearnSkillReady
//               GCDownSkillOK    the skill was dropped, or the drop
//               GCDownSkillFailed   was refused.
//               GCCastingSkill   the cast the server accepted.
//
//               GCUseBonusPointOK    the stat change a spent bonus
//                                point produced, as the same stat
//                                record the combat packets carry;
//               GCUseBonusPointFail  and the bodiless refusal.
//               GCUsePowerPointResult    what the redeemed points
//                                bought, and what is left.
//               GCRequestPowerPointResult   the account's points.
//
//               GCRankBonusInfo  the rank bonuses on offer,
//               GCSelectRankBonusOK  the one that was taken, and
//               GCSelectRankBonusFailed  why one was not.
//               GCSweeperBonusInfo   the sweeper bonuses in force,
//               GCHolyLandBonusInfo  and the holy land ones.
//
//               GCBloodBibleList     the signs an NPC offers,
//               GCBloodBibleSignInfo the signs a character holds, and
//               GCBloodBibleStatus   where one blood bible now is.
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
//               twenty-nine derives from one of them: twenty-eight
//               extend Packet directly and GCUseBonusPointOK extends
//               ModifyInfo, which is a record and calls neither. So
//               each golden is recorded at code 0 and its test also
//               asserts the bytes do not vary with the code - adopting
//               the encrypter fails loudly instead of silently voiding
//               the pin. The four aimed-skill requests the same
//               handlers answer, CGSkillToObject, CGSkillToSelf,
//               CGSkillToTile and CGSkillToInventory, are encrypter
//               users and are pinned there; CGSkillToNamed is the one
//               that is not, so it is here.
//
//               Already pinned elsewhere, so the answers these
//               handlers share with other families are not repeated:
//               GCSkillFailed1 and GCSkillFailed2, the six
//               GCSkillToObjectOK, three GCSkillToSelfOK, six
//               GCSkillToTileOK and two GCSkillToInventoryOK packets
//               and the ModifyInfo record itself
//               (packet_combat_test.cpp), GCSystemMessage
//               (packet_gameserver_handshake_test.cpp), GCNPCResponse
//               (packet_guild_test.cpp). The BloodBibleSignInfo record
//               GCBloodBibleSignInfo carries is pinned in
//               packet_gameserver_handshake_test.cpp, which is where
//               GCUpdateInfo carries it; it appears here only as the
//               contents of a packet, so the packet's own offsets are
//               pinned with it. RideMotorcycleSlotInfo belongs to
//               RideMotorcycleInfo, which GCUpdateInfo carries and the
//               same file pins - no packet in this set holds one.
//
//               GCSkillInfo already had a frame-size pin in
//               packet_roundtrip_test.cpp for the ousters branch, and
//               GCBloodBibleStatus a name-width pin; both are
//               single-aspect and neither has a golden. Both packets
//               are pinned in full here, under test names of their own.
//
//               GCPetUseSkill is excluded: it has a registered factory
//               but its only sender is Pet.cpp's attack path, which
//               belongs to the combat feedback family and not to
//               character progression. Every one of the twenty-nine
//               has a registered factory in
//               tests/ratchet/factory_registrations.txt and at least
//               one sender outside src/Core.
//
//               Each packet gets three pins (SKILL_PACKET_TESTS):
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
//               GCBloodBibleSignInfo gets those pins written out by
//               hand: it holds its record by pointer and needs a
//               fixture that owns it.
//
//               Extra goldens cover the branches one fixture cannot:
//               GCSkillInfo is written once per race, because the
//               three records it may carry have three different
//               shapes (.vampire and .ousters beside the slayer
//               canonical), with no record at all (.empty) and with a
//               record whose skill list is empty (.noskills); the
//               counted lists are written empty (.empty on
//               GCRankBonusInfo, GCSweeperBonusInfo,
//               GCHolyLandBonusInfo, GCBloodBibleList and
//               GCBloodBibleSignInfo) and the sign list is written
//               full (.full on GCBloodBibleSignInfo, the branch
//               signCount() caps); the stat record a spent bonus point
//               produces is written with nothing in it (.nostats on
//               GCUseBonusPointOK, the shape the handler actually
//               sends); and a blood bible that belongs to nobody is
//               written with an empty owner name (.noowner on
//               GCBloodBibleStatus).
//
//               Fixture values are distinct per field and >= 128 in
//               every byte the width allows. Four groups cannot follow
//               that rule and say so at the point of use: the cell
//               number and the target and owner names, which are text;
//               GCSkillInfo's pc type, which selects which record
//               read() builds; the two learn flags and the per-skill
//               enable flag, which are bool; and the ModifyType tags of
//               the stat record, which must be enumerators.
//
//               Findings. Each is stated as a test that fails once the
//               packet is fixed, except where noted:
//
//               - GCSkillInfo derives the record count into a BYTE it
//                 caps nowhere, so the 256th record wraps the count to
//                 zero while write() still emits every record.
//               - GCSkillInfoFactory's maximum is one SlayerSkillInfo
//                 and nothing else: it budgets neither the pc type byte
//                 nor the record count byte, and no second record,
//                 although a slayer's book carries one record per
//                 domain. A single full record already outgrows it.
//               - The three race records keep their skill count in a
//                 BYTE the caller sets by hand while addListElement
//                 leaves it alone, so write() emits that count and then
//                 the whole list: a record whose list is longer than
//                 its count sends skills the reader never consumes, and
//                 getPacketSize() counts them.
//               - GCSkillInfo::read appends to the record list the
//                 packet already holds instead of replacing it, so a
//                 reused packet grows by one book per read.
//               - GCSkillInfo::write emits any pc type byte while
//                 read() refuses every one that is not PC_SLAYER,
//                 PC_VAMPIRE or PC_OUSTERS, so a book announced with
//                 any other type is written and cannot be read back -
//                 and the type is one of the members the constructor
//                 leaves alone.
//               - GCRankBonusInfo, GCSweeperBonusInfo and
//                 GCHolyLandBonusInfo derive their count byte from the
//                 list and cap nothing, so the 256th entry wraps the
//                 count to zero and the body outgrows a factory max
//                 that budgets 100, 12 and 12 entries.
//               - Those three and GCBloodBibleList append to the list
//                 the packet already holds instead of replacing it.
//               - SweeperBonusInfo and BloodBibleBonusInfo write and
//                 read only their race byte: the type byte and the
//                 option list are commented out on both sides, so the
//                 type both managers set on every record never reaches
//                 the wire.
//               - GCBloodBibleSignInfo leaves its record pointer
//                 uninitialised, so getPacketSize() and write()
//                 dereference an indeterminate value when a sender
//                 skips setSignInfo.
//
//               Four findings are recorded here rather than tested,
//               because reaching them is undefined behaviour or has no
//               observable wire effect:
//
//               - The learn flag of each race record and the enable
//                 flag of a slayer skill are read straight into a bool,
//                 so a peer sending any byte but 0 or 1 leaves a bool
//                 holding a value no bool may hold. Every fixture here
//                 writes true or false for that reason.
//               - GCBloodBibleSignInfo::read allocates a new record on
//                 every call without freeing the one the packet held,
//                 so a reused packet leaks a record and a delivered one
//                 is never freed at all - the destructor drops the
//                 pointer.
//               - BloodBibleSignInfo::read appends to the sign list it
//                 already holds, which a second read would grow past
//                 the six slots write() emits. It is unreachable
//                 through the packet only because read() allocates a
//                 fresh record each time.
//               - GCUsePowerPointResult::read and
//                 GCRequestPowerPointResult::read take their code bytes
//                 without comparing them against the last enumerator of
//                 the RESULT_CODE and ITEM_CODE lists their own headers
//                 declare, so a peer can announce a result no branch of
//                 the client handles.
//
//               Eighteen of the twenty-nine leave at least one member
//               the default constructor never sets, so a packet sent
//               without every setter called puts indeterminate bytes on
//               the wire. The poisoned-storage pin at the end of the
//               file asserts today's split. CGUsePowerPoint and
//               GCUseBonusPointFail are in neither list because their
//               body is empty; CGRequestPowerPoint and CGSkillToNamed
//               are in neither because their one text field is required
//               non-empty, so a default-constructed body is refused
//               rather than written; and GCBloodBibleSignInfo is in
//               neither because writing one over poisoned storage would
//               follow the indeterminate record pointer, so it gets a
//               pin on the pointer itself instead.
//
//////////////////////////////////////////////////////////////////////

#include <cstring>
#include <new>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "BloodBibleBonusInfo.h"
#include "BloodBibleSignInfo.h"
#include "CGCastingSkill.h"
#include "CGDownSkill.h"
#include "CGLearnSkill.h"
#include "CGRequestPowerPoint.h"
#include "CGSelectBloodBible.h"
#include "CGSelectRankBonus.h"
#include "CGSkillToNamed.h"
#include "CGUseBonusPoint.h"
#include "CGUsePowerPoint.h"
#include "Exception.h"
#include "GCBloodBibleList.h"
#include "GCBloodBibleSignInfo.h"
#include "GCBloodBibleStatus.h"
#include "GCCastingSkill.h"
#include "GCDownSkillFailed.h"
#include "GCDownSkillOK.h"
#include "GCHolyLandBonusInfo.h"
#include "GCLearnSkillFailed.h"
#include "GCLearnSkillOK.h"
#include "GCLearnSkillReady.h"
#include "GCRankBonusInfo.h"
#include "GCRequestPowerPointResult.h"
#include "GCSelectRankBonusFailed.h"
#include "GCSelectRankBonusOK.h"
#include "GCSkillInfo.h"
#include "GCSweeperBonusInfo.h"
#include "GCTeachSkillInfo.h"
#include "GCUseBonusPointFail.h"
#include "GCUseBonusPointOK.h"
#include "GCUsePowerPointResult.h"
#include "OustersSkillInfo.h"
#include "SlayerSkillInfo.h"
#include "SweeperBonusInfo.h"
#include "TestStreams.h"
#include "VampireSkillInfo.h"

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
// comparing a list consumes it.
//////////////////////////////////////////////////////////////////////

#define SKILL_PACKET_GOLDEN_AND_SIZE(Name)                                                           \
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

#define SKILL_PACKET_TESTS(Name)                  \
    TEST(Name##Test, roundTripsThroughLoopback) { \
        Name src;                                 \
        fill(src);                                \
        Name dst;                                 \
        roundTrip(src, dst, kPlainCode);          \
        expectEqual(src, dst);                    \
    }                                             \
    SKILL_PACKET_GOLDEN_AND_SIZE(Name)

// A second fixture for a packet whose write() has a branch the canonical
// one does not take.
#define SKILL_PACKET_VARIANT(Name, Variant, fillVariant)                       \
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
// Learning, dropping and casting a skill.
//////////////////////////////////////////////////////////////////////

// The domain byte selects no branch in any read(), so it follows the
// >= 128 rule like every other field here.
void fill(CGLearnSkill& packet) {
    packet.setSkillType(0x81A2);
    packet.setSkillDomainType(0x83);
}

void expectEqual(CGLearnSkill& a, CGLearnSkill& b) {
    EXPECT_EQ((int)a.getSkillType(), (int)b.getSkillType());
    EXPECT_EQ((int)a.getSkillDomainType(), (int)b.getSkillDomainType());
}

SKILL_PACKET_TESTS(CGLearnSkill)

void fill(CGDownSkill& packet) {
    packet.setSkillType(0x84A5);
}

void expectEqual(CGDownSkill& a, CGDownSkill& b) {
    EXPECT_EQ((int)a.getSkillType(), (int)b.getSkillType());
}

SKILL_PACKET_TESTS(CGDownSkill)

// CGCastingSkill's only setter is named setObjectID, but the field it
// writes is the skill type.
void fill(CGCastingSkill& packet) {
    packet.setObjectID(0x86A7);
}

void expectEqual(CGCastingSkill& a, CGCastingSkill& b) {
    EXPECT_EQ((int)a.getSkillType(), (int)b.getSkillType());
}

SKILL_PACKET_TESTS(CGCastingSkill)

// The target name is text, so it cannot follow the >= 128 rule; it is
// held to the 20 bytes the factory max budgets.
void fill(CGSkillToNamed& packet) {
    packet.setSkillType(0x88A9);
    packet.setCEffectID(0x8AAB);
    packet.setTargetName("Bloodstained Hunter");
}

void expectEqual(CGSkillToNamed& a, CGSkillToNamed& b) {
    EXPECT_EQ((int)a.getSkillType(), (int)b.getSkillType());
    EXPECT_EQ((int)a.getCEffectID(), (int)b.getCEffectID());
    EXPECT_EQ(a.getTargetName(), b.getTargetName());
}

SKILL_PACKET_TESTS(CGSkillToNamed)

//////////////////////////////////////////////////////////////////////
// Bonus points, power points, rank bonuses and blood bible signs.
//////////////////////////////////////////////////////////////////////

// The stat byte picks a branch in the handler, not in read(), so it
// follows the >= 128 rule.
void fill(CGUseBonusPoint& packet) {
    packet.setWhich(0x8C);
}

void expectEqual(CGUseBonusPoint& a, CGUseBonusPoint& b) {
    EXPECT_EQ((int)a.getWhich(), (int)b.getWhich());
}

SKILL_PACKET_TESTS(CGUseBonusPoint)

// CGUsePowerPoint carries no body at all: the packet id is the whole
// message.
void fill(CGUsePowerPoint&) {}

void expectEqual(CGUsePowerPoint& a, CGUsePowerPoint& b) {
    EXPECT_EQ(0, (int)a.getPacketSize());
    EXPECT_EQ(0, (int)b.getPacketSize());
}

SKILL_PACKET_TESTS(CGUsePowerPoint)

// The cell number is text, so it cannot follow the >= 128 rule; it is
// held to the 12 bytes the factory max budgets.
void fill(CGRequestPowerPoint& packet) {
    packet.setCellNum("01098765432");
}

void expectEqual(CGRequestPowerPoint& a, CGRequestPowerPoint& b) {
    EXPECT_EQ(a.getCellNum(), b.getCellNum());
}

SKILL_PACKET_TESTS(CGRequestPowerPoint)

void fill(CGSelectRankBonus& packet) {
    packet.setRankBonusType(0x8DAEBFC0);
}

void expectEqual(CGSelectRankBonus& a, CGSelectRankBonus& b) {
    EXPECT_EQ(a.getRankBonusType(), b.getRankBonusType());
}

SKILL_PACKET_TESTS(CGSelectRankBonus)

void fill(CGSelectBloodBible& packet) {
    packet.setBloodBibleID(0x91C2);
}

void expectEqual(CGSelectBloodBible& a, CGSelectBloodBible& b) {
    EXPECT_EQ((int)a.getBloodBibleID(), (int)b.getBloodBibleID());
}

SKILL_PACKET_TESTS(CGSelectBloodBible)

//////////////////////////////////////////////////////////////////////
// The skill book, and the three race records it carries.
//////////////////////////////////////////////////////////////////////

// The enable flag is a bool, so it holds true or false rather than a
// high byte.
SubSlayerSkillInfo* makeSlayerSkill(SkillType_t type, Exp_t exp, ExpLevel_t level, Turn_t interval, Turn_t casting,
                                    bool enable) {
    SubSlayerSkillInfo* pSkill = new SubSlayerSkillInfo();
    pSkill->setSkillType(type);
    pSkill->setSkillExp(exp);
    pSkill->setSkillExpLevel(level);
    pSkill->setSkillTurn(interval);
    pSkill->setCastingTime(casting);
    pSkill->setEnable(enable);
    return pSkill;
}

SubVampireSkillInfo* makeVampireSkill(SkillType_t type, Turn_t interval, Turn_t casting) {
    SubVampireSkillInfo* pSkill = new SubVampireSkillInfo();
    pSkill->setSkillType(type);
    pSkill->setSkillTurn(interval);
    pSkill->setCastingTime(casting);
    return pSkill;
}

SubOustersSkillInfo* makeOustersSkill(SkillType_t type, ExpLevel_t level, Turn_t interval, Turn_t casting) {
    SubOustersSkillInfo* pSkill = new SubOustersSkillInfo();
    pSkill->setSkillType(type);
    pSkill->setExpLevel(level);
    pSkill->setSkillTurn(interval);
    pSkill->setCastingTime(casting);
    return pSkill;
}

// The slayer record is the only one of the three that carries a domain
// byte. The count is set by hand because addListElement does not touch
// it - that disagreement is a finding below.
SlayerSkillInfo* makeSlayerRecord(bool learnNew, SkillDomainType_t domain, int skills) {
    SlayerSkillInfo* pRecord = new SlayerSkillInfo();
    pRecord->setLearnNewSkill(learnNew);
    pRecord->setDomainType(domain);
    for (int i = 0; i < skills; i++)
        pRecord->addListElement(makeSlayerSkill((SkillType_t)(0x92A3 + i * 0x0101), (Exp_t)(0x94A5B6C7 + i),
                                                (ExpLevel_t)(0x98A9 + i), (Turn_t)(0x9AABBCCD + i),
                                                (Turn_t)(0x9EAFB0C1 + i), (i % 2) == 0));
    pRecord->setListNum((BYTE)skills);
    return pRecord;
}

VampireSkillInfo* makeVampireRecord(bool learnNew, int skills) {
    VampireSkillInfo* pRecord = new VampireSkillInfo();
    pRecord->setLearnNewSkill(learnNew);
    for (int i = 0; i < skills; i++)
        pRecord->addListElement(
            makeVampireSkill((SkillType_t)(0xA2B3 + i * 0x0101), (Turn_t)(0xA4B5C6D7 + i), (Turn_t)(0xA8B9CADB + i)));
    pRecord->setListNum((BYTE)skills);
    return pRecord;
}

OustersSkillInfo* makeOustersRecord(bool learnNew, int skills) {
    OustersSkillInfo* pRecord = new OustersSkillInfo();
    pRecord->setLearnNewSkill(learnNew);
    for (int i = 0; i < skills; i++)
        pRecord->addListElement(makeOustersSkill((SkillType_t)(0xB2C3 + i * 0x0101), (ExpLevel_t)(0xB4C5 + i),
                                                 (Turn_t)(0xB6C7D8E9 + i), (Turn_t)(0xBACBDCED + i)));
    pRecord->setListNum((BYTE)skills);
    return pRecord;
}

// The pc type selects which record read() builds, so it must be one of
// the three the switch handles.
void fill(GCSkillInfo& packet) {
    packet.setPCType(PC_SLAYER);
    packet.addListElement(makeSlayerRecord(true, 0x91, 2));
    packet.addListElement(makeSlayerRecord(false, 0xB2, 1));
}

void fillVampireBook(GCSkillInfo& packet) {
    packet.setPCType(PC_VAMPIRE);
    packet.addListElement(makeVampireRecord(true, 2));
}

void fillOustersBook(GCSkillInfo& packet) {
    packet.setPCType(PC_OUSTERS);
    packet.addListElement(makeOustersRecord(false, 2));
}

void fillEmptyBook(GCSkillInfo& packet) {
    packet.setPCType(PC_SLAYER);
}

void fillSkilllessRecord(GCSkillInfo& packet) {
    packet.setPCType(PC_SLAYER);
    packet.addListElement(makeSlayerRecord(true, 0xC3, 0));
}

void expectSlayerRecordEqual(SlayerSkillInfo& a, SlayerSkillInfo& b) {
    EXPECT_EQ((int)a.isLearnNewSkill(), (int)b.isLearnNewSkill());
    EXPECT_EQ((int)a.getDomainiType(), (int)b.getDomainiType());
    ASSERT_EQ((int)a.getListNum(), (int)b.getListNum());

    const int skills = (int)a.getListNum();
    for (int i = 0; i < skills; i++) {
        SubSlayerSkillInfo* pLeft = a.popFrontListElement();
        SubSlayerSkillInfo* pRight = b.popFrontListElement();
        EXPECT_EQ((int)pLeft->getSkillType(), (int)pRight->getSkillType()) << "slayer skill " << i;
        EXPECT_EQ(pLeft->getSkillExp(), pRight->getSkillExp()) << "slayer skill " << i;
        EXPECT_EQ((int)pLeft->getSkillExpLevel(), (int)pRight->getSkillExpLevel()) << "slayer skill " << i;
        EXPECT_EQ(pLeft->getSkillTurn(), pRight->getSkillTurn()) << "slayer skill " << i;
        EXPECT_EQ(pLeft->getCastingTime(), pRight->getCastingTime()) << "slayer skill " << i;
        EXPECT_EQ((int)pLeft->getEnable(), (int)pRight->getEnable()) << "slayer skill " << i;
        delete pLeft;
        delete pRight;
    }
}

void expectVampireRecordEqual(VampireSkillInfo& a, VampireSkillInfo& b) {
    EXPECT_EQ((int)a.isLearnNewSkill(), (int)b.isLearnNewSkill());
    ASSERT_EQ((int)a.getListNum(), (int)b.getListNum());

    const int skills = (int)a.getListNum();
    for (int i = 0; i < skills; i++) {
        SubVampireSkillInfo* pLeft = a.popFrontListElement();
        SubVampireSkillInfo* pRight = b.popFrontListElement();
        EXPECT_EQ((int)pLeft->getSkillType(), (int)pRight->getSkillType()) << "vampire skill " << i;
        EXPECT_EQ(pLeft->getSkillTurn(), pRight->getSkillTurn()) << "vampire skill " << i;
        EXPECT_EQ(pLeft->getCastingTime(), pRight->getCastingTime()) << "vampire skill " << i;
        delete pLeft;
        delete pRight;
    }
}

void expectOustersRecordEqual(OustersSkillInfo& a, OustersSkillInfo& b) {
    EXPECT_EQ((int)a.isLearnNewSkill(), (int)b.isLearnNewSkill());
    ASSERT_EQ((int)a.getListNum(), (int)b.getListNum());

    const int skills = (int)a.getListNum();
    for (int i = 0; i < skills; i++) {
        SubOustersSkillInfo* pLeft = a.popFrontListElement();
        SubOustersSkillInfo* pRight = b.popFrontListElement();
        EXPECT_EQ((int)pLeft->getSkillType(), (int)pRight->getSkillType()) << "ousters skill " << i;
        EXPECT_EQ((int)pLeft->getExpLevel(), (int)pRight->getExpLevel()) << "ousters skill " << i;
        EXPECT_EQ(pLeft->getSkillTurn(), pRight->getSkillTurn()) << "ousters skill " << i;
        EXPECT_EQ(pLeft->getCastingTime(), pRight->getCastingTime()) << "ousters skill " << i;
        delete pLeft;
        delete pRight;
    }
}

// popFrontListElement hands the caller the record, so comparing two
// books empties them; the records are deleted here. The book has no
// count accessor, so the records are drained until the declared size is
// down to the two header bytes.
void expectEqual(GCSkillInfo& a, GCSkillInfo& b) {
    ASSERT_EQ((int)a.getPCType(), (int)b.getPCType());
    ASSERT_EQ((int)a.getPacketSize(), (int)b.getPacketSize());

    while (a.getPacketSize() > (PacketSize_t)(szBYTE * 2)) {
        PCSkillInfo* pLeft = a.popFrontListElement();
        PCSkillInfo* pRight = b.popFrontListElement();
        ASSERT_TRUE(pLeft != NULL);
        ASSERT_TRUE(pRight != NULL);
        EXPECT_EQ(pLeft->getSize(), pRight->getSize());

        switch (a.getPCType()) {
        case PC_SLAYER:
            expectSlayerRecordEqual(*dynamic_cast<SlayerSkillInfo*>(pLeft), *dynamic_cast<SlayerSkillInfo*>(pRight));
            break;
        case PC_VAMPIRE:
            expectVampireRecordEqual(*dynamic_cast<VampireSkillInfo*>(pLeft), *dynamic_cast<VampireSkillInfo*>(pRight));
            break;
        default:
            expectOustersRecordEqual(*dynamic_cast<OustersSkillInfo*>(pLeft), *dynamic_cast<OustersSkillInfo*>(pRight));
            break;
        }

        delete pLeft;
        delete pRight;
    }
    EXPECT_EQ((int)b.getPacketSize(), (int)(szBYTE * 2));
}

SKILL_PACKET_TESTS(GCSkillInfo)
SKILL_PACKET_VARIANT(GCSkillInfo, vampire, fillVampireBook)
SKILL_PACKET_VARIANT(GCSkillInfo, ousters, fillOustersBook)
SKILL_PACKET_VARIANT(GCSkillInfo, empty, fillEmptyBook)
SKILL_PACKET_VARIANT(GCSkillInfo, noskills, fillSkilllessRecord)

//////////////////////////////////////////////////////////////////////
// The answers a learn, a drop and a cast draw.
//////////////////////////////////////////////////////////////////////

void fill(GCTeachSkillInfo& packet) {
    packet.setDomainType(0x8D);
    packet.setTargetLevel(0x8E);
}

void expectEqual(GCTeachSkillInfo& a, GCTeachSkillInfo& b) {
    EXPECT_EQ((int)a.getDomainType(), (int)b.getDomainType());
    EXPECT_EQ((int)a.getTargetLevel(), (int)b.getTargetLevel());
}

SKILL_PACKET_TESTS(GCTeachSkillInfo)

void fill(GCLearnSkillOK& packet) {
    packet.setSkillType(0x8FA0);
    packet.setSkillDomainType(0x91);
}

void expectEqual(GCLearnSkillOK& a, GCLearnSkillOK& b) {
    EXPECT_EQ((int)a.getSkillType(), (int)b.getSkillType());
    EXPECT_EQ((int)a.getSkillDomainType(), (int)b.getSkillDomainType());
}

SKILL_PACKET_TESTS(GCLearnSkillOK)

void fill(GCLearnSkillFailed& packet) {
    packet.setSkillType(0x92A3);
    packet.setDesc(0x94);
}

void expectEqual(GCLearnSkillFailed& a, GCLearnSkillFailed& b) {
    EXPECT_EQ((int)a.getSkillType(), (int)b.getSkillType());
    EXPECT_EQ((int)a.getDesc(), (int)b.getDesc());
}

SKILL_PACKET_TESTS(GCLearnSkillFailed)

void fill(GCLearnSkillReady& packet) {
    packet.setSkillDomainType(0x95);
}

void expectEqual(GCLearnSkillReady& a, GCLearnSkillReady& b) {
    EXPECT_EQ((int)a.getSkillDomainType(), (int)b.getSkillDomainType());
}

SKILL_PACKET_TESTS(GCLearnSkillReady)

void fill(GCDownSkillOK& packet) {
    packet.setSkillType(0x96A7);
}

void expectEqual(GCDownSkillOK& a, GCDownSkillOK& b) {
    EXPECT_EQ((int)a.getSkillType(), (int)b.getSkillType());
}

SKILL_PACKET_TESTS(GCDownSkillOK)

void fill(GCDownSkillFailed& packet) {
    packet.setSkillType(0x98A9);
    packet.setDesc(0x9A);
}

void expectEqual(GCDownSkillFailed& a, GCDownSkillFailed& b) {
    EXPECT_EQ((int)a.getSkillType(), (int)b.getSkillType());
    EXPECT_EQ((int)a.getDesc(), (int)b.getDesc());
}

SKILL_PACKET_TESTS(GCDownSkillFailed)

void fill(GCCastingSkill& packet) {
    packet.setSkillType(0x9BAC);
}

void expectEqual(GCCastingSkill& a, GCCastingSkill& b) {
    EXPECT_EQ((int)a.getSkillType(), (int)b.getSkillType());
}

SKILL_PACKET_TESTS(GCCastingSkill)

//////////////////////////////////////////////////////////////////////
// What a spent bonus or power point produces.
//////////////////////////////////////////////////////////////////////

// The type tags are enumerators, so they carry ModifyType values rather
// than high bytes. The values do not.
void fill(GCUseBonusPointOK& packet) {
    packet.addShortData(MODIFY_CURRENT_STR, 0x81A2);
    packet.addShortData(MODIFY_MAX_STR, 0x83A4);
    packet.addLongData(MODIFY_STR_EXP, 0x85A6B7C8);
    packet.addLongData(MODIFY_BONUS_POINT, 0x89AABBCC);
}

// The stat record the handler actually sends is empty: the bonus point
// is applied and the stats follow in a separate packet.
void fillNoStats(GCUseBonusPointOK&) {}

// ModifyInfo exposes its lists only through the destructive popShortData
// / popLongData, so comparing two records empties both.
void expectEqual(GCUseBonusPointOK& a, GCUseBonusPointOK& b) {
    ASSERT_EQ((int)a.getShortCount(), (int)b.getShortCount());
    ASSERT_EQ((int)a.getLongCount(), (int)b.getLongCount());

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

SKILL_PACKET_TESTS(GCUseBonusPointOK)
SKILL_PACKET_VARIANT(GCUseBonusPointOK, nostats, fillNoStats)

// GCUseBonusPointFail carries no body at all.
void fill(GCUseBonusPointFail&) {}

void expectEqual(GCUseBonusPointFail& a, GCUseBonusPointFail& b) {
    EXPECT_EQ(0, (int)a.getPacketSize());
    EXPECT_EQ(0, (int)b.getPacketSize());
}

SKILL_PACKET_TESTS(GCUseBonusPointFail)

// The result and item bytes select no branch in read(), so they follow
// the >= 128 rule; that they are never range-checked is a finding
// recorded in the header.
void fill(GCUsePowerPointResult& packet) {
    packet.setErrorCode(0x9D);
    packet.setItemCode(0x9E);
    packet.setPowerPoint((int)0x9FA0B1C2);
}

void expectEqual(GCUsePowerPointResult& a, GCUsePowerPointResult& b) {
    EXPECT_EQ((int)a.getErrorCode(), (int)b.getErrorCode());
    EXPECT_EQ((int)a.getItemCode(), (int)b.getItemCode());
    EXPECT_EQ(a.getPowerPoint(), b.getPowerPoint());
}

SKILL_PACKET_TESTS(GCUsePowerPointResult)

void fill(GCRequestPowerPointResult& packet) {
    packet.setErrorCode(0xA1);
    packet.setSumPowerPoint((int)0xA2B3C4D5);
    packet.setRequestPowerPoint((int)0xA6B7C8D9);
}

void expectEqual(GCRequestPowerPointResult& a, GCRequestPowerPointResult& b) {
    EXPECT_EQ((int)a.getErrorCode(), (int)b.getErrorCode());
    EXPECT_EQ(a.getSumPowerPoint(), b.getSumPowerPoint());
    EXPECT_EQ(a.getRequestPowerPoint(), b.getRequestPowerPoint());
}

SKILL_PACKET_TESTS(GCRequestPowerPointResult)

//////////////////////////////////////////////////////////////////////
// Rank bonuses, and the two standing bonus tables.
//////////////////////////////////////////////////////////////////////

void fill(GCRankBonusInfo& packet) {
    packet.addListElement(0x81A2B3C4);
    packet.addListElement(0x85A6B7C8);
    packet.addListElement(0x89AABBCC);
    packet.addListElement(0x8DAEBFC0);
    packet.addListElement(0x91C2D3E4);
}

void fillNoBonuses(GCRankBonusInfo&) {}

void expectEqual(GCRankBonusInfo& a, GCRankBonusInfo& b) {
    ASSERT_EQ((int)a.getListNum(), (int)b.getListNum());
    const int entries = (int)a.getListNum();
    for (int i = 0; i < entries; i++)
        EXPECT_EQ(a.popFrontListElement(), b.popFrontListElement()) << "rank bonus " << i;
}

SKILL_PACKET_TESTS(GCRankBonusInfo)
SKILL_PACKET_VARIANT(GCRankBonusInfo, empty, fillNoBonuses)

void fill(GCSelectRankBonusOK& packet) {
    packet.setRankBonusType(0x95C6D7E8);
}

void expectEqual(GCSelectRankBonusOK& a, GCSelectRankBonusOK& b) {
    EXPECT_EQ(a.getRankBonusType(), b.getRankBonusType());
}

SKILL_PACKET_TESTS(GCSelectRankBonusOK)

void fill(GCSelectRankBonusFailed& packet) {
    packet.setRankBonusType(0x99CADBEC);
    packet.setDesc(0x9D);
}

void expectEqual(GCSelectRankBonusFailed& a, GCSelectRankBonusFailed& b) {
    EXPECT_EQ(a.getRankBonusType(), b.getRankBonusType());
    EXPECT_EQ((int)a.getDesc(), (int)b.getDesc());
}

SKILL_PACKET_TESTS(GCSelectRankBonusFailed)

// Both records carry a type the setter accepts and write() drops, which
// is a finding below; the fixtures set it anyway, the way the two
// managers do.
SweeperBonusInfo* makeSweeperBonus(BYTE type, BYTE race) {
    SweeperBonusInfo* pInfo = new SweeperBonusInfo();
    pInfo->setType(type);
    pInfo->setRace(race);
    return pInfo;
}

void fill(GCSweeperBonusInfo& packet) {
    packet.addSweeperBonusInfo(makeSweeperBonus(0x84, 0x81));
    packet.addSweeperBonusInfo(makeSweeperBonus(0x85, 0x82));
    packet.addSweeperBonusInfo(makeSweeperBonus(0x86, 0x83));
}

void fillNoSweeperBonuses(GCSweeperBonusInfo&) {}

void expectEqual(GCSweeperBonusInfo& a, GCSweeperBonusInfo& b) {
    ASSERT_EQ((int)a.getListNum(), (int)b.getListNum());
    const int entries = (int)a.getListNum();
    for (int i = 0; i < entries; i++) {
        SweeperBonusInfo* pLeft = a.popFrontSweeperBonusInfoList();
        SweeperBonusInfo* pRight = b.popFrontSweeperBonusInfoList();
        ASSERT_TRUE(pLeft != NULL);
        ASSERT_TRUE(pRight != NULL);
        EXPECT_EQ((int)pLeft->getRace(), (int)pRight->getRace()) << "sweeper bonus " << i;
        delete pLeft;
        delete pRight;
    }
}

SKILL_PACKET_TESTS(GCSweeperBonusInfo)
SKILL_PACKET_VARIANT(GCSweeperBonusInfo, empty, fillNoSweeperBonuses)

BloodBibleBonusInfo* makeHolyLandBonus(BYTE type, BYTE race) {
    BloodBibleBonusInfo* pInfo = new BloodBibleBonusInfo();
    pInfo->setType(type);
    pInfo->setRace(race);
    return pInfo;
}

void fill(GCHolyLandBonusInfo& packet) {
    packet.addBloodBibleBonusInfo(makeHolyLandBonus(0x8A, 0x87));
    packet.addBloodBibleBonusInfo(makeHolyLandBonus(0x8B, 0x88));
    packet.addBloodBibleBonusInfo(makeHolyLandBonus(0x8C, 0x89));
}

void fillNoHolyLandBonuses(GCHolyLandBonusInfo&) {}

void expectEqual(GCHolyLandBonusInfo& a, GCHolyLandBonusInfo& b) {
    ASSERT_EQ((int)a.getListNum(), (int)b.getListNum());
    const int entries = (int)a.getListNum();
    for (int i = 0; i < entries; i++) {
        BloodBibleBonusInfo* pLeft = a.popFrontBloodBibleBonusInfoList();
        BloodBibleBonusInfo* pRight = b.popFrontBloodBibleBonusInfoList();
        ASSERT_TRUE(pLeft != NULL);
        ASSERT_TRUE(pRight != NULL);
        EXPECT_EQ((int)pLeft->getRace(), (int)pRight->getRace()) << "holy land bonus " << i;
        delete pLeft;
        delete pRight;
    }
}

SKILL_PACKET_TESTS(GCHolyLandBonusInfo)
SKILL_PACKET_VARIANT(GCHolyLandBonusInfo, empty, fillNoHolyLandBonuses)

//////////////////////////////////////////////////////////////////////
// The blood bible.
//////////////////////////////////////////////////////////////////////

void fill(GCBloodBibleList& packet) {
    packet.getList().push_back(0x81A2);
    packet.getList().push_back(0x83A4);
    packet.getList().push_back(0x85A6);
    packet.getList().push_back(0x87A8);
}

void fillNoBibles(GCBloodBibleList&) {}

void expectEqual(GCBloodBibleList& a, GCBloodBibleList& b) {
    EXPECT_EQ(a.getList(), b.getList());
}

SKILL_PACKET_TESTS(GCBloodBibleList)
SKILL_PACKET_VARIANT(GCBloodBibleList, empty, fillNoBibles)

// The owner name is text, so it cannot follow the >= 128 rule.
void fill(GCBloodBibleStatus& packet) {
    packet.setItemType(0x89AA);
    packet.setZoneID(0x8BAC);
    packet.setStorage(0x8D);
    packet.setOwnerName("Bloodstained Keeper");
    packet.setRace(0x8E);
    packet.setShrineRace(0x8F);
    packet.setX(0x90A1);
    packet.setY(0x92A3);
}

// A blood bible lying in a zone or in a corpse belongs to nobody, and
// the name goes out as a bare zero length byte.
void fillUnowned(GCBloodBibleStatus& packet) {
    fill(packet);
    packet.setOwnerName("");
}

void expectEqual(GCBloodBibleStatus& a, GCBloodBibleStatus& b) {
    EXPECT_EQ((int)a.getItemType(), (int)b.getItemType());
    EXPECT_EQ((int)a.getZoneID(), (int)b.getZoneID());
    EXPECT_EQ((int)a.getStorage(), (int)b.getStorage());
    EXPECT_EQ(a.getOwnerName(), b.getOwnerName());
    EXPECT_EQ((int)a.getRace(), (int)b.getRace());
    EXPECT_EQ((int)a.getShrineRace(), (int)b.getShrineRace());
    EXPECT_EQ((int)a.getX(), (int)b.getX());
    EXPECT_EQ((int)a.getY(), (int)b.getY());
}

SKILL_PACKET_TESTS(GCBloodBibleStatus)
SKILL_PACKET_VARIANT(GCBloodBibleStatus, noowner, fillUnowned)

// GCBloodBibleSignInfo holds its record by pointer and never owns it, so
// its three pins are written out around a fixture that does.
struct SignInfoFixture {
    SignInfoFixture(uint openNum, int signs) {
        record.setOpenNum(openNum);
        for (int i = 0; i < signs; i++)
            record.getList().push_back((ItemType_t)(0x98A9 + i * 0x0101));
        packet.setSignInfo(&record);
    }

    BloodBibleSignInfo record;
    GCBloodBibleSignInfo packet;
};

void expectSignInfoEqual(GCBloodBibleSignInfo& a, GCBloodBibleSignInfo& b) {
    ASSERT_TRUE(a.getSignInfo() != NULL);
    ASSERT_TRUE(b.getSignInfo() != NULL);
    EXPECT_EQ(a.getSignInfo()->getOpenNum(), b.getSignInfo()->getOpenNum());
    EXPECT_EQ(a.getSignInfo()->getList(), b.getSignInfo()->getList());
}

TEST(GCBloodBibleSignInfoTest, roundTripsThroughLoopback) {
    SignInfoFixture src(0x94A5B6C7, 3);
    GCBloodBibleSignInfo dst;
    roundTrip(src.packet, dst, kPlainCode);
    expectSignInfoEqual(src.packet, dst);
    delete dst.getSignInfo();
}

TEST(GCBloodBibleSignInfoTest, bodyBytesMatchGolden) {
    SignInfoFixture src(0x94A5B6C7, 3);
    const std::vector<unsigned char> body = writeBody(src.packet, kPlainCode);
    expectGolden("GCBloodBibleSignInfo", kPlainCode, body);
    for (size_t i = 1; i < kEncryptCodeCount; i++)
        EXPECT_EQ(body, writeBody(src.packet, kEncryptCodes[i]))
            << "GCBloodBibleSignInfo now varies with the encrypt code - add per-code goldens";
}

TEST(GCBloodBibleSignInfoTest, sizeMatchesTheBytesWrittenAndFitsTheFactoryMax) {
    SignInfoFixture src(0x94A5B6C7, 3);
    GCBloodBibleSignInfoFactory factory;
    EXPECT_EQ((size_t)src.packet.getPacketSize(), writeBody(src.packet, kPlainCode).size())
        << "GCBloodBibleSignInfo: getPacketSize() disagrees with the bytes write() emits";
    EXPECT_LE(src.packet.getPacketSize(), factory.getPacketMaxSize());
    EXPECT_EQ(factory.getPacketID(), src.packet.getPacketID());
    EXPECT_EQ(factory.getPacketName(), src.packet.getPacketName());
}

TEST(GCBloodBibleSignInfoTest, emptyBodyBytesMatchGolden) {
    SignInfoFixture src(0x9CADBECF, 0);
    const std::vector<unsigned char> body = writeBody(src.packet, kPlainCode);
    expectGolden("GCBloodBibleSignInfo.empty", kPlainCode, body);
    EXPECT_EQ((size_t)src.packet.getPacketSize(), body.size());

    GCBloodBibleSignInfo dst;
    roundTrip(src.packet, dst, kPlainCode);
    expectSignInfoEqual(src.packet, dst);
    delete dst.getSignInfo();
}

// signCount() caps what write() emits at the six slots the record has,
// which is also what the factory max budgets.
TEST(GCBloodBibleSignInfoTest, fullBodyBytesMatchGolden) {
    SignInfoFixture src(0xA0B1C2D3, BLOOD_BIBLE_SIGN_SLOT_NUM);
    const std::vector<unsigned char> body = writeBody(src.packet, kPlainCode);
    expectGolden("GCBloodBibleSignInfo.full", kPlainCode, body);
    EXPECT_EQ((size_t)src.packet.getPacketSize(), body.size());

    GCBloodBibleSignInfoFactory factory;
    EXPECT_EQ((size_t)factory.getPacketMaxSize(), body.size());

    GCBloodBibleSignInfo dst;
    roundTrip(src.packet, dst, kPlainCode);
    expectSignInfoEqual(src.packet, dst);
    delete dst.getSignInfo();
}

//////////////////////////////////////////////////////////////////////
// The write/read disagreements the set found.
//////////////////////////////////////////////////////////////////////

// FINDING, stated as a test that fails once it is fixed.
// GCSkillInfo derives the record count into a BYTE it caps nowhere, so
// the 256th record wraps the count to zero while write() still emits
// every record.
TEST(GCSkillInfoTest, theRecordCountWrapsAtTwoHundredAndFiftySix) {
    GCSkillInfo packet;
    packet.setPCType(PC_SLAYER);
    for (int i = 0; i < 256; i++)
        packet.addListElement(makeSlayerRecord(false, (SkillDomainType_t)i, 0));

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ(0, (int)body[1]) << "GCSkillInfo: the record list is now capped - drop this test";
    EXPECT_EQ((size_t)(szBYTE * 2 + 256 * (szBYTE + szSkillDomainType + szBYTE)), body.size())
        << "GCSkillInfo: write() no longer emits the records the count byte cannot describe";
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());
}

// FINDING, stated as a test that fails once it is fixed.
// GCSkillInfoFactory's maximum is one SlayerSkillInfo and nothing else:
// it budgets neither the pc type byte nor the record count byte, and no
// second record, although a slayer's book carries one per domain.
TEST(GCSkillInfoTest, theFactoryMaxBudgetsOneRecordAndNeitherHeaderByte) {
    GCSkillInfoFactory factory;
    EXPECT_EQ((PacketSize_t)SlayerSkillInfo::getMaxSize(), factory.getPacketMaxSize())
        << "GCSkillInfo: the factory max now budgets more than a bare record - drop this test";

    GCSkillInfo packet;
    packet.setPCType(PC_SLAYER);
    packet.addListElement(makeSlayerRecord(true, 0x81, 255));
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCSkillInfo: one full record now fits the read buffer the receiver sizes for it";
}

// FINDING, stated as a test that fails once it is fixed.
// The three race records keep their skill count in a BYTE the caller
// sets by hand while addListElement leaves it alone, so write() emits
// that count and then the whole list: the skills past the count are sent
// and never consumed, and getPacketSize() counts them.
TEST(GCSkillInfoTest, theSkillCountIsMaintainedByHand) {
    GCSkillInfo packet;
    packet.setPCType(PC_SLAYER);
    SlayerSkillInfo* pRecord = makeSlayerRecord(true, 0x81, 2);
    pRecord->setListNum(1);
    packet.addListElement(pRecord);

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    const size_t countOffset = szBYTE * 2 + szBYTE + szSkillDomainType;
    EXPECT_EQ(1, (int)body[countOffset]) << "GCSkillInfo: the skill count now follows the list - drop this test";
    EXPECT_EQ((size_t)(countOffset + szBYTE + 2 * SubSlayerSkillInfo::getMaxSize()), body.size())
        << "GCSkillInfo: write() no longer emits the skills the count byte does not describe";
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCSkillInfo dst;
    roundTrip(packet, dst, kPlainCode);
    PCSkillInfo* pRead = dst.popFrontListElement();
    ASSERT_TRUE(pRead != NULL);
    EXPECT_EQ((PacketSize_t)(szBYTE + szSkillDomainType + szBYTE + SubSlayerSkillInfo::getMaxSize()), pRead->getSize())
        << "GCSkillInfo: the second skill now reaches the reader";
    delete pRead;
}

// FINDING, stated as a test that fails once it is fixed.
// GCSkillInfo::read appends to the record list the packet already holds
// instead of replacing it, so a reused packet grows by one book per read.
TEST(GCSkillInfoTest, aSecondReadAppendsToTheBookItAlreadyHolds) {
    GCSkillInfo src;
    fill(src);
    const PacketSize_t oneBook = src.getPacketSize();

    GCSkillInfo dst;
    roundTrip(src, dst, kPlainCode);
    ASSERT_EQ((int)oneBook, (int)dst.getPacketSize());

    GCSkillInfo second;
    fill(second);
    roundTrip(second, dst, kPlainCode);
    EXPECT_EQ((int)(oneBook * 2 - szBYTE * 2), (int)dst.getPacketSize())
        << "GCSkillInfo::read() now replaces the record list - drop this test and pin the replacement";
}

// FINDING, stated as a test that fails once it is fixed.
// write() emits any pc type byte while read() refuses every one that is
// not PC_SLAYER, PC_VAMPIRE or PC_OUSTERS, so a book announced with any
// other type is written and cannot be read back.
TEST(GCSkillInfoTest, aPCTypeNoBranchBuildsIsWrittenAndCannotBeReadBack) {
    GCSkillInfo packet;
    packet.setPCType(0x81);

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)(szBYTE * 2), body.size());
    EXPECT_EQ(0x81, (int)body[0]);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCSkillInfo dst;
    EXPECT_THROW(roundTrip(packet, dst, kPlainCode), InvalidProtocolException)
        << "GCSkillInfo: an unknown pc type now reads back - drop this test and pin the refusal";
}

// FINDING, stated as a test that fails once it is fixed.
// The three counted-list packets derive their count byte from the list
// and cap nothing, so the 256th entry wraps the count to zero and the
// body outgrows the factory max.
TEST(GCRankBonusInfoTest, theEntryCountWrapsAtTwoHundredAndFiftySix) {
    GCRankBonusInfo packet;
    for (int i = 0; i < 256; i++)
        packet.addListElement((DWORD)(0x81A2B3C4 + i));

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ(0, (int)body[0]) << "GCRankBonusInfo: the list is now capped - drop this test";
    EXPECT_EQ((size_t)(szBYTE + 256 * szDWORD), body.size());

    GCRankBonusInfoFactory factory;
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCRankBonusInfo: the list now fits the factory max";
}

TEST(GCSweeperBonusInfoTest, theEntryCountWrapsAtTwoHundredAndFiftySix) {
    GCSweeperBonusInfo packet;
    for (int i = 0; i < 256; i++)
        packet.addSweeperBonusInfo(makeSweeperBonus((BYTE)i, (BYTE)i));

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ(0, (int)body[0]) << "GCSweeperBonusInfo: the list is now capped - drop this test";
    EXPECT_EQ((size_t)(szBYTE + 256 * SweeperBonusInfo::getMaxSize()), body.size());

    GCSweeperBonusInfoFactory factory;
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCSweeperBonusInfo: the list now fits the factory max";
}

TEST(GCHolyLandBonusInfoTest, theEntryCountWrapsAtTwoHundredAndFiftySix) {
    GCHolyLandBonusInfo packet;
    for (int i = 0; i < 256; i++)
        packet.addBloodBibleBonusInfo(makeHolyLandBonus((BYTE)i, (BYTE)i));

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ(0, (int)body[0]) << "GCHolyLandBonusInfo: the list is now capped - drop this test";
    EXPECT_EQ((size_t)(szBYTE + 256 * BloodBibleBonusInfo::getMaxSize()), body.size());

    GCHolyLandBonusInfoFactory factory;
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCHolyLandBonusInfo: the list now fits the factory max";
}

// FINDING, stated as a test that fails once it is fixed.
// The four list packets read into whatever the packet already holds
// instead of replacing it, so a reused packet grows by one listing per
// read.
TEST(GCRankBonusInfoTest, aSecondReadAppendsToTheListItAlreadyHolds) {
    GCRankBonusInfo src;
    fill(src);

    GCRankBonusInfo dst;
    roundTrip(src, dst, kPlainCode);
    ASSERT_EQ(5, (int)dst.getListNum());

    GCRankBonusInfo second;
    fill(second);
    roundTrip(second, dst, kPlainCode);
    EXPECT_EQ(10, (int)dst.getListNum())
        << "GCRankBonusInfo::read() now replaces the list - drop this test and pin the replacement";
}

TEST(GCSweeperBonusInfoTest, aSecondReadAppendsToTheListItAlreadyHolds) {
    GCSweeperBonusInfo src;
    fill(src);

    GCSweeperBonusInfo dst;
    roundTrip(src, dst, kPlainCode);
    ASSERT_EQ(3, (int)dst.getListNum());

    GCSweeperBonusInfo second;
    fill(second);
    roundTrip(second, dst, kPlainCode);
    EXPECT_EQ(6, (int)dst.getListNum())
        << "GCSweeperBonusInfo::read() now replaces the list - drop this test and pin the replacement";
}

TEST(GCHolyLandBonusInfoTest, aSecondReadAppendsToTheListItAlreadyHolds) {
    GCHolyLandBonusInfo src;
    fill(src);

    GCHolyLandBonusInfo dst;
    roundTrip(src, dst, kPlainCode);
    ASSERT_EQ(3, (int)dst.getListNum());

    GCHolyLandBonusInfo second;
    fill(second);
    roundTrip(second, dst, kPlainCode);
    EXPECT_EQ(6, (int)dst.getListNum())
        << "GCHolyLandBonusInfo::read() now replaces the list - drop this test and pin the replacement";
}

TEST(GCBloodBibleListTest, aSecondReadAppendsToTheListItAlreadyHolds) {
    GCBloodBibleList src;
    fill(src);

    GCBloodBibleList dst;
    roundTrip(src, dst, kPlainCode);
    ASSERT_EQ(4u, dst.getList().size());

    GCBloodBibleList second;
    fill(second);
    roundTrip(second, dst, kPlainCode);
    EXPECT_EQ(8u, dst.getList().size())
        << "GCBloodBibleList::read() now replaces the list - drop this test and pin the replacement";
}

// FINDING, stated as a test that fails once it is fixed.
// SweeperBonusInfo and BloodBibleBonusInfo write and read only their
// race byte: the type byte and the option list are commented out on both
// sides, so the type both managers set on every record never reaches the
// wire.
TEST(GCSweeperBonusInfoTest, theRecordTypeNeverReachesTheWire) {
    GCSweeperBonusInfo quiet;
    quiet.addSweeperBonusInfo(makeSweeperBonus(0x00, 0x81));

    GCSweeperBonusInfo loud;
    loud.addSweeperBonusInfo(makeSweeperBonus(0xFF, 0x81));

    EXPECT_EQ(writeBody(quiet, kPlainCode), writeBody(loud, kPlainCode))
        << "SweeperBonusInfo: the type now reaches the wire - drop this test and pin it";
    EXPECT_EQ((uint)szBYTE, SweeperBonusInfo::getMaxSize())
        << "SweeperBonusInfo: the record now budgets more than the race byte";
}

TEST(GCHolyLandBonusInfoTest, theRecordTypeNeverReachesTheWire) {
    GCHolyLandBonusInfo quiet;
    quiet.addBloodBibleBonusInfo(makeHolyLandBonus(0x00, 0x87));

    GCHolyLandBonusInfo loud;
    loud.addBloodBibleBonusInfo(makeHolyLandBonus(0xFF, 0x87));

    EXPECT_EQ(writeBody(quiet, kPlainCode), writeBody(loud, kPlainCode))
        << "BloodBibleBonusInfo: the type now reaches the wire - drop this test and pin it";
    EXPECT_EQ((uint)szBYTE, BloodBibleBonusInfo::getMaxSize())
        << "BloodBibleBonusInfo: the record now budgets more than the race byte";
}

//////////////////////////////////////////////////////////////////////
// What the default constructor leaves.
//////////////////////////////////////////////////////////////////////

// Each packet is built twice over storage poisoned with a different
// byte, so a member the constructor leaves alone reaches the wire as that
// byte and the two bodies differ.
template <typename PacketType> std::vector<unsigned char> bodyOverPoison(unsigned char poison) {
    alignas(PacketType) unsigned char storage[sizeof(PacketType)];
    memset(storage, poison, sizeof(storage));
    PacketType* pPacket = new (storage) PacketType();
    std::vector<unsigned char> body = writeBody(*pPacket, kPlainCode);
    pPacket->~PacketType();
    return body;
}

template <typename PacketType> void expectEveryMemberIsInitialised(const char* what) {
    EXPECT_EQ(bodyOverPoison<PacketType>(0x00), bodyOverPoison<PacketType>(0xFF))
        << what << ": its default constructor leaves a member write() emits uninitialised";
}

// FINDING, stated as a test that fails once it is fixed. Each of these
// puts an indeterminate byte on the wire when a sender skips a setter.
template <typename PacketType> void expectAMemberIsLeftUninitialised(const char* what) {
    EXPECT_NE(bodyOverPoison<PacketType>(0x00), bodyOverPoison<PacketType>(0xFF))
        << what
        << ": its default constructor now initialises every member write() emits - move it to the "
           "initialised list";
}

TEST(SkillConstructorTest, thePacketsThatInitialiseEveryMemberTheyWrite) {
    expectEveryMemberIsInitialised<GCUseBonusPointOK>("GCUseBonusPointOK");
    expectEveryMemberIsInitialised<GCRequestPowerPointResult>("GCRequestPowerPointResult");
    expectEveryMemberIsInitialised<GCRankBonusInfo>("GCRankBonusInfo");
    expectEveryMemberIsInitialised<GCSweeperBonusInfo>("GCSweeperBonusInfo");
    expectEveryMemberIsInitialised<GCHolyLandBonusInfo>("GCHolyLandBonusInfo");
    expectEveryMemberIsInitialised<GCBloodBibleList>("GCBloodBibleList");
}

TEST(SkillConstructorTest, thePacketsThatDoNot) {
    expectAMemberIsLeftUninitialised<CGLearnSkill>("CGLearnSkill");
    expectAMemberIsLeftUninitialised<CGDownSkill>("CGDownSkill");
    expectAMemberIsLeftUninitialised<CGCastingSkill>("CGCastingSkill");
    expectAMemberIsLeftUninitialised<CGUseBonusPoint>("CGUseBonusPoint");
    expectAMemberIsLeftUninitialised<CGSelectRankBonus>("CGSelectRankBonus");
    expectAMemberIsLeftUninitialised<CGSelectBloodBible>("CGSelectBloodBible");
    expectAMemberIsLeftUninitialised<GCSkillInfo>("GCSkillInfo");
    expectAMemberIsLeftUninitialised<GCTeachSkillInfo>("GCTeachSkillInfo");
    expectAMemberIsLeftUninitialised<GCLearnSkillOK>("GCLearnSkillOK");
    expectAMemberIsLeftUninitialised<GCLearnSkillFailed>("GCLearnSkillFailed");
    expectAMemberIsLeftUninitialised<GCLearnSkillReady>("GCLearnSkillReady");
    expectAMemberIsLeftUninitialised<GCDownSkillOK>("GCDownSkillOK");
    expectAMemberIsLeftUninitialised<GCDownSkillFailed>("GCDownSkillFailed");
    expectAMemberIsLeftUninitialised<GCCastingSkill>("GCCastingSkill");
    expectAMemberIsLeftUninitialised<GCUsePowerPointResult>("GCUsePowerPointResult");
    expectAMemberIsLeftUninitialised<GCSelectRankBonusOK>("GCSelectRankBonusOK");
    expectAMemberIsLeftUninitialised<GCSelectRankBonusFailed>("GCSelectRankBonusFailed");
    expectAMemberIsLeftUninitialised<GCBloodBibleStatus>("GCBloodBibleStatus");
}

// CGUsePowerPoint and GCUseBonusPointFail are in neither list: their
// body is empty, so there is nothing a constructor could leave.
TEST(SkillConstructorTest, theBodylessMessagesWriteNothingAtAll) {
    EXPECT_TRUE(bodyOverPoison<CGUsePowerPoint>(0xFF).empty());
    EXPECT_TRUE(bodyOverPoison<GCUseBonusPointFail>(0xFF).empty());
}

// CGRequestPowerPoint and CGSkillToNamed are in neither list either:
// their one text field is required non-empty, so the body a default
// constructor leaves is refused rather than written.
template <typename PacketType> void expectTheDefaultBodyIsRefused(const char* what) {
    alignas(PacketType) unsigned char storage[sizeof(PacketType)];
    memset(storage, 0xFF, sizeof(storage));
    PacketType* pPacket = new (storage) PacketType();
    EXPECT_THROW(writeBody(*pPacket, kPlainCode), InvalidProtocolException)
        << what << ": an empty text field now reaches the wire";
    pPacket->~PacketType();
}

TEST(SkillConstructorTest, theRequestsWhoseTextFieldIsRequired) {
    expectTheDefaultBodyIsRefused<CGRequestPowerPoint>("CGRequestPowerPoint");
    expectTheDefaultBodyIsRefused<CGSkillToNamed>("CGSkillToNamed");
}

// FINDING, stated as a test that fails once it is fixed.
// GCBloodBibleSignInfo is in neither list because writing one over
// poisoned storage would follow the record pointer its constructor
// leaves alone. The pointer itself is pinned instead: both
// getPacketSize() and write() dereference it, so a sender that skips
// setSignInfo follows an indeterminate value.
TEST(SkillConstructorTest, theSignInfoPointerIsLeftUninitialised) {
    alignas(GCBloodBibleSignInfo) unsigned char zeroed[sizeof(GCBloodBibleSignInfo)];
    memset(zeroed, 0x00, sizeof(zeroed));
    GCBloodBibleSignInfo* pZeroed = new (zeroed) GCBloodBibleSignInfo();
    BloodBibleSignInfo* pFromZero = pZeroed->getSignInfo();
    pZeroed->~GCBloodBibleSignInfo();

    alignas(GCBloodBibleSignInfo) unsigned char poisoned[sizeof(GCBloodBibleSignInfo)];
    memset(poisoned, 0xFF, sizeof(poisoned));
    GCBloodBibleSignInfo* pPoisoned = new (poisoned) GCBloodBibleSignInfo();
    BloodBibleSignInfo* pFromPoison = pPoisoned->getSignInfo();
    pPoisoned->~GCBloodBibleSignInfo();

    EXPECT_NE(pFromZero, pFromPoison)
        << "GCBloodBibleSignInfo: its constructor now initialises the record pointer - move it to the "
           "initialised list";
}

} // namespace
