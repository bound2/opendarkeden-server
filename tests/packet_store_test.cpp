//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_store_test.cpp
// Description : Golden byte fixtures, loopback round trips and size
//               pins for the packets a client and a game server
//               exchange over a shop counter: the player-run stores
//               that stand in a zone, the NPC shops, the item stash
//               and the pet stash.
//
//               The set is taken from the code that sends them: the
//               store, shop and stash handlers under
//               src/server/gameserver/handler (CGStoreOpen,
//               CGStoreClose, CGStoreSign, CGDisplayItem,
//               CGUndisplayItem, CGBuyStoreItem, CGRequestStoreInfo,
//               CGShopRequestList, CGShopRequestBuy,
//               CGShopRequestSell, CGStashList, CGStashDeposit,
//               CGStashWithdraw, CGStashRequestBuy, CGMouseToStash,
//               CGStashToMouse, CGDepositPet, CGWithdrawPet), the GC
//               packets those handlers construct, and the three quest
//               actions that answer a shop NPC (ActionBuy, ActionSell,
//               ActionStashSell). Thirty-six packets, each with the
//               reason it is here:
//
//               CGStoreOpen      the player opens and closes the store
//               CGStoreClose     stall their character stands in.
//               CGStoreSign      and names it; the sign is the one
//                                free-text field in the family.
//               CGDisplayItem    the player puts an item in the stall
//               CGUndisplayItem  window at a price, and takes it back.
//               CGBuyStoreItem   a passer-by buys one of those items.
//               CGRequestStoreInfo  and asks what a stall holds; an
//                                owner id of zero asks about their own.
//
//               GCMyStoreInfo    the owner's view of their own stall:
//                                the whole record, open or not.
//               GCOtherStoreInfo a passer-by's view, which stops at the
//                                open flag when the stall is closed.
//               GCAddStoreItem   one item appearing in a stall window,
//               GCRemoveStoreItem  and one leaving it.
//
//               CGShopRequestList  the client asks an NPC for a rack,
//               CGShopRequestBuy   buys a count of one rack slot,
//               CGShopRequestSell  and offers an item back.
//
//               GCShopList       the rack itself: up to twenty slots,
//                                each with its option list, plus the
//                                market conditions and the shop kind.
//               GCShopListMysterious  the same rack for a mysterious
//                                shop, which shows only class and type.
//               GCShopBought     what the NPC took off the player.
//               GCShopBuyOK      what the player got, with the money
//               GCShopBuyFail    left; and why they got nothing.
//               GCShopSellOK     the price the NPC paid, and the
//               GCShopSellFail   refusal.
//               GCShopSold       the broadcast that a rack slot is
//                                gone.
//               GCShopVersion    the per-rack version stamps a client
//                                caches its racks against.
//               GCShopMarketCondition  a price move on its own.
//
//               CGStashList      the client asks for the stash,
//               CGStashDeposit   pays money in and takes money out,
//               CGStashWithdraw
//               CGStashRequestBuy  and asks to rent another rack.
//               CGMouseToStash   the cursor puts an item into a stash
//               CGStashToMouse   slot and takes one back out.
//               GCStashList      the stash itself: three racks of
//                                twenty, each slot with its option list
//                                and the sub-items in a belt.
//               GCStashSell      the price the stash keeper offered.
//
//               CGDepositPet     the pet half of the same counter,
//               CGWithdrawPet    addressed by stash index.
//               GCPetStashList   the twenty pet slots, occupied or not.
//               GCPetStashVerify the one-byte answer to either request.
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
//               0..5 by tests/packet_encrypter_test.cpp, and none of
//               the thirty-six derives from one of them: every class
//               here extends Packet directly. So each golden is
//               recorded at code 0 and its test also asserts the bytes
//               do not vary with the code - adopting the encrypter
//               fails loudly instead of silently voiding the pin.
//
//               Already pinned elsewhere, so the packets the same
//               handlers also send are not repeated here: GCCannotAdd,
//               GCDeleteandPickUpOK, GCCreateItem
//               (packet_inventory_test.cpp), GCDeleteObject
//               (packet_zone_scan_test.cpp), GCModifyInformation
//               (packet_combat_test.cpp), GCNoticeEvent and
//               GCSystemMessage (packet_gameserver_handshake_test.cpp),
//               GCNPCResponse (packet_guild_test.cpp). The records
//               these packets embed are pinned too: StoreInfo and
//               StoreOutlook in packet_zone_scan_test.cpp, PCItemInfo
//               and PetInfo in packet_gameserver_handshake_test.cpp,
//               SubItemInfo there and in packet_trade_test.cpp. They
//               appear here only as the contents of a packet, so the
//               offsets around them are pinned as well.
//
//               GCShopList already had a frame-size pin in
//               packet_roundtrip_test.cpp but no golden; it gets one
//               here.
//
//               There is no GCBuyStoreItem answer to write down: the
//               buy handler answers with GCMyStoreInfo,
//               GCRemoveStoreItem and GCShopSellOK. Every one of the
//               thirty-six has a registered factory in
//               tests/ratchet/factory_registrations.txt and at least
//               one sender outside src/Core.
//
//               Each packet gets three pins (STORE_PACKET_TESTS):
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
//               GCMyStoreInfo, GCOtherStoreInfo and GCShopBuyFail get
//               those pins written out by hand: the first two hold
//               their record by pointer and need a fixture that owns
//               it, and the third's declared size is a finding below.
//
//               Extra goldens cover the branches one fixture cannot:
//               the item-bearing records are written with and without
//               an option list (.nooptions on GCAddStoreItem,
//               GCShopList, GCShopBought, GCShopBuyOK, GCStashList),
//               the list packets are written with nothing in them
//               (.empty on GCShopList, GCShopListMysterious,
//               GCStashList, GCPetStashList), a stall is written
//               closed to its owner and to a passer-by (.closed on
//               GCMyStoreInfo and GCOtherStoreInfo, which are the two
//               halves of StoreInfo::write's toOther branch), an empty
//               stall window (.noitem on GCAddStoreItem) and a pet slot
//               holding a record with no pet in it (.petnone on
//               GCPetStashList, PetInfo's own early return). The empty
//               sign gets a golden too (.nosign on CGStoreSign), inside
//               the finding below, because it cannot be read back.
//
//               Fixture values are distinct per field and >= 128 in
//               every byte the width allows. Three groups cannot follow
//               that rule and say so at the point of use: the stall
//               sign and the pet nickname, which are text; the rack,
//               slot and pet-type bytes a read() uses to index a fixed
//               array or to select a branch; and GCShopVersion's rack
//               type, which selects one of three version stamps.
//
//               Findings. Each is stated as a test that fails once the
//               packet is fixed, except where noted:
//
//               - GCShopBuyFail::getPacketSize() counts the NPC id
//                 alone while write() also emits the fail code and the
//                 four-byte amount, and the factory max is the same
//                 four bytes, so every refusal declares a body five
//                 bytes shorter than it sends and overruns the read
//                 buffer the receiver sized for it.
//               - CGStoreSign writes the sign length into a BYTE it
//                 derives by hand and caps nowhere, so a sign past the
//                 80 bytes its factory max budgets is emitted whole,
//                 and at 256 bytes the length byte wraps to zero while
//                 the text still follows it.
//               - CGStoreSign::write() emits an empty sign as a bare
//                 zero length byte, but read() hands that zero to
//                 SocketInputStream::read(string&, uint), which refuses
//                 a zero length, so a stall named with an empty string
//                 is written and cannot be read back.
//               - StoreInfo::setSign caps nothing, and StoreInfo::write
//                 bounds the sign at 255 while StoreInfo::getMaxSize
//                 budgets 80, so a full stall whose sign is longer than
//                 that makes GCMyStoreInfo and GCOtherStoreInfo outgrow
//                 their factory max by the overage. StoreOutlook, the
//                 sign's other carrier, does cut at 80 in its setter.
//               - GCShopBought and GCShopBuyOK derive the option count
//                 from the list and cap nothing, so the 256th option
//                 wraps the count to zero and the body outgrows the
//                 factory max, which budgets 255.
//               - GCShopBought, GCShopBuyOK, GCShopList and GCStashList
//                 append to the option list a packet already holds
//                 instead of replacing it, so a reused packet grows by
//                 one listing per read. PCItemInfo::read, the record
//                 the store packets use, clears first.
//               - GCPetStashList never puts its code byte on the wire
//                 although both pet handlers set it and its factory max
//                 budgets a byte for it, so the answer the sender chose
//                 is dropped and every body is one byte under the
//                 declared maximum.
//               - GCStashList keeps the sub-item count of each slot in
//                 an array of its own while the records come from a
//                 separate list, so a sub-item added through
//                 getSubItems() leaves the count behind: write() emits
//                 the stale count and then the whole list, and
//                 getPacketSize() counts the stale one.
//               - GCMyStoreInfo and GCOtherStoreInfo leave their
//                 StoreInfo pointer uninitialised, so getPacketSize()
//                 and write() dereference an indeterminate value when a
//                 sender skips setStoreInfo.
//
//               Three findings are recorded here rather than tested,
//               because reaching them is undefined behaviour or has no
//               observable wire effect:
//
//               - GCShopList::read, GCShopListMysterious::read and
//                 GCStashList::read index their fixed slot arrays with
//                 a rack or slot byte taken straight off the wire and
//                 never compared against SHOP_RACK_INDEX_MAX,
//                 STASH_RACK_MAX or STASH_INDEX_MAX, so a peer can
//                 write past the end of the packet. Every fixture here
//                 stays in range for that reason.
//               - GCShopBuyFail::read takes its code byte without
//                 checking it against GC_SHOP_BUY_FAIL_MAX, and the
//                 constructor's own default is that very sentinel, so
//                 an untouched packet announces a code no branch of the
//                 client handles.
//               - GCPetStashList::read overwrites a slot without
//                 freeing what it held and the destructor frees the
//                 slot record but not the PetInfo inside it, so both a
//                 reused packet and a delivered one leak the pet.
//
//               Twenty-five of the thirty-six leave at least one member
//               the default constructor never sets, so a packet sent
//               without every setter called puts indeterminate bytes on
//               the wire. The poisoned-storage pin at the end of the
//               file asserts today's split. CGStoreOpen, CGStoreClose
//               and CGStashRequestBuy are in neither list because their
//               body is empty; GCMyStoreInfo and GCOtherStoreInfo are
//               in neither because writing one over poisoned storage
//               would follow the indeterminate record pointer, so they
//               get a pin on the pointer itself instead.
//
//////////////////////////////////////////////////////////////////////

