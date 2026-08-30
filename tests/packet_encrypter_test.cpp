//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_encrypter_test.cpp
// Description : Golden-byte pins and loopback round-trips for every
//               packet whose read()/write() go through the per-session
//               encrypter (SocketEncrypt{Input,Output}Stream::
//               {read,write}Encrypt), i.e. every packet whose wire bytes
//               depend on the session's encrypt code.
//
//               Why every packet, and why every code: the shuffle macros
//               in EncryptUtility.h reorder the fields by `code % N`.
//               SHUFFLE_STATEMENT_2/_3 orders are rotations, but _4 case
//               3 (D A C B) and _5 cases 3/4 (D E B A C, E C D A B) are
//               NOT - exactly the kind of detail a hand-maintained client
//               copy gets wrong, and invisible to any test that stops at
//               code 3. kEncryptCodes (TestStreams.h) runs 0..5, which
//               reaches every case of every arity through the encrypted
//               branch.
//
//               Test values are distinct per field and >= 128 in every
//               byte where the width allows, so a signedness flip, a
//               same-width type swap or a transposed pair of fields all
//               move bytes in the golden. (An all-<128 fixture cannot
//               tell BYTE from char.)
//
//               tests/ratchet/ratchets.sh fails if a src/Core packet
//               starts using the encrypter without a golden here.
//
//////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>

#include "CGAddMouseToZone.h"
#include "CGAddZoneToInventory.h"
#include "CGAddZoneToMouse.h"
#include "CGAttack.h"
#include "CGDissectionCorpse.h"
#include "CGDropMoney.h"
#include "CGNPCAskAnswer.h"
#include "CGPickupMoney.h"
#include "CGSkillToInventory.h"
#include "CGSkillToObject.h"
#include "CGSkillToSelf.h"
#include "CGSkillToTile.h"
#include "CGUseItemFromGear.h"
#include "CGUseItemFromInventory.h"
#include "CGUsePotionFromInventory.h"
#include "EncryptUtility.h"
#include "GCAddInstalledMineToZone.h"
#include "GCAddItemToZone.h"
#include "GCAddNewItemToZone.h"
#include "GCDropItemToZone.h"
#include "GCMoveError.h"
#include "SubItemInfo.h"
#include "TestStreams.h"

using wiretest::expectGolden;
using wiretest::kEncryptCodeCount;
using wiretest::kEncryptCodes;
using wiretest::roundTrip;
using wiretest::writeBody;

