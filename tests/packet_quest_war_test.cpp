//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_quest_war_test.cpp
// Description : Golden byte fixtures, loopback round trips and size
//               pins for the packets a client and a game server
//               exchange around a quest, a war, the flag war, a
//               castle's tax and the portals, waypoints and regen
//               zones a player picks.
//
//               The set is taken from the code that sends them: the
//               handlers under src/server/gameserver/handler
//               (CGSelectQuest, CGFailQuest, CGGQuestAccept,
//               CGGQuestCancel, CGSubmitScore, CGModifyTaxRatio,
//               CGWithdrawTax, CGDonationMoney, CGSelectRegenZone,
//               CGRelicToObject, CGSelectTileEffect, CGSelectWayPoint,
//               CGSelectPortal), GQuestManager.cpp, GQuestStatus.cpp,
//               the five mission/*QuestStatus.cpp classes,
//               PlayerCreature.cpp, LevelWarManager.cpp,
//               CastleInfoManager.cpp, RegenZoneManager.cpp, Zone.cpp,
//               ctf/FlagManager.cpp, ctf/FlagWar.cpp, war/WarSystem.cpp,
//               war/WarSchedule.cpp, war/WarScheduler.cpp,
//               war/GuildWar.cpp, war/RaceWar.cpp, war/SiegeWar.cpp,
//               MasterLairManager.cpp, PacketUtil.cpp, CreatureUtil.cpp
//               and the quest actions that drive the same dialogue
//               (ActionSelectQuest, ActionGiveEventQuest,
//               ActionStartPetQuest, ActionShowWarSchedule,
//               ActionShowDonationDialog, ActionMiniGame,
//               ActionEventMeet). Twenty-seven packets, each with the
//               reason it is here:
//
//               CGSelectQuest    the player picks one quest an NPC
//                                offers, and
//               CGFailQuest      gives up on the one being run.
//               CGGQuestAccept   the player takes a general quest,
//               CGGQuestCancel   and drops it again.
//               CGSubmitScore    the player reports a mini game score.
//
//               CGModifyTaxRatio the castle owner sets the shop tax,
//               CGWithdrawTax    takes the takings out, and
//               CGDonationMoney  a player hands money to an NPC.
//
//               CGSelectRegenZone   the dead player picks where to
//                                come back,
//               CGSelectPortal   picks a portal's destination zone,
//               CGSelectWayPoint picks a waypoint's zone and tile, and
//               CGSelectTileEffect  steps into the tile effect a
//                                vampire portal is.
//               CGRelicToObject  the player puts a relic into a
//                                castle's relic case.
//
//               GCQuestStatus    how far one mission quest has got,
//               GCSelectQuestID  which quests an NPC will hand out,
//               GCMonsterKillQuestInfo  and the goal and time limit of
//                                each kill quest among them.
//               GCGQuestStatusInfo   every general quest a character
//                                holds, sent once as it loads, and
//               GCGQuestStatusModify one of them changing.
//               GCMiniGameScores the mini game's score table.
//
//               GCWarList        the wars now running, one record per
//                                war and three record shapes.
//               GCWarScheduleList    the wars still to come.
//               GCFlagWarStatus  the flag count of each race and the
//                                time left in the flag war.
//               GCNotifyWin      the name of a lottery or event
//                                winner, broadcast to the zone.
//               GCNoticeEvent    the numbered notice every one of
//                                these systems broadcasts, with and
//                                without its parameter.
//
//               GCRegenZoneStatus    which of the eight regen zones
//                                are open.
//               GCEnterVampirePortal a vampire leaving through a
//                                portal, and
//               GCAddHelicopter  the helicopter a waypoint traveller
//                                boards.
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
//               0..5 by tests/packet_encrypter_test.cpp and
//               tests/packet_roundtrip_test.cpp. None of the
//               twenty-seven derives from one of them: every one
//               extends Packet directly. So each golden is recorded at
//               code 0 and its test also asserts the bytes do not vary
//               with the code - adopting the encrypter fails loudly
//               instead of silently voiding the pin.
//
//               Already pinned elsewhere, so the answers these
//               handlers share with other families are not repeated:
//               GCNPCResponse (packet_guild_test.cpp),
//               GCModifyInformation and GCOtherModifyInfo
//               (packet_combat_test.cpp), GCAddEffect, GCRemoveEffect,
//               GCDeleteObject, GCAddVampire, GCAddSlayer and
//               GCAddOusters (packet_zone_scan_test.cpp and
//               packet_movement_test.cpp), GCCannotAdd, GCCreateItem
//               and GCDeleteInventoryItem
//               (packet_inventory_test.cpp), GCUpdateInfo and
//               GCPetInfo (packet_gameserver_handshake_test.cpp),
//               GCPetStashList (packet_store_test.cpp),
//               GCSweeperBonusInfo (packet_skill_test.cpp). The
//               GuildWarInfo record already had a guild-name width pin
//               in packet_roundtrip_test.cpp and the MissionInfo
//               record a string-argument width pin; both are
//               single-aspect and neither had a golden, so both travel
//               here as the contents of a packet and their offsets are
//               pinned with it. GCNotifyWin had a name-width pin in
//               the same file and no golden; it is pinned in full here.
//
//               GCSystemMessage and GCSay are excluded: both are
//               generic in-game text, sent from 138 and 10 server
//               sources across every family, and neither belongs to
//               quests or wars any more than to the rest. Neither is
//               pinned anywhere yet - they belong with the chat set.
//               GCItemNameInfoList is excluded for the same reason:
//               its senders are Zone.cpp's item-name listing and the
//               couple meet path, not a quest or a war.
//
//               Every one of the twenty-seven has a registered factory
//               in tests/ratchet/factory_registrations.txt and at
//               least one sender outside src/Core.
//
//               Each packet gets three pins (QUEST_PACKET_TESTS):
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
//               GCRegenZoneStatus gets the golden and the size pin but
//               not the round trip: it cannot round trip at all, which
//               is the first finding below.
//
//               Extra goldens cover the branches one fixture cannot:
//               the counted lists are written empty (.empty on
//               GCSelectQuestID, GCMonsterKillQuestInfo,
//               GCGQuestStatusInfo, GCMiniGameScores, GCWarList and
//               GCWarScheduleList); the quest record is written with
//               no mission (.nomissions on GCGQuestStatusModify); the
//               score table is written past the ten write() caps it at
//               (.capped on GCMiniGameScores, golden and size only,
//               because the reader cannot give back what write()
//               dropped); the war schedule is written with a guild war
//               whose five challengers and reinforcement are all
//               nameless (.noguildnames); and the notice is written
//               under a code that carries no parameter (.bare on
//               GCNoticeEvent), the other half of the switch its
//               read(), write() and getPacketSize() share.
//
//               Fixture values are distinct per field and >= 128 in
//               every byte the width allows. Five groups cannot follow
//               that rule and say so at the point of use: the winner
//               name, the guild names and the mission string, which are
//               text; the mini game type, the quest status, the mission
//               status and the quest modify type, which are
//               enumerators; the flag war's race index and the war
//               schedule's war type, which select a branch or an array
//               slot; the regen zone statuses, which are eight
//               positions; and CGFailQuest's give-up flag, which is
//               bool.
//
//               Findings. Each is stated as a test that fails once the
//               packet is fixed, except where noted:
//
//               - GCRegenZoneStatus::read appends the eight bytes it
//                 takes off the wire to the eight its constructor
//                 already pushed, so getStatus() keeps returning the
//                 constructor's zeros and write() emits them again: the
//                 packet cannot round trip at all.
//               - GCMiniGameScores::getPacketSize never advances its
//                 iterator, so it counts the first name's length once
//                 per entry. A table whose names differ in length
//                 declares a size it does not send.
//               - GCMiniGameScores derives each name's length byte
//                 from the name with no bound, while its factory max
//                 budgets twenty bytes for one: a longer name outgrows
//                 the read buffer, and a name of 256 wraps the length
//                 byte to zero while write() emits the whole string.
//               - GCNoticeEvent::getCode returns a BYTE from the WORD
//                 it holds and puts on the wire, so the caller cannot
//                 see a code past 255.
//               - GCNoticeEvent::setParameter(WORD, WORD) assigns
//                 makeDWORD of its two halves to the code, not to the
//                 parameter: the code is overwritten with the low half
//                 and the parameter never changes.
//               - GCGQuestStatusInfo derives its record count into a
//                 BYTE it caps nowhere, so the 256th record wraps the
//                 count to zero while write() still emits every record.
//               - GCGQuestStatusInfo, GCSelectQuestID,
//                 GCMonsterKillQuestInfo, GCWarList,
//                 GCWarScheduleList and the ValueList inside
//                 GuildWarInfo and RaceWarInfo all append to the list
//                 the packet already holds instead of replacing it, so
//                 a reused packet grows by one listing per read.
//               - GCWarList's factory max budgets twelve race wars and
//                 twelve guild wars and nothing else: not the count
//                 byte, not the war type byte in front of each record,
//                 and not the level war its own read() builds. Twelve
//                 full guild wars and twelve full race wars already
//                 outgrow it.
//               - GCWarList's count byte is derived from the list and
//                 capped nowhere, so the 256th war wraps it to zero.
//               - GCWarScheduleList derives each guild name's length
//                 byte from the name with no bound and its max budgets
//                 sixteen bytes for one, so a longer name outgrows the
//                 read buffer and a name of 256 wraps the length byte
//                 to zero while write() emits the whole string.
//               - GCWarScheduleList's count byte is capped nowhere
//                 either, and its max budgets twenty entries.
//               - QuestStatusInfo derives its mission count into a
//                 BYTE it caps nowhere, so the 256th mission wraps the
//                 count to zero while write() emits every mission.
//               - ValueList, the join guilds of a guild war and the
//                 castles of a race war, wraps its own count byte at
//                 256 the same way.
//               - GCGQuestStatusModify leaves uninitialised the record
//                 pointer that getPacketSize() and write() dereference,
//                 so a sender that skips setInfo follows an
//                 indeterminate value.
//               - MissionInfo::write prints every mission it emits to
//                 standard output, on the wire path, once per player
//                 the packet is sent to.
//
//               Six findings are recorded here rather than tested,
//               because reaching them is undefined behaviour, has no
//               observable wire effect, or is a leak:
//
//               - GCWarList::read casts the war type byte straight to
//                 the WarType enum before switching on it. The enum
//                 declares three values, so its range is 0..3; any
//                 byte above that is an out-of-range enum load. Every
//                 fixture here writes a real war type for that reason.
//               - GCFlagWarStatus::getFlagCount and setFlagCount index
//                 a three-element array with a Race_t they never bound.
//               - GCGQuestStatusInfo's destructor frees no record: the
//                 loop that did is commented out, so every record a
//                 read() allocated leaks. GCGQuestStatusModify::read
//                 allocates a record on every call without freeing the
//                 one the packet held, and its destructor drops that
//                 one too.
//               - GCSelectQuestID and GCMonsterKillQuestInfo do refuse
//                 a list past 255, through Assert(), which appends to
//                 assertion_failed.log in the working directory before
//                 it throws.
//               - GCAddHelicopter's code getter is named setCode and
//                 overloads the setter.
//               - GCNoticeEvent's read() and write() take a code they
//                 never compare against NOTICE_EVENT_MAX, so a peer
//                 can announce a notice no branch of the client draws.
//
//               Eighteen of the twenty-seven leave at least one
//               member the default constructor never sets, so a packet
//               sent without every setter called puts indeterminate
//               bytes on the wire. The poisoned-storage pin at the end
//               of the file asserts today's split. GCNotifyWin is in
//               neither list because its one text field is required
//               non-empty, so a default-constructed body is refused
//               rather than written; and GCGQuestStatusModify is in
//               neither because writing one over poisoned storage
//               would follow the indeterminate record pointer, so it
//               gets a pin on the pointer itself instead.
//
//////////////////////////////////////////////////////////////////////

