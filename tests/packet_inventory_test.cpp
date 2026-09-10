//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_inventory_test.cpp
// Description : Golden byte fixtures, loopback round trips and size
//               pins for the packets a client and a game server
//               exchange to move, use, make, repair and throw an item:
//               the requests the item handlers read and the answers
//               they build.
//
//               The set is taken from the code that sends them: the
//               item handlers under src/server/gameserver/handler
//               (CGAddGearToMouse, CGAddInventoryToMouse,
//               CGAddMouseToGear, CGAddMouseToInventory, the two
//               quick-slot movers, CGAddItemToItem,
//               CGAddItemToCodeSheet, CGMixItem, CGMakeItem,
//               CGThrowItem, CGThrowBomb, the two reload handlers,
//               CGUsePotionFromQuickSlot,
//               CGUseItemFromGQuestInventory,
//               CGUseMessageItemFromInventory, CGRequestRepair,
//               CGGetEventItem, CGRequestNewbieItem) and the GC
//               packets those handlers construct, plus
//               makeGCCreateItem in PacketUtil.cpp and the time-limit
//               sweep in PlayerCreature.cpp. Thirty-seven packets, each with
//               the reason it is here:
//
//               CGAddGearToMouse      the client picks a worn item off
//                                     a gear slot onto the cursor.
//               CGAddMouseToGear      and puts the cursor's item back
//                                     on a gear slot.
//               CGAddInventoryToMouse the same pair for the inventory
//               CGAddMouseToInventory grid, which takes a cell rather
//                                     than a slot id.
//               CGAddMouseToQuickSlot the same pair for the quick-slot
//               CGAddQuickSlotToMouse bar.
//               CGAddItemToItem       the client drops one item onto
//                                     another: enchant, mix, detach.
//               CGAddItemToCodeSheet  the same gesture aimed at a code
//                                     sheet.
//               CGMixItem             the three-item form of it, which
//                                     carries the two targets.
//               CGMakeItem            the client asks to craft a class
//                                     and type.
//               CGThrowItem           the client throws an inventory
//                                     item at a creature.
//               CGThrowBomb           the client sets off a bomb at a
//                                     zone tile.
//               CGReloadFromInventory the client reloads a gun from an
//               CGReloadFromQuickSlot magazine in the inventory or on
//                                     the quick-slot bar.
//               CGUsePotionFromQuickSlot  drinking from the bar.
//               CGUseItemFromGQuestInventory  using an item out of the
//                                     guild-quest inventory, which is
//                                     addressed by index.
//               CGUseMessageItemFromInventory  using an item that
//                                     carries a written message; it
//                                     extends CGUseItemFromInventory,
//                                     so its body rides the encrypter.
//               CGRequestRepair       the client asks an NPC what a
//                                     repair costs.
//               CGGetEventItem        the client claims an event item.
//               CGRequestNewbieItem   and the starting kit for a class.
//
//               GCCannotAdd      the refusal every mouse/gear/inventory
//                                mover sends when the item may not go
//                                where the client put it.
//               GCCreateItem     the item record makeGCCreateItem
//                                fills: the packet that puts one item
//                                into the client's inventory grid.
//               GCDeleteInventoryItem  its counterpart, sent whenever
//                                an item leaves the grid.
//               GCDeleteandPickUpOK  the acknowledgement the pickup and
//                                drop handlers send the actor.
//               GCUseOK          what every use handler sends when the
//                                item worked: the stat record and
//                                nothing else.
//               GCAddItemToItemVerify  the result of dropping one item
//                                on another, with the parameters the
//                                result code selects.
//               GCReloadOK       the rounds left after a reload.
//               GCRemoveFromGear the broadcast that a gear slot is now
//                                empty.
//               GCAddGearToInventory  where a removed gear item landed
//                                in the grid.
//               GCAddGearToZone  the gear slot emptied onto the ground.
//               GCMakeItemOK     what CGMakeItem answers on success:
//                                the changed material counts, the new
//                                item and the stat record.
//               GCMakeItemFail   the same minus the new item.
//               GCThrowItemOK1   the thrower's copy of a thrown item,
//               GCThrowItemOK2   the target's copy with its stat
//               GCThrowItemOK3   changes, and the observers' copy.
//                                observers'.
//               GCGQuestInventory  the guild-quest inventory listing
//                                the index packet addresses.
//               GCTimeLimitItemInfo  the remaining life of every
//                                time-limited item the character
//                                carries, sent as it enters the world.
//
//               Deliberately excluded:
//
//               The encrypter half of the family is pinned at encrypt
//               codes 0..5 by tests/packet_encrypter_test.cpp:
//               CGAddZoneToInventory, CGAddZoneToMouse,
//               CGAddMouseToZone, CGUseItemFromInventory,
//               CGUseItemFromGear, CGUsePotionFromInventory,
//               CGSkillToInventory, CGDropMoney, CGPickupMoney,
//               GCAddNewItemToZone, GCDropItemToZone,
//               GCAddInstalledMineToZone, and the abstract
//               GCAddItemToZone through those three. Those are the
//               item-family entries in the nineteen src/Core sources
//               that call readEncrypt or writeEncrypt, which is the
//               whole list, so no encrypter user in this family is left
//               unpinned;
//               CGUseMessageItemFromInventory is the one packet that
//               inherits the encrypter without calling it, and it is
//               pinned here at the same six codes.
//
//               Already pinned elsewhere: GCCannotUse
//               (packet_movement_test.cpp), GCModifyInformation,
//               GCSkillFailed1/2, GCStatusCurrentHP and the ModifyInfo
//               record itself (packet_combat_test.cpp), GCDeleteObject,
//               GCAddEffect, GCAddEffectToTile and the StoreInfo record
//               (packet_zone_scan_test.cpp), GCNoticeEvent
//               (packet_quest_war_test.cpp), the
//               PCItemInfo / InventoryInfo / GearInfo / ExtraInfo
//               records (packet_gameserver_handshake_test.cpp),
//               SubItemInfo (there and in packet_trade_test.cpp),
//               GCTradeVerify (packet_trade_test.cpp), GCNPCResponse
//               (packet_guild_test.cpp).
//
//               GCModifyMoney (in GCModyfyMoney.h) and
//               GCSubInventoryInfo have no factory in
//               tests/ratchet/factory_registrations.txt and no source
//               outside src/Core mentions them: no id, no sender.
//               GCAddItemToInventory and GCChangeInventoryItemNum have
//               no packet id either - they are plain records, and they
//               reach the wire only through GCMakeItemOK and
//               GCMakeItemFail, which is where they are pinned.
//
//               CGUseBonusPoint, CGUsePowerPoint and their three
//               answers spend character points, not items.
//               CGDonationMoney hands money to an NPC and is answered
//               with GCNPCResponse. The store, shop, stash and pet-stash
//               dialogue (CGStoreOpen/Close/Sign, CGDisplayItem,
//               CGUndisplayItem, CGBuyStoreItem, CGRequestStoreInfo,
//               the CGShop* and CGStash* requests, CGMouseToStash,
//               CGStashToMouse and the GCStore*/GCShop*/GCStash*/
//               GCPetStash* replies) is a second family with its own
//               handlers and is not pinned here.
//
//               Only CGUseMessageItemFromInventory varies with the
//               encrypt code, so every other golden is recorded at code
//               0 and its test also asserts the bytes do not vary with
//               the code: adopting the encrypter fails loudly instead
//               of silently voiding the pin.
//
//               Each packet gets three pins (INVENTORY_PACKET_TESTS):
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
//               Extra goldens cover the branches one fixture cannot:
//               GCCreateItem writes an item with no options in
//               .nooptions, GCMakeItemOK the same in .nooptions, the
//               empty-list shapes are .empty (GCUseOK,
//               GCThrowItemOK2, GCGQuestInventory,
//               GCTimeLimitItemInfo, GCMakeItemFail), and
//               GCAddItemToItemVerify takes its parameterless branch in
//               .error and its two-parameter branch in .threeenchant.
//
//               Fixture values are distinct per field and >= 128 in
//               every byte the width allows. Two groups cannot follow
//               that rule and say so at the point of use: the message
//               an item carries, which is text and
//               capped at 128 bytes; and GCAddItemToItemVerify's result
//               code, which selects the shape of the rest of the body.
//
//               The bounds section at the end pins what the packets
//               refuse and what they derive:
//
//               - GCAddItemToInventory writes one option count and an
//                 item count getPacketSize() budgets, so GCMakeItemOK
//                 declares the body it sends and round trips. The
//                 option list is held to the widest an item carries,
//                 MAX_ITEM_OPTION_NUM, which is a code sheet's grid.
//                 The GCMakeItemOK goldens carry one count byte where
//                 they used to carry two.
//               - GCAddItemToItemVerify::getPacketSize() counts both
//                 parameters of a THREE_ENCHANT_OK result, and the
//                 second parameter starts at zero.
//               - GCChangeInventoryItemNum's count is its list, refused
//                 past the 255 the count byte carries; there is no
//                 setter to move it away from the list, popping takes
//                 the entry off both, and the two crafting factory
//                 maxima budget a full list.
//               - GCCreateItem holds its option list to the same
//                 MAX_ITEM_OPTION_NUM and its factory max budgets it.
//               - GCGQuestInventory holds its item list to the 100 its
//                 factory max budgets, in the adder, in write() and in
//                 read(), and read() replaces the list rather than
//                 appending to it.
//               - GCTimeLimitItemInfo::getTimeLimit() answers an empty
//                 optional for an item it does not hold: every DWORD is
//                 a remaining time a real item can have, so no sentinel
//                 can say "absent".
//               - GCTimeLimitItemInfo's entry count is the map it
//                 holds, refused past the 100 its factory max budgets in
//                 the adder, in write() and in read(), and
//                 CGTypeStringList's string count is refused past the 20
//                 its own budgets the same way. The typed string list is
//                 not one of the 37; it is here only for that bound,
//                 beside the count with the same shape.
//
//               Every packet in the set initialises every member it
//               writes, pinned by constructing each over storage
//               poisoned with two different bytes and requiring the same
//               body. CGUseMessageItemFromInventory is not in that list,
//               because its message field refuses an empty value and a
//               default-constructed one cannot be written at all.
//
//////////////////////////////////////////////////////////////////////

