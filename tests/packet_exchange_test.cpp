//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_exchange_test.cpp
// Description : Round-trip, golden and size pins for the four Exchange
//               packets (CG/GCExchangeList, CG/GCExchangeBuy).
//
//               These four were found divergent between the server and
//               client repos by the docs/RESTRUCTURING.md 1.4 cross-check:
//               the two hand-maintained copies disagreed on string
//               framing and on getPacketSize() vs write(). The tests
//               here pin the RECONCILED contract, in particular:
//
//               - every string is a "bstr": a BYTE length prefix that is
//                 ALWAYS written, followed by that many raw bytes. An
//                 empty string is still one 0x00 byte on the wire — the
//                 empty-string tests below exist because a copy that
//                 skips the prefix for "" reads garbage from the next
//                 field with no error anywhere;
//               - getPacketSize() equals the byte count write() emits
//                 (the drift class the 1.4 diff kept finding), checked
//                 both directly and implicitly by roundTrip(), which
//                 pumps exactly getPacketSize() bytes.
//
//               None of the four references the encrypter — they always
//               take the plain path — so, as with CGSay/CGWhisper in
//               packet_roundtrip_test.cpp, goldens are recorded at code 0
//               only. exchangePacketsAreStillEncrypterFree proves that
//               assumption on every run: if one of them ever adopts the
//               encrypter, that test fails loudly and per-code goldens
//               must be added.
//
//               Fixture values are distinct per field and >= 128 in every
//               byte where the width allows (see packet_encrypter_test.cpp
//               for the rationale: an all-<128 fixture cannot tell BYTE
//               from char, and identical values hide transposed fields).
//
//////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>

#include "CGExchangeBuy.h"
#include "CGExchangeList.h"
#include "GCExchangeBuy.h"
#include "GCExchangeList.h"
#include "TestStreams.h"

using wiretest::expectGolden;
using wiretest::kEncryptCodeCount;
using wiretest::kEncryptCodes;
using wiretest::roundTrip;
using wiretest::writeBody;