#include <cstring>
#include <list>
#include <new>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "CGBuyStoreItem.h"
#include "CGDepositPet.h"
#include "CGDisplayItem.h"
#include "CGMouseToStash.h"
#include "CGRequestStoreInfo.h"
#include "CGShopRequestBuy.h"
#include "CGShopRequestList.h"
#include "CGShopRequestSell.h"
#include "CGStashDeposit.h"
#include "CGStashList.h"
#include "CGStashRequestBuy.h"
#include "CGStashToMouse.h"
#include "CGStashWithdraw.h"
#include "CGStoreClose.h"
#include "CGStoreOpen.h"
#include "CGStoreSign.h"
#include "CGUndisplayItem.h"
#include "CGWithdrawPet.h"
#include "Exception.h"
#include "GCAddStoreItem.h"
#include "GCMyStoreInfo.h"
#include "GCOtherStoreInfo.h"
#include "GCPetStashList.h"
#include "GCPetStashVerify.h"
#include "GCRemoveStoreItem.h"
#include "GCShopBought.h"
#include "GCShopBuyFail.h"
#include "GCShopBuyOK.h"
#include "GCShopList.h"
#include "GCShopListMysterious.h"
#include "GCShopMarketCondition.h"
#include "GCShopSellFail.h"
#include "GCShopSellOK.h"
#include "GCShopSold.h"
#include "GCShopVersion.h"
#include "GCStashList.h"
#include "GCStashSell.h"
#include "StoreInfo.h"
#include "TestStreams.h"
#include "WireString.h"

using wiretest::expectGolden;
using wiretest::kEncryptCodeCount;
using wiretest::kEncryptCodes;
using wiretest::Loopback;
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

#define STORE_PACKET_GOLDEN_AND_SIZE(Name)                                                           \
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

#define STORE_PACKET_TESTS(Name)                  \
    TEST(Name##Test, roundTripsThroughLoopback) { \
        Name src;                                 \
        fill(src);                                \
        Name dst;                                 \
        roundTrip(src, dst, kPlainCode);          \
        expectEqual(src, dst);                    \
    }                                             \
    STORE_PACKET_GOLDEN_AND_SIZE(Name)

// A second fixture for a packet whose write() has a branch the canonical
// one does not take.
#define STORE_PACKET_VARIANT(Name, Variant, fillVariant)                       \
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
// Shared helpers.
//////////////////////////////////////////////////////////////////////

// Three packets here have no setter in the wire library at all: the
// gameserver fills them from live Item objects
// (packetfill/GCShopListFill.cpp, GCShopListMysteriousFill.cpp,
// GCStashListFill.cpp), which the kernel must not depend on. They are
// populated the only way the library allows, by reading a body emitted
// field by field.
template <typename Emit> void readFields(Packet& packet, Emit emit) {
    Loopback link;
    link.setCodes(kPlainCode);
    emit(link.out());
    const uint length = link.out().length();
    link.pump(length);
    packet.read(link.in());
}

// The item record the store packets embed. Its own layout is pinned in
// packet_gameserver_handshake_test.cpp; here it only has to be a record
// with entries in both of its lists, so the offsets around it are
// pinned too.
void fillPCItemInfo(PCItemInfo& info, ObjectID_t base) {
    info.setObjectID(base);
    info.setItemClass(0x8A);
    info.setItemType(0x8B9C);
    info.addOptionType(0x8D);
    info.addOptionType(0x8E);
    info.setDurability(0x8FA0B1C2);
    info.setSilver(0x91A2);
    info.setGrade(0x93A4B5C6);
    info.setEnchantLevel((EnchantLevel_t)0x97);
    info.setItemNum(0x98);
    info.setMainColor(0x99AA);

    SubItemInfo* pSubItem = new SubItemInfo();
    pSubItem->setObjectID(base + 0x01010101u);
    pSubItem->setItemClass(0x9B);
    pSubItem->setItemType(0x9CAD);
    pSubItem->setItemNum(0x9E);
    pSubItem->setSlotID(0x9F);
    info.addListElement(pSubItem);
}

// popFrontListElement hands the caller the record, so comparing the two
// sub-item lists empties them; the entries are deleted here.
void expectPCItemInfoEqual(PCItemInfo& a, PCItemInfo& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ((int)a.getItemClass(), (int)b.getItemClass());
    EXPECT_EQ((int)a.getItemType(), (int)b.getItemType());
    EXPECT_EQ(a.getOptionType(), b.getOptionType());
    EXPECT_EQ(a.getDurability(), b.getDurability());
    EXPECT_EQ((int)a.getSilver(), (int)b.getSilver());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    EXPECT_EQ((int)a.getEnchantLevel(), (int)b.getEnchantLevel());
    EXPECT_EQ((int)a.getItemNum(), (int)b.getItemNum());
    EXPECT_EQ((int)a.getMainColor(), (int)b.getMainColor());

    ASSERT_EQ((int)a.getListNum(), (int)b.getListNum());
    const int subItems = (int)a.getListNum();
    for (int i = 0; i < subItems; i++) {
        SubItemInfo* pLeft = a.popFrontListElement();
        SubItemInfo* pRight = b.popFrontListElement();
        EXPECT_EQ(pLeft->getObjectID(), pRight->getObjectID()) << "sub-item " << i;
        EXPECT_EQ((int)pLeft->getItemClass(), (int)pRight->getItemClass()) << "sub-item " << i;
        EXPECT_EQ((int)pLeft->getItemType(), (int)pRight->getItemType()) << "sub-item " << i;
        EXPECT_EQ((int)pLeft->getItemNum(), (int)pRight->getItemNum()) << "sub-item " << i;
        EXPECT_EQ((int)pLeft->getSlotID(), (int)pRight->getSlotID()) << "sub-item " << i;
        delete pLeft;
        delete pRight;
    }
}

void fillStoreItemInfo(StoreItemInfo& item, ObjectID_t base) {
    item.setItemExist(1);
    fillPCItemInfo(item, base);
    item.setPrice(0xA1B2C3D4);
}

void expectStoreItemInfoEqual(StoreItemInfo& a, StoreItemInfo& b) {
    ASSERT_EQ((int)a.isItemExist(), (int)b.isItemExist());
    if (a.isItemExist() == 0)
        return;
    expectPCItemInfoEqual(a, b);
    EXPECT_EQ(a.getPrice(), b.getPrice());
}

// The stall record. The sign is text and the two occupied window slots
// are addressed by position, so neither can follow the >= 128 rule.
void fillStoreInfo(StoreInfo& info, BYTE open) {
    info.setOpen(open);
    info.setSign("Bloodstained Bargains");
    fillStoreItemInfo(info.getStoreItemInfo(0), 0x81A2B3C4);
    fillStoreItemInfo(info.getStoreItemInfo(7), 0x82A3B4C5);
}

void expectStoreInfoEqual(StoreInfo& a, StoreInfo& b) {
    ASSERT_EQ((int)a.isOpen(), (int)b.isOpen());
    EXPECT_EQ(a.getSign(), b.getSign());
    ASSERT_EQ(a.getItems().size(), b.getItems().size());
    for (size_t i = 0; i < a.getItems().size(); i++) {
        SCOPED_TRACE(testing::Message() << "store slot " << i);
        expectStoreItemInfoEqual(a.getItems()[i], b.getItems()[i]);
    }
}

// The pet record GCPetStashList embeds. Its own layout is pinned in
// packet_gameserver_handshake_test.cpp. The pet type selects the
// record's shape, so it must be a real enumerator; the nickname is text.
void fillPetInfo(PetInfo& pet, MonsterType_t creature) {
    pet.setPetType(PET_STIRGE);
    pet.setPetCreatureType(creature);
    pet.setPetLevel(0x83);
    pet.setPetExp(0x84A5B6C7);
    pet.setPetHP(0x85A6);
    pet.setPetAttr(0x87);
    pet.setPetAttrLevel(0x88);
    pet.setPetOption(0x89);
    pet.setFoodType(0x8AAB);
    pet.setGamble(0x8C);
    pet.setCutHead(0x8D);
    pet.setAttack(0x8E);
    pet.setSummonInfo(0x8F);
    pet.setNickname("Stashed Stirge");
}

void expectPetInfoEqual(const PetInfo& a, const PetInfo& b) {
    EXPECT_EQ((int)a.getPetType(), (int)b.getPetType());
    if (a.getPetType() == PET_NONE)
        return;
    EXPECT_EQ((int)a.getPetCreatureType(), (int)b.getPetCreatureType());
    EXPECT_EQ((int)a.getPetLevel(), (int)b.getPetLevel());
    EXPECT_EQ(a.getPetExp(), b.getPetExp());
    EXPECT_EQ((int)a.getPetHP(), (int)b.getPetHP());
    EXPECT_EQ((int)a.getPetAttr(), (int)b.getPetAttr());
    EXPECT_EQ((int)a.getPetAttrLevel(), (int)b.getPetAttrLevel());
    EXPECT_EQ((int)a.getPetOption(), (int)b.getPetOption());
    EXPECT_EQ((int)a.getFoodType(), (int)b.getFoodType());
    EXPECT_EQ((int)a.canGamble(), (int)b.canGamble());
    EXPECT_EQ((int)a.canCutHead(), (int)b.canCutHead());
    EXPECT_EQ((int)a.canAttack(), (int)b.canAttack());
    EXPECT_EQ((int)a.isSummonInfo(), (int)b.isSummonInfo());
    EXPECT_EQ(a.getNickname(), b.getNickname());
}