#include <cstring>
#include <list>
#include <new>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "CGAddGearToMouse.h"
#include "CGAddInventoryToMouse.h"
#include "CGAddItemToCodeSheet.h"
#include "CGAddItemToItem.h"
#include "CGAddMouseToGear.h"
#include "CGAddMouseToInventory.h"
#include "CGAddMouseToQuickSlot.h"
#include "CGAddQuickSlotToMouse.h"
#include "CGGetEventItem.h"
#include "CGMakeItem.h"
#include "CGMixItem.h"
#include "CGReloadFromInventory.h"
#include "CGReloadFromQuickSlot.h"
#include "CGRequestNewbieItem.h"
#include "CGRequestRepair.h"
#include "CGThrowBomb.h"
#include "CGThrowItem.h"
#include "CGTypeStringList.h"
#include "CGUseItemFromGQuestInventory.h"
#include "CGUseMessageItemFromInventory.h"
#include "CGUsePotionFromQuickSlot.h"
#include "Exception.h"
#include "GCAddGearToInventory.h"
#include "GCAddGearToZone.h"
#include "GCAddItemToItemVerify.h"
#include "GCCannotAdd.h"
#include "GCCreateItem.h"
#include "GCDeleteInventoryItem.h"
#include "GCDeleteandPickUpOK.h"
#include "GCGQuestInventory.h"
#include "GCMakeItemFail.h"
#include "GCMakeItemOK.h"
#include "GCReloadOK.h"
#include "GCRemoveFromGear.h"
#include "GCThrowItemOK1.h"
#include "GCThrowItemOK2.h"
#include "GCThrowItemOK3.h"
#include "GCTimeLimitItemInfo.h"
#include "GCUseOK.h"
#include "ModifyInfo.h"
#include "TestStreams.h"

using wiretest::expectGolden;
using wiretest::kEncryptCodeCount;
using wiretest::kEncryptCodes;
using wiretest::roundTrip;
using wiretest::writeBody;