#include <cstring>
#include <list>
#include <new>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "CGDonationMoney.h"
#include "CGFailQuest.h"
#include "CGGQuestAccept.h"
#include "CGGQuestCancel.h"
#include "CGModifyTaxRatio.h"
#include "CGRelicToObject.h"
#include "CGSelectPortal.h"
#include "CGSelectQuest.h"
#include "CGSelectRegenZone.h"
#include "CGSelectTileEffect.h"
#include "CGSelectWayPoint.h"
#include "CGSubmitScore.h"
#include "CGWithdrawTax.h"
#include "Exception.h"
#include "GCAddHelicopter.h"
#include "GCEnterVampirePortal.h"
#include "GCFlagWarStatus.h"
#include "GCGQuestStatusInfo.h"
#include "GCGQuestStatusModify.h"
#include "GCMiniGameScores.h"
#include "GCMonsterKillQuestInfo.h"
#include "GCNoticeEvent.h"
#include "GCNotifyWin.h"
#include "GCQuestStatus.h"
#include "GCRegenZoneStatus.h"
#include "GCSelectQuestID.h"
#include "GCWarList.h"
#include "GCWarScheduleList.h"
#include "GuildWarInfo.h"
#include "LevelWarInfo.h"
#include "QuestStatusInfo.h"
#include "RaceWarInfo.h"
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
// comparing a list consumes it.
//////////////////////////////////////////////////////////////////////

#define QUEST_PACKET_GOLDEN_AND_SIZE(Name)                                                           \
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

#define QUEST_PACKET_TESTS(Name)                  \
    TEST(Name##Test, roundTripsThroughLoopback) { \
        Name src;                                 \
        fill(src);                                \
        Name dst;                                 \
        roundTrip(src, dst, kPlainCode);          \
        expectEqual(src, dst);                    \
    }                                             \
    QUEST_PACKET_GOLDEN_AND_SIZE(Name)

// A second fixture for a packet whose write() has a branch the canonical
// one does not take.
#define QUEST_PACKET_VARIANT(Name, Variant, fillVariant)                       \
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

// The same, for a branch the reader cannot give back: write() drops part
// of what the packet holds, so only the bytes and the size are pinned.
#define QUEST_PACKET_WRITE_ONLY_VARIANT(Name, Variant, fillVariant)            \
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
// Picking, giving up and running a quest.
//////////////////////////////////////////////////////////////////////

void fill(CGSelectQuest& packet) {
    packet.setQuestID(0x81A2B3C4);
    packet.setNPCObjectID(0x85A6B7C8);
}

void expectEqual(CGSelectQuest& a, CGSelectQuest& b) {
    EXPECT_EQ(a.getQuestID(), b.getQuestID());
    EXPECT_EQ(a.getNPCObjectID(), b.getNPCObjectID());
}

QUEST_PACKET_TESTS(CGSelectQuest)

// The give-up flag is bool on the setter and a BYTE on the wire, so it
// cannot follow the >= 128 rule.
void fill(CGFailQuest& packet) {
    packet.setFail(true);
}

void expectEqual(CGFailQuest& a, CGFailQuest& b) {
    EXPECT_EQ((int)a.isFail(), (int)b.isFail());
}

QUEST_PACKET_TESTS(CGFailQuest)

void fill(CGGQuestAccept& packet) {
    packet.setQuestID(0x89AABBCC);
}

void expectEqual(CGGQuestAccept& a, CGGQuestAccept& b) {
    EXPECT_EQ(a.getQuestID(), b.getQuestID());
}

QUEST_PACKET_TESTS(CGGQuestAccept)

void fill(CGGQuestCancel& packet) {
    packet.setQuestID(0x8DAEBFC0);
}

