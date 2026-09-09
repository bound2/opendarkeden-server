//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_trade_test.cpp
// Description : Golden byte fixtures, loopback round trips and size
//               pins for the player-to-player trade protocol: every CG
//               and GC packet exchanged between the moment one player
//               offers a trade and the moment both sides accept it.
//               Twelve packets, each with the reason it is here:
//
//               CGTradePrepare   the offer and every answer to it:
//                                request, cancel, accept, reject and
//                                busy, next to the other player's
//                                object id.
//               GCTradePrepare   the server's half of the same
//                                exchange, forwarded to the other side.
//               CGTradeAddItem   the client putting one item of its
//                                inventory on the table.
//               GCTradeAddItem   what the other side is shown for that
//                                item: where it sits on the table, its
//                                class and type, its option list, its
//                                durability, count, silver coating,
//                                grade and enchant level, and the
//                                sub-items slotted into it.
//               CGTradeRemoveItem  the client taking one item back off
//                                the table.
//               GCTradeRemoveItem  the same removal shown to the other
//                                side.
//               CGTradeMoney     the client raising or lowering the
//                                money it is offering.
//               GCTradeMoney     that amount shown to the other side,
//                                and the result codes that answer the
//                                client's own change.
//               CGTradeFinish    the client accepting, rejecting or
//                                reopening the finished trade.
//               GCTradeFinish    the same three codes forwarded, plus
//                                the execute code that says the trade
//                                went through.
//               GCTradeVerify    the single-byte acknowledgement the
//                                server sends for each step the client
//                                took, so the client can undo the step
//                                it drew optimistically.
//               GCTradeError     the refusal that answers a step the
//                                server will not take: a missing
//                                target, a different race, an unsafe
//                                zone, a motorcycle, a bat or wolf
//                                form, a double offer, no trade in
//                                progress, an item or money change that
//                                did not apply, no room for the goods,
//                                and the event gift box.
//
//               Deliberately excluded: CGExchangeBuy, CGExchangeList,
//               GCExchangeBuy and GCExchangeList, the offline
//               marketplace rather than a face-to-face trade, are
//               already pinned by tests/packet_exchange_test.cpp.
//
//               No packet in this file calls readEncrypt/writeEncrypt,
//               so the goldens are recorded at encrypt code 0 only, and
//               every golden test also asserts the bytes do not vary
//               with the code, so adopting the encrypter fails loudly
//               instead of silently voiding the pin.
//
//               Each packet gets three pins (TRADE_PACKET_TESTS):
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
//               GCTradeAddItem carries two optional lists, and a second
//               golden covers the branch one fixture cannot:
//               GCTradeAddItem.bare has neither an option nor a
//               sub-item, against a canonical fixture that has two of
//               each.
//
//               Fixture values are distinct per field and >= 128 in
//               every byte the width allows, except the code bytes,
//               which hold a handful of small enumerators.
//
//               Two pins cover GCTradeAddItem's lists, in
//               theSubItemCountIsTheListItself and
//               listsPastTheFactoryBudgetAreRefused: the sub-item count
//               on the wire is the list itself, and both lists stop at
//               the 255 options and eight sub-items the factory max
//               budgets.
//
//               Not expressible as a test: every packet in this file
//               leaves its scalar members uninitialised except
//               GCTradeAddItem, which initialises only its sub-item
//               count and its grade. A packet that is written without
//               every setter being called puts whatever the allocation
//               held on the wire, and no test can pin that value.
//
//////////////////////////////////////////////////////////////////////

#include <list>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "CGTradeAddItem.h"
#include "CGTradeFinish.h"
#include "CGTradeMoney.h"
#include "CGTradePrepare.h"
#include "CGTradeRemoveItem.h"
#include "GCTradeAddItem.h"
#include "GCTradeError.h"
#include "GCTradeFinish.h"
#include "GCTradeMoney.h"
#include "GCTradePrepare.h"
#include "GCTradeRemoveItem.h"
#include "GCTradeVerify.h"
#include "SubItemInfo.h"
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

#define TRADE_PACKET_TESTS(Name)                                                                     \
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
#define TRADE_PACKET_VARIANT(Name, Variant, fillVariant)                       \
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
// Opening and closing the trade.
//////////////////////////////////////////////////////////////////////

void fill(CGTradePrepare& packet) {
    packet.setTargetObjectID(0x8A9BACBD);
    // The code is an enum byte with five enumerators, so it carries a
    // valid one rather than a high byte.
    packet.setCode(CG_TRADE_PREPARE_CODE_ACCEPT);
}

void expectEqual(const CGTradePrepare& a, const CGTradePrepare& b) {
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getCode(), b.getCode());
}

TRADE_PACKET_TESTS(CGTradePrepare)

void fill(GCTradePrepare& packet) {
    packet.setTargetObjectID(0x8C9DAEBF);
    packet.setCode(GC_TRADE_PREPARE_CODE_BUSY);
}