namespace {

//////////////////////////////////////////////////////////////////////
// Fixtures — fill() / expectEqual() per packet, as in
// packet_encrypter_test.cpp. The same fixture feeds the round-trip, the
// golden and the size test, so all three pin the same instance.
//////////////////////////////////////////////////////////////////////

void fill(CGExchangeList& p) {
    p.setPage((int)0x89ABCDEF);
    p.setPageSize((int)0x9ABCDE8F);
    p.setItemClass(0xA1);
    p.setItemType(0xB2C3);
    p.setMinPrice((int)0xABCDEF89);
    p.setMaxPrice((int)0xBCDEF89A);
    p.setSellerFilter("SellerFilter");
}
void expectEqual(const CGExchangeList& a, const CGExchangeList& b) {
    EXPECT_EQ(a.getPage(), b.getPage());
    EXPECT_EQ(a.getPageSize(), b.getPageSize());
    EXPECT_EQ(a.getItemClass(), b.getItemClass());
    EXPECT_EQ(a.getItemType(), b.getItemType());
    EXPECT_EQ(a.getMinPrice(), b.getMinPrice());
    EXPECT_EQ(a.getMaxPrice(), b.getMaxPrice());
    EXPECT_EQ(a.getSellerFilter(), b.getSellerFilter());
}

void fill(CGExchangeBuy& p) {
    p.setListingID(static_cast<int64_t>(0xF9E8D7C6B5A49382ULL));
    p.setIdempotencyKey("idem-key-0123456789abcdef");
}
void expectEqual(const CGExchangeBuy& a, const CGExchangeBuy& b) {
    EXPECT_EQ(a.getListingID(), b.getListingID());
    EXPECT_EQ(a.getIdempotencyKey(), b.getIdempotencyKey());
}

void fill(GCExchangeBuy& p) {
    p.setSuccess(true);
    p.setMessage("purchase complete");
    p.setOrderID(static_cast<int64_t>(0x8192A3B4C5D6E7F8ULL));
}
void expectEqual(const GCExchangeBuy& a, const GCExchangeBuy& b) {
    EXPECT_EQ(a.getSuccess(), b.getSuccess());
    EXPECT_EQ(a.getMessage(), b.getMessage());
    EXPECT_EQ(a.getOrderID(), b.getOrderID());
}

// Listing 1: every string non-empty and distinct.
ExchangeListing makeFullListing() {
    ExchangeListing l = ExchangeListing();
    l.listingID = static_cast<int64_t>(0x8899AABBCCDDEEFFULL);
    l.serverID = (int16_t)0x9A8B;
    l.sellerAccount = "sellerAcct";
    l.sellerPlayer = "SellerHero";
    l.sellerRace = 0x81;
    l.itemClass = 0x92;
    l.itemType = 0xA3B4;
    l.itemID = static_cast<int64_t>(0xC5D6E7F889192A3BULL);
    l.objectID = (int)0xB4C5D6E7;
    l.pricePoint = (int)0xC6D7E8F9;
    l.currency = 0xD9;
    l.status = 0xEA;
    l.buyerAccount = "buyerAcct";
    l.buyerPlayer = "BuyerHero";
    l.taxRate = 0xFB;
    l.taxAmount = (int)0x8C9DAEBF;
    l.createdAt = "2026-08-30 12:34:56";
    l.expireAt = "2026-09-06 12:34:56";
    l.version = (int)0x9DAEBFC8;
    l.itemName = "Blood Sword";
    l.enchantLevel = 0x8D;
    l.grade = 0xAEBF;
    l.durability = (int)0xBFC8D9EA;
    l.silver = 0xC8D9;
    l.optionType1 = 0xE2;
    l.optionType2 = 0xF3;
    l.optionType3 = 0x84;
    l.optionValue1 = 0x95A6;
    l.optionValue2 = 0xA6B7;
    l.optionValue3 = 0xB7C8;
    l.stackCount = (int)0xD8E9FA8B;
    // soldAt / cancelledAt / updatedAt are NOT on the wire — left default
    // on purpose; nothing below asserts on them.
    return l;
}

// Listing 2: every string EMPTY. Pins that the BYTE length prefix is
// still written when the string is "" — the framing bug the 1.4
// cross-check found is exactly a copy that omits it.
ExchangeListing makeEmptyStringListing() {
    ExchangeListing l = ExchangeListing();
    l.listingID = static_cast<int64_t>(0x91A2B3C4D5E6F788ULL);
    l.serverID = (int16_t)0xABCC;
    l.sellerAccount = "";
    l.sellerPlayer = "";
    l.sellerRace = 0x83;
    l.itemClass = 0x94;
    l.itemType = 0xA5B6;
    l.itemID = static_cast<int64_t>(0xA2B3C4D5E6F78899ULL);
    l.objectID = (int)0xB3C4D5E6;
    l.pricePoint = (int)0xC4D5E6F7;
    l.currency = 0xDB;
    l.status = 0xEC;
    l.buyerAccount = "";
    l.buyerPlayer = "";
    l.taxRate = 0xFD;
    l.taxAmount = (int)0x8E9FA8B1;
    l.createdAt = "";
    l.expireAt = "";
    l.version = (int)0x9FA8B1C2;
    l.itemName = "";
    l.enchantLevel = 0x8F;
    l.grade = 0xB8C1;
    l.durability = (int)0xC1D2E3F4;
    l.silver = 0xD2E3;
    l.optionType1 = 0xE4;
    l.optionType2 = 0xF5;
    l.optionType3 = 0x86;
    l.optionValue1 = 0x97A8;
    l.optionValue2 = 0xA8B9;
    l.optionValue3 = 0xB9CA;
    l.stackCount = (int)0xE8F19293;
    return l;
}

void fill(GCExchangeList& p) {
    p.setPage((int)0x8A9BACBD);
    p.setPageSize((int)0x9BACBDCE);
    p.setTotal((int)0xACBDCEDF);
    std::vector<ExchangeListing> listings;
    listings.push_back(makeFullListing());
    listings.push_back(makeEmptyStringListing());
    p.setListings(listings);
}

// Every wire field of a listing; soldAt/cancelledAt/updatedAt are not on
// the wire and are deliberately not compared.
void expectListingEqual(const ExchangeListing& a, const ExchangeListing& b) {
    EXPECT_EQ(a.listingID, b.listingID);
    EXPECT_EQ(a.serverID, b.serverID);
    EXPECT_EQ(a.sellerAccount, b.sellerAccount);
    EXPECT_EQ(a.sellerPlayer, b.sellerPlayer);
    EXPECT_EQ(a.sellerRace, b.sellerRace);
    EXPECT_EQ(a.itemClass, b.itemClass);
    EXPECT_EQ(a.itemType, b.itemType);
    EXPECT_EQ(a.itemID, b.itemID);
    EXPECT_EQ(a.objectID, b.objectID);
    EXPECT_EQ(a.pricePoint, b.pricePoint);
    EXPECT_EQ(a.currency, b.currency);
    EXPECT_EQ(a.status, b.status);
    EXPECT_EQ(a.buyerAccount, b.buyerAccount);
    EXPECT_EQ(a.buyerPlayer, b.buyerPlayer);
    EXPECT_EQ(a.taxRate, b.taxRate);
    EXPECT_EQ(a.taxAmount, b.taxAmount);
    EXPECT_EQ(a.createdAt, b.createdAt);
    EXPECT_EQ(a.expireAt, b.expireAt);
    EXPECT_EQ(a.version, b.version);
    EXPECT_EQ(a.itemName, b.itemName);
    EXPECT_EQ(a.enchantLevel, b.enchantLevel);
    EXPECT_EQ(a.grade, b.grade);
    EXPECT_EQ(a.durability, b.durability);
    EXPECT_EQ(a.silver, b.silver);
    EXPECT_EQ(a.optionType1, b.optionType1);
    EXPECT_EQ(a.optionType2, b.optionType2);
    EXPECT_EQ(a.optionType3, b.optionType3);
    EXPECT_EQ(a.optionValue1, b.optionValue1);
    EXPECT_EQ(a.optionValue2, b.optionValue2);
    EXPECT_EQ(a.optionValue3, b.optionValue3);
    EXPECT_EQ(a.stackCount, b.stackCount);
}

void expectEqual(const GCExchangeList& a, const GCExchangeList& b) {
    EXPECT_EQ(a.getPage(), b.getPage());
    EXPECT_EQ(a.getPageSize(), b.getPageSize());
    EXPECT_EQ(a.getTotal(), b.getTotal());
    ASSERT_EQ(a.getListings().size(), b.getListings().size());
    for (size_t i = 0; i < a.getListings().size(); i++) {
        SCOPED_TRACE(testing::Message() << "listing " << i);
        expectListingEqual(a.getListings()[i], b.getListings()[i]);
    }
}

//////////////////////////////////////////////////////////////////////
// Encrypter coverage — justifies the single-code goldens below
//////////////////////////////////////////////////////////////////////

// The four Exchange packets are pinned at encrypt code 0 only because
// their read()/write() never touch the encrypter. Prove that rather than
// trusting it (pattern: encrypterFreePacketsAreStillEncrypterFree in
// packet_roundtrip_test.cpp): if one of them adopts the encrypter, its
// bytes would vary with the code and this fails loudly — the signal to
// record per-code goldens for it.
TEST(ExchangeEncrypterCoverageTest, exchangePacketsAreStillEncrypterFree) {
    CGExchangeList cgList;
    fill(cgList);
    CGExchangeBuy cgBuy;
    fill(cgBuy);
    GCExchangeList gcList;
    fill(gcList);
    GCExchangeBuy gcBuy;
    fill(gcBuy);

    for (size_t i = 1; i < kEncryptCodeCount; i++) {
        EXPECT_EQ(writeBody(cgList, 0), writeBody(cgList, kEncryptCodes[i]))
            << "CGExchangeList now varies with the encrypt code — add per-code goldens";
        EXPECT_EQ(writeBody(cgBuy, 0), writeBody(cgBuy, kEncryptCodes[i]))
            << "CGExchangeBuy now varies with the encrypt code — add per-code goldens";
        EXPECT_EQ(writeBody(gcList, 0), writeBody(gcList, kEncryptCodes[i]))
            << "GCExchangeList now varies with the encrypt code — add per-code goldens";
        EXPECT_EQ(writeBody(gcBuy, 0), writeBody(gcBuy, kEncryptCodes[i]))
            << "GCExchangeBuy now varies with the encrypt code — add per-code goldens";
    }
}

//////////////////////////////////////////////////////////////////////
// CGExchangeList — int32 page, int32 pageSize, u8 itemClass,
// u16 itemType, int32 minPrice, int32 maxPrice, bstr sellerFilter
//////////////////////////////////////////////////////////////////////

TEST(CGExchangeListTest, roundTripsThroughLoopback) {
    for (size_t i = 0; i < kEncryptCodeCount; i++) {
        SCOPED_TRACE(testing::Message() << "encrypt code " << (int)kEncryptCodes[i]);
        CGExchangeList src;
        fill(src);
        CGExchangeList dst;
        roundTrip(src, dst, kEncryptCodes[i]);
        expectEqual(src, dst);
    }
}

TEST(CGExchangeListTest, roundTripsWithEmptySellerFilter) {
    CGExchangeList src;
    fill(src);
    src.setSellerFilter("");
    CGExchangeList dst;
    roundTrip(src, dst, 0);
    expectEqual(src, dst);
}

TEST(CGExchangeListTest, getPacketSizeMatchesWrittenBody) {
    CGExchangeList packet;
    fill(packet);
    EXPECT_EQ(packet.getPacketSize(), writeBody(packet, 0).size());
}

TEST(CGExchangeListTest, bodyBytesMatchGolden) {
    CGExchangeList packet;
    fill(packet);
    expectGolden("CGExchangeList", 0, writeBody(packet, 0));
}

//////////////////////////////////////////////////////////////////////
// CGExchangeBuy — u64 listingID, bstr idempotencyKey
//////////////////////////////////////////////////////////////////////

TEST(CGExchangeBuyTest, roundTripsThroughLoopback) {
    for (size_t i = 0; i < kEncryptCodeCount; i++) {
        SCOPED_TRACE(testing::Message() << "encrypt code " << (int)kEncryptCodes[i]);
        CGExchangeBuy src;
        fill(src);
        CGExchangeBuy dst;
        roundTrip(src, dst, kEncryptCodes[i]);
        expectEqual(src, dst);
    }
}

TEST(CGExchangeBuyTest, roundTripsWithEmptyIdempotencyKey) {
    CGExchangeBuy src;
    fill(src);
    src.setIdempotencyKey("");
    CGExchangeBuy dst;
    roundTrip(src, dst, 0);
    expectEqual(src, dst);
}

TEST(CGExchangeBuyTest, getPacketSizeMatchesWrittenBody) {
    CGExchangeBuy packet;
    fill(packet);
    EXPECT_EQ(packet.getPacketSize(), writeBody(packet, 0).size());
}

TEST(CGExchangeBuyTest, bodyBytesMatchGolden) {
    CGExchangeBuy packet;
    fill(packet);
    expectGolden("CGExchangeBuy", 0, writeBody(packet, 0));
}

//////////////////////////////////////////////////////////////////////
// GCExchangeList — int32 page, int32 pageSize, int32 total, u16 count,
// then per listing the 31-field layout pinned by expectListingEqual().
// The fixture carries one all-strings-set listing and one all-strings-
// empty listing, so both sides of the bstr framing are on the wire.
//////////////////////////////////////////////////////////////////////

TEST(GCExchangeListTest, roundTripsThroughLoopback) {
    for (size_t i = 0; i < kEncryptCodeCount; i++) {
        SCOPED_TRACE(testing::Message() << "encrypt code " << (int)kEncryptCodes[i]);
        GCExchangeList src;
        fill(src);
        GCExchangeList dst;
        roundTrip(src, dst, kEncryptCodes[i]);
        expectEqual(src, dst);
    }
}

TEST(GCExchangeListTest, getPacketSizeMatchesWrittenBody) {
    GCExchangeList packet;
    fill(packet);
    EXPECT_EQ(packet.getPacketSize(), writeBody(packet, 0).size());
}

TEST(GCExchangeListTest, bodyBytesMatchGolden) {
    GCExchangeList packet;
    fill(packet);
    expectGolden("GCExchangeList", 0, writeBody(packet, 0));
}

//////////////////////////////////////////////////////////////////////
// GCExchangeBuy — u8 success, bstr message, u64 orderID
//////////////////////////////////////////////////////////////////////

TEST(GCExchangeBuyTest, roundTripsThroughLoopback) {
    for (size_t i = 0; i < kEncryptCodeCount; i++) {
        SCOPED_TRACE(testing::Message() << "encrypt code " << (int)kEncryptCodes[i]);
        GCExchangeBuy src;
        fill(src);
        GCExchangeBuy dst;
        roundTrip(src, dst, kEncryptCodes[i]);
        expectEqual(src, dst);
    }
}

TEST(GCExchangeBuyTest, getPacketSizeMatchesWrittenBody) {
    GCExchangeBuy packet;
    fill(packet);
    EXPECT_EQ(packet.getPacketSize(), writeBody(packet, 0).size());
}

TEST(GCExchangeBuyTest, bodyBytesMatchGolden) {
    GCExchangeBuy packet;
    fill(packet);
    expectGolden("GCExchangeBuy", 0, writeBody(packet, 0));
}

} // namespace