void expectEqual(CGGQuestCancel& a, CGGQuestCancel& b) {
    EXPECT_EQ(a.getQuestID(), b.getQuestID());
}

QUEST_PACKET_TESTS(CGGQuestCancel)

// The game type is a plain BYTE on both sides of this packet - only the
// handler's switch gives it meaning - so it follows the >= 128 rule.
void fill(CGSubmitScore& packet) {
    packet.setGameType(0x91);
    packet.setLevel(0x92);
    packet.setScore(0x93A4);
}

void expectEqual(CGSubmitScore& a, CGSubmitScore& b) {
    EXPECT_EQ((int)a.getGameType(), (int)b.getGameType());
    EXPECT_EQ((int)a.getLevel(), (int)b.getLevel());
    EXPECT_EQ((int)a.getScore(), (int)b.getScore());
}

QUEST_PACKET_TESTS(CGSubmitScore)

//////////////////////////////////////////////////////////////////////
// A castle's tax, and the money handed over an NPC's counter.
//////////////////////////////////////////////////////////////////////

void fill(CGModifyTaxRatio& packet) {
    packet.setRatio(0x95A6B7C8);
}

void expectEqual(CGModifyTaxRatio& a, CGModifyTaxRatio& b) {
    EXPECT_EQ(a.getRatio(), b.getRatio());
}

QUEST_PACKET_TESTS(CGModifyTaxRatio)

void fill(CGWithdrawTax& packet) {
    packet.setGold(0x99AABBCC);
}

void expectEqual(CGWithdrawTax& a, CGWithdrawTax& b) {
    EXPECT_EQ(a.getGold(), b.getGold());
}

QUEST_PACKET_TESTS(CGWithdrawTax)

void fill(CGDonationMoney& packet) {
    packet.setGold(0x9DAEBFC0);
    packet.setDonationType(0xA1);
}

void expectEqual(CGDonationMoney& a, CGDonationMoney& b) {
    EXPECT_EQ(a.getGold(), b.getGold());
    EXPECT_EQ((int)a.getDonationType(), (int)b.getDonationType());
}

QUEST_PACKET_TESTS(CGDonationMoney)

//////////////////////////////////////////////////////////////////////
// The places a player picks: a regen zone, a portal, a waypoint, the
// tile effect a vampire portal is, and a castle's relic case.
//////////////////////////////////////////////////////////////////////

void fill(CGSelectRegenZone& packet) {
    packet.setRegenZoneID(0xA2);
}

void expectEqual(CGSelectRegenZone& a, CGSelectRegenZone& b) {
    EXPECT_EQ((int)a.getRegenZoneID(), (int)b.getRegenZoneID());
}

QUEST_PACKET_TESTS(CGSelectRegenZone)

void fill(CGSelectPortal& packet) {
    packet.setZoneID(0xA3B4);
}

void expectEqual(CGSelectPortal& a, CGSelectPortal& b) {
    EXPECT_EQ(a.getZoneID(), b.getZoneID());
}

QUEST_PACKET_TESTS(CGSelectPortal)

void fill(CGSelectWayPoint& packet) {
    packet.setZoneID(0xA5B6);
    packet.setX(0xA7);
    packet.setY(0xA8);
}

void expectEqual(CGSelectWayPoint& a, CGSelectWayPoint& b) {
    EXPECT_EQ((int)a.getZoneID(), (int)b.getZoneID());
    EXPECT_EQ((int)a.getX(), (int)b.getX());
    EXPECT_EQ((int)a.getY(), (int)b.getY());
}

QUEST_PACKET_TESTS(CGSelectWayPoint)

void fill(CGSelectTileEffect& packet) {
    packet.setEffectObjectID(0xA9BACBDC);
}

void expectEqual(CGSelectTileEffect& a, CGSelectTileEffect& b) {
    EXPECT_EQ(a.getEffectObjectID(), b.getEffectObjectID());
}

QUEST_PACKET_TESTS(CGSelectTileEffect)

void fill(CGRelicToObject& packet) {
    packet.setItemObjectID(0xADBECFD0);
    packet.setObjectID(0xB1C2D3E4);
    packet.setX(0xB5);
    packet.setY(0xB6);
}

void expectEqual(CGRelicToObject& a, CGRelicToObject& b) {
    EXPECT_EQ(a.getItemObjectID(), b.getItemObjectID());
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ((int)a.getX(), (int)b.getX());
    EXPECT_EQ((int)a.getY(), (int)b.getY());
}

QUEST_PACKET_TESTS(CGRelicToObject)

//////////////////////////////////////////////////////////////////////
// What the server answers about a quest.
//////////////////////////////////////////////////////////////////////

void fill(GCQuestStatus& packet) {
    packet.setQuestID(0xB7C8);
    packet.setCurrentNum(0xB9CA);
    packet.setRemainTime(0xBBCCDDEE);
}

void expectEqual(GCQuestStatus& a, GCQuestStatus& b) {
    EXPECT_EQ((int)a.getQuestID(), (int)b.getQuestID());
    EXPECT_EQ((int)a.getCurrentNum(), (int)b.getCurrentNum());
    EXPECT_EQ(a.getRemainTime(), b.getRemainTime());
}

QUEST_PACKET_TESTS(GCQuestStatus)

// GCSelectQuestID takes its list only through the iterator constructor,
// which is how ActionSelectQuest builds one, so the fixture assigns a
// packet built that way.
void fill(GCSelectQuestID& packet) {
    const QuestID_t ids[] = {0x81A2B3C4, 0x85A6B7C8, 0x89AABBCC};
    packet = GCSelectQuestID(ids, ids + 3);
}

void fillNoQuests(GCSelectQuestID&) {}

void expectEqual(GCSelectQuestID& a, GCSelectQuestID& b) {
    while (!a.empty()) {
        ASSERT_FALSE(b.empty());
        EXPECT_EQ(a.popQuestID(), b.popQuestID());
    }
    EXPECT_TRUE(b.empty());
}

QUEST_PACKET_TESTS(GCSelectQuestID)
QUEST_PACKET_VARIANT(GCSelectQuestID, empty, fillNoQuests)

GCMonsterKillQuestInfo::QuestInfo* makeKillQuest(QuestID_t questID, SpriteType_t sType, WORD goal, DWORD timeLimit) {
    GCMonsterKillQuestInfo::QuestInfo* pInfo = new GCMonsterKillQuestInfo::QuestInfo;
    pInfo->questID = questID;
    pInfo->sType = sType;
    pInfo->goal = goal;
    pInfo->timeLimit = timeLimit;
    return pInfo;
}

void fill(GCMonsterKillQuestInfo& packet) {
    packet.addQuestInfo(makeKillQuest(0x8DAEBFC0, 0x91C2, 0x93D4, 0x95E6F708));
    packet.addQuestInfo(makeKillQuest(0x99AABBCC, 0x9DEE, 0xA1F2, 0xA3B4C5D6));
}

void fillNoKillQuests(GCMonsterKillQuestInfo&) {}

void expectEqual(GCMonsterKillQuestInfo& a, GCMonsterKillQuestInfo& b) {
    while (!a.empty()) {
        ASSERT_FALSE(b.empty());
        GCMonsterKillQuestInfo::QuestInfo* pLeft = a.popQuestInfo();
        GCMonsterKillQuestInfo::QuestInfo* pRight = b.popQuestInfo();
        EXPECT_EQ(pLeft->questID, pRight->questID);
        EXPECT_EQ((int)pLeft->sType, (int)pRight->sType);
        EXPECT_EQ((int)pLeft->goal, (int)pRight->goal);
        EXPECT_EQ(pLeft->timeLimit, pRight->timeLimit);
        delete pLeft;
        delete pRight;
    }
    EXPECT_TRUE(b.empty());
}

QUEST_PACKET_TESTS(GCMonsterKillQuestInfo)
QUEST_PACKET_VARIANT(GCMonsterKillQuestInfo, empty, fillNoKillQuests)