//////////////////////////////////////////////////////////////////////
// Opening, naming and browsing a player-run stall.
//////////////////////////////////////////////////////////////////////

// CGStoreOpen, CGStoreClose and CGStashRequestBuy carry no body at all:
// the packet id is the whole message.
void fill(CGStoreOpen&) {}

void expectEqual(CGStoreOpen& a, CGStoreOpen& b) {
    EXPECT_EQ(0, (int)a.getPacketSize());
    EXPECT_EQ(0, (int)b.getPacketSize());
}

STORE_PACKET_TESTS(CGStoreOpen)

void fill(CGStoreClose&) {}

void expectEqual(CGStoreClose& a, CGStoreClose& b) {
    EXPECT_EQ(0, (int)a.getPacketSize());
    EXPECT_EQ(0, (int)b.getPacketSize());
}

STORE_PACKET_TESTS(CGStoreClose)

// The sign is text the player types, so it cannot follow the >= 128
// rule.
void fill(CGStoreSign& packet) {
    packet.setSign("Bloodstained Bargains");
}

void expectEqual(CGStoreSign& a, CGStoreSign& b) {
    EXPECT_EQ(a.getSign(), b.getSign());
}

STORE_PACKET_TESTS(CGStoreSign)

// FINDING, stated as a test that fails once it is fixed.
// write() emits an empty sign as a bare zero length byte, but read()
// hands that zero to SocketInputStream::read(string&, uint), which
// refuses a zero length, so a stall named with an empty string is
// written and cannot be read back.
TEST(CGStoreSignTest, anEmptySignIsWrittenAndCannotBeReadBack) {
    CGStoreSign packet;
    packet.setSign("");

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)szBYTE, body.size());
    EXPECT_EQ(0, (int)body[0]);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());
    expectGolden("CGStoreSign.nosign", kPlainCode, body);

    CGStoreSign dst;
    EXPECT_THROW(roundTrip(packet, dst, kPlainCode), InvalidProtocolException)
        << "CGStoreSign: an empty sign now reads back - drop this test and pin the round trip";
}

void fill(CGDisplayItem& packet) {
    packet.setXY(0x81, 0x82);
    packet.setItemObjectID(0x83A4B5C6);
    packet.setPrice(0x84A5B6C7);
    packet.setIndex(0x85);
}

void expectEqual(CGDisplayItem& a, CGDisplayItem& b) {
    EXPECT_EQ((int)a.getX(), (int)b.getX());
    EXPECT_EQ((int)a.getY(), (int)b.getY());
    EXPECT_EQ(a.getItemObjectID(), b.getItemObjectID());
    EXPECT_EQ(a.getPrice(), b.getPrice());
    EXPECT_EQ((int)a.getIndex(), (int)b.getIndex());
}

STORE_PACKET_TESTS(CGDisplayItem)

void fill(CGUndisplayItem& packet) {
    packet.setXY(0x86, 0x87);
    packet.setItemObjectID(0x88A9BACB);
    packet.setIndex(0x89);
}

void expectEqual(CGUndisplayItem& a, CGUndisplayItem& b) {
    EXPECT_EQ((int)a.getX(), (int)b.getX());
    EXPECT_EQ((int)a.getY(), (int)b.getY());
    EXPECT_EQ(a.getItemObjectID(), b.getItemObjectID());
    EXPECT_EQ((int)a.getIndex(), (int)b.getIndex());
}

STORE_PACKET_TESTS(CGUndisplayItem)

void fill(CGBuyStoreItem& packet) {
    packet.setOwnerObjectID(0x8AABBCCD);
    packet.setItemObjectID(0x8BACBDCE);
    packet.setIndex(0x8C);
}

void expectEqual(CGBuyStoreItem& a, CGBuyStoreItem& b) {
    EXPECT_EQ(a.getOwnerObjectID(), b.getOwnerObjectID());
    EXPECT_EQ(a.getItemObjectID(), b.getItemObjectID());
    EXPECT_EQ((int)a.getIndex(), (int)b.getIndex());
}

STORE_PACKET_TESTS(CGBuyStoreItem)

void fill(CGRequestStoreInfo& packet) {
    packet.setOwnerObjectID(0x8DAEBFD0);
}

void expectEqual(CGRequestStoreInfo& a, CGRequestStoreInfo& b) {
    EXPECT_EQ(a.getOwnerObjectID(), b.getOwnerObjectID());
}

STORE_PACKET_TESTS(CGRequestStoreInfo)

//////////////////////////////////////////////////////////////////////
// What the stall looks like to its owner and to a passer-by.
//////////////////////////////////////////////////////////////////////

// Both packets keep the stall record by pointer, so the fixture owns it.
struct MyStoreFixture {
    StoreInfo info;
    GCMyStoreInfo packet;

    MyStoreFixture() {
        packet.setStoreInfo(&info);
    }
};

struct OtherStoreFixture {
    StoreInfo info;
    GCOtherStoreInfo packet;

    OtherStoreFixture() {
        packet.setStoreInfo(&info);
    }
};

void fillMyStore(MyStoreFixture& f, BYTE open) {
    f.packet.setOpenUI(0x81);
    fillStoreInfo(f.info, open);
}

void fillOtherStore(OtherStoreFixture& f, BYTE open) {
    f.packet.setObjectID(0x82A3B4C5);
    f.packet.setRequested(0x83);
    fillStoreInfo(f.info, open);
}

TEST(GCMyStoreInfoTest, roundTripsThroughLoopback) {
    MyStoreFixture src;
    fillMyStore(src, 1);
    MyStoreFixture dst;
    roundTrip(src.packet, dst.packet, kPlainCode);

    EXPECT_EQ((int)src.packet.getOpenUI(), (int)dst.packet.getOpenUI());
    expectStoreInfoEqual(src.info, dst.info);
}

TEST(GCMyStoreInfoTest, bodyBytesMatchGolden) {
    MyStoreFixture f;
    fillMyStore(f, 1);
    const std::vector<unsigned char> body = writeBody(f.packet, kPlainCode);
    expectGolden("GCMyStoreInfo", kPlainCode, body);
    for (size_t i = 1; i < kEncryptCodeCount; i++)
        EXPECT_EQ(body, writeBody(f.packet, kEncryptCodes[i]))
            << "GCMyStoreInfo now varies with the encrypt code - add per-code goldens";
}

TEST(GCMyStoreInfoTest, sizeMatchesTheBytesWrittenAndFitsTheFactoryMax) {
    MyStoreFixture f;
    fillMyStore(f, 1);
    GCMyStoreInfoFactory factory;
    EXPECT_EQ((size_t)f.packet.getPacketSize(), writeBody(f.packet, kPlainCode).size());
    EXPECT_LE(f.packet.getPacketSize(), factory.getPacketMaxSize());
    EXPECT_EQ(factory.getPacketID(), f.packet.getPacketID());
    EXPECT_EQ(factory.getPacketName(), f.packet.getPacketName());
}

// The owner sees the whole record even when the stall is shut: the
// toOther half of StoreInfo::write is false here, so the open flag never
// cuts the body short.
TEST(GCMyStoreInfoTest, closedBodyBytesMatchGolden) {
    MyStoreFixture f;
    fillMyStore(f, 0);
    const std::vector<unsigned char> body = writeBody(f.packet, kPlainCode);
    expectGolden("GCMyStoreInfo.closed", kPlainCode, body);
    EXPECT_EQ((size_t)f.packet.getPacketSize(), body.size());

    MyStoreFixture dst;
    roundTrip(f.packet, dst.packet, kPlainCode);
    EXPECT_EQ(0, (int)dst.info.isOpen());
    expectStoreInfoEqual(f.info, dst.info);
}

TEST(GCOtherStoreInfoTest, roundTripsThroughLoopback) {
    OtherStoreFixture src;
    fillOtherStore(src, 1);
    OtherStoreFixture dst;
    roundTrip(src.packet, dst.packet, kPlainCode);

    EXPECT_EQ(src.packet.getObjectID(), dst.packet.getObjectID());
    EXPECT_EQ((int)src.packet.isRequested(), (int)dst.packet.isRequested());
    expectStoreInfoEqual(src.info, dst.info);
}

TEST(GCOtherStoreInfoTest, bodyBytesMatchGolden) {
    OtherStoreFixture f;
    fillOtherStore(f, 1);
    const std::vector<unsigned char> body = writeBody(f.packet, kPlainCode);
    expectGolden("GCOtherStoreInfo", kPlainCode, body);
    for (size_t i = 1; i < kEncryptCodeCount; i++)
        EXPECT_EQ(body, writeBody(f.packet, kEncryptCodes[i]))
            << "GCOtherStoreInfo now varies with the encrypt code - add per-code goldens";
}

TEST(GCOtherStoreInfoTest, sizeMatchesTheBytesWrittenAndFitsTheFactoryMax) {
    OtherStoreFixture f;
    fillOtherStore(f, 1);
    GCOtherStoreInfoFactory factory;
    EXPECT_EQ((size_t)f.packet.getPacketSize(), writeBody(f.packet, kPlainCode).size());
    EXPECT_LE(f.packet.getPacketSize(), factory.getPacketMaxSize());
    EXPECT_EQ(factory.getPacketID(), f.packet.getPacketID());
    EXPECT_EQ(factory.getPacketName(), f.packet.getPacketName());
}