namespace {

// The unencrypted branch. Every packet in this file except
// CGUseMessageItemFromInventory reads and writes its body with plain
// read()/write() calls, so this is the only code whose bytes differ from
// any other.
const uchar kPlainCode = 0;

//////////////////////////////////////////////////////////////////////
// The three pins every packet gets. fill() / expectEqual() are
// overloaded per packet, so the same canonical instance feeds all three.
// Both take a non-const reference: several getters are not const, and
// comparing a list consumes it.
//////////////////////////////////////////////////////////////////////

#define INVENTORY_PACKET_GOLDEN_AND_SIZE(Name)                                                       \
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

#define INVENTORY_PACKET_TESTS(Name)              \
    TEST(Name##Test, roundTripsThroughLoopback) { \
        Name src;                                 \
        fill(src);                                \
        Name dst;                                 \
        roundTrip(src, dst, kPlainCode);          \
        expectEqual(src, dst);                    \
    }                                             \
    INVENTORY_PACKET_GOLDEN_AND_SIZE(Name)

// A second fixture for a packet whose write() has a branch the canonical
// one does not take.
#define INVENTORY_PACKET_VARIANT(Name, Variant, fillVariant)                   \
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

// The one packet here whose body rides the encrypter, through the base
// class it extends. Recorded at every code the shuffle branches need.
#define INVENTORY_ENCRYPTED_PACKET_TESTS(Name)                                            \
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
    TEST(Name##Test, bodyBytesMatchGoldenForEveryEncryptCode) {                           \
        Name packet;                                                                      \
        fill(packet);                                                                     \
        for (size_t i = 0; i < kEncryptCodeCount; i++)                                    \
            expectGolden(#Name, kEncryptCodes[i], writeBody(packet, kEncryptCodes[i]));   \
    }                                                                                     \
    TEST(Name##Test, sizeMatchesTheBytesWrittenAndFitsTheFactoryMax) {                    \
        Name packet;                                                                      \
        fill(packet);                                                                     \
        Name##Factory factory;                                                            \
        EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());  \
        EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize());                    \
        EXPECT_EQ(factory.getPacketID(), packet.getPacketID());                           \
        EXPECT_EQ(factory.getPacketName(), packet.getPacketName());                       \
    }

//////////////////////////////////////////////////////////////////////
// Shared records.
//////////////////////////////////////////////////////////////////////

// The stat record five packets in this set embed. Its own layout is
// pinned in packet_combat_test.cpp; here it only has to be a record with
// entries in both lists, so the offsets around it are pinned too.
void fillModifyInfo(ModifyInfo& info) {
    info.addShortData(MODIFY_CURRENT_HP, 0x81A2);
    info.addShortData(MODIFY_BULLET, 0x83A4);
    info.addLongData(MODIFY_DURABILITY, 0x85A6B7C8);
    info.addLongData(MODIFY_GOLD, 0x87A8B9CA);
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

// The changed-material list GCMakeItemOK and GCMakeItemFail share. Its
// two halves travel as two runs, ids then counts, behind one length.
template <typename MakePacket> void fillChangedItems(MakePacket& packet, int count, ObjectID_t base) {
    for (int i = 0; i < count; i++)
        packet.addChangedItemListElement((ObjectID_t)(base + (ObjectID_t)i * 0x01010101u), (ItemNum_t)(0x91 + i));
}

template <typename MakePacket> void expectChangedItemsEqual(MakePacket& a, MakePacket& b) {
    ASSERT_EQ(a.getChangedItemListNum(), b.getChangedItemListNum());
    const int entries = (int)a.getChangedItemListNum();
    for (int i = 0; i < entries; i++)
        EXPECT_EQ(a.popFrontChangedItemListElement(), b.popFrontChangedItemListElement()) << "material " << i;
    for (int i = 0; i < entries; i++)
        EXPECT_EQ(a.popFrontChangedItemNumListElement(), b.popFrontChangedItemNumListElement()) << "count " << i;
}

//////////////////////////////////////////////////////////////////////
// Moving an item between the cursor and a gear or quick slot.
//////////////////////////////////////////////////////////////////////

void fill(CGAddGearToMouse& packet) {
    packet.setObjectID(0x81A2B3C4);
    packet.setSlotID(0x85);
}

void expectEqual(CGAddGearToMouse& a, CGAddGearToMouse& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getSlotID(), b.getSlotID());
}

INVENTORY_PACKET_TESTS(CGAddGearToMouse)

void fill(CGAddMouseToGear& packet) {
    packet.setObjectID(0x82A3B4C5);
    packet.setSlotID(0x86);
}

void expectEqual(CGAddMouseToGear& a, CGAddMouseToGear& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getSlotID(), b.getSlotID());
}

INVENTORY_PACKET_TESTS(CGAddMouseToGear)

void fill(CGAddMouseToQuickSlot& packet) {
    packet.setObjectID(0x83A4B5C6);
    packet.setSlotID(0x87);
}

void expectEqual(CGAddMouseToQuickSlot& a, CGAddMouseToQuickSlot& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getSlotID(), b.getSlotID());
}

INVENTORY_PACKET_TESTS(CGAddMouseToQuickSlot)

void fill(CGAddQuickSlotToMouse& packet) {
    packet.setObjectID(0x84A5B6C7);
    packet.setSlotID(0x88);
}

void expectEqual(CGAddQuickSlotToMouse& a, CGAddQuickSlotToMouse& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getSlotID(), b.getSlotID());
}

INVENTORY_PACKET_TESTS(CGAddQuickSlotToMouse)

//////////////////////////////////////////////////////////////////////
// Moving an item between the cursor and the inventory grid.
//////////////////////////////////////////////////////////////////////

void fill(CGAddInventoryToMouse& packet) {
    packet.setObjectID(0x87A8B9CA);
    packet.setX(0x8B);
    packet.setY(0x9C);
}

void expectEqual(CGAddInventoryToMouse& a, CGAddInventoryToMouse& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
}

INVENTORY_PACKET_TESTS(CGAddInventoryToMouse)

void fill(CGAddMouseToInventory& packet) {
    packet.setObjectID(0x88A9BACB);
    packet.setInvenX(0x8C);
    packet.setInvenY(0x9D);
}

void expectEqual(CGAddMouseToInventory& a, CGAddMouseToInventory& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getInvenX(), b.getInvenX());
    EXPECT_EQ(a.getInvenY(), b.getInvenY());
}

INVENTORY_PACKET_TESTS(CGAddMouseToInventory)

//////////////////////////////////////////////////////////////////////
// Dropping one item onto another.
//////////////////////////////////////////////////////////////////////

void fill(CGAddItemToItem& packet) {
    packet.setObjectID(0x89AABBCC);
    packet.setX(0x8D);
    packet.setY(0x9E);
}

void expectEqual(CGAddItemToItem& a, CGAddItemToItem& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
}

INVENTORY_PACKET_TESTS(CGAddItemToItem)

void fill(CGAddItemToCodeSheet& packet) {
    packet.setObjectID(0x8AABBCCD);
    packet.setX(0x8E);
    packet.setY(0x9F);
}

void expectEqual(CGAddItemToCodeSheet& a, CGAddItemToCodeSheet& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
}

INVENTORY_PACKET_TESTS(CGAddItemToCodeSheet)

void fill(CGMixItem& packet) {
    packet.setObjectID(0x8BACBDCE);
    packet.setX(0x8F);
    packet.setY(0xA0);
    packet.setTargetObjectID(0, 0x90B1C2D3);
    packet.setTargetObjectID(1, 0x91B2C3D4);
}

void expectEqual(CGMixItem& a, CGMixItem& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getTargetObjectID(0), b.getTargetObjectID(0));
    EXPECT_EQ(a.getTargetObjectID(1), b.getTargetObjectID(1));
}

