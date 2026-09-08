//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_zone_scan_test.cpp
// Description : Golden byte fixtures, loopback round trips and size
//               pins for the packets that populate a player's view of
//               a zone the moment Zone::addPC places the character,
//               and for the packets that later add or remove a single
//               object from that view.
//
//               The set is taken from the code that runs in that
//               window: Zone::addPC and the Zone::scan() it calls, the
//               two helpers scan() delegates to
//               (Zone::createMonsterAddPacket and sendRelicEffect),
//               and Zone::moveFastPC, which reframes an existing view
//               one object at a time. Eighteen packets, each with the
//               reason it is here:
//
//               GCAddSlayer      what scan() sends the arriving player
//                                for every visible Slayer, and what
//                                addPC broadcasts to the players who
//                                can see the arrival.
//               GCAddVampire     the same for a visible Vampire that
//                                is not hiding; its FromFlag says the
//                                character stepped out of a portal.
//               GCAddOusters     the same for a visible Ousters.
//               GCAddMonster     the default branch of
//                                createMonsterAddPacket, the packet
//                                for an ordinary or invisible monster.
//               GCAddBurrowingCreature  the branch above it: a monster
//                                or a Vampire carrying the HIDE
//                                effect, which the client draws as a
//                                mound rather than a creature.
//               GCAddBat         createMonsterAddPacket's branch for a
//               GCAddWolf        monster under TRANSFORM_TO_BAT and
//                                TRANSFORM_TO_WOLF.
//               GCAddNPC         what scan() sends for every visible
//                                NPC.
//               GCAddSlayerCorpse   the four corpse packets scan()
//               GCAddVampireCorpse  sends for an ITEM_CLASS_CORPSE on
//               GCAddOustersCorpse  a visible tile, one per race plus
//               GCAddMonsterCorpse  the monster corpse, which also
//                                carries the kill and treasure state.
//               GCAddEffect      what sendRelicEffect() puts on a
//                                monster corpse right after
//                                GCAddMonsterCorpse: the relic, shrine,
//                                blood bible, castle symbol, flag and
//                                sweeper markers a corpse can hold.
//               GCAddEffectToTile   what scan() sends for a sanctuary
//                                centre tile and for every effect that
//                                answers isBroadcastingEffect().
//               GCAddVampirePortal  what scan() sends for a
//                                VAMPIRE_PORTAL effect standing on a
//                                visible tile.
//               GCDeleteObject   the removal half: moveFastPC and every
//                                creature, item and corpse teardown
//                                path take one object back out of the
//                                view with it.
//               GCDeleteEffectFromTile  the same removal for a tile
//                                effect, the counterpart of
//                                GCAddEffectToTile.
//               GCFastMove       what moveFastPC sends the moving
//                                player before it walks the old and new
//                                viewports emitting the adds and
//                                deletes above.
//
//               Deliberately excluded:
//
//               GCSetPosition, which Zone::addPC sends before the scan,
//               is already pinned by
//               tests/packet_gameserver_handshake_test.cpp.
//
//               GCAddNewItemToZone and GCAddInstalledMineToZone, the
//               two packets scan() sends for a non-corpse item and for
//               an installed mine, use the encrypter and are pinned at
//               encrypt codes 0..5 by tests/packet_encrypter_test.cpp.
//               No packet in this file calls readEncrypt/writeEncrypt,
//               so the goldens here are recorded at code 0 only, and
//               every golden test also asserts the bytes do not vary
//               with the code, so adopting the encrypter fails loudly
//               instead of silently voiding the pin.
//
//               GCAddMonsterFromBurrowing, GCAddVampireFromBurrowing,
//               GCAddMonsterFromTransformation and
//               GCAddVampireFromTransformation belong to the unburrow
//               and untransform transitions, not to view population.
//               GCAddHelicopter (waypoint travel),
//               GCAddInjuriousCreature (a skill result) and
//               GCRemoveEffect (effect expiry on an object already in
//               view) are likewise sent from other paths; scan() builds
//               none of them.
//
//               Each packet gets three pins (ZONE_SCAN_PACKET_TESTS):
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
//               The creature-add packets carry optional records, and
//               extra goldens cover the branches one fixture cannot:
//               GCAddSlayer.bare (no pet, a NICK_NONE nickname, a
//               closed store, an empty effect list) against the
//               canonical GCAddSlayer (pet, NICK_CUSTOM string, open
//               store with a sign, three effects); GCAddVampire carries
//               a NICK_BUILT_IN index and GCAddOusters a
//               NICK_CUSTOM_FORCED string; GCAddMonster.noname and
//               GCAddMonsterCorpse.noname take the empty-name branch of
//               their write(); GCAddVampirePortal.noowner takes the
//               zero-length owner branch.
//
//               Fixture values are distinct per field and >= 128 in
//               every byte the width allows. Three groups cannot follow
//               that rule and say so at the point of use: enum-valued
//               bytes, whose domain is a handful of small values; the
//               Slayer outlook bitset, whose fields are 2 to 5 bits
//               wide and hold enumerators; and the names, which each
//               packet caps well below 128 characters.
//
//               Eight findings are stated below as tests that FAIL when
//               the underlying code is fixed, which is the signal to
//               retire them:
//               oversizedSlayerNameIsSwallowedAndUnderflowsTheBody,
//               readConsumesALeadingFlagByteThatWriteNeverEmits,
//               vampireInfoMaxSizeOmitsTheAlignmentField,
//               oustersInfoMaxSizeOmitsTheAlignmentField,
//               coatTypeIsTruncatedToAByteOnTheWire,
//               unboundedMonsterNamesWrapTheLengthByteAndOutgrowTheFactoryMax,
//               unboundedOptionalStringsOutgrowTheirRecordMax and
//               unboundedPortalOwnerOutgrowsTheFactoryMax.
//
//////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "EffectInfo.h"
#include "Exception.h"
#include "GCAddBat.h"
#include "GCAddBurrowingCreature.h"
#include "GCAddEffect.h"
#include "GCAddEffectToTile.h"
#include "GCAddMonster.h"
#include "GCAddMonsterCorpse.h"
#include "GCAddNPC.h"
#include "GCAddOusters.h"
#include "GCAddOustersCorpse.h"
#include "GCAddSlayer.h"
#include "GCAddSlayerCorpse.h"
#include "GCAddVampire.h"
#include "GCAddVampireCorpse.h"
#include "GCAddVampirePortal.h"
#include "GCAddWolf.h"
#include "GCDeleteEffectFromTile.h"
#include "GCDeleteObject.h"
#include "GCFastMove.h"
#include "NicknameInfo.h"
#include "PCOustersInfo3.h"
#include "PCSlayerInfo3.h"
#include "PCVampireInfo3.h"
#include "PetInfo.h"
#include "StoreInfo.h"
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
// overloaded per packet, as in packet_login_test.cpp, so the same
// canonical instance feeds all three.
//////////////////////////////////////////////////////////////////////