// A passer-by gets the open flag and nothing else when the stall is
// shut: the other half of the same branch.
TEST(GCOtherStoreInfoTest, closedBodyBytesMatchGolden) {
    OtherStoreFixture f;
    fillOtherStore(f, 0);
    const std::vector<unsigned char> body = writeBody(f.packet, kPlainCode);
    expectGolden("GCOtherStoreInfo.closed", kPlainCode, body);
    EXPECT_EQ((size_t)f.packet.getPacketSize(), body.size());
    EXPECT_EQ((size_t)(szObjectID + szBYTE + szBYTE), body.size());

    OtherStoreFixture dst;
    roundTrip(f.packet, dst.packet, kPlainCode);
    EXPECT_EQ(f.packet.getObjectID(), dst.packet.getObjectID());
    EXPECT_EQ(0, (int)dst.info.isOpen());
    EXPECT_TRUE(dst.info.getSign().empty());
}

void fill(GCAddStoreItem& packet) {
    packet.setOwnerObjectID(0x84A5B6C7);
    packet.setIndex(0x85);
    fillStoreItemInfo(packet.getItem(), 0x86A7B8C9);
}

void expectEqual(GCAddStoreItem& a, GCAddStoreItem& b) {
    EXPECT_EQ(a.getOwnerObjectID(), b.getOwnerObjectID());
    EXPECT_EQ((int)a.getIndex(), (int)b.getIndex());
    expectStoreItemInfoEqual(a.getItem(), b.getItem());
}

STORE_PACKET_TESTS(GCAddStoreItem)

void fillNoOptions(GCAddStoreItem& packet) {
    fill(packet);
    packet.getItem().setOptionType(std::list<OptionType_t>());
}

STORE_PACKET_VARIANT(GCAddStoreItem, nooptions, fillNoOptions)

// An empty window slot: the record stops at its exist byte.
void fillNoItem(GCAddStoreItem& packet) {
    packet.setOwnerObjectID(0x87A8B9CA);
    packet.setIndex(0x88);
    packet.getItem().setItemExist(0);
}

STORE_PACKET_VARIANT(GCAddStoreItem, noitem, fillNoItem)

void fill(GCRemoveStoreItem& packet) {
    packet.setOwnerObjectID(0x89AABBCC);
    packet.setIndex(0x8A);
}

void expectEqual(GCRemoveStoreItem& a, GCRemoveStoreItem& b) {
    EXPECT_EQ(a.getOwnerObjectID(), b.getOwnerObjectID());
    EXPECT_EQ((int)a.getIndex(), (int)b.getIndex());
}

STORE_PACKET_TESTS(GCRemoveStoreItem)

//////////////////////////////////////////////////////////////////////
// Asking an NPC shop for a rack, buying from it and selling to it.
//////////////////////////////////////////////////////////////////////

void fill(CGShopRequestList& packet) {
    packet.setObjectID(0x81A2B3C4);
    packet.setRackType(SHOP_RACK_MYSTERIOUS);
}

void expectEqual(CGShopRequestList& a, CGShopRequestList& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ((int)a.getRackType(), (int)b.getRackType());
}

STORE_PACKET_TESTS(CGShopRequestList)

// The rack type selects one of three racks, so it stays an enumerator.
void fill(CGShopRequestBuy& packet) {
    packet.setObjectID(0x82A3B4C5);
    packet.setShopType(SHOP_RACK_SPECIAL);
    packet.setShopIndex(0x84);
    packet.setItemNum(0x85);
    packet.setX(0x86);
    packet.setY(0x87);
}

void expectEqual(CGShopRequestBuy& a, CGShopRequestBuy& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ((int)a.getShopType(), (int)b.getShopType());
    EXPECT_EQ((int)a.getShopIndex(), (int)b.getShopIndex());
    EXPECT_EQ((int)a.getItemNum(), (int)b.getItemNum());
    EXPECT_EQ((int)a.getX(), (int)b.getX());
    EXPECT_EQ((int)a.getY(), (int)b.getY());
}

STORE_PACKET_TESTS(CGShopRequestBuy)

void fill(CGShopRequestSell& packet) {
    packet.setObjectID(0x88A9BACB);
    packet.setItemObjectID(0x89AABBCC);
    packet.setOpCode(0x8A);
}

void expectEqual(CGShopRequestSell& a, CGShopRequestSell& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getItemObjectID(), b.getItemObjectID());
    EXPECT_EQ((int)a.getOpCode(), (int)b.getOpCode());
}

STORE_PACKET_TESTS(CGShopRequestSell)

//////////////////////////////////////////////////////////////////////
// The rack itself.
//////////////////////////////////////////////////////////////////////

struct ShopItemFixture {
    BYTE index;
    ObjectID_t objectID;
    BYTE itemClass;
    ItemType_t itemType;
    std::vector<OptionType_t> options;
    Durability_t durability;
    Silver_t silver;
    Grade_t grade;
    EnchantLevel_t enchantLevel;
};

// The slot index addresses m_pBuffer, so it stays below
// SHOP_RACK_INDEX_MAX; every other field follows the >= 128 rule.
std::vector<ShopItemFixture> canonicalShopItems() {
    std::vector<ShopItemFixture> items;

    ShopItemFixture first;
    first.index = 0;
    first.objectID = 0x81A2B3C4;
    first.itemClass = 0x85;
    first.itemType = 0x86A7;
    first.options.push_back(0x88);
    first.options.push_back(0x89);
    first.durability = 0x8AABBCCD;
    first.silver = 0x8BAC;
    first.grade = 0x8CADBECF;
    first.enchantLevel = (EnchantLevel_t)0x8D;
    items.push_back(first);

    ShopItemFixture second;
    second.index = (BYTE)(SHOP_RACK_INDEX_MAX - 1);
    second.objectID = 0x8EAFC0D1;
    second.itemClass = 0x8F;
    second.itemType = 0x90A1;
    second.options.push_back(0x92);
    second.durability = 0x93A4B5C6;
    second.silver = 0x94A5;
    second.grade = 0x95A6B7C8;
    second.enchantLevel = (EnchantLevel_t)0x96;
    items.push_back(second);

    return items;
}

void emitShopList(SocketEncryptOutputStream& out, const std::vector<ShopItemFixture>& items) {
    out.write((ObjectID_t)0x97A8B9CA);
    out.write((ShopVersion_t)0x98A9BACB);
    out.write((ShopRackType_t)SHOP_RACK_NORMAL);
    out.write((BYTE)items.size());

    for (size_t i = 0; i < items.size(); i++) {
        const ShopItemFixture& item = items[i];
        out.write(item.index);
        out.write(item.objectID);
        out.write(item.itemClass);
        out.write(item.itemType);
        out.write((BYTE)item.options.size());
        for (size_t j = 0; j < item.options.size(); j++)
            out.write(item.options[j]);
        out.write(item.durability);
        out.write(item.silver);
        out.write(item.grade);
        out.write(item.enchantLevel);
    }

    out.write((MarketCond_t)0x99AA);
    out.write((MarketCond_t)0x9ABB);
    out.write((BYTE)0x9B);
}

void fillShopList(GCShopList& packet, const std::vector<ShopItemFixture>& items) {
    readFields(packet, [&items](SocketEncryptOutputStream& out) { emitShopList(out, items); });
}

void fill(GCShopList& packet) {
    fillShopList(packet, canonicalShopItems());
}

void expectEqual(GCShopList& a, GCShopList& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getShopVersion(), b.getShopVersion());
    EXPECT_EQ((int)a.getShopType(), (int)b.getShopType());
    EXPECT_EQ((int)a.getMarketCondBuy(), (int)b.getMarketCondBuy());
    EXPECT_EQ((int)a.getMarketCondSell(), (int)b.getMarketCondSell());
    EXPECT_EQ((int)a.getNPCShopType(), (int)b.getNPCShopType());

    for (int i = 0; i < SHOP_RACK_INDEX_MAX; i++) {
        SCOPED_TRACE(testing::Message() << "rack slot " << i);
        const SHOPLISTITEM left = a.getShopItem((BYTE)i);
        const SHOPLISTITEM right = b.getShopItem((BYTE)i);
        ASSERT_EQ(left.bExist, right.bExist);
        if (!left.bExist)
            continue;
        EXPECT_EQ(left.objectID, right.objectID);
        EXPECT_EQ((int)left.itemClass, (int)right.itemClass);
        EXPECT_EQ((int)left.itemType, (int)right.itemType);
        EXPECT_EQ(left.optionType, right.optionType);
        EXPECT_EQ(left.durability, right.durability);
        EXPECT_EQ((int)left.silver, (int)right.silver);
        EXPECT_EQ(left.grade, right.grade);
        EXPECT_EQ((int)left.enchantLevel, (int)right.enchantLevel);
    }
}

STORE_PACKET_TESTS(GCShopList)

void fillShopListEmpty(GCShopList& packet) {
    fillShopList(packet, std::vector<ShopItemFixture>());
}

STORE_PACKET_VARIANT(GCShopList, empty, fillShopListEmpty)

void fillShopListNoOptions(GCShopList& packet) {
    std::vector<ShopItemFixture> items = canonicalShopItems();
    for (size_t i = 0; i < items.size(); i++)
        items[i].options.clear();
    fillShopList(packet, items);
}

STORE_PACKET_VARIANT(GCShopList, nooptions, fillShopListNoOptions)

struct MysteriousItemFixture {
    BYTE index;
    BYTE itemClass;
    ItemType_t itemType;
};

std::vector<MysteriousItemFixture> canonicalMysteriousItems() {
    std::vector<MysteriousItemFixture> items;

    MysteriousItemFixture first;
    first.index = 1;
    first.itemClass = 0x81;
    first.itemType = 0x82A3;
    items.push_back(first);

    MysteriousItemFixture second;
    second.index = (BYTE)(SHOP_RACK_INDEX_MAX - 2);
    second.itemClass = 0x84;
    second.itemType = 0x85A6;
    items.push_back(second);

    return items;
}