INVENTORY_PACKET_TESTS(CGMixItem)

//////////////////////////////////////////////////////////////////////
// Making, throwing, reloading and using.
//////////////////////////////////////////////////////////////////////

void fill(CGMakeItem& packet) {
    packet.setItemClass(0x92);
    packet.setItemType(0x93A4);
}

void expectEqual(CGMakeItem& a, CGMakeItem& b) {
    EXPECT_EQ(a.getItemClass(), b.getItemClass());
    EXPECT_EQ(a.getItemType(), b.getItemType());
}

INVENTORY_PACKET_TESTS(CGMakeItem)

void fill(CGThrowItem& packet) {
    packet.setObjectID(0x94A5B6C7);
    packet.setTargetObjectID(0x95A6B7C8);
    packet.setX(0x96);
    packet.setY(0xA7);
}

void expectEqual(CGThrowItem& a, CGThrowItem& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
}

INVENTORY_PACKET_TESTS(CGThrowItem)

void fill(CGThrowBomb& packet) {
    packet.setZoneX(0x97);
    packet.setZoneY(0xA8);
    packet.setBombX(0x99);
    packet.setBombY(0xAA);
    packet.setAttackSlayerFlag(0x9B);
}

void expectEqual(CGThrowBomb& a, CGThrowBomb& b) {
    EXPECT_EQ(a.getZoneX(), b.getZoneX());
    EXPECT_EQ(a.getZoneY(), b.getZoneY());
    EXPECT_EQ(a.getBombX(), b.getBombX());
    EXPECT_EQ(a.getBombY(), b.getBombY());
    EXPECT_EQ(a.getAttackSlayerFlag(), b.getAttackSlayerFlag());
}

INVENTORY_PACKET_TESTS(CGThrowBomb)

void fill(CGReloadFromInventory& packet) {
    packet.setObjectID(0x9CADBECF);
    packet.setX(0x9D);
    packet.setY(0xAE);
}

void expectEqual(CGReloadFromInventory& a, CGReloadFromInventory& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
}

INVENTORY_PACKET_TESTS(CGReloadFromInventory)

void fill(CGReloadFromQuickSlot& packet) {
    packet.setObjectID(0x9EAFC0D1);
    packet.setSlotID(0x9F);
}

void expectEqual(CGReloadFromQuickSlot& a, CGReloadFromQuickSlot& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getSlotID(), b.getSlotID());
}

INVENTORY_PACKET_TESTS(CGReloadFromQuickSlot)

void fill(CGUsePotionFromQuickSlot& packet) {
    packet.setObjectID(0xA0B1C2D3);
    packet.setSlotID(0xA1);
}

void expectEqual(CGUsePotionFromQuickSlot& a, CGUsePotionFromQuickSlot& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getSlotID(), b.getSlotID());
}

INVENTORY_PACKET_TESTS(CGUsePotionFromQuickSlot)

void fill(CGUseItemFromGQuestInventory& packet) {
    packet.setIndex(0xA2);
}

void expectEqual(CGUseItemFromGQuestInventory& a, CGUseItemFromGQuestInventory& b) {
    EXPECT_EQ(a.getIndex(), b.getIndex());
}

INVENTORY_PACKET_TESTS(CGUseItemFromGQuestInventory)

void fill(CGRequestRepair& packet) {
    packet.setObjectID(0xA3B4C5D6);
}

void expectEqual(CGRequestRepair& a, CGRequestRepair& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
}

INVENTORY_PACKET_TESTS(CGRequestRepair)

void fill(CGGetEventItem& packet) {
    packet.setEventType(0xA4);
}

void expectEqual(CGGetEventItem& a, CGGetEventItem& b) {
    EXPECT_EQ(a.getEventType(), b.getEventType());
}

INVENTORY_PACKET_TESTS(CGGetEventItem)

void fill(CGRequestNewbieItem& packet) {
    packet.setItemClass(0xA5);
}

void expectEqual(CGRequestNewbieItem& a, CGRequestNewbieItem& b) {
    EXPECT_EQ(a.getItemClass(), b.getItemClass());
}

INVENTORY_PACKET_TESTS(CGRequestNewbieItem)

//////////////////////////////////////////////////////////////////////
// The one request in the set whose body rides the encrypter: it extends
// CGUseItemFromInventory, whose read()/write() call readEncrypt and
// writeEncrypt, and adds a message. The message is text and its field
// admits 1 to 128 bytes, so it is the one fixture value below 128.
//////////////////////////////////////////////////////////////////////

void fill(CGUseMessageItemFromInventory& packet) {
    packet.setObjectID(0xA6B7C8D9);
    packet.setX(0xA7);
    packet.setY(0xB8);
    packet.setMessage("message item");
}

void expectEqual(CGUseMessageItemFromInventory& a, CGUseMessageItemFromInventory& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getMessage(), b.getMessage());
}

INVENTORY_ENCRYPTED_PACKET_TESTS(CGUseMessageItemFromInventory)

//////////////////////////////////////////////////////////////////////
// The single-field answers.
//////////////////////////////////////////////////////////////////////

void fill(GCCannotAdd& packet) {
    packet.setObjectID(0xA8B9CADB);
}

void expectEqual(GCCannotAdd& a, GCCannotAdd& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
}

INVENTORY_PACKET_TESTS(GCCannotAdd)

void fill(GCDeleteInventoryItem& packet) {
    packet.setObjectID(0xA9BACBDC);
}

void expectEqual(GCDeleteInventoryItem& a, GCDeleteInventoryItem& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
}

INVENTORY_PACKET_TESTS(GCDeleteInventoryItem)

void fill(GCDeleteandPickUpOK& packet) {
    packet.setObjectID(0xAABBCCDD);
}

void expectEqual(GCDeleteandPickUpOK& a, GCDeleteandPickUpOK& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
}

INVENTORY_PACKET_TESTS(GCDeleteandPickUpOK)

void fill(GCRemoveFromGear& packet) {
    packet.setSlotID(0xAB);
}

void expectEqual(GCRemoveFromGear& a, GCRemoveFromGear& b) {
    EXPECT_EQ(a.getSlotID(), b.getSlotID());
}

INVENTORY_PACKET_TESTS(GCRemoveFromGear)

void fill(GCAddGearToZone& packet) {
    packet.setSlotID(0xAC);
}

void expectEqual(GCAddGearToZone& a, GCAddGearToZone& b) {
    EXPECT_EQ(a.getSlotID(), b.getSlotID());
}

INVENTORY_PACKET_TESTS(GCAddGearToZone)