namespace {

// Each packet gets a fill() (the canonical fixture), an expectEqual(), a
// round-trip test over every encrypt code, and a golden per code. The
// same fixture feeds both, so the golden and the round-trip pin the same
// instance.
#define ENCRYPTER_PACKET_TESTS(Name)                                                      \
    TEST(Name##Test, roundTripsThroughLoopbackForEveryEncryptCode) {                      \
        for (size_t i = 0; i < kEncryptCodeCount; i++) {                                  \
            SCOPED_TRACE(testing::Message() << "encrypt code " << (int)kEncryptCodes[i]); \
            Name src;                                                                     \
            fill(src);                                                                    \
            Name dst;                                                                     \
            roundTrip(src, dst, kEncryptCodes[i]);                                        \
            expectEqual(src, dst);                                                        \
        }                                                                                 \
    }                                                                                     \
    ENCRYPTER_PACKET_GOLDENS(Name)

#define ENCRYPTER_PACKET_GOLDENS(Name)                                                  \
    TEST(Name##Test, bodyBytesMatchGoldenForEveryEncryptCode) {                         \
        Name packet;                                                                    \
        fill(packet);                                                                   \
        for (size_t i = 0; i < kEncryptCodeCount; i++)                                  \
            expectGolden(#Name, kEncryptCodes[i], writeBody(packet, kEncryptCodes[i])); \
    }

// For packets whose getPacketSize() disagrees with write() (see the two
// below), the stock roundTrip() cannot be used: it pumps getPacketSize()
// bytes and stalls. Pump exactly what write() produced instead, so the
// read()/write() field agreement is still proven under every code, and
// state the over-report as a fact so its fix is a visible change.
#define ENCRYPTER_PACKET_TESTS_WITH_SIZE_DRIFT(Name, OVERREPORTED_BY)                      \
    TEST(Name##Test, roundTripsThroughLoopbackForEveryEncryptCode) {                       \
        for (size_t i = 0; i < kEncryptCodeCount; i++) {                                   \
            SCOPED_TRACE(testing::Message() << "encrypt code " << (int)kEncryptCodes[i]);  \
            Name src;                                                                      \
            fill(src);                                                                     \
            Name dst;                                                                      \
            wiretest::Loopback loopback;                                                   \
            loopback.setCodes(kEncryptCodes[i]);                                           \
            src.write(loopback.out());                                                     \
            loopback.pump(writeBody(src, kEncryptCodes[i]).size());                        \
            dst.read(loopback.in());                                                       \
            expectEqual(src, dst);                                                         \
        }                                                                                  \
    }                                                                                      \
    TEST(Name##Test, getPacketSizeStillOverReportsTheBody) {                               \
        Name packet;                                                                       \
        fill(packet);                                                                      \
        EXPECT_EQ(writeBody(packet, 0).size() + (OVERREPORTED_BY), packet.getPacketSize()) \
            << "getPacketSize() now matches write() — switch " #Name " back to "           \
               "ENCRYPTER_PACKET_TESTS and close the entry in docs/RESTRUCTURING.md 1.2";  \
    }                                                                                      \
    ENCRYPTER_PACKET_GOLDENS(Name)

//////////////////////////////////////////////////////////////////////
// SHUFFLE_STATEMENT_2
//////////////////////////////////////////////////////////////////////

void fill(CGSkillToSelf& p) {
    p.setSkillType(0x8A1B);
    p.setCEffectID(0x9C2D);
}
void expectEqual(const CGSkillToSelf& a, const CGSkillToSelf& b) {
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getCEffectID(), b.getCEffectID());
}
ENCRYPTER_PACKET_TESTS(CGSkillToSelf)

void fill(CGUseItemFromGear& p) {
    p.setObjectID(0xA1B2C3D4);
    p.setPart(0x85);
}
void expectEqual(const CGUseItemFromGear& a, const CGUseItemFromGear& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getPart(), b.getPart());
}
ENCRYPTER_PACKET_TESTS(CGUseItemFromGear)

void fill(GCMoveError& p) {
    p.setXY(0x91, 0xA3);
}
void expectEqual(const GCMoveError& a, const GCMoveError& b) {
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
}
ENCRYPTER_PACKET_TESTS(GCMoveError)

//////////////////////////////////////////////////////////////////////
// SHUFFLE_STATEMENT_3 (GCMoveOK and CGMove live in packet_roundtrip_test)
//////////////////////////////////////////////////////////////////////

void fill(CGAddZoneToMouse& p) {
    p.setObjectID(0xB3C4D5E6);
    p.setZoneX(0x97);
    p.setZoneY(0xA9);
}
void expectEqual(CGAddZoneToMouse& a, CGAddZoneToMouse& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getZoneX(), b.getZoneX());
    EXPECT_EQ(a.getZoneY(), b.getZoneY());
}
ENCRYPTER_PACKET_TESTS(CGAddZoneToMouse)

void fill(CGNPCAskAnswer& p) {
    p.setObjectID(0xC5D6E7F8);
    p.setScriptID(0x89ABCDEF);
    p.setAnswerID(0xB1);
}
void expectEqual(CGNPCAskAnswer& a, CGNPCAskAnswer& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getScriptID(), b.getScriptID());
    EXPECT_EQ(a.getAnswerID(), b.getAnswerID());
}
ENCRYPTER_PACKET_TESTS(CGNPCAskAnswer)

void fill(CGPickupMoney& p) {
    p.setObjectID(0xD7E8F90A);
    p.setZoneX(0x9B);
    p.setZoneY(0xAD);
}
void expectEqual(CGPickupMoney& a, CGPickupMoney& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getZoneX(), b.getZoneX());
    EXPECT_EQ(a.getZoneY(), b.getZoneY());
}
ENCRYPTER_PACKET_TESTS(CGPickupMoney)