#define ZONE_SCAN_PACKET_TESTS(Name)                                                                 \
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

// The golden and size half on its own, for a packet whose read() cannot
// consume what its write() emits.
#define ZONE_SCAN_GOLDEN_AND_SIZE(Name)                                                              \
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
// Shared optional records.
//////////////////////////////////////////////////////////////////////

// The creature-add packets take ownership of the EffectInfo they are
// handed, so every fixture allocates a fresh one.
EffectInfo* makeEffectInfo(int count) {
    EffectInfo* pInfo = new EffectInfo();
    for (int i = 0; i < count; i++)
        pInfo->addListElement((EffectID_t)(0x81A2 + i * 0x0101), (WORD)(0x93B4 + i * 0x0101));
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

void fillPetInfo(PetInfo& info) {
    // Pet type is an enum byte with a handful of enumerators, so it
    // carries a valid enumerator rather than a high byte.
    info.setPetType(PET_PIXIE);
    info.setPetCreatureType(0x8394);
    info.setPetLevel(0x85);
    info.setPetExp(0x8697A8B9);
    info.setPetHP(0x8CAD);
    info.setPetAttr(0x8E);
    info.setPetAttrLevel(0x8F);
    info.setPetOption(0x90);
    info.setFoodType(0x91A2);
    info.setGamble(0x93);
    info.setCutHead(0x94);
    info.setAttack(0x95);
    info.setNickname("GoldScanPet");
}

// write() clears the summon flag on every pet it sends, so the value the
// fixture sets is not part of the comparison.
void expectPetInfoEqual(PetInfo* a, PetInfo* b) {
    if (a == NULL || b == NULL) {
        EXPECT_TRUE(a == NULL) << "one side carries a pet and the other does not";
        EXPECT_TRUE(b == NULL) << "one side carries a pet and the other does not";
        return;
    }
    EXPECT_EQ(a->getPetType(), b->getPetType());
    EXPECT_EQ(a->getPetCreatureType(), b->getPetCreatureType());
    EXPECT_EQ(a->getPetLevel(), b->getPetLevel());
    EXPECT_EQ(a->getPetExp(), b->getPetExp());
    EXPECT_EQ(a->getPetHP(), b->getPetHP());
    EXPECT_EQ(a->getPetAttr(), b->getPetAttr());
    EXPECT_EQ(a->getPetAttrLevel(), b->getPetAttrLevel());
    EXPECT_EQ(a->getPetOption(), b->getPetOption());
    EXPECT_EQ(a->getFoodType(), b->getFoodType());
    EXPECT_EQ(a->canGamble(), b->canGamble());
    EXPECT_EQ(a->canCutHead(), b->canCutHead());
    EXPECT_EQ(a->canAttack(), b->canAttack());
    EXPECT_EQ(a->getNickname(), b->getNickname());
}

void expectNicknameInfoEqual(NicknameInfo* a, NicknameInfo* b) {
    ASSERT_TRUE(a != NULL);
    ASSERT_TRUE(b != NULL);
    EXPECT_EQ(a->getNicknameID(), b->getNicknameID());
    ASSERT_EQ(a->getNicknameType(), b->getNicknameType());
    switch (a->getNicknameType()) {
    case NicknameInfo::NICK_NONE:
        break;
    case NicknameInfo::NICK_BUILT_IN:
    case NicknameInfo::NICK_QUEST:
    case NicknameInfo::NICK_FORCED:
        EXPECT_EQ(a->getNicknameIndex(), b->getNicknameIndex());
        break;
    default:
        EXPECT_EQ(a->getNickname(), b->getNickname());
        break;
    }
}

void expectStoreOutlookEqual(const StoreOutlook& a, const StoreOutlook& b) {
    EXPECT_EQ(a.isOpen(), b.isOpen());
    if (a.isOpen() != 0)
        EXPECT_EQ(a.getSign(), b.getSign());
}

//////////////////////////////////////////////////////////////////////
// The three PC records the creature-add and corpse packets share.
//////////////////////////////////////////////////////////////////////

// The outlook fields are 2 to 5 bit wide slices of one DWORD bitset and
// hold enumerators, so each carries its highest valid enumerator rather
// than a high byte. The nine colours are WORDs and do carry high bytes.
void fillSlayerInfo(PCSlayerInfo3& info) {
    info.setObjectID(0x8A9BACBD);
    info.setName("GoldScanSlayerName");
    info.setX(0x8C);
    info.setY(0x9D);
    info.setDir(0xAE);
    info.setSex(MALE);
    info.setHairStyle(HAIR_STYLE3);
    info.setHelmetType(HELMET3);
    info.setJacketType(JACKET4);
    info.setPantsType(PANTS4);
    info.setWeaponType(WEAPON_MACE);
    info.setShieldType(SHIELD2);
    info.setMotorcycleType(MOTORCYCLE3);
    info.setShoulderType(3);
    info.setHairColor(0x81C2);
    info.setSkinColor(0x83C4);
    info.setHelmetColor(0x85C6);
    info.setJacketColor(0x87C8, MAIN_COLOR);
    info.setJacketColor(0x89CA, SUB_COLOR);
    info.setPantsColor(0x8BCC, MAIN_COLOR);
    info.setPantsColor(0x8DCE, SUB_COLOR);
    info.setWeaponColor(0x8FD0);
    info.setShieldColor(0x91D2);
    info.setMotorcycleColor(0x93D4);
    info.setShoulderColor(0x95D6);
    info.setMasterEffectColor(0x97);
    info.setCurrentHP(0x98D9);
    info.setMaxHP(0x9ADB);
    info.setAttackSpeed(0x9C);
    info.setAlignment((Alignment_t)0x9DDEEFA0);
    info.setCompetence(0xA1);
    info.setGuildID(0xA2B3);
    info.setUnionID(0xA4B5C6D7);
    info.setRank(0xA8);
    info.setAdvancementLevel(0xA9);
}

void expectSlayerInfoEqual(const PCSlayerInfo3& a, const PCSlayerInfo3& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
    EXPECT_EQ(a.getSex(), b.getSex());
    EXPECT_EQ(a.getHairStyle(), b.getHairStyle());
    EXPECT_EQ(a.getHelmetType(), b.getHelmetType());
    EXPECT_EQ(a.getJacketType(), b.getJacketType());
    EXPECT_EQ(a.getPantsType(), b.getPantsType());
    EXPECT_EQ(a.getWeaponType(), b.getWeaponType());
    EXPECT_EQ(a.getShieldType(), b.getShieldType());
    EXPECT_EQ(a.getMotorcycleType(), b.getMotorcycleType());
    EXPECT_EQ(a.getShoulderType(), b.getShoulderType());
    EXPECT_EQ(a.getHairColor(), b.getHairColor());
    EXPECT_EQ(a.getSkinColor(), b.getSkinColor());
    EXPECT_EQ(a.getHelmetColor(), b.getHelmetColor());
    EXPECT_EQ(a.getJacketColor(MAIN_COLOR), b.getJacketColor(MAIN_COLOR));
    EXPECT_EQ(a.getJacketColor(SUB_COLOR), b.getJacketColor(SUB_COLOR));
    EXPECT_EQ(a.getPantsColor(MAIN_COLOR), b.getPantsColor(MAIN_COLOR));
    EXPECT_EQ(a.getPantsColor(SUB_COLOR), b.getPantsColor(SUB_COLOR));
    EXPECT_EQ(a.getWeaponColor(), b.getWeaponColor());
    EXPECT_EQ(a.getShieldColor(), b.getShieldColor());
    EXPECT_EQ(a.getMotorcycleColor(), b.getMotorcycleColor());
    EXPECT_EQ(a.getShoulderColor(), b.getShoulderColor());
    EXPECT_EQ(a.getMasterEffectColor(), b.getMasterEffectColor());
    EXPECT_EQ(a.getCurrentHP(), b.getCurrentHP());
    EXPECT_EQ(a.getMaxHP(), b.getMaxHP());
    EXPECT_EQ(a.getAttackSpeed(), b.getAttackSpeed());
    EXPECT_EQ(a.getAlignment(), b.getAlignment());
    EXPECT_EQ(a.getCompetence(), b.getCompetence());
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getUnionID(), b.getUnionID());
    EXPECT_EQ(a.getRank(), b.getRank());
    EXPECT_EQ(a.getAdvancementLevel(), b.getAdvancementLevel());
}