void fill(GCAddGearToInventory& packet) {
    packet.setSlotID(0xAD);
    packet.setInvenX(0xAE);
    packet.setInvenY(0xBF);
}

void expectEqual(GCAddGearToInventory& a, GCAddGearToInventory& b) {
    EXPECT_EQ(a.getSlotID(), b.getSlotID());
    EXPECT_EQ(a.getInvenX(), b.getInvenX());
    EXPECT_EQ(a.getInvenY(), b.getInvenY());
}

INVENTORY_PACKET_TESTS(GCAddGearToInventory)

void fill(GCReloadOK& packet) {
    packet.setBulletNum(0xB0);
}

void expectEqual(GCReloadOK& a, GCReloadOK& b) {
    EXPECT_EQ(a.getBulletNum(), b.getBulletNum());
}

INVENTORY_PACKET_TESTS(GCReloadOK)

//////////////////////////////////////////////////////////////////////
// The item record that puts one item into the inventory grid. The
// enchant level is a signed char, so the fixture's high byte arrives as
// a negative level - which is what the field is for.
//////////////////////////////////////////////////////////////////////

void fill(GCCreateItem& packet) {
    packet.setObjectID(0xB1C2D3E4);
    packet.setItemClass(0xB2);
    packet.setItemType(0xB3C4);
    packet.addOptionType(0xB5);
    packet.addOptionType(0xC6);
    packet.addOptionType(0xD7);
    packet.setDurability(0xB8C9DAEB);
    packet.setSilver(0xBCCD);
    packet.setGrade(0xBEBFC0C1);
    packet.setEnchantLevel((EnchantLevel_t)0xC2);
    packet.setItemNum(0xC3);
    packet.setInvenX(0xC4);
    packet.setInvenY(0xD5);
}

void fillNoOptions(GCCreateItem& packet) {
    fill(packet);
    packet.setOptionType(std::list<OptionType_t>());
}

void expectEqual(GCCreateItem& a, GCCreateItem& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getItemClass(), b.getItemClass());
    EXPECT_EQ(a.getItemType(), b.getItemType());
    EXPECT_EQ(a.getOptionType(), b.getOptionType());
    EXPECT_EQ(a.getDurability(), b.getDurability());
    EXPECT_EQ(a.getSilver(), b.getSilver());
    EXPECT_EQ(a.getGrade(), b.getGrade());
    EXPECT_EQ(a.getEnchantLevel(), b.getEnchantLevel());
    EXPECT_EQ(a.getItemNum(), b.getItemNum());
    EXPECT_EQ(a.getInvenX(), b.getInvenX());
    EXPECT_EQ(a.getInvenY(), b.getInvenY());
}

INVENTORY_PACKET_TESTS(GCCreateItem)
INVENTORY_PACKET_VARIANT(GCCreateItem, nooptions, fillNoOptions)

//////////////////////////////////////////////////////////////////////
// The stat record on its own, which is the whole of a successful use.
//////////////////////////////////////////////////////////////////////

void fill(GCUseOK& packet) {
    fillModifyInfo(packet);
}

void fillEmpty(GCUseOK& packet) {
    (void)packet;
}

void expectEqual(GCUseOK& a, GCUseOK& b) {
    expectModifyInfoEqual(a, b);
}

INVENTORY_PACKET_TESTS(GCUseOK)
INVENTORY_PACKET_VARIANT(GCUseOK, empty, fillEmpty)

//////////////////////////////////////////////////////////////////////
// The guild-quest inventory listing.
//////////////////////////////////////////////////////////////////////

void fill(GCGQuestInventory& packet) {
    packet.addItem(0xC6D7);
    packet.addItem(0xC8D9);
    packet.addItem(0xCADB);
}

void fillEmpty(GCGQuestInventory& packet) {
    (void)packet;
}

void expectEqual(GCGQuestInventory& a, GCGQuestInventory& b) {
    EXPECT_EQ(a.getItemList(), b.getItemList());
}

INVENTORY_PACKET_TESTS(GCGQuestInventory)
INVENTORY_PACKET_VARIANT(GCGQuestInventory, empty, fillEmpty)

//////////////////////////////////////////////////////////////////////
// The remaining life of every time-limited item. The map is keyed by
// object id, so the entries reach the wire in id order whatever order
// they were added in.
//////////////////////////////////////////////////////////////////////

const ObjectID_t kTimeLimitIDs[] = {0xCCDDEEFF, 0xCDDEEFF0, 0xCEDFE0F1};
const DWORD kTimeLimitValues[] = {0xD0E1F2C3, 0xD1E2F3C4, 0xD2E3F4C5};

void fill(GCTimeLimitItemInfo& packet) {
    for (int i = 0; i < 3; i++)
        packet.addTimeLimit(kTimeLimitIDs[i], kTimeLimitValues[i]);
}

void fillEmpty(GCTimeLimitItemInfo& packet) {
    (void)packet;
}

void expectEqual(GCTimeLimitItemInfo& a, GCTimeLimitItemInfo& b) {
    EXPECT_EQ(a.getPacketSize(), b.getPacketSize());
    for (int i = 0; i < 3; i++) {
        SCOPED_TRACE(testing::Message() << "entry " << i);
        ASSERT_EQ(a.hasTimeLimit(kTimeLimitIDs[i]), b.hasTimeLimit(kTimeLimitIDs[i]));
        if (a.hasTimeLimit(kTimeLimitIDs[i]))
            EXPECT_EQ(*a.getTimeLimit(kTimeLimitIDs[i]), *b.getTimeLimit(kTimeLimitIDs[i]));
    }
}

INVENTORY_PACKET_TESTS(GCTimeLimitItemInfo)
INVENTORY_PACKET_VARIANT(GCTimeLimitItemInfo, empty, fillEmpty)

//////////////////////////////////////////////////////////////////////
// The result of dropping one item on another. The code is not a value
// to vary: it selects how many parameters follow, so the canonical
// fixture takes the one-parameter branch and the variants take the
// other two.
//////////////////////////////////////////////////////////////////////

void fill(GCAddItemToItemVerify& packet) {
    packet.setCode(ADD_ITEM_TO_ITEM_VERIFY_ENCHANT_OK);
    packet.setParameter(0xD3E4F5C6);
}

void fillError(GCAddItemToItemVerify& packet) {
    packet.setCode(ADD_ITEM_TO_ITEM_VERIFY_ERROR);
    packet.setParameter(0);
}

void fillThreeEnchant(GCAddItemToItemVerify& packet) {
    packet.setCode(ADD_ITEM_TO_ITEM_VERIFY_THREE_ENCHANT_OK);
    packet.setParameter(0x81C2D3E4);
    packet.setParameter2(0x82C3D4E5);
}