void fill(CGSkillToObject& p) {
    p.setSkillType(0x8E3F);
    p.setCEffectID(0x9D4A);
    p.setTargetObjectID(0xE9FA0B1C);
}
void expectEqual(CGSkillToObject& a, CGSkillToObject& b) {
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getCEffectID(), b.getCEffectID());
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
}
ENCRYPTER_PACKET_TESTS(CGSkillToObject)

// m_InventoryItemObjectID is declared but commented out of read()/write()
// — yet getPacketSize() still counts it, so the size over-reports the body
// by szObjectID. It is deliberately left unset here so the fixture
// documents that it is not on the wire; if someone puts it back, the
// golden moves. Logged in docs/RESTRUCTURING.md 1.2.
void fill(CGUseItemFromInventory& p) {
    p.setObjectID(0xFB0C1D2E);
    p.setX(0x93);
    p.setY(0xA5);
}
void expectEqual(CGUseItemFromInventory& a, CGUseItemFromInventory& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
}
ENCRYPTER_PACKET_TESTS_WITH_SIZE_DRIFT(CGUseItemFromInventory, szObjectID)

void fill(CGUsePotionFromInventory& p) {
    p.setObjectID(0x8C9DAEBF);
    p.setX(0xB5);
    p.setY(0xC7);
}
void expectEqual(CGUsePotionFromInventory& a, CGUsePotionFromInventory& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
}
ENCRYPTER_PACKET_TESTS(CGUsePotionFromInventory)

//////////////////////////////////////////////////////////////////////
// SHUFFLE_STATEMENT_4 - case 3 is D A C B, not a rotation
//////////////////////////////////////////////////////////////////////

void fill(CGAttack& p) {
    p.setObjectID(0x9EAFB0C1);
    p.setX(0x83);
    p.setY(0x95);
    p.setDir(0xA7);
}
void expectEqual(const CGAttack& a, const CGAttack& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
}
ENCRYPTER_PACKET_TESTS(CGAttack)

void fill(CGDissectionCorpse& p) {
    p.setObjectID(0xACBDCEDF);
    p.setX(0x89);
    p.setY(0x9B);
    p.setPet(0xAD);
}
void expectEqual(const CGDissectionCorpse& a, const CGDissectionCorpse& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.isPet(), b.isPet());
}
ENCRYPTER_PACKET_TESTS(CGDissectionCorpse)

void fill(CGSkillToTile& p) {
    p.setSkillType(0x8F5B);
    p.setCEffectID(0x9E6C);
    p.setX(0xB9);
    p.setY(0xCB);
}
void expectEqual(const CGSkillToTile& a, const CGSkillToTile& b) {
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getCEffectID(), b.getCEffectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
}
ENCRYPTER_PACKET_TESTS(CGSkillToTile)

//////////////////////////////////////////////////////////////////////
// SHUFFLE_STATEMENT_5 - cases 3 (D E B A C) and 4 (E C D A B) are not
// rotations
//////////////////////////////////////////////////////////////////////

void fill(CGAddZoneToInventory& p) {
    p.setObjectID(0xBACBDCED);
    p.setZoneX(0x8B);
    p.setZoneY(0x9D);
    p.setInvenX(0xAF);
    p.setInvenY(0xC1);
}
void expectEqual(CGAddZoneToInventory& a, CGAddZoneToInventory& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getZoneX(), b.getZoneX());
    EXPECT_EQ(a.getZoneY(), b.getZoneY());
    EXPECT_EQ(a.getInvenX(), b.getInvenX());
    EXPECT_EQ(a.getInvenY(), b.getInvenY());
}
ENCRYPTER_PACKET_TESTS(CGAddZoneToInventory)

// Five fields go through the shuffle; m_TargetY is written after it,
// unshuffled. m_InventoryItemObjectID is not on the wire at all, but
// getPacketSize() still counts it (over-reports by szObjectID). Logged in
// docs/RESTRUCTURING.md 1.2.
void fill(CGSkillToInventory& p) {
    p.setSkillType(0x8D7E);
    p.setObjectID(0xCEDFE0F1);
    p.setX(0x91);
    p.setY(0xA3);
    p.setTargetX(0xB5);
    p.setTargetY(0xC7);
}
void expectEqual(CGSkillToInventory& a, CGSkillToInventory& b) {
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getTargetX(), b.getTargetX());
    EXPECT_EQ(a.getTargetY(), b.getTargetY());
}
ENCRYPTER_PACKET_TESTS_WITH_SIZE_DRIFT(CGSkillToInventory, szObjectID)