void emitShopListMysterious(SocketEncryptOutputStream& out, const std::vector<MysteriousItemFixture>& items) {
    out.write((ObjectID_t)0x87A8B9CA);
    out.write((ShopVersion_t)0x88A9BACB);
    out.write((ShopRackType_t)SHOP_RACK_MYSTERIOUS);
    out.write((BYTE)items.size());

    for (size_t i = 0; i < items.size(); i++) {
        out.write(items[i].index);
        out.write(items[i].itemClass);
        out.write(items[i].itemType);
    }

    out.write((MarketCond_t)0x89AA);
    out.write((MarketCond_t)0x8ABB);
}

void fillMysterious(GCShopListMysterious& packet, const std::vector<MysteriousItemFixture>& items) {
    readFields(packet, [&items](SocketEncryptOutputStream& out) { emitShopListMysterious(out, items); });
}

void fill(GCShopListMysterious& packet) {
    fillMysterious(packet, canonicalMysteriousItems());
}

void expectEqual(GCShopListMysterious& a, GCShopListMysterious& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getShopVersion(), b.getShopVersion());
    EXPECT_EQ((int)a.getShopType(), (int)b.getShopType());
    EXPECT_EQ((int)a.getMarketCondBuy(), (int)b.getMarketCondBuy());
    EXPECT_EQ((int)a.getMarketCondSell(), (int)b.getMarketCondSell());

    for (int i = 0; i < SHOP_RACK_INDEX_MAX; i++) {
        SCOPED_TRACE(testing::Message() << "rack slot " << i);
        const SHOPLISTITEM_MYSTERIOUS left = a.getShopItem((BYTE)i);
        const SHOPLISTITEM_MYSTERIOUS right = b.getShopItem((BYTE)i);
        ASSERT_EQ(left.bExist, right.bExist);
        if (!left.bExist)
            continue;
        EXPECT_EQ((int)left.itemClass, (int)right.itemClass);
        EXPECT_EQ((int)left.itemType, (int)right.itemType);
    }
}

STORE_PACKET_TESTS(GCShopListMysterious)

void fillMysteriousEmpty(GCShopListMysterious& packet) {
    fillMysterious(packet, std::vector<MysteriousItemFixture>());
}

STORE_PACKET_VARIANT(GCShopListMysterious, empty, fillMysteriousEmpty)

//////////////////////////////////////////////////////////////////////
// What a purchase or a sale answers with.
//////////////////////////////////////////////////////////////////////

void fill(GCShopBought& packet) {
    packet.setObjectID(0x81A2B3C4);
    packet.setShopVersion(0x82A3B4C5);
    packet.setShopType(SHOP_RACK_SPECIAL);
    packet.setShopIndex(0x84);
    packet.setItemObjectID(0x85A6B7C8);
    packet.setItemClass(0x86);
    packet.setItemType(0x87A8);
    packet.addOptionType(0x89);
    packet.addOptionType(0x8A);
    packet.setDurability(0x8BACBDCE);
    packet.setSilver(0x8CAD);
    packet.setGrade(0x8DAEBFD0);
    packet.setEnchantLevel((EnchantLevel_t)0x8E);
}

void expectEqual(GCShopBought& a, GCShopBought& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getShopVersion(), b.getShopVersion());
    EXPECT_EQ((int)a.getShopType(), (int)b.getShopType());
    EXPECT_EQ((int)a.getShopIndex(), (int)b.getShopIndex());
    EXPECT_EQ(a.getItemObjectID(), b.getItemObjectID());
    EXPECT_EQ(a.getItemClass(), b.getItemClass());
    EXPECT_EQ((int)a.getItemType(), (int)b.getItemType());
    EXPECT_EQ(a.getOptionType(), b.getOptionType());
    EXPECT_EQ(a.getDurability(), b.getDurability());
    EXPECT_EQ((int)a.getSilver(), (int)b.getSilver());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    EXPECT_EQ((int)a.getEnchantLevel(), (int)b.getEnchantLevel());
}

STORE_PACKET_TESTS(GCShopBought)

void fillBoughtNoOptions(GCShopBought& packet) {
    fill(packet);
    packet.setOptionType(std::list<OptionType_t>());
}

STORE_PACKET_VARIANT(GCShopBought, nooptions, fillBoughtNoOptions)

void fill(GCShopBuyOK& packet) {
    packet.setObjectID(0x8FA0B1C2);
    packet.setShopVersion(0x90A1B2C3);
    packet.setItemObjectID(0x91A2B3C4);
    packet.setItemClass(0x92);
    packet.setItemType(0x93A4);
    packet.addOptionType(0x95);
    packet.addOptionType(0x96);
    packet.setDurability(0x97A8B9CA);
    packet.setItemNum(0x98);
    packet.setSilver(0x99AA);
    packet.setGrade(0x9AABBCCD);
    packet.setEnchantLevel((EnchantLevel_t)0x9B);
    packet.setPrice(0x9CADBECF);
}

void expectEqual(GCShopBuyOK& a, GCShopBuyOK& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getShopVersion(), b.getShopVersion());
    EXPECT_EQ(a.getItemObjectID(), b.getItemObjectID());
    EXPECT_EQ((int)a.getItemClass(), (int)b.getItemClass());
    EXPECT_EQ((int)a.getItemType(), (int)b.getItemType());
    EXPECT_EQ(a.getOptionType(), b.getOptionType());
    EXPECT_EQ(a.getDurability(), b.getDurability());
    EXPECT_EQ((int)a.getItemNum(), (int)b.getItemNum());
    EXPECT_EQ((int)a.getSilver(), (int)b.getSilver());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    EXPECT_EQ((int)a.getEnchantLevel(), (int)b.getEnchantLevel());
    EXPECT_EQ(a.getPrice(), b.getPrice());
}

STORE_PACKET_TESTS(GCShopBuyOK)

void fillBuyOKNoOptions(GCShopBuyOK& packet) {
    fill(packet);
    packet.setOptionType(std::list<OptionType_t>());
}

STORE_PACKET_VARIANT(GCShopBuyOK, nooptions, fillBuyOKNoOptions)

// The fail code selects the message the client shows, so it stays an
// enumerator.
void fill(GCShopBuyFail& packet) {
    packet.setObjectID(0x81A2B3C4);
    packet.setCode(GC_SHOP_BUY_FAIL_NOT_ENOUGH_CYAN_STAR);
    packet.setAmount(0x83A4B5C6);
}

void expectEqual(GCShopBuyFail& a, GCShopBuyFail& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ((int)a.getCode(), (int)b.getCode());
    EXPECT_EQ(a.getAmount(), b.getAmount());
}

TEST(GCShopBuyFailTest, roundTripsThroughLoopback) {
    GCShopBuyFail src;
    fill(src);
    GCShopBuyFail dst;
    roundTrip(src, dst, kPlainCode);
    expectEqual(src, dst);
}

TEST(GCShopBuyFailTest, bodyBytesMatchGolden) {
    GCShopBuyFail packet;
    fill(packet);
    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    expectGolden("GCShopBuyFail", kPlainCode, body);
    for (size_t i = 1; i < kEncryptCodeCount; i++)
        EXPECT_EQ(body, writeBody(packet, kEncryptCodes[i]))
            << "GCShopBuyFail now varies with the encrypt code - add per-code goldens";
}

void fill(GCShopSellOK& packet) {
    packet.setObjectID(0x84A5B6C7);
    packet.setShopVersion(0x85A6B7C8);
    packet.setItemObjectID(0x86A7B8C9);
    packet.setPrice(0x87A8B9CA);
}

void expectEqual(GCShopSellOK& a, GCShopSellOK& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getShopVersion(), b.getShopVersion());
    EXPECT_EQ(a.getItemObjectID(), b.getItemObjectID());
    EXPECT_EQ(a.getPrice(), b.getPrice());
}

STORE_PACKET_TESTS(GCShopSellOK)

void fill(GCShopSellFail& packet) {
    packet.setObjectID(0x88A9BACB);
}

void expectEqual(GCShopSellFail& a, GCShopSellFail& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
}

STORE_PACKET_TESTS(GCShopSellFail)

void fill(GCShopSold& packet) {
    packet.setObjectID(0x89AABBCC);
    packet.setShopVersion(0x8AABBCCD);
    packet.setShopType(SHOP_RACK_SPECIAL);
    packet.setShopIndex(0x8C);
}

void expectEqual(GCShopSold& a, GCShopSold& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getShopVersion(), b.getShopVersion());
    EXPECT_EQ((int)a.getShopType(), (int)b.getShopType());
    EXPECT_EQ((int)a.getShopIndex(), (int)b.getShopIndex());
}

STORE_PACKET_TESTS(GCShopSold)

// The rack type is the index of the version stamp being set, so it runs
// over the three racks rather than following the >= 128 rule.
void fill(GCShopVersion& packet) {
    packet.setObjectID(0x8DAEBFD0);
    packet.setVersion(SHOP_RACK_NORMAL, 0x8EAFC0D1);
    packet.setVersion(SHOP_RACK_SPECIAL, 0x8FA0B1C2);
    packet.setVersion(SHOP_RACK_MYSTERIOUS, 0x90A1B2C3);
    packet.setMarketCondSell(0x91A2);
}

void expectEqual(GCShopVersion& a, GCShopVersion& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    for (ShopRackType_t type = 0; type < SHOP_RACK_TYPE_MAX; type++)
        EXPECT_EQ(a.getVersion(type), b.getVersion(type)) << "rack type " << (int)type;
    EXPECT_EQ((int)a.getMarketCondSell(), (int)b.getMarketCondSell());
}

STORE_PACKET_TESTS(GCShopVersion)

void fill(GCShopMarketCondition& packet) {
    packet.setObjectID(0x92A3B4C5);
    packet.setMarketCondBuy(0x93A4);
    packet.setMarketCondSell(0x94A5);
}