// The name stays at twelve characters: PCVampireInfo3::getMaxSize()
// budgets four bytes less than getSize() returns (see
// vampireInfoMaxSizeOmitsTheAlignmentField), so a full twenty-character
// name would put every carrier of this record over its factory max.
// Sex is an enum byte with two enumerators and the coat type is written
// as a single byte, so neither carries a high byte.
void fillVampireInfo(PCVampireInfo3& info) {
    info.setObjectID(0x8B9CADBE);
    info.setName("GoldScanVamp");
    info.setX(0x8D);
    info.setY(0x9E);
    info.setDir(0xAF);
    info.setSex(MALE);
    info.setCoatType(VAMPIRE_COAT4);
    info.setBatColor(0x82C3);
    info.setSkinColor(0x84C5);
    info.setCoatColor(0x86C7, MAIN_COLOR);
    info.setCoatColor(0x88C9, SUB_COLOR);
    info.setMasterEffectColor(0x8A);
    info.setCurrentHP(0x8BDC);
    info.setMaxHP(0x8DDE);
    info.setAttackSpeed(0x8F);
    info.setAlignment((Alignment_t)0x90A1B2C3);
    info.setShape(0x94);
    info.setCompetence(0x95);
    info.setGuildID(0x96A7);
    info.setUnionID(0x98A9BACB);
    info.setRank(0x9C);
    info.setAdvancementLevel(0x9D);
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

// The name stays at twelve characters for the same reason as the
// Vampire record (see oustersInfoMaxSizeOmitsTheAlignmentField). Coat,
// arm and sylph type share one packed byte and hold enumerators, so
// they carry their highest valid enumerator rather than a high byte.
void fillOustersInfo(PCOustersInfo3& info) {
    info.setObjectID(0x8C9DAEBF);
    info.setName("GoldScanOust");
    info.setX(0x8E);
    info.setY(0x9F);
    info.setDir(0xB0);
    info.setSex(MALE);
    info.setCoatType(OUSTERS_COAT4);
    info.setArmType(OUSTERS_ARM_CHAKRAM);
    info.setSylphType(OUSTERS_SYLPH1);
    info.setCoatColor(0x83C4);
    info.setHairColor(0x85C6);
    info.setArmColor(0x87C8);
    info.setBootsColor(0x89CA);
    info.setMasterEffectColor(0x8B);
    info.setCurrentHP(0x8CDD);
    info.setMaxHP(0x8EDF);
    info.setAttackSpeed(0x90);
    info.setAlignment((Alignment_t)0x91A2B3C4);
    info.setCompetence(0x95);
    info.setGuildID(0x96A7);
    info.setUnionID(0x98A9BACB);
    info.setRank(0x9C);
    info.setAdvancementLevel(0x9D);
}

void expectOustersInfoEqual(const PCOustersInfo3& a, const PCOustersInfo3& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
    EXPECT_EQ(a.getSex(), b.getSex());
    EXPECT_EQ(a.getCoatType(), b.getCoatType());
    EXPECT_EQ(a.getArmType(), b.getArmType());
    EXPECT_EQ(a.getSylphType(), b.getSylphType());
    EXPECT_EQ(a.getCoatColor(), b.getCoatColor());
    EXPECT_EQ(a.getHairColor(), b.getHairColor());
    EXPECT_EQ(a.getArmColor(), b.getArmColor());
    EXPECT_EQ(a.getBootsColor(), b.getBootsColor());
    EXPECT_EQ(a.getMasterEffectColor(), b.getMasterEffectColor());
    EXPECT_EQ(a.getCurrentHP(), b.getCurrentHP());
    EXPECT_EQ(a.getMaxHP(), b.getMaxHP());
    EXPECT_EQ(a.getAttackSpeed(), b.getAttackSpeed());
    EXPECT_EQ(a.getAlignment(), b.getAlignment());
    EXPECT_EQ(a.getCompetence(), b.getCompetence());
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getUnionID(), b.getUnionID());
    EXPECT_EQ(a.getRank(), b.getRank());
    EXPECT_EQ(a.getAdvancementLevel(), b.getAdvancementLevel());
}