// GCAddItemToZone is the abstract base (no packet ID of its own) of the
// three item-to-zone packets. It is the only encrypter layout with a
// variable-length tail: a BYTE-counted option list and a BYTE-counted
// SubItemInfo list follow the shuffled header, unencrypted. Two options
// and one sub-item, so both counts are non-trivial and the per-element
// layout is pinned. Each concrete subclass below reuses this fixture.
void fillItemBase(GCAddItemToZone& p) {
    p.setObjectID(0xE0F10213);
    p.setX(0x87);
    p.setY(0x99);
    p.setItemClass(0xAB);
    p.setItemType(0x8ABC);
    p.addOptionType(0xBD);
    p.addOptionType(0xCF);
    p.setSilver(0x9DEF);
    p.setGrade(-0x12345678);
    p.setDurability(0xF2031425);
    p.setEnchantLevel(-0x37);
    p.setItemNum(0xD1);
    SubItemInfo* pSub = new SubItemInfo();
    pSub->setObjectID(0x03142536);
    pSub->setItemClass(0xE3);
    pSub->setItemType(0x9BCD);
    pSub->setItemNum(0xF5);
    pSub->setSlotID(0x86);
    p.addListElement(pSub); // the packet's destructor owns it
    p.setListNum(1);
}
void expectItemBaseEqual(GCAddItemToZone& a, GCAddItemToZone& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getItemClass(), b.getItemClass());
    EXPECT_EQ(a.getItemType(), b.getItemType());
    EXPECT_EQ(a.getOptionType(), b.getOptionType());
    EXPECT_EQ(a.getSilver(), b.getSilver());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    EXPECT_EQ(a.getDurability(), b.getDurability());
    EXPECT_EQ(a.getEnchantLevel(), b.getEnchantLevel());
    EXPECT_EQ(a.getItemNum(), b.getItemNum());
    ASSERT_EQ(a.getListNum(), b.getListNum());
    // There is no const accessor for the sub-item list; pop the single
    // element from each side and compare. Popped elements are ours to free.
    SubItemInfo* pa = a.popFrontListElement();
    SubItemInfo* pb = b.popFrontListElement();
    ASSERT_TRUE(pa != NULL && pb != NULL);
    EXPECT_EQ(pa->getObjectID(), pb->getObjectID());
    EXPECT_EQ(pa->getItemClass(), pb->getItemClass());
    EXPECT_EQ(pa->getItemType(), pb->getItemType());
    EXPECT_EQ(pa->getItemNum(), pb->getItemNum());
    EXPECT_EQ(pa->getSlotID(), pb->getSlotID());
    delete pa;
    delete pb;
}

void fill(GCAddNewItemToZone& p) {
    fillItemBase(p);
}
void expectEqual(GCAddNewItemToZone& a, GCAddNewItemToZone& b) {
    expectItemBaseEqual(a, b);
}
ENCRYPTER_PACKET_TESTS(GCAddNewItemToZone)

void fill(GCAddInstalledMineToZone& p) {
    fillItemBase(p);
}
void expectEqual(GCAddInstalledMineToZone& a, GCAddInstalledMineToZone& b) {
    expectItemBaseEqual(a, b);
}
ENCRYPTER_PACKET_TESTS(GCAddInstalledMineToZone)

// GCDropItemToZone appends a pet ObjectID after the base layout. Its
// read() also consumes a leading BYTE `flag` that write() no longer emits
// (the write is commented out, and getPacketSize() does not count it), so
// the server-side read() is out of step with what the server sends. The
// server only ever WRITES this GC packet, so the wire contract is write();
// that is what the golden pins. No loopback round-trip: it cannot pass
// until read() is fixed or deleted. Logged in docs/RESTRUCTURING.md 1.2.
void fill(GCDropItemToZone& p) {
    fillItemBase(p);
    p.setDropPetOID(0x47586970);
}

ENCRYPTER_PACKET_GOLDENS(GCDropItemToZone)