void expectEqual(GCShopMarketCondition& a, GCShopMarketCondition& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ((int)a.getMarketCondBuy(), (int)b.getMarketCondBuy());
    EXPECT_EQ((int)a.getMarketCondSell(), (int)b.getMarketCondSell());
}

STORE_PACKET_TESTS(GCShopMarketCondition)

//////////////////////////////////////////////////////////////////////
// The stash.
//////////////////////////////////////////////////////////////////////

void fill(CGStashList& packet) {
    packet.setObjectID(0x81A2B3C4);
}

void expectEqual(CGStashList& a, CGStashList& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
}

STORE_PACKET_TESTS(CGStashList)

void fill(CGStashDeposit& packet) {
    packet.setAmount(0x82A3B4C5);
}

void expectEqual(CGStashDeposit& a, CGStashDeposit& b) {
    EXPECT_EQ(a.getAmount(), b.getAmount());
}

STORE_PACKET_TESTS(CGStashDeposit)

void fill(CGStashWithdraw& packet) {
    packet.setAmount(0x83A4B5C6);
}

void expectEqual(CGStashWithdraw& a, CGStashWithdraw& b) {
    EXPECT_EQ(a.getAmount(), b.getAmount());
}

STORE_PACKET_TESTS(CGStashWithdraw)

void fill(CGStashRequestBuy&) {}

void expectEqual(CGStashRequestBuy& a, CGStashRequestBuy& b) {
    EXPECT_EQ(0, (int)a.getPacketSize());
    EXPECT_EQ(0, (int)b.getPacketSize());
}

STORE_PACKET_TESTS(CGStashRequestBuy)

void fill(CGMouseToStash& packet) {
    packet.setObjectID(0x84A5B6C7);
    packet.setRack(0x85);
    packet.setIndex(0x86);
}

void expectEqual(CGMouseToStash& a, CGMouseToStash& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ((int)a.getRack(), (int)b.getRack());
    EXPECT_EQ((int)a.getIndex(), (int)b.getIndex());
}

STORE_PACKET_TESTS(CGMouseToStash)

void fill(CGStashToMouse& packet) {
    packet.setObjectID(0x87A8B9CA);
    packet.setRack(0x88);
    packet.setIndex(0x89);
}

void expectEqual(CGStashToMouse& a, CGStashToMouse& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ((int)a.getRack(), (int)b.getRack());
    EXPECT_EQ((int)a.getIndex(), (int)b.getIndex());
}

STORE_PACKET_TESTS(CGStashToMouse)

struct StashSubItemFixture {
    ObjectID_t objectID;
    BYTE itemClass;
    ItemType_t itemType;
    ItemNum_t num;
    SlotID_t slotID;
};

struct StashItemFixture {
    BYTE rack;
    BYTE index;
    ObjectID_t objectID;
    BYTE itemClass;
    ItemType_t itemType;
    std::vector<OptionType_t> options;
    Durability_t durability;
    ItemNum_t num;
    Silver_t silver;
    Grade_t grade;
    EnchantLevel_t enchantLevel;
    std::vector<StashSubItemFixture> subItems;
};

// The rack and slot bytes address m_pItems, so they stay below
// STASH_RACK_MAX and STASH_INDEX_MAX; every other field follows the
// >= 128 rule.
std::vector<StashItemFixture> canonicalStashItems() {
    std::vector<StashItemFixture> items;

    StashItemFixture first;
    first.rack = 0;
    first.index = 2;
    first.objectID = 0x81A2B3C4;
    first.itemClass = 0x85;
    first.itemType = 0x86A7;
    first.options.push_back(0x88);
    first.options.push_back(0x89);
    first.durability = 0x8AABBCCD;
    first.num = 0x8B;
    first.silver = 0x8CAD;
    first.grade = 0x8DAEBFD0;
    first.enchantLevel = (EnchantLevel_t)0x8E;

    StashSubItemFixture belt;
    belt.objectID = 0x8FA0B1C2;
    belt.itemClass = 0x90;
    belt.itemType = 0x91A2;
    belt.num = 0x93;
    belt.slotID = 0x94;
    first.subItems.push_back(belt);

    StashSubItemFixture belt2;
    belt2.objectID = 0x95A6B7C8;
    belt2.itemClass = 0x96;
    belt2.itemType = 0x97A8;
    belt2.num = 0x99;
    belt2.slotID = 0x9A;
    first.subItems.push_back(belt2);

    items.push_back(first);

    StashItemFixture second;
    second.rack = (BYTE)(STASH_RACK_MAX - 1);
    second.index = (BYTE)(STASH_INDEX_MAX - 1);
    second.objectID = 0x9BACBDCE;
    second.itemClass = 0x9C;
    second.itemType = 0x9DAE;
    second.options.push_back(0x9F);
    second.durability = 0xA0B1C2D3;
    second.num = 0xA1;
    second.silver = 0xA2B3;
    second.grade = 0xA3B4C5D6;
    second.enchantLevel = (EnchantLevel_t)0xA4;
    items.push_back(second);

    return items;
}

void emitStashList(SocketEncryptOutputStream& out, const std::vector<StashItemFixture>& items) {
    out.write((BYTE)0x81);
    out.write((BYTE)items.size());

    for (size_t i = 0; i < items.size(); i++) {
        const StashItemFixture& item = items[i];
        out.write(item.rack);
        out.write(item.index);
        out.write(item.objectID);
        out.write(item.itemClass);
        out.write(item.itemType);
        out.write((BYTE)item.options.size());
        for (size_t j = 0; j < item.options.size(); j++)
            out.write(item.options[j]);
        out.write(item.durability);
        out.write(item.num);
        out.write(item.silver);
        out.write(item.grade);
        out.write(item.enchantLevel);
        out.write((BYTE)item.subItems.size());
        for (size_t j = 0; j < item.subItems.size(); j++) {
            out.write(item.subItems[j].objectID);
            out.write(item.subItems[j].itemClass);
            out.write(item.subItems[j].itemType);
            out.write(item.subItems[j].num);
            out.write(item.subItems[j].slotID);
        }
    }

    out.write((Gold_t)0xA5B6C7D8);
}

void fillStashList(GCStashList& packet, const std::vector<StashItemFixture>& items) {
    readFields(packet, [&items](SocketEncryptOutputStream& out) { emitStashList(out, items); });
}

void fill(GCStashList& packet) {
    fillStashList(packet, canonicalStashItems());
}

void expectEqual(GCStashList& a, GCStashList& b) {
    EXPECT_EQ((int)a.getStashNum(), (int)b.getStashNum());
    EXPECT_EQ(a.getStashGold(), b.getStashGold());

    for (int r = 0; r < STASH_RACK_MAX; r++) {
        for (int i = 0; i < STASH_INDEX_MAX; i++) {
            SCOPED_TRACE(testing::Message() << "stash rack " << r << " slot " << i);
            ASSERT_EQ(a.isExist((BYTE)r, (BYTE)i), b.isExist((BYTE)r, (BYTE)i));
            if (!a.isExist((BYTE)r, (BYTE)i))
                continue;

            const STASHITEM left = a.getStashItem((BYTE)r, (BYTE)i);
            const STASHITEM right = b.getStashItem((BYTE)r, (BYTE)i);
            EXPECT_EQ(left.objectID, right.objectID);
            EXPECT_EQ((int)left.itemClass, (int)right.itemClass);
            EXPECT_EQ((int)left.itemType, (int)right.itemType);
            EXPECT_EQ(left.optionType, right.optionType);
            EXPECT_EQ(left.durability, right.durability);
            EXPECT_EQ((int)left.num, (int)right.num);
            EXPECT_EQ((int)left.silver, (int)right.silver);
            EXPECT_EQ(left.grade, right.grade);
            EXPECT_EQ((int)left.enchantLevel, (int)right.enchantLevel);

            ASSERT_EQ((int)a.getSubItemCount((BYTE)r, (BYTE)i), (int)b.getSubItemCount((BYTE)r, (BYTE)i));
            const std::list<SubItemInfo*>& leftBelt = a.getSubItems((BYTE)r, (BYTE)i);
            const std::list<SubItemInfo*>& rightBelt = b.getSubItems((BYTE)r, (BYTE)i);
            ASSERT_EQ(leftBelt.size(), rightBelt.size());
            std::list<SubItemInfo*>::const_iterator leftItr = leftBelt.begin();
            std::list<SubItemInfo*>::const_iterator rightItr = rightBelt.begin();
            for (; leftItr != leftBelt.end(); ++leftItr, ++rightItr) {
                EXPECT_EQ((*leftItr)->getObjectID(), (*rightItr)->getObjectID());
                EXPECT_EQ((int)(*leftItr)->getItemClass(), (int)(*rightItr)->getItemClass());
                EXPECT_EQ((int)(*leftItr)->getItemType(), (int)(*rightItr)->getItemType());
                EXPECT_EQ((int)(*leftItr)->getItemNum(), (int)(*rightItr)->getItemNum());
                EXPECT_EQ((int)(*leftItr)->getSlotID(), (int)(*rightItr)->getSlotID());
            }
        }
    }
}

STORE_PACKET_TESTS(GCStashList)

void fillStashListEmpty(GCStashList& packet) {
    fillStashList(packet, std::vector<StashItemFixture>());
}

STORE_PACKET_VARIANT(GCStashList, empty, fillStashListEmpty)

void fillStashListNoOptions(GCStashList& packet) {
    std::vector<StashItemFixture> items = canonicalStashItems();
    for (size_t i = 0; i < items.size(); i++) {
        items[i].options.clear();
        items[i].subItems.clear();
    }
    fillStashList(packet, items);
}

STORE_PACKET_VARIANT(GCStashList, nooptions, fillStashListNoOptions)

void fill(GCStashSell& packet) {
    packet.setPrice(0xA6B7C8D9);
}