void expectEqual(const GCTradePrepare& a, const GCTradePrepare& b) {
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getCode(), b.getCode());
}

TRADE_PACKET_TESTS(GCTradePrepare)

void fill(CGTradeFinish& packet) {
    packet.setTargetObjectID(0x8E9FB0C1);
    packet.setCode(CG_TRADE_FINISH_RECONSIDER);
}

void expectEqual(const CGTradeFinish& a, const CGTradeFinish& b) {
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getCode(), b.getCode());
}

TRADE_PACKET_TESTS(CGTradeFinish)

void fill(GCTradeFinish& packet) {
    packet.setTargetObjectID(0x90A1B2C3);
    packet.setCode(GC_TRADE_FINISH_EXECUTE);
}

void expectEqual(const GCTradeFinish& a, const GCTradeFinish& b) {
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getCode(), b.getCode());
}

TRADE_PACKET_TESTS(GCTradeFinish)

void fill(GCTradeVerify& packet) {
    packet.setCode(GC_TRADE_VERIFY_CODE_ADD_ITEM_FAIL);
}

void expectEqual(const GCTradeVerify& a, const GCTradeVerify& b) {
    EXPECT_EQ(a.getCode(), b.getCode());
}

TRADE_PACKET_TESTS(GCTradeVerify)

void fill(GCTradeError& packet) {
    packet.setTargetObjectID(0x92A3B4C5);
    packet.setCode(GC_TRADE_ERROR_CODE_NOT_ENOUGH_SPACE);
}

void expectEqual(const GCTradeError& a, const GCTradeError& b) {
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getCode(), b.getCode());
}

TRADE_PACKET_TESTS(GCTradeError)

//////////////////////////////////////////////////////////////////////
// Money.
//////////////////////////////////////////////////////////////////////

void fill(CGTradeMoney& packet) {
    packet.setTargetObjectID(0x94A5B6C7);
    packet.setAmount(0x96A7B8C9);
    packet.setCode(CG_TRADE_MONEY_DECREASE);
}

void expectEqual(const CGTradeMoney& a, const CGTradeMoney& b) {
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getAmount(), b.getAmount());
    EXPECT_EQ(a.getCode(), b.getCode());
}

TRADE_PACKET_TESTS(CGTradeMoney)

void fill(GCTradeMoney& packet) {
    packet.setTargetObjectID(0x98A9BACB);
    packet.setAmount(0x9AABBCCD);
    packet.setCode(GC_TRADE_MONEY_INCREASE_RESULT);
}

void expectEqual(const GCTradeMoney& a, const GCTradeMoney& b) {
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getAmount(), b.getAmount());
    EXPECT_EQ(a.getCode(), b.getCode());
}

TRADE_PACKET_TESTS(GCTradeMoney)

//////////////////////////////////////////////////////////////////////
// Items on the table.
//////////////////////////////////////////////////////////////////////

void fill(CGTradeAddItem& packet) {
    packet.setTargetObjectID(0x9CADBECF);
    packet.setItemObjectID(0x9EAFC0D1);
}

void expectEqual(const CGTradeAddItem& a, const CGTradeAddItem& b) {
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getItemObjectID(), b.getItemObjectID());
}

TRADE_PACKET_TESTS(CGTradeAddItem)

void fill(CGTradeRemoveItem& packet) {
    packet.setTargetObjectID(0xA0B1C2D3);
    packet.setItemObjectID(0xA2B3C4D5);
}

void expectEqual(const CGTradeRemoveItem& a, const CGTradeRemoveItem& b) {
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getItemObjectID(), b.getItemObjectID());
}

TRADE_PACKET_TESTS(CGTradeRemoveItem)

void fill(GCTradeRemoveItem& packet) {
    packet.setTargetObjectID(0xA4B5C6D7);
    packet.setItemObjectID(0xA6B7C8D9);
}

void expectEqual(const GCTradeRemoveItem& a, const GCTradeRemoveItem& b) {
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getItemObjectID(), b.getItemObjectID());
}

TRADE_PACKET_TESTS(GCTradeRemoveItem)

// GCTradeAddItem takes ownership of every sub-item it is handed, so each
// fixture allocates fresh ones.
SubItemInfo* makeSubItemInfo(int index) {
    SubItemInfo* pInfo = new SubItemInfo();
    pInfo->setObjectID((ObjectID_t)(0xA8B9CADB + index * 0x01010101));
    pInfo->setItemClass((BYTE)(0x8D + index));
    pInfo->setItemType((ItemType_t)(0x8EAF + index * 0x0101));
    pInfo->setItemNum((ItemNum_t)(0x90 + index));
    pInfo->setSlotID((SlotID_t)(0x92 + index));
    return pInfo;
}