//////////////////////////////////////////////////////////////////////
// The general quest listing and its record.
//
// QuestStatusInfo keeps its status and its mission list protected, and
// the gameserver's GQuestStatus is the only class that fills them in,
// so the fixtures build the record through a subclass of their own.
// Reading gives back plain QuestStatusInfo objects, which nothing can
// look inside, so the comparisons go through what the record writes.
//////////////////////////////////////////////////////////////////////

class FixtureQuestStatus : public QuestStatusInfo {
public:
    FixtureQuestStatus(DWORD questID, BYTE status) : QuestStatusInfo(questID) {
        m_Status = status;
    }
    ~FixtureQuestStatus() {
        for (list<MissionInfo*>::iterator itr = m_Missions.begin(); itr != m_Missions.end(); ++itr)
            delete *itr;
        m_Missions.clear();
    }
    void addMission(MissionInfo* pMission) {
        m_Missions.push_back(pMission);
    }
};

// The condition index and both status bytes are enumerators, so they
// cannot follow the >= 128 rule; the string argument is text.
MissionInfo* makeMission(BYTE condition, WORD index, BYTE status, const std::string& strArg, DWORD numArg) {
    MissionInfo* pMission = new MissionInfo;
    pMission->m_Condition = condition;
    pMission->m_Index = index;
    pMission->m_Status = status;
    pMission->m_StrArg = strArg;
    pMission->m_NumArg = numArg;
    return pMission;
}

FixtureQuestStatus* makeQuestStatus(DWORD questID, BYTE status, int missions) {
    FixtureQuestStatus* pInfo = new FixtureQuestStatus(questID, status);
    for (int i = 0; i < missions; i++)
        pInfo->addMission(makeMission((BYTE)(MissionInfo::HIDE + i), (WORD)(0x81A2 + i),
                                      (BYTE)(MissionInfo::CURRENT + i), i == 0 ? "Bring the sealed relic" : "",
                                      (DWORD)(0x85A6B7C8 + i)));
    return pInfo;
}

void fill(GCGQuestStatusInfo& packet) {
    packet.getInfos().push_back(makeQuestStatus(0x89AABBCC, QuestStatusInfo::DOING, 2));
    packet.getInfos().push_back(makeQuestStatus(0x8DAEBFC0, QuestStatusInfo::COMPLETE, 0));
}

void fillNoQuestStatus(GCGQuestStatusInfo&) {}

// The read side gives back plain QuestStatusInfo records, whose fields
// are protected and have no getter, so the records are compared by what
// they put back on the wire.
void expectEqual(GCGQuestStatusInfo& a, GCGQuestStatusInfo& b) {
    ASSERT_EQ(a.getInfos().size(), b.getInfos().size());
    EXPECT_EQ(writeBody(a, kPlainCode), writeBody(b, kPlainCode));
}

QUEST_PACKET_TESTS(GCGQuestStatusInfo)
QUEST_PACKET_VARIANT(GCGQuestStatusInfo, empty, fillNoQuestStatus)

// The modify type is an enumerator, so it cannot follow the >= 128 rule.
// The record is a function-local static because the packet neither owns
// nor frees the one a sender hands it.
void fill(GCGQuestStatusModify& packet) {
    static FixtureQuestStatus* pInfo = makeQuestStatus(0x91C2D3E4, QuestStatusInfo::SUCCESS, 2);
    packet.setType(GCGQuestStatusModify::SUCCESS);
    packet.setInfo(pInfo);
}

void fillNoMissions(GCGQuestStatusModify& packet) {
    static FixtureQuestStatus* pInfo = makeQuestStatus(0x95E6F708, QuestStatusInfo::CAN_ACCEPT, 0);
    packet.setType(GCGQuestStatusModify::CURRENT);
    packet.setInfo(pInfo);
}

void expectEqual(GCGQuestStatusModify& a, GCGQuestStatusModify& b) {
    EXPECT_EQ((int)a.getType(), (int)b.getType());
    ASSERT_TRUE(b.getInfo() != NULL);
    EXPECT_EQ((int)a.getInfo()->getSize(), (int)b.getInfo()->getSize());
    EXPECT_EQ(writeBody(a, kPlainCode), writeBody(b, kPlainCode));
}

QUEST_PACKET_TESTS(GCGQuestStatusModify)
QUEST_PACKET_VARIANT(GCGQuestStatusModify, nomissions, fillNoMissions)

//////////////////////////////////////////////////////////////////////
// The mini game's score table.
//////////////////////////////////////////////////////////////////////

// The game type is an enumerator on the setter. The names are text and
// are all the same length: getPacketSize() counts the first name's
// length once per entry, so any other shape declares a size write()
// does not send - the finding below states that on its own.
void fill(GCMiniGameScores& packet) {
    packet.setGameType(GAME_ARROW);
    packet.setLevel(0x99);
    packet.addScore("Ashen Duelist", 0x9AAB);
    packet.addScore("Gloom Sapper.", 0x9CAD);
    packet.addScore("Ember Warden.", 0x9EAF);
}

void fillNoScores(GCMiniGameScores& packet) {
    packet.setGameType(GAME_MINE);
    packet.setLevel(0xA0);
}

// Twelve entries, two past the ten write() emits.
void fillCappedScores(GCMiniGameScores& packet) {
    packet.setGameType(GAME_NEMO);
    packet.setLevel(0xA1);
    for (int i = 0; i < 12; i++)
        packet.addScore(std::string("Rank runner ") + (char)('A' + i), (WORD)(0xA2B3 + i));
}

void expectEqual(GCMiniGameScores& a, GCMiniGameScores& b) {
    EXPECT_EQ((int)a.getGameType(), (int)b.getGameType());
    EXPECT_EQ((int)a.getLevel(), (int)b.getLevel());
    ASSERT_EQ(a.getSize(), b.getSize());
    while (a.getSize() != 0) {
        const std::pair<std::string, WORD> left = a.popScore();
        const std::pair<std::string, WORD> right = b.popScore();
        EXPECT_EQ(left.first, right.first);
        EXPECT_EQ((int)left.second, (int)right.second);
    }
}

QUEST_PACKET_TESTS(GCMiniGameScores)
QUEST_PACKET_VARIANT(GCMiniGameScores, empty, fillNoScores)
QUEST_PACKET_WRITE_ONLY_VARIANT(GCMiniGameScores, capped, fillCappedScores)

//////////////////////////////////////////////////////////////////////
// The wars: the ones running, the ones scheduled, and the flag war.
//////////////////////////////////////////////////////////////////////

// The guild names are text and stop at the 40 and 30 the record's max
// budgets.
GuildWarInfo* makeGuildWar() {
    GuildWarInfo* pInfo = new GuildWarInfo;
    pInfo->setRemainTime(0xA5B6C7D8);
    pInfo->setStartTime(0xA9BACBDC);
    pInfo->setCastleID(0xADBE);
    pInfo->setAttackGuildName("Crimson Vanguard");
    pInfo->setDefenseGuildName("Ashen Bulwark");
    pInfo->addJoinGuild(0xB1C2);
    pInfo->addJoinGuild(0xB3C4);
    return pInfo;
}

RaceWarInfo* makeRaceWar() {
    RaceWarInfo* pInfo = new RaceWarInfo;
    pInfo->setRemainTime(0xB5C6D7E8);
    pInfo->setStartTime(0xB9CADBEC);
    pInfo->addCastleID(0xBDCE);
    pInfo->addCastleID(0xBFD0);
    return pInfo;
}

LevelWarInfo* makeLevelWar() {
    LevelWarInfo* pInfo = new LevelWarInfo;
    pInfo->setRemainTime(0xC1D2E3F4);
    pInfo->setStartTime(0xC5D6E7F8);
    pInfo->setLevel(0x79C8D9EA);
    return pInfo;
}