// State the asymmetry as a fact so its eventual fix is a deliberate,
// visible change (this test flips to failing, and the round-trip can then
// be added) rather than a silent one.
TEST(GCDropItemToZoneTest, readStillExpectsALeadingFlagByteThatWriteDoesNotEmit) {
    GCDropItemToZone packet;
    fill(packet);
    std::vector<unsigned char> body = writeBody(packet, 0);
    EXPECT_EQ(packet.getPacketSize(), body.size()) << "write() and getPacketSize() agree";

    GCDropItemToZone dst;
    EXPECT_THROW(roundTrip(packet, dst, 0), Throwable)
        << "read() now consumes exactly what write() emits — add GCDropItemToZone to "
           "ENCRYPTER_PACKET_TESTS and drop this test";
}

//////////////////////////////////////////////////////////////////////
// Single-field encrypter packets - no shuffle, but the byte transform
// still depends on the code
//////////////////////////////////////////////////////////////////////

void fill(CGAddMouseToZone& p) {
    p.setObjectID(0x14253647);
}
void expectEqual(CGAddMouseToZone& a, CGAddMouseToZone& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
}
ENCRYPTER_PACKET_TESTS(CGAddMouseToZone)

void fill(CGDropMoney& p) {
    p.setAmount(0x8899AABB);
}
void expectEqual(CGDropMoney& a, CGDropMoney& b) {
    EXPECT_EQ(a.getAmount(), b.getAmount());
}
ENCRYPTER_PACKET_TESTS(CGDropMoney)

//////////////////////////////////////////////////////////////////////
// The shuffle tables themselves, stated as facts
//////////////////////////////////////////////////////////////////////

// Pin the field ORDER each macro produces, independently of any packet, so
// a golden diff on a _4/_5 packet can be attributed: if this still passes
// the packet changed; if this fails, EncryptUtility.h changed.
std::string order2(uchar code) {
    std::string s;
    SHUFFLE_STATEMENT_2(code, s += 'A', s += 'B');
    return s;
}
std::string order3(uchar code) {
    std::string s;
    SHUFFLE_STATEMENT_3(code, s += 'A', s += 'B', s += 'C');
    return s;
}
std::string order4(uchar code) {
    std::string s;
    SHUFFLE_STATEMENT_4(code, s += 'A', s += 'B', s += 'C', s += 'D');
    return s;
}
std::string order5(uchar code) {
    std::string s;
    SHUFFLE_STATEMENT_5(code, s += 'A', s += 'B', s += 'C', s += 'D', s += 'E');
    return s;
}

TEST(ShuffleTableTest, ordersAreUnchanged) {
    EXPECT_EQ("AB", order2(0));
    EXPECT_EQ("BA", order2(1));

    EXPECT_EQ("ABC", order3(0));
    EXPECT_EQ("BCA", order3(1));
    EXPECT_EQ("CAB", order3(2));

    EXPECT_EQ("ABCD", order4(0));
    EXPECT_EQ("BCDA", order4(1));
    EXPECT_EQ("CDAB", order4(2));
    EXPECT_EQ("DACB", order4(3)) << "not a rotation, on purpose";

    EXPECT_EQ("ABCDE", order5(0));
    EXPECT_EQ("BCDEA", order5(1));
    EXPECT_EQ("CDEAB", order5(2));
    EXPECT_EQ("DEBAC", order5(3)) << "not a rotation, on purpose";
    EXPECT_EQ("ECDAB", order5(4)) << "not a rotation, on purpose";
}

// kEncryptCodes must reach every case of every arity through the
// encrypted branch (code != 0). If someone trims the table, this names
// the case that went dark.
TEST(ShuffleTableTest, encryptCodesReachEveryShuffleCase) {
    for (int n = 2; n <= 5; n++) {
        for (int wanted = 0; wanted < n; wanted++) {
            bool reached = false;
            for (size_t i = 0; i < kEncryptCodeCount; i++)
                if (kEncryptCodes[i] != 0 && kEncryptCodes[i] % n == wanted)
                    reached = true;
            EXPECT_TRUE(reached) << "SHUFFLE_STATEMENT_" << n << " case " << wanted
                                 << " is not reached by any non-zero code in kEncryptCodes";
        }
    }
}

} // namespace