//////////////////////////////////////////////////////////////////////
// GCAddSlayer / GCAddVampire / GCAddOusters.
//
// The three packets own their sub-records inconsistently: GCAddOusters
// deletes the effect, pet and nickname records, while GCAddSlayer and
// GCAddVampire delete only the effect record. The fixtures follow what
// each destructor does, so the pet and nickname records for the first
// two are members that outlive the packet — members are destroyed in
// reverse declaration order, so `packet` goes first.
//
// A NULL nickname pointer is not goldened separately: write() then
// emits a default NicknameInfo, the same NICK_NONE shape the explicit
// record below pins. The three nickname shapes are pinned through
// explicitly constructed records.
//////////////////////////////////////////////////////////////////////

struct SlayerAddFixture {
    PetInfo pet;
    NicknameInfo nickname;
    StoreInfo store;
    GCAddSlayer packet;
};

void fillSlayerAdd(SlayerAddFixture& f) {
    PCSlayerInfo3 info;
    fillSlayerInfo(info);
    f.packet.setSlayerInfo(info);
    f.packet.setEffectInfo(makeEffectInfo(3));

    fillPetInfo(f.pet);
    f.packet.setPetInfo(&f.pet);

    f.nickname.setNicknameID(0xAABB);
    f.nickname.setNicknameType(NicknameInfo::NICK_CUSTOM);
    f.nickname.setNickname("GoldScanNick");
    f.packet.setNicknameInfo(&f.nickname);

    f.store.setOpen(0xCC);
    f.store.setSign("GoldScanSlayerStoreSign");
    f.packet.setStoreInfo(&f.store);
}

// The branch every ordinary character takes: no pet, no nickname, a
// closed store and no effects.
void fillSlayerAddBare(SlayerAddFixture& f) {
    PCSlayerInfo3 info;
    fillSlayerInfo(info);
    f.packet.setSlayerInfo(info);
    f.packet.setEffectInfo(makeEffectInfo(0));

    f.nickname.setNicknameID(0xB1C2);
    f.nickname.setNicknameType(NicknameInfo::NICK_NONE);
    f.packet.setNicknameInfo(&f.nickname);

    f.store.setOpen(0);
    f.packet.setStoreInfo(&f.store);
}

void expectSlayerAddEqual(GCAddSlayer& a, GCAddSlayer& b) {
    expectSlayerInfoEqual(a.getSlayerInfo(), b.getSlayerInfo());
    expectEffectInfoEqual(a.getEffectInfo(), b.getEffectInfo());
    expectPetInfoEqual(a.getPetInfo(), b.getPetInfo());
    expectNicknameInfoEqual(a.getNicknameInfo(), b.getNicknameInfo());
    expectStoreOutlookEqual(a.getStoreOutlook(), b.getStoreOutlook());
}

void expectSlayerAddPins(GCAddSlayer& packet, const char* goldenName) {
    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    expectGolden(goldenName, kPlainCode, body);
    for (size_t i = 1; i < kEncryptCodeCount; i++)
        EXPECT_EQ(body, writeBody(packet, kEncryptCodes[i]))
            << goldenName << " now varies with the encrypt code — add per-code goldens";

    GCAddSlayerFactory factory;
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size())
        << goldenName
        << ": getPacketSize() disagrees with the bytes write() emits; writePacket() puts the former "
           "on the wire, so the stream never resynchronises";
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize())
        << goldenName << ": the body outgrows the read buffer the receiver sizes from the factory max";
    EXPECT_EQ(factory.getPacketID(), packet.getPacketID());
    EXPECT_EQ(factory.getPacketName(), packet.getPacketName());
}

TEST(GCAddSlayerTest, roundTripsThroughLoopback) {
    SlayerAddFixture f;
    fillSlayerAdd(f);
    GCAddSlayer dst;
    roundTrip(f.packet, dst, kPlainCode);
    expectSlayerAddEqual(f.packet, dst);
}

TEST(GCAddSlayerTest, bodyBytesMatchGoldenAndSizeMatchesTheBytesWritten) {
    SlayerAddFixture f;
    fillSlayerAdd(f);
    expectSlayerAddPins(f.packet, "GCAddSlayer");
}

TEST(GCAddSlayerTest, bareRoundTripsThroughLoopback) {
    SlayerAddFixture f;
    fillSlayerAddBare(f);
    GCAddSlayer dst;
    roundTrip(f.packet, dst, kPlainCode);
    EXPECT_TRUE(f.packet.getPetInfo() == NULL);
    EXPECT_TRUE(dst.getPetInfo() == NULL) << "a PET_NONE record must read back as no pet";
    expectSlayerAddEqual(f.packet, dst);
}

TEST(GCAddSlayerTest, bareBodyBytesMatchGoldenAndSizeMatchesTheBytesWritten) {
    SlayerAddFixture f;
    fillSlayerAddBare(f);
    expectSlayerAddPins(f.packet, "GCAddSlayer.bare");
}

struct VampireAddFixture {
    PetInfo pet;
    NicknameInfo nickname;
    StoreInfo store;
    GCAddVampire packet;
};

void fillVampireAdd(VampireAddFixture& f) {
    PCVampireInfo3 info;
    fillVampireInfo(info);
    f.packet.setVampireInfo(info);
    f.packet.setEffectInfo(makeEffectInfo(2));

    fillPetInfo(f.pet);
    f.packet.setPetInfo(&f.pet);

    // The indexed nickname shape.
    f.nickname.setNicknameID(0xB2C3);
    f.nickname.setNicknameType(NicknameInfo::NICK_BUILT_IN);
    f.nickname.setNicknameIndex(0xB4D5);
    f.packet.setNicknameInfo(&f.nickname);

    f.store.setOpen(0xD6);
    f.store.setSign("GoldScanVampStoreSign");
    f.packet.setStoreInfo(&f.store);

    // What makeGCAddVampire sets when the character stepped out of a
    // personal portal.
    f.packet.setFromFlag(1);
}

TEST(GCAddVampireTest, roundTripsThroughLoopback) {
    VampireAddFixture f;
    fillVampireAdd(f);
    GCAddVampire dst;
    roundTrip(f.packet, dst, kPlainCode);
    expectVampireInfoEqual(f.packet.getVampireInfo(), dst.getVampireInfo());
    expectEffectInfoEqual(f.packet.getEffectInfo(), dst.getEffectInfo());
    expectPetInfoEqual(f.packet.getPetInfo(), dst.getPetInfo());
    expectNicknameInfoEqual(f.packet.getNicknameInfo(), dst.getNicknameInfo());
    expectStoreOutlookEqual(f.packet.getStoreOutlook(), dst.getStoreOutlook());
    EXPECT_EQ(f.packet.getFromFlag(), dst.getFromFlag());
}