void fill(GCWarList& packet) {
    packet.addWarInfo(makeGuildWar());
    packet.addWarInfo(makeRaceWar());
    packet.addWarInfo(makeLevelWar());
}

void fillNoWars(GCWarList&) {}

void expectEqual(GCWarList& a, GCWarList& b) {
    ASSERT_EQ(a.getSize(), b.getSize());
    EXPECT_EQ(writeBody(a, kPlainCode), writeBody(b, kPlainCode));

    while (!a.isEmpty()) {
        WarInfo* pLeft = a.popWarInfo();
        WarInfo* pRight = b.popWarInfo();
        ASSERT_TRUE(pRight != NULL);
        EXPECT_EQ((int)pLeft->getWarType(), (int)pRight->getWarType());
        EXPECT_EQ(pLeft->getRemainTime(), pRight->getRemainTime());
        EXPECT_EQ(pLeft->getStartTime(), pRight->getStartTime());

        GuildWarInfo* pLeftGuild = dynamic_cast<GuildWarInfo*>(pLeft);
        if (pLeftGuild != NULL) {
            GuildWarInfo* pRightGuild = dynamic_cast<GuildWarInfo*>(pRight);
            ASSERT_TRUE(pRightGuild != NULL);
            EXPECT_EQ((int)pLeftGuild->getCastleID(), (int)pRightGuild->getCastleID());
            EXPECT_EQ(pLeftGuild->getAttackGuildName(), pRightGuild->getAttackGuildName());
            EXPECT_EQ(pLeftGuild->getDefenseGuildName(), pRightGuild->getDefenseGuildName());
            ASSERT_EQ(pLeftGuild->getJoinGuilds().getSize(), pRightGuild->getJoinGuilds().getSize());
            while (!pLeftGuild->getJoinGuilds().isEmpty())
                EXPECT_EQ((int)pLeftGuild->getJoinGuilds().popValue(), (int)pRightGuild->getJoinGuilds().popValue());
        }

        RaceWarInfo* pLeftRace = dynamic_cast<RaceWarInfo*>(pLeft);
        if (pLeftRace != NULL) {
            RaceWarInfo* pRightRace = dynamic_cast<RaceWarInfo*>(pRight);
            ASSERT_TRUE(pRightRace != NULL);
            ASSERT_EQ(pLeftRace->getCastleIDs().getSize(), pRightRace->getCastleIDs().getSize());
            while (!pLeftRace->getCastleIDs().isEmpty())
                EXPECT_EQ((int)pLeftRace->getCastleIDs().popValue(), (int)pRightRace->getCastleIDs().popValue());
        }

        LevelWarInfo* pLeftLevel = dynamic_cast<LevelWarInfo*>(pLeft);
        if (pLeftLevel != NULL) {
            LevelWarInfo* pRightLevel = dynamic_cast<LevelWarInfo*>(pRight);
            ASSERT_TRUE(pRightLevel != NULL);
            EXPECT_EQ(pLeftLevel->getLevel(), pRightLevel->getLevel());
        }

        delete pLeft;
        delete pRight;
    }
}

QUEST_PACKET_TESTS(GCWarList)
QUEST_PACKET_VARIANT(GCWarList, empty, fillNoWars)

// The war type selects the branch write() takes: 0 carries the five
// challengers and the reinforcement, anything else carries nothing past
// the hour. The guild names are text and the date fields are a real
// date, so neither follows the >= 128 rule.
WarScheduleInfo* makeGuildSchedule(bool named) {
    WarScheduleInfo* pInfo = new WarScheduleInfo;
    pInfo->warType = 0;
    pInfo->year = 0x07E6;
    pInfo->month = 0x09;
    pInfo->day = 0x0A;
    pInfo->hour = 0x14;
    for (int i = 0; i < 5; i++) {
        pInfo->challengerGuildID[i] = (GuildID_t)(0x81A2 + i);
        pInfo->challengerGuildName[i] = named ? std::string("Challenger ") + (char)('A' + i) : std::string();
    }
    pInfo->reinforceGuildID = 0x91C2;
    pInfo->reinforceGuildName = named ? "Reinforcing Host" : "";
    return pInfo;
}

WarScheduleInfo* makeRaceSchedule() {
    WarScheduleInfo* pInfo = new WarScheduleInfo;
    pInfo->warType = 1;
    pInfo->year = 0x07E7;
    pInfo->month = 0x0B;
    pInfo->day = 0x1D;
    pInfo->hour = 0x15;
    // The five challenger slots and the reinforcement are not written
    // for this war type and are not read back either, so they are left
    // as they are.
    for (int i = 0; i < 5; i++)
        pInfo->challengerGuildID[i] = 0;
    pInfo->reinforceGuildID = 0;
    return pInfo;
}

void fill(GCWarScheduleList& packet) {
    packet.addWarScheduleInfo(makeGuildSchedule(true));
    packet.addWarScheduleInfo(makeRaceSchedule());
}

void fillNoSchedules(GCWarScheduleList&) {}

void fillNoGuildNames(GCWarScheduleList& packet) {
    packet.addWarScheduleInfo(makeGuildSchedule(false));
}

void expectEqual(GCWarScheduleList& a, GCWarScheduleList& b) {
    for (WarScheduleInfo* pLeft = a.popWarScheduleInfo(); pLeft != NULL; pLeft = a.popWarScheduleInfo()) {
        WarScheduleInfo* pRight = b.popWarScheduleInfo();
        ASSERT_TRUE(pRight != NULL);
        EXPECT_EQ((int)pLeft->warType, (int)pRight->warType);
        EXPECT_EQ((int)pLeft->year, (int)pRight->year);
        EXPECT_EQ((int)pLeft->month, (int)pRight->month);
        EXPECT_EQ((int)pLeft->day, (int)pRight->day);
        EXPECT_EQ((int)pLeft->hour, (int)pRight->hour);
        if (pLeft->warType == 0) {
            for (int i = 0; i < 5; i++) {
                EXPECT_EQ((int)pLeft->challengerGuildID[i], (int)pRight->challengerGuildID[i]);
                EXPECT_EQ(pLeft->challengerGuildName[i], pRight->challengerGuildName[i]);
            }
            EXPECT_EQ((int)pLeft->reinforceGuildID, (int)pRight->reinforceGuildID);
            EXPECT_EQ(pLeft->reinforceGuildName, pRight->reinforceGuildName);
        }
        delete pLeft;
        delete pRight;
    }
    EXPECT_TRUE(b.popWarScheduleInfo() == NULL);
}

QUEST_PACKET_TESTS(GCWarScheduleList)
QUEST_PACKET_VARIANT(GCWarScheduleList, empty, fillNoSchedules)
QUEST_PACKET_VARIANT(GCWarScheduleList, noguildnames, fillNoGuildNames)

// The race index picks one of three array slots, so it cannot follow the
// >= 128 rule.
void fill(GCFlagWarStatus& packet) {
    packet.setTimeRemain(0x81A2);
    packet.setFlagCount(0, 0x83);
    packet.setFlagCount(1, 0x84);
    packet.setFlagCount(2, 0x85);
}

void expectEqual(GCFlagWarStatus& a, GCFlagWarStatus& b) {
    EXPECT_EQ((int)a.getTimeRemain(), (int)b.getTimeRemain());
    for (Race_t race = 0; race < 3; race++)
        EXPECT_EQ((int)a.getFlagCount(race), (int)b.getFlagCount(race)) << "flag count " << (int)race;
}

QUEST_PACKET_TESTS(GCFlagWarStatus)

//////////////////////////////////////////////////////////////////////
// The notices these systems broadcast.
//////////////////////////////////////////////////////////////////////

// The code selects the branch every one of read(), write() and
// getPacketSize() takes, so it cannot follow the >= 128 rule.
void fill(GCNoticeEvent& packet) {
    packet.setCode(NOTICE_EVENT_WAR_OVER);
    packet.setParameter(0x86A7B8C9);
}