void expectEqual(GCAddItemToItemVerify& a, GCAddItemToItemVerify& b) {
    EXPECT_EQ(a.getCode(), b.getCode());
    EXPECT_EQ(a.getParameter(), b.getParameter());
    EXPECT_EQ(a.getParameter2(), b.getParameter2());
}

INVENTORY_PACKET_TESTS(GCAddItemToItemVerify)
INVENTORY_PACKET_VARIANT(GCAddItemToItemVerify, error, fillError)
INVENTORY_PACKET_VARIANT(GCAddItemToItemVerify, threeenchant, fillThreeEnchant)

//////////////////////////////////////////////////////////////////////
// A thrown item.
//////////////////////////////////////////////////////////////////////

void fill(GCThrowItemOK1& packet) {
    packet.setObjectID(0xD4E5F6C7);
}

void expectEqual(GCThrowItemOK1& a, GCThrowItemOK1& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
}

INVENTORY_PACKET_TESTS(GCThrowItemOK1)

void fill(GCThrowItemOK2& packet) {
    packet.setObjectID(0xD5E6F7C8);
    fillModifyInfo(packet);
}

void fillEmpty(GCThrowItemOK2& packet) {
    packet.setObjectID(0xD6E7F8C9);
}

void expectEqual(GCThrowItemOK2& a, GCThrowItemOK2& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    expectModifyInfoEqual(a, b);
}

INVENTORY_PACKET_TESTS(GCThrowItemOK2)
INVENTORY_PACKET_VARIANT(GCThrowItemOK2, empty, fillEmpty)

void fill(GCThrowItemOK3& packet) {
    packet.setObjectID(0xD7E8F9CA);
    packet.setTargetObjectID(0xD8E9FACB);
}

void expectEqual(GCThrowItemOK3& a, GCThrowItemOK3& b) {
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getTargetObjectID(), b.getTargetObjectID());
}

INVENTORY_PACKET_TESTS(GCThrowItemOK3)

//////////////////////////////////////////////////////////////////////
// Crafting. The failure carries the materials it consumed and the stat
// record; the success adds the item it produced.
//////////////////////////////////////////////////////////////////////

void fill(GCMakeItemFail& packet) {
    fillChangedItems(packet, 3, 0xF6C7D8E9);
    fillModifyInfo(packet);
}

void fillEmpty(GCMakeItemFail& packet) {
    (void)packet;
}

void expectEqual(GCMakeItemFail& a, GCMakeItemFail& b) {
    expectChangedItemsEqual(a, b);
    expectModifyInfoEqual(a, b);
}

INVENTORY_PACKET_TESTS(GCMakeItemFail)
INVENTORY_PACKET_VARIANT(GCMakeItemFail, empty, fillEmpty)

void fill(GCMakeItemOK& packet) {
    fillChangedItems(packet, 2, 0xF7C8D9EA);
    packet.setObjectID(0xF8C9DAEB);
    packet.setX(0xF9);
    packet.setY(0xCA);
    packet.setItemClass(0xFB);
    packet.setItemType(0xFCCD);
    packet.addOptionType(0xFE);
    packet.addOptionType(0xCF);
    packet.setDurability(0x80C1D2E3);
    packet.setItemNum(0x84);
    fillModifyInfo(packet);
}

void fillNoOptions(GCMakeItemOK& packet) {
    fill(packet);
    packet.setOptionType(std::list<OptionType_t>());
}

void expectEqual(GCMakeItemOK& a, GCMakeItemOK& b) {
    expectChangedItemsEqual(a, b);
    EXPECT_EQ(a.getObjectID(), b.getObjectID());
    EXPECT_EQ(a.getX(), b.getX());
    EXPECT_EQ(a.getY(), b.getY());
    EXPECT_EQ(a.getItemClass(), b.getItemClass());
    EXPECT_EQ(a.getItemType(), b.getItemType());
    EXPECT_EQ(a.getOptionType(), b.getOptionType());
    EXPECT_EQ(a.getDurability(), b.getDurability());
    EXPECT_EQ(a.getItemNum(), b.getItemNum());
    expectModifyInfoEqual(a, b);
}

INVENTORY_PACKET_TESTS(GCMakeItemOK)
INVENTORY_PACKET_VARIANT(GCMakeItemOK, nooptions, fillNoOptions)

//////////////////////////////////////////////////////////////////////
// The bounds and the counts.
//////////////////////////////////////////////////////////////////////

// The item record inside GCMakeItemOK writes one option count and an
// item count, and its declared size budgets both. writePacket() puts
// getPacketSize() on the wire before write() runs, so anything else is
// a stream that never resynchronises - the stock size pin above covers
// the whole packet, this one isolates the record.
TEST(GCMakeItemOKTest, theItemRecordDeclaresTheCountsItWrites) {
    GCMakeItemOK packet;
    fill(packet);

    EXPECT_EQ(szObjectID + szCoordInven * 2 + szBYTE + szItemType + szBYTE + 2 * szOptionType + szDurability +
                  szItemNum,
              (uint)packet.GCAddItemToInventory::getPacketSize());

    // Two materials, then the record's fixed head, then the one option
    // count and the two options behind it.
    const size_t optionCount =
        szBYTE + 2 * (szObjectID + szItemNum) + szObjectID + szCoordInven * 2 + szBYTE + szItemType;
    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    ASSERT_GT(body.size(), optionCount + 2);
    EXPECT_EQ(2, (int)body[optionCount]);
    EXPECT_EQ(0xFE, (int)body[optionCount + 1]);
    EXPECT_EQ(0xCF, (int)body[optionCount + 2]);
}

// The option list is held to the widest an item carries: a code sheet
// keeps its stone grid there and holds exactly MAX_ITEM_OPTION_NUM, so
// that many must reach the wire and one more is refused.
TEST(GCMakeItemOKTest, theOptionListIsHeldToTheItemsOptionSlots) {
    GCMakeItemOK packet;
    fill(packet);
    packet.setOptionType(std::list<OptionType_t>());

    for (uint i = 0; i < GCAddItemToInventory::kMaxOptionCount; i++)
        packet.addOptionType((OptionType_t)(0x80 + i));

    EXPECT_THROW(packet.addOptionType(0xFF), InvalidProtocolException);
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());

    GCMakeItemOKFactory factory;
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize());
}