TEST(GCAddVampireTest, bodyBytesMatchGoldenAndSizeMatchesTheBytesWritten) {
    VampireAddFixture f;
    fillVampireAdd(f);

    const std::vector<unsigned char> body = writeBody(f.packet, kPlainCode);
    expectGolden("GCAddVampire", kPlainCode, body);
    for (size_t i = 1; i < kEncryptCodeCount; i++)
        EXPECT_EQ(body, writeBody(f.packet, kEncryptCodes[i]))
            << "GCAddVampire now varies with the encrypt code — add per-code goldens";

    GCAddVampireFactory factory;
    EXPECT_EQ((size_t)f.packet.getPacketSize(), body.size())
        << "GCAddVampire: getPacketSize() disagrees with the bytes write() emits; writePacket() puts "
           "the former on the wire, so the stream never resynchronises";
    EXPECT_LE(f.packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCAddVampire: the body outgrows the read buffer the receiver sizes from the factory max";
    EXPECT_EQ(factory.getPacketID(), f.packet.getPacketID());
    EXPECT_EQ(factory.getPacketName(), f.packet.getPacketName());
}

// PCVampireInfo3::write() lets its name refusal out, unlike
// PCSlayerInfo3::write() below. The empty-name half of the same refusal
// is not exercised here: setName() asserts on it before write() ever
// sees the record.
TEST(GCAddVampireTest, refusesOversizedNames) {
    SocketEncryptOutputStream oStream(NULL);

    VampireAddFixture tooLong;
    fillVampireAdd(tooLong);
    tooLong.packet.getVampireInfo().setName(std::string(21, 'v'));
    EXPECT_THROW(tooLong.packet.write(oStream), InvalidProtocolException);
}

// GCAddOusters deletes all three sub-records, so all three are
// allocated for it and nothing outlives the packet.
void fillOustersAdd(GCAddOusters& p) {
    PCOustersInfo3 info;
    fillOustersInfo(info);
    p.setOustersInfo(info);
    p.setEffectInfo(makeEffectInfo(1));

    // The forced-string nickname shape.
    NicknameInfo* pNickname = new NicknameInfo();
    pNickname->setNicknameID(0xB5C6);
    pNickname->setNicknameType(NicknameInfo::NICK_CUSTOM_FORCED);
    pNickname->setNickname("GoldScanForced");
    p.setNicknameInfo(pNickname);

    StoreInfo store;
    store.setOpen(0xD7);
    store.setSign("GoldScanOustStoreSign");
    p.setStoreInfo(&store);
}

TEST(GCAddOustersTest, roundTripsThroughLoopback) {
    GCAddOusters src;
    fillOustersAdd(src);
    GCAddOusters dst;
    roundTrip(src, dst, kPlainCode);
    expectOustersInfoEqual(src.getOustersInfo(), dst.getOustersInfo());
    expectEffectInfoEqual(src.getEffectInfo(), dst.getEffectInfo());
    expectPetInfoEqual(src.getPetInfo(), dst.getPetInfo());
    expectNicknameInfoEqual(src.getNicknameInfo(), dst.getNicknameInfo());
    expectStoreOutlookEqual(src.getStoreOutlook(), dst.getStoreOutlook());
}

TEST(GCAddOustersTest, bodyBytesMatchGoldenAndSizeMatchesTheBytesWritten) {
    GCAddOusters packet;
    fillOustersAdd(packet);

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    expectGolden("GCAddOusters", kPlainCode, body);
    for (size_t i = 1; i < kEncryptCodeCount; i++)
        EXPECT_EQ(body, writeBody(packet, kEncryptCodes[i]))
            << "GCAddOusters now varies with the encrypt code — add per-code goldens";

    GCAddOustersFactory factory;
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size())
        << "GCAddOusters: getPacketSize() disagrees with the bytes write() emits; writePacket() puts "
           "the former on the wire, so the stream never resynchronises";
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCAddOusters: the body outgrows the read buffer the receiver sizes from the factory max";
    EXPECT_EQ(factory.getPacketID(), packet.getPacketID());
    EXPECT_EQ(factory.getPacketName(), packet.getPacketName());
}

//////////////////////////////////////////////////////////////////////
// The corpse packets.
//////////////////////////////////////////////////////////////////////

void fill(GCAddSlayerCorpse& p) {
    fillSlayerInfo(p.getSlayerInfo());
    p.setTreasureCount(0x8B);
}
void expectEqual(const GCAddSlayerCorpse& a, const GCAddSlayerCorpse& b) {
    expectSlayerInfoEqual(a.getSlayerInfo(), b.getSlayerInfo());
    EXPECT_EQ(a.getTreasureCount(), b.getTreasureCount());
}
ZONE_SCAN_PACKET_TESTS(GCAddSlayerCorpse)

void fill(GCAddVampireCorpse& p) {
    fillVampireInfo(p.getVampireInfo());
    p.setTreasureCount(0x9C);
}
void expectEqual(const GCAddVampireCorpse& a, const GCAddVampireCorpse& b) {
    expectVampireInfoEqual(a.getVampireInfo(), b.getVampireInfo());
    EXPECT_EQ(a.getTreasureCount(), b.getTreasureCount());
}
ZONE_SCAN_PACKET_TESTS(GCAddVampireCorpse)

void fill(GCAddOustersCorpse& p) {
    fillOustersInfo(p.getOustersInfo());
    p.setTreasureCount(0xAD);
}
void expectEqual(const GCAddOustersCorpse& a, const GCAddOustersCorpse& b) {
    expectOustersInfoEqual(a.getOustersInfo(), b.getOustersInfo());
    EXPECT_EQ(a.getTreasureCount(), b.getTreasureCount());
}
ZONE_SCAN_PACKET_TESTS(GCAddOustersCorpse)