void fillBareNotice(GCNoticeEvent& packet) {
    packet.setCode(NOTICE_EVENT_MASTER_COMBAT_END);
    packet.setParameter(0x8ACBDCED);
}

void expectEqual(GCNoticeEvent& a, GCNoticeEvent& b) {
    EXPECT_EQ((int)a.getCode(), (int)b.getCode());
    EXPECT_EQ(a.getParameter(), b.getParameter());
}

QUEST_PACKET_TESTS(GCNoticeEvent)

// The bare branch writes no parameter, so the reader keeps the one its
// constructor set rather than the sender's: only the bytes and the size
// are pinned.
QUEST_PACKET_WRITE_ONLY_VARIANT(GCNoticeEvent, bare, fillBareNotice)

// The winner's name is text and stops at what its length byte carries.
void fill(GCNotifyWin& packet) {
    packet.setGiftID(0x8EAFC0D1);
    packet.setName("Crimson Lantern Bearer");
}

void expectEqual(GCNotifyWin& a, GCNotifyWin& b) {
    EXPECT_EQ(a.getGiftID(), b.getGiftID());
    EXPECT_EQ(a.getName(), b.getName());
}

QUEST_PACKET_TESTS(GCNotifyWin)

//////////////////////////////////////////////////////////////////////
// The regen zones, the portal a vampire leaves through and the
// helicopter a waypoint traveller boards.
//////////////////////////////////////////////////////////////////////

// The eight statuses are positions, so each carries its slot rather than
// a value >= 128.
void fill(GCRegenZoneStatus& packet) {
    for (uint i = 0; i < 8; i++)
        packet.setStatus(i, (BYTE)(0x81 + i));
}

// GCRegenZoneStatus gets the golden and the size pin only: it cannot
// round trip, which the first finding below states.
QUEST_PACKET_GOLDEN_AND_SIZE(GCRegenZoneStatus)

void fill(GCEnterVampirePortal& packet) {
    packet.setObjectID(0x92B3C4D5);
    packet.setX(0x96);
    packet.setY(0x97);
}

void expectEqual(GCEnterVampirePortal& a, GCEnterVampirePortal& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ((int)a.getX(), (int)b.getX());
    EXPECT_EQ((int)a.getY(), (int)b.getY());
}

QUEST_PACKET_TESTS(GCEnterVampirePortal)

void fill(GCAddHelicopter& packet) {
    packet.setObjectID(0x98A9BACB);
    packet.setCode(0x9C);
}

void expectEqual(GCAddHelicopter& a, GCAddHelicopter& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    // The code getter is named setCode and overloads the setter.
    EXPECT_EQ((int)a.setCode(), (int)b.setCode());
}

QUEST_PACKET_TESTS(GCAddHelicopter)

//////////////////////////////////////////////////////////////////////
// Write/read disagreements, each stated as a test that fails once the
// packet is fixed.
//////////////////////////////////////////////////////////////////////

// FINDING, stated as a test that fails once it is fixed.
// GCRegenZoneStatus::read appends the eight bytes it takes off the wire
// to the eight its constructor already pushed, so getStatus() keeps
// returning the constructor's zeros and write() emits them again. The
// packet cannot round trip at all.
TEST(GCRegenZoneStatusTest, readAppendsBehindTheEightTheConstructorAlreadyPushed) {
    GCRegenZoneStatus src;
    fill(src);

    GCRegenZoneStatus dst;
    roundTrip(src, dst, kPlainCode);

    for (uint i = 0; i < 8; i++)
        EXPECT_EQ(0, (int)dst.getStatus(i))
            << "GCRegenZoneStatus::read() now replaces the statuses - drop this test and pin the round trip";

    EXPECT_NE(writeBody(src, kPlainCode), writeBody(dst, kPlainCode))
        << "GCRegenZoneStatus now round trips - pin it with QUEST_PACKET_TESTS instead";
}

// FINDING, stated as a test that fails once it is fixed.
// GCMiniGameScores::getPacketSize never advances its iterator: it counts
// the first name's length once per entry. A table whose names differ in
// length declares a size it does not send, and writePacket() puts that
// size on the wire before write() runs.
TEST(GCMiniGameScoresTest, theSizeCountsTheFirstNameOncePerEntry) {
    GCMiniGameScores packet;
    packet.setGameType(GAME_PUSH);
    packet.setLevel(0x81);
    packet.addScore("Ash", 0x82A3);
    packet.addScore("Gloomhollow Sapper", 0x84A5);

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_NE((size_t)packet.getPacketSize(), body.size())
        << "GCMiniGameScores::getPacketSize() now walks the table - drop this test and pin the agreement";
    EXPECT_EQ((size_t)(szBYTE * 3 + (szBYTE + 3 + szWORD) * 2), (size_t)packet.getPacketSize());
    EXPECT_EQ((size_t)(szBYTE * 3 + (szBYTE + 3 + szWORD) + (szBYTE + 18 + szWORD)), body.size());
}

// FINDING, stated as a test that fails once it is fixed.
// GCMiniGameScores derives each name's length byte from the name with no
// bound, while its factory max budgets twenty bytes for one name.
TEST(GCMiniGameScoresTest, theNameOutgrowsWhatTheFactoryMaxBudgets) {
    GCMiniGameScores packet;
    packet.setGameType(GAME_MINE);
    packet.setLevel(0x81);
    for (int i = 0; i < 10; i++)
        packet.addScore(std::string(200, 'n'), (WORD)(0x82A3 + i));

    GCMiniGameScoresFactory factory;
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCMiniGameScores: the name is now held to what the factory max budgets";

    GCMiniGameScores wrapped;
    wrapped.setGameType(GAME_MINE);
    wrapped.setLevel(0x81);
    wrapped.addScore(std::string(256, 'n'), 0x84A5);

    const std::vector<unsigned char> body = writeBody(wrapped, kPlainCode);
    EXPECT_EQ(0, (int)body[3]) << "GCMiniGameScores: the name length is now bounded - drop this test";
    EXPECT_EQ((size_t)(szBYTE * 3 + szBYTE + 256 + szWORD), body.size());
}

// FINDING, stated as a test that fails once it is fixed.
// GCNoticeEvent::getCode returns a BYTE from the WORD it holds and puts
// on the wire, so a caller cannot see a code past 255.
TEST(GCNoticeEventTest, theCodeGetterReturnsAByteOfTheWordItSends) {
    GCNoticeEvent packet;
    packet.setCode(0x81A2);

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)szWORD, body.size());
    EXPECT_NE(0x81A2, (int)packet.getCode())
        << "GCNoticeEvent::getCode() now returns the WORD it holds - drop this test";
    EXPECT_EQ(0xA2, (int)packet.getCode());
}

// FINDING, stated as a test that fails once it is fixed.
// GCNoticeEvent::setParameter(WORD, WORD) assigns makeDWORD of its two
// halves to the code, not to the parameter, so it overwrites the notice
// with the low half and leaves the parameter alone.
TEST(GCNoticeEventTest, theTwoHalfParameterSetterWritesTheCodeInstead) {
    GCNoticeEvent packet;
    packet.setCode(NOTICE_EVENT_WAR_OVER);
    packet.setParameter(0x81A2B3C4);
    packet.setParameter((WORD)0x85A6, (WORD)0x87B8);

    EXPECT_EQ(0x81A2B3C4u, packet.getParameter())
        << "GCNoticeEvent::setParameter(WORD, WORD) now sets the parameter - drop this test";
    EXPECT_EQ(0xB8, (int)packet.getCode());
}