// The material count is the list. It travels in a BYTE, so the list is
// refused past what that byte describes, and both crafting factory
// maxima budget a full one.
TEST(GCMakeItemFailTest, theMaterialListIsHeldToWhatTheCountByteDescribes) {
    GCMakeItemFail packet;
    fillChangedItems(packet, (int)GCChangeInventoryItemNum::kMaxCount, 0x84C5D6E7);

    EXPECT_EQ(255, (int)packet.getChangedItemListNum());
    EXPECT_THROW(packet.addChangedItemListElement(0x88C9DAEB, 0x92), InvalidProtocolException);

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((size_t)szBYTE + 255 * (szObjectID + szItemNum) + szBYTE * 2, body.size());
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCMakeItemFailFactory failFactory;
    EXPECT_LE(packet.getPacketSize(), failFactory.getPacketMaxSize());

    GCMakeItemOK okPacket;
    fill(okPacket);
    okPacket.clearChangedItemList();
    fillChangedItems(okPacket, (int)GCChangeInventoryItemNum::kMaxCount, 0x85C6D7E8);

    GCMakeItemOKFactory okFactory;
    EXPECT_LE(okPacket.getPacketSize(), okFactory.getPacketMaxSize());
}

// Popping a material takes it off the count with the list, so what is
// left declares what it writes.
TEST(GCMakeItemFailTest, poppingAMaterialTakesItOffTheCount) {
    GCMakeItemFail packet;
    fillChangedItems(packet, 2, 0x87C8D9EA);

    packet.popFrontChangedItemListElement();
    packet.popFrontChangedItemNumListElement();

    EXPECT_EQ(1, (int)packet.getChangedItemListNum());
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());

    packet.popFrontChangedItemListElement();
    packet.popFrontChangedItemNumListElement();
    EXPECT_THROW(packet.popFrontChangedItemListElement(), InvalidProtocolException);
    EXPECT_THROW(packet.popFrontChangedItemNumListElement(), InvalidProtocolException);
}

// GCCreateItem holds the same option list to the same bound, and its
// factory max budgets a full one.
TEST(GCCreateItemTest, theOptionListIsHeldToTheItemsOptionSlots) {
    GCCreateItem packet;
    fill(packet);
    packet.setOptionType(std::list<OptionType_t>());

    for (uint i = 0; i < GCCreateItem::kMaxOptionCount; i++)
        packet.addOptionType((OptionType_t)(0x80 + i));

    EXPECT_THROW(packet.addOptionType(0xFF), InvalidProtocolException);
    EXPECT_THROW(packet.setOptionType(std::list<OptionType_t>(GCCreateItem::kMaxOptionCount + 1, 0x81)),
                 InvalidProtocolException);

    const std::vector<unsigned char> body = writeBody(packet, kPlainCode);
    EXPECT_EQ((int)GCCreateItem::kMaxOptionCount, (int)body[7]);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());

    GCCreateItemFactory factory;
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize());
}

// The guild-quest listing is held to the 100 its factory max budgets,
// in the adder and again in write() for a list reached through the
// mutable accessor.
TEST(GCGQuestInventoryTest, theItemListIsHeldToWhatTheFactoryMaxBudgets) {
    GCGQuestInventory packet;
    for (uint i = 0; i < GCGQuestInventory::kMaxCount; i++)
        packet.addItem((ItemType_t)(0x8000 + i));

    EXPECT_THROW(packet.addItem(0x9000), InvalidProtocolException);

    GCGQuestInventoryFactory factory;
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize());

    packet.getItemList().push_back(0x9001);
    EXPECT_THROW(writeBody(packet, kPlainCode), InvalidProtocolException);
}

// read() replaces the list the packet already holds, so a packet read
// into twice declares one listing and writes one.
TEST(GCGQuestInventoryTest, aSecondReadReplacesTheListItAlreadyHolds) {
    GCGQuestInventory src;
    fill(src);

    GCGQuestInventory dst;
    roundTrip(src, dst, kPlainCode);
    ASSERT_EQ(3u, dst.getItemList().size());

    GCGQuestInventory second;
    second.addItem(0xCCDD);
    roundTrip(second, dst, kPlainCode);
    EXPECT_EQ(1u, dst.getItemList().size());
    expectEqual(second, dst);
}