void expectEqual(GCStashSell& a, GCStashSell& b) {
    EXPECT_EQ(a.getPrice(), b.getPrice());
}

STORE_PACKET_TESTS(GCStashSell)

//////////////////////////////////////////////////////////////////////
// The pet stash.
//////////////////////////////////////////////////////////////////////

void fill(CGDepositPet& packet) {
    packet.setObjectID(0x81A2B3C4);
    packet.setIndex(0x82);
}

void expectEqual(CGDepositPet& a, CGDepositPet& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ((int)a.getIndex(), (int)b.getIndex());
}

STORE_PACKET_TESTS(CGDepositPet)

void fill(CGWithdrawPet& packet) {
    packet.setObjectID(0x83A4B5C6);
    packet.setIndex(0x84);
}

void expectEqual(CGWithdrawPet& a, CGWithdrawPet& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ((int)a.getIndex(), (int)b.getIndex());
}

STORE_PACKET_TESTS(CGWithdrawPet)

// The answer code selects the message the client shows, so it stays an
// enumerator.
void fill(GCPetStashVerify& packet) {
    packet.setCode(PET_STASH_INVALID_INDEX);
}

void expectEqual(GCPetStashVerify& a, GCPetStashVerify& b) {
    EXPECT_EQ((int)a.getCode(), (int)b.getCode());
}

STORE_PACKET_TESTS(GCPetStashVerify)

PetStashItemInfo* newPetStashItem(MonsterType_t creature, DWORD keepDays) {
    PetStashItemInfo* pInfo = new PetStashItemInfo;
    pInfo->pPetInfo = new PetInfo;
    fillPetInfo(*pInfo->pPetInfo, creature);
    pInfo->KeepDays = keepDays;
    return pInfo;
}

void fill(GCPetStashList& packet) {
    packet.setCode(0x81);
    std::vector<PetStashItemInfo*>& slots = packet.getPetStashItemInfos();
    slots[0] = newPetStashItem(0x82A3, 0x84A5B6C7);
    slots[MAX_PET_STASH - 1] = newPetStashItem(0x85A6, 0x87A8B9CA);
}

void expectEqual(GCPetStashList& a, GCPetStashList& b) {
    std::vector<PetStashItemInfo*>& left = a.getPetStashItemInfos();
    std::vector<PetStashItemInfo*>& right = b.getPetStashItemInfos();
    ASSERT_EQ(left.size(), right.size());
    for (size_t i = 0; i < left.size(); i++) {
        SCOPED_TRACE(testing::Message() << "pet slot " << i);
        ASSERT_EQ(left[i] == NULL, right[i] == NULL);
        if (left[i] == NULL)
            continue;
        EXPECT_EQ(left[i]->KeepDays, right[i]->KeepDays);
        expectPetInfoEqual(*left[i]->pPetInfo, *right[i]->pPetInfo);
    }
}

STORE_PACKET_TESTS(GCPetStashList)

void fillPetStashEmpty(GCPetStashList& packet) {
    packet.setCode(0x82);
}

STORE_PACKET_VARIANT(GCPetStashList, empty, fillPetStashEmpty)

// A slot whose record holds no pet: PetInfo::write stops at the type
// byte.
void fillPetStashNone(GCPetStashList& packet) {
    PetStashItemInfo* pInfo = new PetStashItemInfo;
    pInfo->pPetInfo = new PetInfo;
    pInfo->KeepDays = 0x88A9BACB;
    packet.getPetStashItemInfos()[3] = pInfo;
}

STORE_PACKET_VARIANT(GCPetStashList, petnone, fillPetStashNone)

//////////////////////////////////////////////////////////////////////
// Findings.
//////////////////////////////////////////////////////////////////////

// FINDING, stated as a test that fails once it is fixed.
// GCShopBuyFail::getPacketSize() counts the NPC id alone while write()
// also emits the fail code and the four-byte amount, and the factory max
// is the same four bytes.
TEST(GCShopBuyFailTest, theDeclaredSizeOmitsTheCodeAndTheAmount) {
    GCShopBuyFail packet;
    fill(packet);
    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);

    EXPECT_EQ((size_t)szObjectID, (size_t)packet.getPacketSize())
        << "GCShopBuyFail: the declared size now counts more than the NPC id - drop this test";
    EXPECT_EQ((size_t)(szObjectID + szBYTE + szuint), body.size());
    EXPECT_LT((size_t)packet.getPacketSize(), body.size())
        << "GCShopBuyFail: the declared size now matches write() - pin the agreement instead";

    GCShopBuyFailFactory factory;
    EXPECT_GT(body.size(), (size_t)factory.getPacketMaxSize()) << "GCShopBuyFail: the body now fits the factory max";
}

// FINDING, stated as a test that fails once it is fixed.
// CGStoreSign derives the sign's length byte by hand and caps nothing:
// a sign past the 80 bytes its factory max budgets is emitted whole, and
// at 256 the length byte wraps to zero while the text still follows it.
TEST(CGStoreSignTest, theSignIsUncappedAndItsLengthByteWraps) {
    CGStoreSign overBudget;
    overBudget.setSign(std::string(MAX_SIGN_SIZE + 1, 's'));
    CGStoreSignFactory factory;
    EXPECT_GT(overBudget.getPacketSize(), factory.getPacketMaxSize())
        << "CGStoreSign: the sign now fits the factory max";

    CGStoreSign wrapped;
    wrapped.setSign(std::string(256, 'w'));
    const std::vector<unsigned char> body = writeBody(wrapped, kPlainCode);
    EXPECT_EQ(0, (int)body[0]) << "CGStoreSign: the sign is now capped - drop this test";
    EXPECT_EQ((size_t)szBYTE + 256, body.size());
}

// A stall filled to every budget its record's max size grants: twenty
// occupied slots, each with the 255 options and eight sub-items
// PCItemInfo::getMaxSize() reserves. Only the sign is left to set.
void fillMaximalStoreInfo(StoreInfo& info) {
    info.setOpen(1);

    std::list<OptionType_t> options;
    for (int i = 0; i < 255; i++)
        options.push_back((OptionType_t)(0x80 + (i & 0x7F)));

    for (size_t slot = 0; slot < info.getItems().size(); slot++) {
        StoreItemInfo& item = info.getItems()[slot];
        item.setItemExist(1);
        item.setPrice(0xA1B2C3D4);
        item.setOptionType(options);
        for (int i = 0; i < 8; i++)
            item.addListElement(new SubItemInfo());
    }
}