// FINDING, stated as a test that fails once it is fixed.
// GCGQuestStatusInfo derives its record count into a BYTE it caps
// nowhere, so the 256th record wraps the count to zero while write()
// still emits every record.
TEST(GCGQuestStatusInfoTest, theRecordCountWrapsAtTwoHundredAndFiftySix) {
    // QuestStatusInfo has no virtual destructor, so the records are freed
    // through the type they were built as.
    std::vector<FixtureQuestStatus*> owned;
    GCGQuestStatusInfo packet;
    for (int i = 0; i < 256; i++) {
        FixtureQuestStatus* pInfo = makeQuestStatus((DWORD)(0x81A2B3C4 + i), QuestStatusInfo::DOING, 0);
        owned.push_back(pInfo);
        packet.getInfos().push_back(pInfo);
    }

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ(0, (int)body[0]) << "GCGQuestStatusInfo: the listing is now capped - drop this test";
    EXPECT_EQ((size_t)(szBYTE + 256 * (szDWORD + szBYTE + szBYTE)), body.size());

    packet.getInfos().clear();
    for (size_t i = 0; i < owned.size(); i++)
        delete owned[i];
}

// FINDING, stated as a test that fails once it is fixed.
// QuestStatusInfo derives its mission count into a BYTE it caps nowhere,
// so the 256th mission wraps the count to zero while write() emits every
// mission. Standard output is captured because MissionInfo::write prints
// one line per mission, which the finding below states on its own.
TEST(GCGQuestStatusModifyTest, theMissionCountWrapsAtTwoHundredAndFiftySix) {
    FixtureQuestStatus* pInfo = makeQuestStatus(0x85A6B7C8, QuestStatusInfo::DOING, 0);
    for (int i = 0; i < 256; i++)
        pInfo->addMission(makeMission(MissionInfo::HIDE, (WORD)(0x81A2 + i), MissionInfo::CURRENT, "", 0x89AABBCC));

    GCGQuestStatusModify packet;
    packet.setType(GCGQuestStatusModify::CURRENT);
    packet.setInfo(pInfo);

    testing::internal::CaptureStdout();
    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    testing::internal::GetCapturedStdout();

    EXPECT_EQ(0, (int)body[1 + szDWORD + szBYTE]) << "QuestStatusInfo: the mission list is now capped - drop this test";
    EXPECT_EQ((size_t)(szBYTE + szDWORD + szBYTE + szBYTE + 256 * (szBYTE + szWORD + szBYTE + szBYTE + szDWORD)),
              body.size());

    delete pInfo;
}

// FINDING, stated as a test that fails once it is fixed.
// MissionInfo::write prints every mission it emits to standard output,
// on the wire path, once per player the packet is sent to.
TEST(GCGQuestStatusModifyTest, writingAMissionPrintsItToStandardOutput) {
    GCGQuestStatusModify packet;
    fill(packet);

    testing::internal::CaptureStdout();
    writeBody(packet, kPlainCode);
    const std::string printed = testing::internal::GetCapturedStdout();

    EXPECT_NE(std::string::npos, printed.find("write mission"))
        << "MissionInfo::write() no longer prints to standard output - drop this test";
}

// FINDING, stated as a test that fails once it is fixed.
// GCWarList's factory max budgets twelve race wars and twelve guild wars
// and nothing else: not the count byte, not the war type byte in front
// of each record, and not the level war its own read() builds.
TEST(GCWarListTest, theMaxBudgetsNeitherTheCountByteNorTheWarTypeBytes) {
    GCWarList packet;
    for (int i = 0; i < 12; i++) {
        GuildWarInfo* pGuild = new GuildWarInfo;
        pGuild->setAttackGuildName(std::string(40, 'a'));
        pGuild->setDefenseGuildName(std::string(30, 'd'));
        for (int j = 0; j < 255; j++)
            pGuild->addJoinGuild((GuildID_t)j);
        packet.addWarInfo(pGuild);

        RaceWarInfo* pRace = new RaceWarInfo;
        for (int j = 0; j < 255; j++)
            pRace->addCastleID((ZoneID_t)j);
        packet.addWarInfo(pRace);
    }

    GCWarListFactory factory;
    EXPECT_EQ((int)((RaceWarInfo::getMaxSize() + GuildWarInfo::getMaxSize()) * 12), (int)factory.getPacketMaxSize());
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCWarList: the max now budgets the count byte and the war type bytes";
    EXPECT_EQ((int)(szBYTE + 24 * szWarType), (int)packet.getPacketSize() - (int)factory.getPacketMaxSize());
}

// FINDING, stated as a test that fails once it is fixed.
// GCWarList's count byte is derived from the list and capped nowhere, so
// the 256th war wraps it to zero while write() emits every record.
TEST(GCWarListTest, theWarCountWrapsAtTwoHundredAndFiftySix) {
    GCWarList packet;
    for (int i = 0; i < 256; i++) {
        LevelWarInfo* pLevel = new LevelWarInfo;
        pLevel->setLevel(i);
        packet.addWarInfo(pLevel);
    }

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ(0, (int)body[0]) << "GCWarList: the war list is now capped - drop this test";
    EXPECT_EQ((size_t)(szBYTE + 256 * (szWarType + szint + szDWORD + szDWORD)), body.size());
}

// FINDING, stated as a test that fails once it is fixed.
// ValueList, which carries a guild war's join guilds and a race war's
// castles, derives its own count byte from the list and caps nothing.
TEST(GuildWarInfoTest, theJoinGuildCountWrapsAtTwoHundredAndFiftySix) {
    GCWarList packet;
    GuildWarInfo* pGuild = new GuildWarInfo;
    pGuild->setCastleID(0x81A2);
    for (int i = 0; i < 256; i++)
        pGuild->addJoinGuild((GuildID_t)i);
    packet.addWarInfo(pGuild);

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    const size_t countAt = szBYTE + szWarType + szDWORD + szDWORD + szZoneID + szBYTE + szBYTE;
    EXPECT_EQ(0, (int)body[countAt]) << "ValueList: the list is now capped - drop this test";
    EXPECT_EQ(countAt + szBYTE + 256 * szGuildID, body.size());
}

// FINDING, stated as a test that fails once it is fixed.
// GCWarScheduleList derives each guild name's length byte from the name
// with no bound, so a name of 256 wraps the byte to zero while write()
// emits the whole string and the reader stops at the wrong offset. The
// factory max budgets sixteen bytes for a name.
TEST(GCWarScheduleListTest, theGuildNameLengthWrapsAndOutgrowsWhatTheMaxBudgets) {
    GCWarScheduleList wrapped;
    WarScheduleInfo* pInfo = makeGuildSchedule(false);
    pInfo->challengerGuildName[0] = std::string(256, 'c');
    wrapped.addWarScheduleInfo(pInfo);

    const std::vector<unsigned char> body = writeBody(wrapped, kPlainCode);
    const size_t lengthAt = szBYTE + szBYTE + szWORD + szBYTE * 3 + szGuildID;
    EXPECT_EQ(0, (int)body[lengthAt]) << "GCWarScheduleList: the guild name is now bounded - drop this test";

    GCWarScheduleList wide;
    for (int i = 0; i < 2; i++) {
        WarScheduleInfo* pWide = makeGuildSchedule(false);
        for (int j = 0; j < 5; j++)
            pWide->challengerGuildName[j] = std::string(255, 'c');
        pWide->reinforceGuildName = std::string(255, 'r');
        wide.addWarScheduleInfo(pWide);
    }

    GCWarScheduleListFactory factory;
    EXPECT_GT(wide.getPacketSize(), factory.getPacketMaxSize())
        << "GCWarScheduleList: the guild names are now held to what the max budgets";
}

// FINDING, stated as a test that fails once it is fixed.
// GCWarScheduleList's count byte is capped nowhere either, and its max
// budgets twenty entries.
TEST(GCWarScheduleListTest, theEntryCountWrapsAtTwoHundredAndFiftySix) {
    GCWarScheduleList packet;
    for (int i = 0; i < 256; i++)
        packet.addWarScheduleInfo(makeRaceSchedule());

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ(0, (int)body[0]) << "GCWarScheduleList: the schedule is now capped - drop this test";
    EXPECT_EQ((size_t)(szBYTE + 256 * (szBYTE + szWORD + szBYTE * 3)), body.size());
}