// The item's own fields, shared by the canonical fixture and the one
// with neither list.
void fillTradeItem(GCTradeAddItem& packet) {
    packet.setTargetObjectID(0xAABBCCDD);
    packet.setX(0x81);
    packet.setY(0x83);
    packet.setItemObjectID(0xACBDCEDF);
    packet.setItemClass(0x85);
    packet.setItemType(0x87A8);
    packet.setDurability(0xAEBFD0E1);
    packet.setItemNum(0x89);
    packet.setSilver(0x8BAC);
    packet.setGrade(0xB0C1D2E3);
    packet.setEnchantLevel((EnchantLevel_t)0x8F);
}

void fill(GCTradeAddItem& packet) {
    fillTradeItem(packet);

    packet.addOptionType((OptionType_t)0x94);
    packet.addOptionType((OptionType_t)0x96);

    packet.addListElement(makeSubItemInfo(0));
    packet.addListElement(makeSubItemInfo(1));
}

// Neither list: the item was handed over with no option and nothing
// slotted into it.
void fillBare(GCTradeAddItem& packet) {
    fillTradeItem(packet);
}

void expectSubItemEqual(SubItemInfo* a, SubItemInfo* b, int index) {
    ASSERT_TRUE(a != NULL);
    ASSERT_TRUE(b != NULL);
    EXPECT_EQ(a->getObjectID(), b->getObjectID()) << "sub-item " << index;
    EXPECT_EQ(a->getItemClass(), b->getItemClass()) << "sub-item " << index;
    EXPECT_EQ(a->getItemType(), b->getItemType()) << "sub-item " << index;
    EXPECT_EQ(a->getItemNum(), b->getItemNum()) << "sub-item " << index;
    EXPECT_EQ(a->getSlotID(), b->getSlotID()) << "sub-item " << index;
}

void expectEqual(GCTradeAddItem& a, GCTradeAddItem& b) {
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getItemObjectID(), b.getItemObjectID());
    EXPECT_EQ(a.getItemClass(), b.getItemClass());
    EXPECT_EQ(a.getItemType(), b.getItemType());
    EXPECT_EQ(a.getOptionType(), b.getOptionType());
    EXPECT_EQ(a.getDurability(), b.getDurability());
    EXPECT_EQ(a.getItemNum(), b.getItemNum());
    EXPECT_EQ(a.getSilver(), b.getSilver());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    EXPECT_EQ(a.getEnchantLevel(), b.getEnchantLevel());

    ASSERT_EQ(a.getListNum(), b.getListNum());

    // The sub-items are reachable only through the destructive
    // popListElement(), which hands the record over; nothing reads the
    // packets afterwards.
    const int records = (int)a.getListNum();
    for (int i = 0; i < records; i++) {
        SubItemInfo* pLeft = a.popListElement();
        SubItemInfo* pRight = b.popListElement();
        expectSubItemEqual(pLeft, pRight, i);
        delete pLeft;
        delete pRight;
    }
}

TRADE_PACKET_TESTS(GCTradeAddItem)
TRADE_PACKET_VARIANT(GCTradeAddItem, bare, fillBare)

//////////////////////////////////////////////////////////////////////
// The counts and the bounds.
//////////////////////////////////////////////////////////////////////

// The sub-item count on the wire is the list itself, so the size the
// packet declares and the count it announces move with the records
// write() emits. This is the shape InventoryInfo, GearInfo, ExtraInfo
// and RideMotorcycleInfo already have.
TEST(GCTradeAddItemTest, theSubItemCountIsTheListItself) {
    GCTradeAddItem packet;
    fillTradeItem(packet);

    const PacketSize_t empty = packet.getPacketSize();
    EXPECT_EQ(0, (int)packet.getListNum());

    packet.addListElement(makeSubItemInfo(0));
    packet.addListElement(makeSubItemInfo(1));

    EXPECT_EQ(2, (int)packet.getListNum());
    EXPECT_EQ(empty + 2 * SubItemInfo::getSize(), packet.getPacketSize());

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());
}

// Both lists stop where the factory max does: 255 options and the eight
// sub-items a belt or an armsband holds.
TEST(GCTradeAddItemTest, listsPastTheFactoryBudgetAreRefused) {
    GCTradeAddItem packet;
    fillTradeItem(packet);
    for (uint i = 0; i < GCTradeAddItem::kMaxOptionTypes; i++)
        packet.addOptionType((OptionType_t)(0x80 + (i % 0x80)));
    for (uint i = 0; i < GCTradeAddItem::kMaxSubItems; i++)
        packet.addListElement(makeSubItemInfo(i));

    GCTradeAddItemFactory factory;
    EXPECT_EQ(factory.getPacketMaxSize(), packet.getPacketSize())
        << "a full option list and a full belt are exactly the factory max";
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());

    EXPECT_THROW(packet.addOptionType((OptionType_t)0x99), InvalidProtocolException);
    EXPECT_THROW(packet.addListElement(makeSubItemInfo(9)), InvalidProtocolException);
    EXPECT_EQ((int)GCTradeAddItem::kMaxSubItems, (int)packet.getListNum());
    EXPECT_EQ(factory.getPacketMaxSize(), packet.getPacketSize());
}

} // namespace