// FINDING, stated as a test that fails once it is fixed.
// StoreInfo::setSign caps nothing and StoreInfo::write bounds the sign at
// 255 while StoreInfo::getMaxSize budgets 80, so a full stall whose sign
// is longer than that outgrows the factory max by the overage.
// StoreOutlook, the sign's other carrier, cuts at 80 in its setter.
TEST(GCMyStoreInfoTest, aSignPastTheRecordBudgetOutgrowsTheFactoryMax) {
    MyStoreFixture f;
    fillMaximalStoreInfo(f.info);
    GCMyStoreInfoFactory factory;

    f.info.setSign(std::string(MAX_SIGN_SIZE, 's'));
    ASSERT_EQ(factory.getPacketMaxSize(), f.packet.getPacketSize())
        << "GCMyStoreInfo: a stall filled to every budget no longer hits the factory max exactly";

    f.info.setSign(std::string(MAX_SIGN_SIZE + 1, 's'));
    EXPECT_EQ((size_t)MAX_SIGN_SIZE + 1, f.info.getSign().size())
        << "StoreInfo::setSign now caps the sign - drop this test";
    EXPECT_GT(f.packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCMyStoreInfo: the sign now fits the factory max";

    StoreOutlook outlook;
    outlook.setOpen(1);
    outlook.setSign(std::string(MAX_SIGN_SIZE + 1, 's'));
    EXPECT_EQ((size_t)MAX_SIGN_SIZE, outlook.getSign().size());
}

// FINDING, stated as a test that fails once it is fixed.
// GCShopBought derives its option count from the list and caps nothing,
// so the 256th option wraps the count to zero and the body outgrows the
// factory max, which budgets 255.
TEST(GCShopBoughtTest, theOptionCountWrapsAndTheListOutgrowsTheFactoryMax) {
    GCShopBought packet;
    fill(packet);
    packet.setOptionType(std::list<OptionType_t>());
    for (int i = 0; i < 256; i++)
        packet.addOptionType((OptionType_t)(0x80 + (i & 0x7F)));

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    const size_t countOffset = szObjectID + szShopVersion + szShopRackType + szBYTE + szObjectID + szBYTE + szItemType;
    EXPECT_EQ(0, (int)body[countOffset]) << "GCShopBought: the option list is now capped - drop this test";

    GCShopBoughtFactory factory;
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCShopBought: the option list now fits the factory max";
}

// FINDING, stated as a test that fails once it is fixed. The same shape
// in GCShopBuyOK.
TEST(GCShopBuyOKTest, theOptionCountWrapsAndTheListOutgrowsTheFactoryMax) {
    GCShopBuyOK packet;
    fill(packet);
    packet.setOptionType(std::list<OptionType_t>());
    for (int i = 0; i < 256; i++)
        packet.addOptionType((OptionType_t)(0x80 + (i & 0x7F)));

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    const size_t countOffset = szObjectID + szShopVersion + szObjectID + szBYTE + szItemType;
    EXPECT_EQ(0, (int)body[countOffset]) << "GCShopBuyOK: the option list is now capped - drop this test";

    GCShopBuyOKFactory factory;
    EXPECT_GT(packet.getPacketSize(), factory.getPacketMaxSize())
        << "GCShopBuyOK: the option list now fits the factory max";
}

// FINDING, stated as a test that fails once it is fixed.
// The four packets that carry an option list read it into whatever the
// packet already holds instead of replacing it, so a reused packet grows
// by one listing per read.
TEST(GCShopBoughtTest, aSecondReadAppendsToTheOptionListItAlreadyHolds) {
    GCShopBought src;
    fill(src);

    GCShopBought dst;
    roundTrip(src, dst, kPlainCode);
    ASSERT_EQ(2u, dst.getOptionType().size());

    GCShopBought second;
    fill(second);
    roundTrip(second, dst, kPlainCode);
    EXPECT_EQ(4u, dst.getOptionType().size())
        << "GCShopBought::read() now replaces the option list - drop this test and pin the replacement";
}

TEST(GCShopBuyOKTest, aSecondReadAppendsToTheOptionListItAlreadyHolds) {
    GCShopBuyOK src;
    fill(src);

    GCShopBuyOK dst;
    roundTrip(src, dst, kPlainCode);
    ASSERT_EQ(2u, dst.getOptionType().size());

    GCShopBuyOK second;
    fill(second);
    roundTrip(second, dst, kPlainCode);
    EXPECT_EQ(4u, dst.getOptionType().size())
        << "GCShopBuyOK::read() now replaces the option list - drop this test and pin the replacement";
}

TEST(GCShopListTest, aSecondReadAppendsToTheOptionListItAlreadyHolds) {
    GCShopList src;
    fill(src);

    GCShopList dst;
    roundTrip(src, dst, kPlainCode);
    ASSERT_EQ(2u, dst.getShopItem(0).optionType.size());

    GCShopList second;
    fill(second);
    roundTrip(second, dst, kPlainCode);
    EXPECT_EQ(4u, dst.getShopItem(0).optionType.size())
        << "GCShopList::read() now replaces the option list - drop this test and pin the replacement";
}

TEST(GCStashListTest, aSecondReadAppendsToTheOptionListItAlreadyHolds) {
    GCStashList src;
    fill(src);

    GCStashList dst;
    roundTrip(src, dst, kPlainCode);
    ASSERT_EQ(2u, dst.getStashItem(0, 2).optionType.size());

    GCStashList second;
    fill(second);
    roundTrip(second, dst, kPlainCode);
    EXPECT_EQ(4u, dst.getStashItem(0, 2).optionType.size())
        << "GCStashList::read() now replaces the option list - drop this test and pin the replacement";
}

// FINDING, stated as a test that fails once it is fixed.
// GCPetStashList never puts its code byte on the wire although both pet
// handlers set it and its factory max budgets a byte for it, so the
// answer the sender chose is dropped and every body is one byte under
// the declared maximum.
TEST(GCPetStashListTest, theCodeNeverReachesTheWire) {
    GCPetStashList quiet;
    fill(quiet);
    quiet.setCode(0);

    GCPetStashList loud;
    fill(loud);
    loud.setCode(0xFF);

    EXPECT_EQ(writeBody(quiet, kPlainCode), writeBody(loud, kPlainCode))
        << "GCPetStashList: the code now reaches the wire - drop this test and pin it";

    GCPetStashList dst;
    roundTrip(loud, dst, kPlainCode);
    EXPECT_NE(0xFF, (int)dst.getCode()) << "GCPetStashList: the code now survives a round trip";

    GCPetStashListFactory factory;
    EXPECT_EQ((PacketSize_t)(szBYTE + PetStashItemInfo::getPacketMaxSize() * MAX_PET_STASH), factory.getPacketMaxSize())
        << "GCPetStashList: the factory max no longer budgets a byte the body never carries";
}

// FINDING, stated as a test that fails once it is fixed.
// GCStashList keeps each slot's sub-item count in an array of its own
// while the records come from a separate list, so a sub-item added
// through getSubItems() leaves the count behind: write() emits the stale
// count and then the whole list, and getPacketSize() counts the stale
// one.
TEST(GCStashListTest, theSubItemCountIsMaintainedByHand) {
    GCStashList packet;
    fill(packet);
    ASSERT_EQ(2, (int)packet.getSubItemCount(0, 2));

    SubItemInfo* pExtra = new SubItemInfo();
    pExtra->setObjectID(0xB1C2D3E4);
    pExtra->setItemClass(0xB2);
    pExtra->setItemType(0xB3C4);
    pExtra->setItemNum(0xB5);
    pExtra->setSlotID(0xB6);
    packet.getSubItems(0, 2).push_back(pExtra);

    EXPECT_EQ(2, (int)packet.getSubItemCount(0, 2))
        << "GCStashList: the sub-item count now follows the list - drop this test";
    EXPECT_LT((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size())
        << "GCStashList: the declared size now covers the whole sub-item list";
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

TEST(StoreConstructorTest, thePacketsThatInitialiseEveryMemberTheyWrite) {
    // CGStoreSign qualifies because its one member is a string, which
    // is empty however the storage around it is poisoned.
    expectEveryMemberIsInitialised<CGStoreSign>("CGStoreSign");
    expectEveryMemberIsInitialised<CGShopRequestSell>("CGShopRequestSell");
    expectEveryMemberIsInitialised<GCShopBuyOK>("GCShopBuyOK");
    expectEveryMemberIsInitialised<GCShopBuyFail>("GCShopBuyFail");
    expectEveryMemberIsInitialised<GCPetStashVerify>("GCPetStashVerify");
    // GCPetStashList only qualifies because the one member its
    // constructor leaves alone, the code byte, never reaches the wire.
    expectEveryMemberIsInitialised<GCPetStashList>("GCPetStashList");
}

TEST(StoreConstructorTest, thePacketsThatDoNot) {
    expectAMemberIsLeftUninitialised<CGDisplayItem>("CGDisplayItem");
    expectAMemberIsLeftUninitialised<CGUndisplayItem>("CGUndisplayItem");
    expectAMemberIsLeftUninitialised<CGBuyStoreItem>("CGBuyStoreItem");
    expectAMemberIsLeftUninitialised<CGRequestStoreInfo>("CGRequestStoreInfo");
    expectAMemberIsLeftUninitialised<CGShopRequestList>("CGShopRequestList");
    expectAMemberIsLeftUninitialised<CGShopRequestBuy>("CGShopRequestBuy");
    expectAMemberIsLeftUninitialised<CGStashList>("CGStashList");
    expectAMemberIsLeftUninitialised<CGStashDeposit>("CGStashDeposit");
    expectAMemberIsLeftUninitialised<CGStashWithdraw>("CGStashWithdraw");
    expectAMemberIsLeftUninitialised<CGMouseToStash>("CGMouseToStash");
    expectAMemberIsLeftUninitialised<CGStashToMouse>("CGStashToMouse");
    expectAMemberIsLeftUninitialised<CGDepositPet>("CGDepositPet");
    expectAMemberIsLeftUninitialised<CGWithdrawPet>("CGWithdrawPet");
    expectAMemberIsLeftUninitialised<GCAddStoreItem>("GCAddStoreItem");
    expectAMemberIsLeftUninitialised<GCRemoveStoreItem>("GCRemoveStoreItem");
    expectAMemberIsLeftUninitialised<GCShopList>("GCShopList");
    expectAMemberIsLeftUninitialised<GCShopListMysterious>("GCShopListMysterious");
    expectAMemberIsLeftUninitialised<GCShopBought>("GCShopBought");
    expectAMemberIsLeftUninitialised<GCShopSellOK>("GCShopSellOK");
    expectAMemberIsLeftUninitialised<GCShopSellFail>("GCShopSellFail");
    expectAMemberIsLeftUninitialised<GCShopSold>("GCShopSold");
    expectAMemberIsLeftUninitialised<GCShopVersion>("GCShopVersion");
    expectAMemberIsLeftUninitialised<GCShopMarketCondition>("GCShopMarketCondition");
    expectAMemberIsLeftUninitialised<GCStashList>("GCStashList");
    expectAMemberIsLeftUninitialised<GCStashSell>("GCStashSell");
}

// CGStoreOpen, CGStoreClose and CGStashRequestBuy are in neither list:
// their body is empty, so there is nothing a constructor could leave.
TEST(StoreConstructorTest, theBodylessRequestsWriteNothingAtAll) {
    EXPECT_TRUE(bodyOverPoison<CGStoreOpen>(0xFF).empty());
    EXPECT_TRUE(bodyOverPoison<CGStoreClose>(0xFF).empty());
    EXPECT_TRUE(bodyOverPoison<CGStashRequestBuy>(0xFF).empty());
}

// FINDING, stated as a test that fails once it is fixed.
// GCMyStoreInfo and GCOtherStoreInfo are in neither list because writing
// one over poisoned storage would follow the record pointer their
// constructors leave alone. The pointer itself is pinned instead: both
// getPacketSize() and write() dereference it, so a sender that skips
// setStoreInfo follows an indeterminate value.
template <typename PacketType> void expectTheStorePointerIsLeftUninitialised(const char* what) {
    alignas(PacketType) unsigned char zeroed[sizeof(PacketType)];
    memset(zeroed, 0x00, sizeof(zeroed));
    PacketType* pZeroed = new (zeroed) PacketType();
    StoreInfo* pFromZero = pZeroed->getStoreInfo();
    pZeroed->~PacketType();

    alignas(PacketType) unsigned char poisoned[sizeof(PacketType)];
    memset(poisoned, 0xFF, sizeof(poisoned));
    PacketType* pPoisoned = new (poisoned) PacketType();
    StoreInfo* pFromPoison = pPoisoned->getStoreInfo();
    pPoisoned->~PacketType();

    EXPECT_NE(pFromZero, pFromPoison)
        << what << ": its constructor now initialises the record pointer - move it to the initialised list";
}

TEST(StoreConstructorTest, theStoreInfoPointerIsLeftUninitialised) {
    expectTheStorePointerIsLeftUninitialised<GCMyStoreInfo>("GCMyStoreInfo");
    expectTheStorePointerIsLeftUninitialised<GCOtherStoreInfo>("GCOtherStoreInfo");
}

} // namespace