// FINDING, stated as a test that fails once it is fixed.
// The six list packets and the two records read into whatever the packet
// already holds instead of replacing it, so a reused packet grows by one
// listing per read.
TEST(GCSelectQuestIDTest, aSecondReadAppendsToTheListItAlreadyHolds) {
    GCSelectQuestID src;
    fill(src);

    GCSelectQuestID dst;
    roundTrip(src, dst, kPlainCode);

    GCSelectQuestID second;
    fill(second);
    roundTrip(second, dst, kPlainCode);

    int held = 0;
    while (!dst.empty()) {
        dst.popQuestID();
        held++;
    }
    EXPECT_EQ(6, held) << "GCSelectQuestID::read() now replaces the list - drop this test and pin the replacement";
}

TEST(GCGQuestStatusInfoTest, aSecondReadAppendsToTheListItAlreadyHolds) {
    GCGQuestStatusInfo src;
    fill(src);

    GCGQuestStatusInfo dst;
    roundTrip(src, dst, kPlainCode);
    ASSERT_EQ((size_t)2, dst.getInfos().size());

    GCGQuestStatusInfo second;
    fill(second);
    roundTrip(second, dst, kPlainCode);
    EXPECT_EQ((size_t)4, dst.getInfos().size())
        << "GCGQuestStatusInfo::read() now replaces the listing - drop this test and pin the replacement";
}

TEST(GCWarListTest, aSecondReadAppendsToTheListItAlreadyHolds) {
    GCWarList src;
    fill(src);

    GCWarList dst;
    roundTrip(src, dst, kPlainCode);
    ASSERT_EQ(3, dst.getSize());

    GCWarList second;
    fill(second);
    roundTrip(second, dst, kPlainCode);
    EXPECT_EQ(6, dst.getSize()) << "GCWarList::read() now replaces the list - drop this test and pin the replacement";
}

TEST(GCWarScheduleListTest, aSecondReadAppendsToTheListItAlreadyHolds) {
    GCWarScheduleList src;
    fill(src);

    GCWarScheduleList dst;
    roundTrip(src, dst, kPlainCode);

    GCWarScheduleList second;
    fill(second);
    roundTrip(second, dst, kPlainCode);

    int held = 0;
    while (dst.popWarScheduleInfo() != NULL)
        held++;
    EXPECT_EQ(4, held) << "GCWarScheduleList::read() now replaces the list - drop this test and pin the replacement";
}

TEST(GCMonsterKillQuestInfoTest, aSecondReadAppendsToTheListItAlreadyHolds) {
    GCMonsterKillQuestInfo src;
    fill(src);

    GCMonsterKillQuestInfo dst;
    roundTrip(src, dst, kPlainCode);

    GCMonsterKillQuestInfo second;
    fill(second);
    roundTrip(second, dst, kPlainCode);

    int held = 0;
    while (!dst.empty()) {
        delete dst.popQuestInfo();
        held++;
    }
    EXPECT_EQ(4, held)
        << "GCMonsterKillQuestInfo::read() now replaces the list - drop this test and pin the replacement";
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

TEST(QuestWarConstructorTest, thePacketsThatInitialiseEveryMemberTheyWrite) {
    expectEveryMemberIsInitialised<GCSelectQuestID>("GCSelectQuestID");
    expectEveryMemberIsInitialised<GCMonsterKillQuestInfo>("GCMonsterKillQuestInfo");
    expectEveryMemberIsInitialised<GCGQuestStatusInfo>("GCGQuestStatusInfo");
    expectEveryMemberIsInitialised<GCNoticeEvent>("GCNoticeEvent");
    expectEveryMemberIsInitialised<GCRegenZoneStatus>("GCRegenZoneStatus");
    expectEveryMemberIsInitialised<GCWarList>("GCWarList");
    expectEveryMemberIsInitialised<GCWarScheduleList>("GCWarScheduleList");
}

TEST(QuestWarConstructorTest, thePacketsThatDoNot) {
    expectAMemberIsLeftUninitialised<CGSelectQuest>("CGSelectQuest");
    expectAMemberIsLeftUninitialised<CGFailQuest>("CGFailQuest");
    expectAMemberIsLeftUninitialised<CGGQuestAccept>("CGGQuestAccept");
    expectAMemberIsLeftUninitialised<CGGQuestCancel>("CGGQuestCancel");
    expectAMemberIsLeftUninitialised<CGSubmitScore>("CGSubmitScore");
    expectAMemberIsLeftUninitialised<CGModifyTaxRatio>("CGModifyTaxRatio");
    expectAMemberIsLeftUninitialised<CGWithdrawTax>("CGWithdrawTax");
    expectAMemberIsLeftUninitialised<CGDonationMoney>("CGDonationMoney");
    expectAMemberIsLeftUninitialised<CGSelectRegenZone>("CGSelectRegenZone");
    expectAMemberIsLeftUninitialised<CGSelectPortal>("CGSelectPortal");
    expectAMemberIsLeftUninitialised<CGSelectWayPoint>("CGSelectWayPoint");
    expectAMemberIsLeftUninitialised<CGSelectTileEffect>("CGSelectTileEffect");
    expectAMemberIsLeftUninitialised<CGRelicToObject>("CGRelicToObject");
    expectAMemberIsLeftUninitialised<GCQuestStatus>("GCQuestStatus");
    expectAMemberIsLeftUninitialised<GCMiniGameScores>("GCMiniGameScores");
    expectAMemberIsLeftUninitialised<GCFlagWarStatus>("GCFlagWarStatus");
    expectAMemberIsLeftUninitialised<GCEnterVampirePortal>("GCEnterVampirePortal");
    expectAMemberIsLeftUninitialised<GCAddHelicopter>("GCAddHelicopter");
}

// GCNotifyWin is in neither list: its one text field is required
// non-empty, so the body a default constructor leaves is refused rather
// than written.
TEST(QuestWarConstructorTest, theWinnerNoticeRefusesItsDefaultBody) {
    alignas(GCNotifyWin) unsigned char storage[sizeof(GCNotifyWin)];
    memset(storage, 0xFF, sizeof(storage));
    GCNotifyWin* pPacket = new (storage) GCNotifyWin();
    EXPECT_THROW(writeBody(*pPacket, kPlainCode), InvalidProtocolException)
        << "GCNotifyWin: an empty name now reaches the wire";
    pPacket->~GCNotifyWin();
}

// FINDING, stated as a test that fails once it is fixed.
// GCGQuestStatusModify is in neither list because writing one over
// poisoned storage would follow the record pointer its constructor
// leaves alone. The pointer itself is pinned instead: both
// getPacketSize() and write() dereference it, so a sender that skips
// setInfo follows an indeterminate value.
TEST(QuestWarConstructorTest, theQuestStatusRecordPointerIsLeftUninitialised) {
    alignas(GCGQuestStatusModify) unsigned char zeroed[sizeof(GCGQuestStatusModify)];
    memset(zeroed, 0x00, sizeof(zeroed));
    GCGQuestStatusModify* pZeroed = new (zeroed) GCGQuestStatusModify();
    QuestStatusInfo* pFromZero = pZeroed->getInfo();
    pZeroed->~GCGQuestStatusModify();

    alignas(GCGQuestStatusModify) unsigned char poisoned[sizeof(GCGQuestStatusModify)];
    memset(poisoned, 0xFF, sizeof(poisoned));
    GCGQuestStatusModify* pPoisoned = new (poisoned) GCGQuestStatusModify();
    QuestStatusInfo* pFromPoison = pPoisoned->getInfo();
    pPoisoned->~GCGQuestStatusModify();

    EXPECT_NE(pFromZero, pFromPoison)
        << "GCGQuestStatusModify: its constructor now initialises the record pointer - move it to the "
           "initialised list";
}

} // namespace