void fill(GCAddMonsterCorpse& p) {
    p.setObjectID(0x8D9EAFB0);
    p.setMonsterType(0x81C2);
    p.setMonsterName("GoldScanMonsterCorpse");
    p.setX(0x83);
    p.setY(0x94);
    p.setDir(0xA5);
    p.sethasHead(true);
    p.setTreasureCount(0xB6);
    p.setLastKiller(0x87C8D9EA);
}
void expectEqual(const GCAddMonsterCorpse& a, const GCAddMonsterCorpse& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getMonsterType(), b.getMonsterType());
    EXPECT_EQ(a.getMonsterName(), b.getMonsterName());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
    EXPECT_EQ(a.gethasHead(), b.gethasHead());
    EXPECT_EQ(a.getTreasureCount(), b.getTreasureCount());
    EXPECT_EQ(a.getLastKiller(), b.getLastKiller());
}
ZONE_SCAN_PACKET_TESTS(GCAddMonsterCorpse)

// The other branch of write(): a corpse with no name emits a zero
// length byte and nothing else for the name.
TEST(GCAddMonsterCorpseTest, namelessBodyBytesMatchGoldenAndRoundTrips) {
    GCAddMonsterCorpse packet;
    fill(packet);
    packet.setMonsterName("");

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    expectGolden("GCAddMonsterCorpse.noname", kPlainCode, body);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCAddMonsterCorpse dst;
    roundTrip(packet, dst, kPlainCode);
    expectEqual(packet, dst);
}

//////////////////////////////////////////////////////////////////////
// The creature packets scan() sends for a monster, a hidden creature
// and an NPC.
//////////////////////////////////////////////////////////////////////

void fill(GCAddMonster& p) {
    p.setObjectID(0x8E9FB0C1);
    p.setMonsterType(0x82C3);
    p.setMonsterName("GoldScanMonsterName");
    p.setMainColor(0x84C5);
    p.setSubColor(0x86C7);
    p.setX(0x88);
    p.setY(0x99);
    p.setDir(0xAA);
    p.setEffectInfo(makeEffectInfo(2));
    p.setCurrentHP(0x8BCC);
    p.setMaxHP(0x8DCE);
    p.setFromFlag(0x8F);
}
void expectEqual(GCAddMonster& a, GCAddMonster& b) {
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
    EXPECT_EQ(a.getFromFlag(), b.getFromFlag());
    expectEffectInfoEqual(a.getEffectInfo(), b.getEffectInfo());
}
ZONE_SCAN_PACKET_TESTS(GCAddMonster)

// The other branch of write(): an unnamed monster emits a zero length
// byte and nothing else for the name.
TEST(GCAddMonsterTest, namelessBodyBytesMatchGoldenAndRoundTrips) {
    GCAddMonster packet;
    fill(packet);
    packet.setMonsterName("");

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    expectGolden("GCAddMonster.noname", kPlainCode, body);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCAddMonster dst;
    roundTrip(packet, dst, kPlainCode);
    expectEqual(packet, dst);
}

void fill(GCAddBurrowingCreature& p) {
    p.setObjectID(0x8FA0B1C2);
    p.setName("GoldScanBurrow");
    p.setX(0x83);
    p.setY(0x94);
}
void expectEqual(const GCAddBurrowingCreature& a, const GCAddBurrowingCreature& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
}
ZONE_SCAN_PACKET_TESTS(GCAddBurrowingCreature)

void fill(GCAddBat& p) {
    p.setObjectID(0x90A1B2C3);
    p.setName("GoldScanBat");
    p.setItemType(0x84C5);
    p.setXYDir(0x86, 0x97, 0xA8);
    p.setCurrentHP(0x89CA);
    p.setMaxHP(0x8BCC);
    p.setGuildID(0x8DCE);
    p.setColor(0x8FD0);
}
void expectEqual(const GCAddBat& a, const GCAddBat& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getItemType(), b.getItemType());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
    EXPECT_EQ(a.getCurrentHP(), b.getCurrentHP());
    EXPECT_EQ(a.getMaxHP(), b.getMaxHP());
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
    EXPECT_EQ(a.getColor(), b.getColor());
}
ZONE_SCAN_PACKET_TESTS(GCAddBat)

void fill(GCAddWolf& p) {
    p.setObjectID(0x91A2B3C4);
    p.setName("GoldScanWolf");
    p.setMainColor(0x85C6);
    p.setItemType(0x87C8);
    p.setXYDir(0x89, 0x9A, 0xAB);
    p.setCurrentHP(0x8CCD);
    p.setMaxHP(0x8ECF);
    p.setGuildID(0x90D1);
}
void expectEqual(const GCAddWolf& a, const GCAddWolf& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getMainColor(), b.getMainColor());
    EXPECT_EQ(a.getItemType(), b.getItemType());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
    EXPECT_EQ(a.getCurrentHP(), b.getCurrentHP());
    EXPECT_EQ(a.getMaxHP(), b.getMaxHP());
    EXPECT_EQ(a.getGuildID(), b.getGuildID());
}
ZONE_SCAN_PACKET_TESTS(GCAddWolf)

void fill(GCAddNPC& p) {
    p.setObjectID(0x92A3B4C5);
    p.setName("GoldScanNPCName");
    p.setNPCID(0x86C7);
    p.setSpriteType(0x88C9);
    p.setMainColor(0x8ACB);
    p.setSubColor(0x8CCD);
    p.setX(0x8E);
    p.setY(0x9F);
    p.setDir(0xB0);
}
void expectEqual(const GCAddNPC& a, const GCAddNPC& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getName(), b.getName());
    EXPECT_EQ(a.getNPCID(), b.getNPCID());
    EXPECT_EQ(a.getSpriteType(), b.getSpriteType());
    EXPECT_EQ(a.getMainColor(), b.getMainColor());
    EXPECT_EQ(a.getSubColor(), b.getSubColor());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getDir(), b.getDir());
}
ZONE_SCAN_PACKET_TESTS(GCAddNPC)

// The three name-carrying creature packets whose write() lets its own
// refusals out. A caller that hands one an empty or over-length name
// gets an exception, not a short body.
TEST(ZoneScanNamesTest, creaturePacketsRefuseEmptyOrOversizedNames) {
    SocketEncryptOutputStream oStream(NULL);

    GCAddNPC npc;
    fill(npc);
    npc.setName("");
    EXPECT_THROW(npc.write(oStream), InvalidProtocolException);
    npc.setName(std::string(41, 'n'));
    EXPECT_THROW(npc.write(oStream), InvalidProtocolException);

    GCAddBurrowingCreature burrow;
    fill(burrow);
    burrow.setName("");
    EXPECT_THROW(burrow.write(oStream), InvalidProtocolException);
    burrow.setName(std::string(21, 'b'));
    EXPECT_THROW(burrow.write(oStream), InvalidProtocolException);

    GCAddBat bat;
    fill(bat);
    bat.setName("");
    EXPECT_THROW(bat.write(oStream), InvalidProtocolException);
    bat.setName(std::string(21, 'x'));
    EXPECT_THROW(bat.write(oStream), InvalidProtocolException);

    GCAddWolf wolf;
    fill(wolf);
    wolf.setName("");
    EXPECT_THROW(wolf.write(oStream), InvalidProtocolException);
    wolf.setName(std::string(21, 'w'));
    EXPECT_THROW(wolf.write(oStream), InvalidProtocolException);
}