// getTimeLimit() answers an empty optional for an item the packet does
// not hold: every DWORD is a remaining time a real item can have, so no
// sentinel can say "absent".
TEST(GCTimeLimitItemInfoTest, anAbsentItemIsAnEmptyAnswer) {
    GCTimeLimitItemInfo packet;
    packet.addTimeLimit(0xCCDDEEFF, 0xFFFF);

    EXPECT_TRUE(packet.hasTimeLimit(0xCCDDEEFF));
    ASSERT_TRUE(packet.getTimeLimit(0xCCDDEEFF).has_value());
    EXPECT_EQ((DWORD)0xFFFF, *packet.getTimeLimit(0xCCDDEEFF));

    EXPECT_FALSE(packet.hasTimeLimit(0xCCDDEEFE));
    EXPECT_FALSE(packet.getTimeLimit(0xCCDDEEFE).has_value());
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

TEST(InventoryConstructorTest, everyPacketInitialisesEveryMemberItWrites) {
    expectEveryMemberIsInitialised<CGAddGearToMouse>("CGAddGearToMouse");
    expectEveryMemberIsInitialised<CGAddMouseToGear>("CGAddMouseToGear");
    expectEveryMemberIsInitialised<CGAddMouseToQuickSlot>("CGAddMouseToQuickSlot");
    expectEveryMemberIsInitialised<CGAddQuickSlotToMouse>("CGAddQuickSlotToMouse");
    expectEveryMemberIsInitialised<CGAddInventoryToMouse>("CGAddInventoryToMouse");
    expectEveryMemberIsInitialised<CGAddMouseToInventory>("CGAddMouseToInventory");
    expectEveryMemberIsInitialised<CGAddItemToItem>("CGAddItemToItem");
    expectEveryMemberIsInitialised<CGAddItemToCodeSheet>("CGAddItemToCodeSheet");
    expectEveryMemberIsInitialised<CGMixItem>("CGMixItem");
    expectEveryMemberIsInitialised<CGMakeItem>("CGMakeItem");
    expectEveryMemberIsInitialised<CGThrowItem>("CGThrowItem");
    expectEveryMemberIsInitialised<CGThrowBomb>("CGThrowBomb");
    expectEveryMemberIsInitialised<CGReloadFromInventory>("CGReloadFromInventory");
    expectEveryMemberIsInitialised<CGReloadFromQuickSlot>("CGReloadFromQuickSlot");
    expectEveryMemberIsInitialised<CGUsePotionFromQuickSlot>("CGUsePotionFromQuickSlot");
    expectEveryMemberIsInitialised<CGUseItemFromGQuestInventory>("CGUseItemFromGQuestInventory");
    expectEveryMemberIsInitialised<CGRequestRepair>("CGRequestRepair");
    expectEveryMemberIsInitialised<CGGetEventItem>("CGGetEventItem");
    expectEveryMemberIsInitialised<CGRequestNewbieItem>("CGRequestNewbieItem");
    expectEveryMemberIsInitialised<GCCannotAdd>("GCCannotAdd");
    expectEveryMemberIsInitialised<GCCreateItem>("GCCreateItem");
    expectEveryMemberIsInitialised<GCDeleteInventoryItem>("GCDeleteInventoryItem");
    expectEveryMemberIsInitialised<GCDeleteandPickUpOK>("GCDeleteandPickUpOK");
    expectEveryMemberIsInitialised<GCUseOK>("GCUseOK");
    expectEveryMemberIsInitialised<GCReloadOK>("GCReloadOK");
    expectEveryMemberIsInitialised<GCRemoveFromGear>("GCRemoveFromGear");
    expectEveryMemberIsInitialised<GCAddGearToInventory>("GCAddGearToInventory");
    expectEveryMemberIsInitialised<GCAddGearToZone>("GCAddGearToZone");
    expectEveryMemberIsInitialised<GCMakeItemOK>("GCMakeItemOK");
    expectEveryMemberIsInitialised<GCMakeItemFail>("GCMakeItemFail");
    expectEveryMemberIsInitialised<GCThrowItemOK1>("GCThrowItemOK1");
    expectEveryMemberIsInitialised<GCThrowItemOK2>("GCThrowItemOK2");
    expectEveryMemberIsInitialised<GCThrowItemOK3>("GCThrowItemOK3");
    expectEveryMemberIsInitialised<GCGQuestInventory>("GCGQuestInventory");
    expectEveryMemberIsInitialised<GCTimeLimitItemInfo>("GCTimeLimitItemInfo");
}

// GCAddItemToItemVerify needs its own case: the branch that writes both
// parameters is the one the default code does not take, so the record
// has to be given that code before it is written.
TEST(InventoryConstructorTest, theThreeEnchantResultInitialisesBothParameters) {
    alignas(GCAddItemToItemVerify) unsigned char storage[sizeof(GCAddItemToItemVerify)];
    const unsigned char poison[2] = {0x00, 0xFF};
    std::vector<unsigned char> bodies[2];

    for (int i = 0; i < 2; i++) {
        memset(storage, poison[i], sizeof(storage));
        GCAddItemToItemVerify* pPacket = new (storage) GCAddItemToItemVerify();
        pPacket->setCode(ADD_ITEM_TO_ITEM_VERIFY_THREE_ENCHANT_OK);
        pPacket->setParameter(0x83C4D5E6);
        bodies[i] = writeBody(*pPacket, kPlainCode);
        pPacket->~GCAddItemToItemVerify();
    }

    EXPECT_EQ(bodies[0], bodies[1])
        << "GCAddItemToItemVerify: its default constructor leaves the second parameter uninitialised";
}

// CGUseMessageItemFromInventory is not in the list above: its message
// field refuses an empty value, so a default-constructed one cannot be
// written at all.
TEST(CGUseMessageItemFromInventoryTest, anEmptyMessageIsRefused) {
    CGUseMessageItemFromInventory packet;
    packet.setObjectID(0x91D2E3F4);
    packet.setX(0x92);
    packet.setY(0xA3);
    EXPECT_THROW(writeBody(packet, kPlainCode), InvalidProtocolException);
}

//////////////////////////////////////////////////////////////////////
// The two entry counts the bounds above did not reach.
//////////////////////////////////////////////////////////////////////

// A body write() would never produce, so a read()'s own bound is
// reachable.
template <typename Emit> void readHandBuiltBody(Packet& packet, Emit emit) {
    wiretest::Loopback link;
    link.setCodes(kPlainCode);
    emit(link.out());
    const uint length = link.out().length();
    link.pump(length);
    packet.read(link.in());
}

// The listing's entry count is the map it holds, refused past the 100
// its factory max budgets in the adder, in write() and in read(), so a
// full listing fits the read buffer exactly and the count cannot wrap.
TEST(GCTimeLimitItemInfoTest, theEntryCountStopsAtTheNumberTheFactoryMaxBudgets) {
    GCTimeLimitItemInfo packet;
    for (size_t i = 0; i < GCTimeLimitItemInfo::kMaxEntryCount; i++)
        packet.addTimeLimit((ObjectID_t)(0x81A20000 + i), (DWORD)(0x83A4B5C6 + i));

    GCTimeLimitItemInfoFactory factory;
    EXPECT_EQ(packet.getPacketSize(), factory.getPacketMaxSize());
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());

    EXPECT_THROW(packet.addTimeLimit(0x91A2B3C4, 0x92A3B4C5), InvalidProtocolException);

    GCTimeLimitItemInfo tooMany;
    EXPECT_THROW(readHandBuiltBody(tooMany,
                                   [](SocketEncryptOutputStream& out) {
                                       out.write((BYTE)(GCTimeLimitItemInfo::kMaxEntryCount + 1));
                                       for (size_t i = 0; i < GCTimeLimitItemInfo::kMaxEntryCount + 1; i++) {
                                           out.write((ObjectID_t)(0x81A20000 + i));
                                           out.write((DWORD)0x83A4B5C6);
                                       }
                                   }),
                 InvalidProtocolException);
}

// The typed string list is bounded the same way, at the 20 strings of 50
// bytes its factory max budgets.
TEST(CGTypeStringListTest, theStringCountStopsAtTheNumberTheFactoryMaxBudgets) {
    CGTypeStringList packet;
    packet.setType(CGTypeStringList::STRING_TYPE_WAIT_FOR_MEET);
    packet.setParam(0x84A5B6C7);
    for (size_t i = 0; i < CGTypeStringList::kMaxStringCount; i++)
        packet.addString(std::string(MAX_STRING_LENGTH, 's'));

    CGTypeStringListFactory factory;
    EXPECT_EQ(packet.getPacketSize(), factory.getPacketMaxSize());
    EXPECT_EQ((size_t)packet.getPacketSize(), writeBody(packet, kPlainCode).size());

    EXPECT_THROW(packet.addString("one string too many"), InvalidProtocolException);

    CGTypeStringList dst;
    roundTrip(packet, dst, kPlainCode);
    EXPECT_EQ((int)CGTypeStringList::kMaxStringCount, dst.getSize());

    CGTypeStringList tooMany;
    EXPECT_THROW(readHandBuiltBody(tooMany,
                                   [](SocketEncryptOutputStream& out) {
                                       out.write((BYTE)CGTypeStringList::STRING_TYPE_WAIT_FOR_MEET);
                                       out.write((BYTE)(CGTypeStringList::kMaxStringCount + 1));
                                   }),
                 InvalidProtocolException);
}

} // namespace