//////////////////////////////////////////////////////////////////////
// The effect and portal packets scan() sends for a tile, and the
// relic markers sendRelicEffect() puts on a monster corpse.
//////////////////////////////////////////////////////////////////////

void fill(GCAddEffect& p) {
    p.setObjectID(0x93A4B5C6);
    p.setEffectID(0x87C8);
    p.setDuration(0x89CA);
}
ZONE_SCAN_GOLDEN_AND_SIZE(GCAddEffect)

void fill(GCAddEffectToTile& p) {
    p.setObjectID(0x94A5B6C7);
    p.setXY(0x88, 0x99);
    p.setEffectID(0x8ACB);
    p.setDuration(0x8CCD);
}
void expectEqual(const GCAddEffectToTile& a, const GCAddEffectToTile& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getEffectID(), b.getEffectID());
    EXPECT_EQ(a.getDuration(), b.getDuration());
}
ZONE_SCAN_PACKET_TESTS(GCAddEffectToTile)

void fill(GCAddVampirePortal& p) {
    p.setObjectID(0x95A6B7C8);
    p.setOwnerID("GoldScanPortalOwner");
    p.setDuration(0x89CA);
    p.setX(0x8B);
    p.setY(0x9C);
    p.setTargetZoneID(0x8DCE);
    p.setTargetX(0x8F);
    p.setTargetY(0xA0);
    p.setCreateFlag(0xB1);
}
void expectEqual(const GCAddVampirePortal& a, const GCAddVampirePortal& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getOwnerID(), b.getOwnerID());
    EXPECT_EQ(a.getDuration(), b.getDuration());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getTargetZoneID(), b.getTargetZoneID());
    EXPECT_EQ(a.getTargetX(), b.getTargetX());
    EXPECT_EQ(a.getTargetY(), b.getTargetY());
    EXPECT_EQ(a.getCreateFlag(), b.getCreateFlag());
}
ZONE_SCAN_PACKET_TESTS(GCAddVampirePortal)

// The other branch of write(): an ownerless portal emits a zero length
// byte and nothing else for the owner.
TEST(GCAddVampirePortalTest, ownerlessBodyBytesMatchGoldenAndRoundTrips) {
    GCAddVampirePortal packet;
    fill(packet);
    packet.setOwnerID("");

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    expectGolden("GCAddVampirePortal.noowner", kPlainCode, body);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCAddVampirePortal dst;
    roundTrip(packet, dst, kPlainCode);
    expectEqual(packet, dst);
}

//////////////////////////////////////////////////////////////////////
// The removal half, and the packet that reframes a view in place.
//////////////////////////////////////////////////////////////////////

void fill(GCDeleteObject& p) {
    p.setObjectID(0x96A7B8C9);
}
void expectEqual(const GCDeleteObject& a, const GCDeleteObject& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
}
ZONE_SCAN_PACKET_TESTS(GCDeleteObject)

void fill(GCDeleteEffectFromTile& p) {
    p.setObjectID(0x97A8B9CA);
    p.setXY(0x8B, 0x9C);
    p.setEffectID(0x8DCE);
}
void expectEqual(const GCDeleteEffectFromTile& a, const GCDeleteEffectFromTile& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getEffectID(), b.getEffectID());
}
ZONE_SCAN_PACKET_TESTS(GCDeleteEffectFromTile)

void fill(GCFastMove& p) {
    p.setObjectID(0x98A9BACB);
    p.setXY(0x8C, 0x9D, 0xAE, 0xBF);
    p.setSkillType(0x90D1);
}
void expectEqual(const GCFastMove& a, const GCFastMove& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getFromX(), b.getFromX());
    EXPECT_EQ(a.getFromY(), b.getFromY());
    EXPECT_EQ(a.getToX(), b.getToX());
    EXPECT_EQ(a.getToY(), b.getToY());
    EXPECT_EQ(a.getSkillType(), b.getSkillType());
}
ZONE_SCAN_PACKET_TESTS(GCFastMove)

//////////////////////////////////////////////////////////////////////
// Findings.
//////////////////////////////////////////////////////////////////////

// FINDING, stated as a test that fails once it is fixed.
// PCSlayerInfo3::write() wraps its whole body in
// `catch (Throwable&) { cout << ... }`, so neither of its refusals — an
// empty name and a name longer than 20 — ever leaves the function. The
// object id has already been written when the throw happens, so the PC
// record stops after four bytes while GCAddSlayer::getPacketSize(),
// which writePacket() puts on the wire ahead of the body, still counts
// the whole record. PCVampireInfo3 and PCOustersInfo3 let the same
// refusals out.
TEST(GCAddSlayerTest, oversizedSlayerNameIsSwallowedAndUnderflowsTheBody) {
    SlayerAddFixture f;
    fillSlayerAdd(f);
    f.packet.getSlayerInfo().setName(std::string(21, 's'));

    const size_t written = writeBody(f.packet, kPlainCode).size();
    EXPECT_LT(written, (size_t)f.packet.getPacketSize())
        << "PCSlayerInfo3::write() no longer swallows its name refusal — delete this test";
}

// FINDING, stated as a test that fails once it is fixed.
// GCAddEffect::read() consumes a leading BYTE flag that write() no
// longer emits — the `oStream.write((BYTE)48)` is commented out — and
// that getPacketSize() does not count. The server only ever writes this
// packet, so write() is the pinned contract and there is no round trip
// until read() is fixed or removed.
TEST(GCAddEffectTest, readConsumesALeadingFlagByteThatWriteNeverEmits) {
    GCAddEffect src;
    fill(src);

    std::vector<unsigned char> body = writeBody(src, kPlainCode);
    ASSERT_EQ((size_t)src.getPacketSize(), body.size());
    // read() wants one byte more than write() emits, so the image has to
    // be padded before it can be parsed at all.
    body.push_back(0xEF);

    Loopback loopback;
    loopback.setCodes(kPlainCode);
    loopback.out().write(reinterpret_cast<const char*>(&body[0]), (uint)body.size());
    loopback.pump((uint)body.size());

    GCAddEffect dst;
    dst.read(loopback.in());

    EXPECT_NE(src.getObjectID(), dst.getObjectID())
        << "GCAddEffect::read() no longer consumes a flag byte write() does not emit — delete this test";
}

// FINDING, stated as a test that fails once it is fixed.
// PCVampireInfo3::getSize() counts szAlignment; getMaxSize() does not.
// Alignment_t is an int, so every carrier of the record — GCAddVampire
// and GCAddVampireCorpse — budgets four bytes too few, and a name of 17
// characters or more already declares a body larger than the read
// buffer the receiver sizes from the factory max.
TEST(GCAddVampireCorpseTest, vampireInfoMaxSizeOmitsTheAlignmentField) {
    GCAddVampireCorpse packet;
    fill(packet);
    packet.getVampireInfo().setName(std::string(20, 'v'));

    GCAddVampireCorpseFactory factory;
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size())
        << "the declared size still matches the bytes written; only the budget is short";
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "PCVampireInfo3::getMaxSize() now counts the alignment field — delete this test";
}

// FINDING, stated as a test that fails once it is fixed.
// The same omission in PCOustersInfo3::getMaxSize(), affecting
// GCAddOusters and GCAddOustersCorpse.
TEST(GCAddOustersCorpseTest, oustersInfoMaxSizeOmitsTheAlignmentField) {
    GCAddOustersCorpse packet;
    fill(packet);
    packet.getOustersInfo().setName(std::string(20, 'o'));

    GCAddOustersCorpseFactory factory;
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size())
        << "the declared size still matches the bytes written; only the budget is short";
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "PCOustersInfo3::getMaxSize() now counts the alignment field — delete this test";
}

// FINDING, stated as a test that fails once it is fixed.
// PCVampireInfo3 holds the coat type as an ItemType_t, which is a WORD,
// but write() casts it to a BYTE and getSize() budgets one byte for it.
// A coat item type above 255 loses its high byte on the way out and the
// client sees a different coat.
TEST(GCAddVampireCorpseTest, coatTypeIsTruncatedToAByteOnTheWire) {
    GCAddVampireCorpse src;
    fill(src);
    src.getVampireInfo().setCoatType(0x8194);

    GCAddVampireCorpse dst;
    roundTrip(src, dst, kPlainCode);

    EXPECT_EQ(0x94, (int)dst.getVampireInfo().getCoatType())
        << "PCVampireInfo3 no longer truncates the coat type — delete this test";
}

// FINDING, stated as a test that fails once it is fixed.
// Neither GCAddMonster::write() nor GCAddMonsterCorpse::write() bounds
// the monster name; both derive a BYTE length from it and then emit the
// whole string. Past 255 characters the length byte wraps, so the
// declared count and the bytes that follow disagree and the reader
// stops mid-name; the body also outgrows the read buffer the receiver
// sizes from the factory max, which budgets 32 and 128 characters.
TEST(ZoneScanBoundsTest, unboundedMonsterNamesWrapTheLengthByteAndOutgrowTheFactoryMax) {
    GCAddMonster monster;
    fill(monster);
    GCAddMonsterFactory monsterFactory;
    size_t monsterNameLength = monsterFactory.getPacketMaxSize() + 1;
    if (monsterNameLength < 300)
        monsterNameLength = 300;
    monster.setMonsterName(std::string(monsterNameLength, 'm'));
    const std::vector<unsigned char> monsterBody = writeBody(monster, kPlainCode);
    // object id, monster type, then the name length byte.
    EXPECT_EQ((int)(monsterNameLength & 0xFF), (int)monsterBody[szObjectID + szMonsterType])
        << "GCAddMonster::write() now bounds the monster name — delete this half of the test";
    EXPECT_GT(monster.getPacketSize(), monsterFactory.getPacketMaxSize());

    GCAddMonsterCorpse corpse;
    fill(corpse);
    GCAddMonsterCorpseFactory corpseFactory;
    size_t corpseNameLength = corpseFactory.getPacketMaxSize() + 1;
    if (corpseNameLength < 300)
        corpseNameLength = 300;
    corpse.setMonsterName(std::string(corpseNameLength, 'c'));
    const std::vector<unsigned char> corpseBody = writeBody(corpse, kPlainCode);
    EXPECT_EQ((int)(corpseNameLength & 0xFF), (int)corpseBody[szObjectID + szMonsterType])
        << "GCAddMonsterCorpse::write() now bounds the monster name — delete this half of the test";
    EXPECT_GT(corpse.getPacketSize(), corpseFactory.getPacketMaxSize());
}

// FINDING, stated as a test that fails once it is fixed.
// The two optional strings the creature-add packets carry unbounded,
// StoreOutlook's shop sign and PetInfo's pet nickname, are emitted
// whole while their record's getMaxSize() budgets a cap (MAX_SIGN_SIZE
// and 22) that nothing enforces. Each factory max is the sum of those
// record maxima, so a player who names a shop or a pet past the cap
// overruns the read buffer of every client that sees them.
TEST(ZoneScanBoundsTest, unboundedOptionalStringsOutgrowTheirRecordMax) {
    StoreOutlook sign;
    sign.setOpen(1);
    sign.setSign(std::string(MAX_SIGN_SIZE + 40, 'g'));
    EXPECT_GT(sign.getSize(), StoreOutlook::getMaxSize())
        << "StoreOutlook::write() now bounds the shop sign — delete this half of the test";

    PetInfo pet;
    fillPetInfo(pet);
    pet.setNickname(std::string(62, 'p'));
    EXPECT_GT(pet.getSize(), PetInfo::getMaxSize())
        << "PetInfo::write() now bounds the pet nickname — delete this half of the test";
}

// FINDING, stated as a test that fails once it is fixed.
// GCAddVampirePortal::write() emits the owner name whole while its
// factory budgets 20 characters for it.
TEST(ZoneScanBoundsTest, unboundedPortalOwnerOutgrowsTheFactoryMax) {
    GCAddVampirePortal packet;
    fill(packet);
    packet.setOwnerID(std::string(60, 'o'));

    GCAddVampirePortalFactory factory;
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCAddVampirePortal::write() now bounds the owner name — delete this test";
}

} // namespace
