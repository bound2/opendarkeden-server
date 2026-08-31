//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_exchange_test.cpp
// Description : Round-trip, golden, cap and size pins for the four
//               Exchange packets (CG/GCExchangeList, CG/GCExchangeBuy).
//
//               These four were found divergent between the server and
//               client repos by the docs/RESTRUCTURING.md 1.4 cross-check:
//               the two hand-maintained copies disagreed on string
//               framing and on getPacketSize() vs write(). The tests
//               here pin the RECONCILED contract:
//
//               - every string is a "bstr": a BYTE length prefix that is
//                 ALWAYS written, followed by that many raw bytes, with
//                 the length CLAMPED to a per-field cap that write() and
//                 getPacketSize() share. An empty string is still one
//                 0x00 byte on the wire;
//               - getPacketSize() equals the byte count write() emits for
//                 ANY value, including strings past the cap (the drift
//                 class the 1.4 diff kept finding), checked directly and
//                 implicitly by roundTrip(), which pumps exactly
//                 getPacketSize() bytes. writePacket() puts
//                 getPacketSize() on the wire BEFORE calling write(), so
//                 a disagreement is not a wrong length, it is a stream
//                 that never resynchronises;
//               - getPacketSize() never exceeds the factory's
//                 getPacketMaxSize(), because the receiving side sizes its
//                 read buffer from the factory max. A body that outgrows
//                 it is a truncated packet, not a caught error;
//               - read() RESETS the destination. The servers reuse one
//                 packet object per connection, so a read() that only
//                 assigns when the length byte is non-zero leaves the
//                 PREVIOUS message in place and hands the handler a value
//                 that was never sent.
//
//               Three things about these pins are deliberate and easy to
//               weaken by accident:
//
//               1. The empty-string cases are pinned as GOLDEN BYTES, not
//                  only as round-trips. A copy that skips the length byte
//                  for "" round-trips against ITSELF perfectly — src and
//                  dst agree, every field matches — and only the byte
//                  count gives it away. GCExchangeBuy is the sharpest
//                  case: its message sits mid-packet, followed by a u64
//                  orderID, so dropping the prefix silently shifts the
//                  order id by one byte for the real client.
//               2. The over-cap cases exist because `(uint8_t)len` wraps.
//                  A 256-byte string wraps to a length byte of 0; a
//                  300-byte one wraps to 44. Both make write() and
//                  getPacketSize() disagree unless they clamp identically.
//               3. The worst-case sizes are asserted against an
//                  instantiated factory, not against a literal, so the cap
//                  constants and the factory formula are checked to be the
//                  same arithmetic. The literals live in exactly one test
//                  (factoryMaxSizesMatchTheClientRepo), because they are a
//                  CROSS-REPO contract: the client's hand-maintained
//                  factories must return the same numbers, and
//                  tests/wire-layout.txt diffs them.
//
//               None of the four references the encrypter — they always
//               take the plain path — so, as with CGSay/CGWhisper in
//               packet_roundtrip_test.cpp, goldens are recorded at code 0
//               only. exchangePacketsAreStillEncrypterFree proves that
//               assumption on every run: if one of them ever adopts the
//               encrypter, that test fails loudly and per-code goldens
//               must be added.
//
//               Scalar fixture values are distinct per field and >= 128 in
//               every byte where the width allows (see
//               packet_encrypter_test.cpp for the rationale: an all-<128
//               fixture cannot tell BYTE from char, and identical values
//               hide transposed fields). A bool is the one exception — it
//               only ever puts 0 or 1 on the wire. String fixtures for the
//               goldens are readable ASCII; the high-byte string cases are
//               carried by highBytes() in the cap tests below.
//
//////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>

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
// Caps
//
// Copied out of the packet classes into plain size_t constants, once, so
// the length comparisons below read against std::string::size() without a
// signed/width dance, and so a rename in the headers breaks compilation
// here in one place instead of thirty. The copy is an initialiser, so the
// VALUE always comes from the header; nothing here restates it except
// stringCapsMatchTheClientRepo, which restates it on purpose.
//////////////////////////////////////////////////////////////////////

const size_t kSellerFilterCap = CGExchangeList::kMaxSellerFilter;
const size_t kIdempotencyKeyCap = CGExchangeBuy::kMaxIdempotencyKey;
const size_t kMessageCap = GCExchangeBuy::kMaxMessage;
const size_t kListingStringCap = GCExchangeList::kMaxListingString;
const size_t kListingsPerPage = GCExchangeList::kMaxListingsPerPage;

// Lengths every over-cap test runs. 256 is the adversarial one: a naive
// `(uint8_t)str.length()` wraps it to 0, so a broken write() emits a
// zero-length string while getPacketSize() still counts the bytes. 300
// wraps to 44 instead, so both a zero and a non-zero wrap are covered.
// Both exceed every cap above.
const size_t kOverCapLengths[] = {256, 300};
const size_t kOverCapLengthCount = sizeof(kOverCapLengths) / sizeof(kOverCapLengths[0]);

// A string of `n` bytes whose content varies with the index and whose
// every byte is >= 128. Both properties earn their keep: a run of one
// repeated character cannot tell a truncation that kept the WRONG end
// from one that kept the right end, and an all-ASCII payload cannot tell
// a raw byte copy from something that sign-extends or re-encodes. Never
// emits 0x00, so read()'s `buf[len] = '\0'; m_X = buf;` cannot silently
// shorten the result and make a framing bug look like a passing test.
std::string highBytes(size_t n, unsigned char seed) {
    std::string s;
    s.reserve(n);
    for (size_t i = 0; i < n; i++)
        s.push_back((char)(unsigned char)(0x80 + ((seed + i) % 0x7F)));
    return s;
}

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
    // Every byte >= 0x80, like every other scalar here. The older value
    // 0xC5D6E7F889192A3B had three bytes below 128 (0x19, 0x2A, 0x3B),
    // which quietly falsified the file header's claim and cost the low
    // half of this field the BYTE-vs-char coverage the fixture exists for.
    l.itemID = static_cast<int64_t>(0xC5D6E7F899A8B7C6ULL);
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

// Listing 3: the seven wire strings all at `len` bytes, distinct per
// field so a transposition inside the listing is visible. Feeds both the
// over-cap tests (len > kListingStringCap) and the worst-case size test
// (len == kListingStringCap).
ExchangeListing makeLongStringListing(size_t len) {
    ExchangeListing l = makeFullListing();
    l.sellerAccount = highBytes(len, 0x01);
    l.sellerPlayer = highBytes(len, 0x11);
    l.buyerAccount = highBytes(len, 0x21);
    l.buyerPlayer = highBytes(len, 0x31);
    l.createdAt = highBytes(len, 0x41);
    l.expireAt = highBytes(len, 0x51);
    l.itemName = highBytes(len, 0x61);
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

// The over-cap expectation for a listing: each of the seven strings comes
// back clamped to exactly the cap, and the scalars around them are
// untouched — proof the clamp moved the write cursor by exactly as many
// bytes as read() consumed.
void expectListingTruncatedToCap(const ExchangeListing& src, const ExchangeListing& dst) {
    EXPECT_EQ(src.sellerAccount.substr(0, kListingStringCap), dst.sellerAccount);
    EXPECT_EQ(src.sellerPlayer.substr(0, kListingStringCap), dst.sellerPlayer);
    EXPECT_EQ(src.buyerAccount.substr(0, kListingStringCap), dst.buyerAccount);
    EXPECT_EQ(src.buyerPlayer.substr(0, kListingStringCap), dst.buyerPlayer);
    EXPECT_EQ(src.createdAt.substr(0, kListingStringCap), dst.createdAt);
    EXPECT_EQ(src.expireAt.substr(0, kListingStringCap), dst.expireAt);
    EXPECT_EQ(src.itemName.substr(0, kListingStringCap), dst.itemName);
    EXPECT_EQ(kListingStringCap, dst.itemName.size());
    EXPECT_EQ(src.listingID, dst.listingID);
    EXPECT_EQ(src.pricePoint, dst.pricePoint);
    EXPECT_EQ(src.version, dst.version);
    EXPECT_EQ(src.stackCount, dst.stackCount);
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
// Factory maxes and caps — the cross-repo contract
//////////////////////////////////////////////////////////////////////

// The one place a literal max size is written down. These four numbers
// are a CROSS-REPO contract: the client repo's hand-maintained
// CGExchangeListFactory / CGExchangeBuyFactory / GCExchangeBuyFactory /
// GCExchangeListFactory must return exactly the same values, because the
// receiving side sizes its packet buffer from them. tests/wire-layout.txt
// carries the same numbers, so a change here is also a wire-layout diff
// and must ship in the client before it ships here.
//
// The arithmetic behind each:
//   CGExchangeList  4 + 4 + 1 + 2 + 4 + 4 + (1 + 255)         =   275
//   CGExchangeBuy   8 + (1 + 64)                              =    73
//   GCExchangeBuy   1 + (1 + 255) + 8                         =   265
//   GCExchangeList  4 + 4 + 4 + 2 + 20 * 1855                 = 37114
TEST(ExchangeFactoryTest, factoryMaxSizesMatchTheClientRepo) {
    CGExchangeListFactory cgListFactory;
    CGExchangeBuyFactory cgBuyFactory;
    GCExchangeBuyFactory gcBuyFactory;
    GCExchangeListFactory gcListFactory;

    EXPECT_EQ((PacketSize_t)275, cgListFactory.getPacketMaxSize());
    EXPECT_EQ((PacketSize_t)73, cgBuyFactory.getPacketMaxSize());
    EXPECT_EQ((PacketSize_t)265, gcBuyFactory.getPacketMaxSize());
    EXPECT_EQ((PacketSize_t)37114, gcListFactory.getPacketMaxSize());
}

// The caps are the other half of that contract: they are what write()
// clamps to, so the client must clamp to the same numbers or the two
// sides disagree about how many bytes a long string occupies.
TEST(ExchangeFactoryTest, stringCapsMatchTheClientRepo) {
    EXPECT_EQ((size_t)255, kSellerFilterCap);
    EXPECT_EQ((size_t)64, kIdempotencyKeyCap);
    EXPECT_EQ((size_t)255, kMessageCap);
    EXPECT_EQ((size_t)255, kListingStringCap);
    EXPECT_EQ((size_t)20, kListingsPerPage);

    // A cap above 255 cannot be expressed by a BYTE length prefix at all:
    // write() would emit a wrapped length and every later field would
    // shift. Stated here so a "let's allow longer names" change fails in
    // one obvious place instead of four subtle ones.
    EXPECT_LE(kSellerFilterCap, (size_t)255);
    EXPECT_LE(kIdempotencyKeyCap, (size_t)255);
    EXPECT_LE(kMessageCap, (size_t)255);
    EXPECT_LE(kListingStringCap, (size_t)255);
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

// The round-trip above cannot fail an implementation that omits the
// length byte for "" AND expects it omitted — sellerFilter is the last
// field, so there is nothing after it to shift, and the only symptom is a
// pump() stall. Only the bytes tell. Every non-string field is non-zero,
// so a golden recorded from a half-written packet is still obviously
// wrong.
TEST(CGExchangeListTest, emptySellerFilterBytesMatchGolden) {
    CGExchangeList packet;
    fill(packet);
    packet.setSellerFilter("");
    expectGolden("CGExchangeList.empty", 0, writeBody(packet, 0));
}

// 255 is the largest length a BYTE prefix can express and the exact
// boundary of read()'s `char buf[256]` (it writes buf[len] = '\0', so
// len == 256 would run off the end). Exercise it for real rather than
// only in the size arithmetic.
TEST(CGExchangeListTest, roundTripsWithMaxLengthSellerFilter) {
    CGExchangeList src;
    fill(src);
    src.setSellerFilter(highBytes(kSellerFilterCap, 0x07));
    ASSERT_EQ(kSellerFilterCap, src.getSellerFilter().size());
    CGExchangeList dst;
    roundTrip(src, dst, 0);
    expectEqual(src, dst);
}

TEST(CGExchangeListTest, overCapSellerFilterIsTruncatedToTheCap) {
    for (size_t i = 0; i < kOverCapLengthCount; i++) {
        SCOPED_TRACE(testing::Message() << "sellerFilter length " << kOverCapLengths[i]);
        CGExchangeList src;
        fill(src);
        src.setSellerFilter(highBytes(kOverCapLengths[i], 0x13));

        // (a) the two size paths agree even past the cap. Without this the
        // round-trip below would hang rather than fail: roundTrip() pumps
        // getPacketSize() bytes.
        EXPECT_EQ(src.getPacketSize(), writeBody(src, 0).size());

        // (b) what comes back is the FIRST kSellerFilterCap bytes — not a
        // wrapped-length fragment, and not the whole string.
        CGExchangeList dst;
        roundTrip(src, dst, 0);
        EXPECT_EQ(src.getSellerFilter().substr(0, kSellerFilterCap), dst.getSellerFilter());
        EXPECT_EQ(kSellerFilterCap, dst.getSellerFilter().size());
        EXPECT_EQ(src.getPage(), dst.getPage());
        EXPECT_EQ(src.getMaxPrice(), dst.getMaxPrice());
    }
}

TEST(CGExchangeListTest, getPacketSizeMatchesWrittenBody) {
    CGExchangeList packet;
    fill(packet);
    EXPECT_EQ(packet.getPacketSize(), writeBody(packet, 0).size());
}

// The invariant the receiver depends on, and one nothing else in the
// suite asserts. It is silently violated the moment a cap and the factory
// formula drift apart.
TEST(CGExchangeListTest, worstCaseSizeFitsTheFactoryMax) {
    CGExchangeList packet;
    fill(packet);
    packet.setSellerFilter(highBytes(kSellerFilterCap, 0x17));

    CGExchangeListFactory factory;
    EXPECT_EQ(packet.getPacketSize(), writeBody(packet, 0).size());
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize())
        << "the largest legal CGExchangeList does not fit the buffer the receiver allocates";
    // The max is meant to be exactly the worst case, not a guess with
    // headroom: that is what proves the factory formula and the cap are
    // the same arithmetic. If headroom is added on purpose, relax this
    // line and keep the EXPECT_LE above.
    EXPECT_EQ(factory.getPacketMaxSize(), packet.getPacketSize())
        << "getPacketMaxSize() is no longer the exact worst case — was that deliberate?";
}

// The servers reuse one packet object per connection. A read() that only
// assigns the string when the length byte is non-zero hands the handler
// the PREVIOUS request's filter — a wrong answer and a cross-request leak
// in one.
TEST(CGExchangeListTest, readClearsTheSellerFilterOnAReusedDestination) {
    CGExchangeList dst;

    CGExchangeList first;
    fill(first);
    roundTrip(first, dst, 0);
    ASSERT_FALSE(dst.getSellerFilter().empty());

    CGExchangeList second;
    fill(second);
    second.setPage((int)0xC1D2E3F4);
    second.setSellerFilter("");
    roundTrip(second, dst, 0);
    expectEqual(second, dst);
    EXPECT_TRUE(dst.getSellerFilter().empty()) << "read() kept the previous request's seller filter";
}

// A default-constructed packet must be fully determined: a handler builds
// one and hands it to read(), and anything read() does not overwrite goes
// out on the wire on the next write(). The two sentinels are semantics,
// not padding — 0xFF/0xFFFF mean "no class/type filter" — so the client's
// copy has to default to the same values.
TEST(CGExchangeListTest, defaultConstructedFieldsAreTheDocumentedDefaults) {
    CGExchangeList packet;
    EXPECT_EQ(1, packet.getPage());
    EXPECT_EQ(20, packet.getPageSize());
    EXPECT_EQ((uint8_t)0xFF, packet.getItemClass()) << "0xFF is the 'any item class' sentinel";
    EXPECT_EQ((uint16_t)0xFFFF, packet.getItemType()) << "0xFFFF is the 'any item type' sentinel";
    EXPECT_EQ(0, packet.getMinPrice());
    EXPECT_EQ(0, packet.getMaxPrice());
    EXPECT_TRUE(packet.getSellerFilter().empty());
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

// As with CGExchangeList: the key is the last field, so a self-consistent
// "no prefix when empty" implementation round-trips clean. The bytes are
// the only witness.
TEST(CGExchangeBuyTest, emptyIdempotencyKeyBytesMatchGolden) {
    CGExchangeBuy packet;
    fill(packet);
    packet.setIdempotencyKey("");
    expectGolden("CGExchangeBuy.empty", 0, writeBody(packet, 0));
}

TEST(CGExchangeBuyTest, overCapIdempotencyKeyIsTruncatedToTheCap) {
    for (size_t i = 0; i < kOverCapLengthCount; i++) {
        SCOPED_TRACE(testing::Message() << "idempotencyKey length " << kOverCapLengths[i]);
        CGExchangeBuy src;
        fill(src);
        src.setIdempotencyKey(highBytes(kOverCapLengths[i], 0x23));

        EXPECT_EQ(src.getPacketSize(), writeBody(src, 0).size());

        // This cap is 64, well under the 255 a length byte could carry, so
        // it is the one field where the clamp is a real policy decision
        // rather than a consequence of the prefix width.
        CGExchangeBuy dst;
        roundTrip(src, dst, 0);
        EXPECT_EQ(src.getIdempotencyKey().substr(0, kIdempotencyKeyCap), dst.getIdempotencyKey());
        EXPECT_EQ(kIdempotencyKeyCap, dst.getIdempotencyKey().size());
        EXPECT_EQ(src.getListingID(), dst.getListingID());
    }
}

TEST(CGExchangeBuyTest, getPacketSizeMatchesWrittenBody) {
    CGExchangeBuy packet;
    fill(packet);
    EXPECT_EQ(packet.getPacketSize(), writeBody(packet, 0).size());
}

TEST(CGExchangeBuyTest, worstCaseSizeFitsTheFactoryMax) {
    CGExchangeBuy packet;
    fill(packet);
    packet.setIdempotencyKey(highBytes(kIdempotencyKeyCap, 0x27));

    CGExchangeBuyFactory factory;
    EXPECT_EQ(packet.getPacketSize(), writeBody(packet, 0).size());
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize())
        << "the largest legal CGExchangeBuy does not fit the buffer the receiver allocates";
    EXPECT_EQ(factory.getPacketMaxSize(), packet.getPacketSize())
        << "getPacketMaxSize() is no longer the exact worst case — was that deliberate?";
}

// A key that survives from a previous request is worse than a wrong
// string: it is the key the purchase is deduplicated on.
TEST(CGExchangeBuyTest, readClearsTheIdempotencyKeyOnAReusedDestination) {
    CGExchangeBuy dst;

    CGExchangeBuy first;
    fill(first);
    roundTrip(first, dst, 0);
    ASSERT_FALSE(dst.getIdempotencyKey().empty());

    CGExchangeBuy second;
    fill(second);
    second.setListingID(static_cast<int64_t>(0xA1B2C3D4E5F68798ULL));
    second.setIdempotencyKey("");
    roundTrip(second, dst, 0);
    expectEqual(second, dst);
    EXPECT_TRUE(dst.getIdempotencyKey().empty()) << "read() kept the previous request's idempotency key";
}

TEST(CGExchangeBuyTest, bodyBytesMatchGolden) {
    CGExchangeBuy packet;
    fill(packet);
    expectGolden("CGExchangeBuy", 0, writeBody(packet, 0));
}

//////////////////////////////////////////////////////////////////////
// GCExchangeBuy — u8 success, bstr message, u64 orderID
//
// The only Exchange packet whose string is NOT the last field. That makes
// it the one where dropping the empty-string length byte is invisible to
// a same-implementation round-trip on the field values alone, while
// shifting orderID by one byte for the real client — hence the empty
// golden below.
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

// "Purchase failed with no explanation" is a real response, and it is the
// one that puts an empty string in front of a u64.
TEST(GCExchangeBuyTest, roundTripsWithEmptyMessage) {
    GCExchangeBuy src;
    fill(src);
    src.setSuccess(false);
    src.setMessage("");
    GCExchangeBuy dst;
    roundTrip(src, dst, 0);
    expectEqual(src, dst);
    EXPECT_TRUE(dst.getMessage().empty());
    EXPECT_EQ(static_cast<int64_t>(0x8192A3B4C5D6E7F8ULL), dst.getOrderID())
        << "orderID shifted — the empty message's length byte is missing from the wire";
}

// success stays true and orderID stays non-zero so the golden shows an
// otherwise fully populated packet: 0x01, 0x00, then eight order-id
// bytes. Ten bytes if the prefix is there, nine if it is not.
TEST(GCExchangeBuyTest, emptyMessageBytesMatchGolden) {
    GCExchangeBuy packet;
    fill(packet);
    packet.setMessage("");
    expectGolden("GCExchangeBuy.empty", 0, writeBody(packet, 0));
}

TEST(GCExchangeBuyTest, roundTripsWithMaxLengthMessage) {
    GCExchangeBuy src;
    fill(src);
    src.setMessage(highBytes(kMessageCap, 0x33));
    ASSERT_EQ(kMessageCap, src.getMessage().size());
    GCExchangeBuy dst;
    roundTrip(src, dst, 0);
    expectEqual(src, dst);
}

TEST(GCExchangeBuyTest, overCapMessageIsTruncatedToTheCap) {
    for (size_t i = 0; i < kOverCapLengthCount; i++) {
        SCOPED_TRACE(testing::Message() << "message length " << kOverCapLengths[i]);
        GCExchangeBuy src;
        fill(src);
        src.setMessage(highBytes(kOverCapLengths[i], 0x37));

        EXPECT_EQ(src.getPacketSize(), writeBody(src, 0).size());

        GCExchangeBuy dst;
        roundTrip(src, dst, 0);
        EXPECT_EQ(src.getMessage().substr(0, kMessageCap), dst.getMessage());
        EXPECT_EQ(kMessageCap, dst.getMessage().size());
        // The field AFTER the string: proof the clamp moved the write
        // cursor by exactly as many bytes as read() consumed.
        EXPECT_EQ(src.getOrderID(), dst.getOrderID());
        EXPECT_EQ(src.getSuccess(), dst.getSuccess());
    }
}

TEST(GCExchangeBuyTest, getPacketSizeMatchesWrittenBody) {
    GCExchangeBuy packet;
    fill(packet);
    EXPECT_EQ(packet.getPacketSize(), writeBody(packet, 0).size());
}

TEST(GCExchangeBuyTest, worstCaseSizeFitsTheFactoryMax) {
    GCExchangeBuy packet;
    fill(packet);
    packet.setMessage(highBytes(kMessageCap, 0x3B));

    GCExchangeBuyFactory factory;
    EXPECT_EQ(packet.getPacketSize(), writeBody(packet, 0).size());
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize())
        << "the largest legal GCExchangeBuy does not fit the buffer the receiver allocates";
    EXPECT_EQ(factory.getPacketMaxSize(), packet.getPacketSize())
        << "getPacketMaxSize() is no longer the exact worst case — was that deliberate?";
}

TEST(GCExchangeBuyTest, readClearsTheMessageOnAReusedDestination) {
    GCExchangeBuy dst;

    GCExchangeBuy first;
    fill(first);
    roundTrip(first, dst, 0);
    ASSERT_FALSE(dst.getMessage().empty());

    GCExchangeBuy second;
    fill(second);
    second.setSuccess(false);
    second.setMessage("");
    second.setOrderID(static_cast<int64_t>(0xB2C3D4E5F6879AABULL));
    roundTrip(second, dst, 0);
    expectEqual(second, dst);
    EXPECT_TRUE(dst.getMessage().empty()) << "read() kept the previous response's message";
}

TEST(GCExchangeBuyTest, bodyBytesMatchGolden) {
    GCExchangeBuy packet;
    fill(packet);
    expectGolden("GCExchangeBuy", 0, writeBody(packet, 0));
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

// count == 0 is the most common real response — an empty search page —
// and was untested. It is also the only shape where the listing loop
// never runs, so a read() that mishandles the count starts consuming the
// next packet's bytes as a listing.
TEST(GCExchangeListTest, roundTripsWithNoListings) {
    GCExchangeList src;
    fill(src);
    src.setListings(std::vector<ExchangeListing>());
    ASSERT_TRUE(src.getListings().empty());

    GCExchangeList dst;
    roundTrip(src, dst, 0);
    expectEqual(src, dst);
    EXPECT_TRUE(dst.getListings().empty());
}

// 14 bytes: three int32 and a u16 zero count. Page/pageSize/total stay
// non-zero so the golden cannot be confused with an unwritten packet.
TEST(GCExchangeListTest, emptyPageBytesMatchGolden) {
    GCExchangeList packet;
    fill(packet);
    packet.setListings(std::vector<ExchangeListing>());
    expectGolden("GCExchangeList.empty", 0, writeBody(packet, 0));
}

TEST(GCExchangeListTest, overCapListingStringsAreTruncatedToTheCap) {
    for (size_t i = 0; i < kOverCapLengthCount; i++) {
        SCOPED_TRACE(testing::Message() << "listing string length " << kOverCapLengths[i]);
        GCExchangeList src;
        fill(src);
        std::vector<ExchangeListing> listings;
        listings.push_back(makeLongStringListing(kOverCapLengths[i]));
        src.setListings(listings);

        EXPECT_EQ(src.getPacketSize(), writeBody(src, 0).size());

        GCExchangeList dst;
        roundTrip(src, dst, 0);
        ASSERT_EQ((size_t)1, dst.getListings().size());
        expectListingTruncatedToCap(src.getListings()[0], dst.getListings()[0]);
        // The last scalar of the listing: seven clamps in a row still land
        // the tail of the record where read() expects it.
        EXPECT_EQ(src.getListings()[0].stackCount, dst.getListings()[0].stackCount);
    }
}

TEST(GCExchangeListTest, getPacketSizeMatchesWrittenBody) {
    GCExchangeList packet;
    fill(packet);
    EXPECT_EQ(packet.getPacketSize(), writeBody(packet, 0).size());
}

// A full page of maximal listings — kMaxListingsPerPage entries, all
// seven strings at kMaxListingString. This is the body the receiver's
// buffer has to hold, and the factory max is derived from exactly this
// shape, so if the two disagree one of them was edited without the other.
// No loopback here on purpose: 37KB through a blocking socket with nobody
// draining it would deadlock, and the invariant is about size, not bytes.
TEST(GCExchangeListTest, worstCaseSizeFitsTheFactoryMax) {
    GCExchangeList packet;
    fill(packet);
    packet.setListings(std::vector<ExchangeListing>(kListingsPerPage, makeLongStringListing(kListingStringCap)));
    ASSERT_EQ(kListingsPerPage, packet.getListings().size());

    GCExchangeListFactory factory;
    EXPECT_EQ(packet.getPacketSize(), writeBody(packet, 0).size());
    EXPECT_LE(packet.getPacketSize(), factory.getPacketMaxSize())
        << "the largest legal GCExchangeList does not fit the buffer the receiver allocates";
    EXPECT_EQ(factory.getPacketMaxSize(), packet.getPacketSize())
        << "getPacketMaxSize() is no longer the exact worst case — was that deliberate?";
}

// read() must clear m_Listings before filling it. Without that, a reused
// packet object accumulates: page two of a search arrives appended to
// page one, and the count on the wire no longer describes the object.
TEST(GCExchangeListTest, readClearsPreviousListingsOnAReusedDestination) {
    GCExchangeList dst;

    GCExchangeList first;
    fill(first);
    roundTrip(first, dst, 0);
    ASSERT_EQ((size_t)2, dst.getListings().size());

    // A shorter page whose one listing has every string empty: catches the
    // vector not being cleared and per-listing strings surviving the read.
    GCExchangeList second;
    fill(second);
    second.setPage((int)0xD3E4F5A6);
    std::vector<ExchangeListing> onlyEmptyStrings;
    onlyEmptyStrings.push_back(makeEmptyStringListing());
    second.setListings(onlyEmptyStrings);
    roundTrip(second, dst, 0);
    expectEqual(second, dst);
    ASSERT_EQ((size_t)1, dst.getListings().size()) << "read() appended to the previous page instead of replacing it";
    EXPECT_TRUE(dst.getListings()[0].sellerAccount.empty());
    EXPECT_TRUE(dst.getListings()[0].itemName.empty());

    // And an empty page after a non-empty one leaves nothing behind.
    GCExchangeList third;
    fill(third);
    third.setListings(std::vector<ExchangeListing>());
    roundTrip(third, dst, 0);
    expectEqual(third, dst);
    EXPECT_TRUE(dst.getListings().empty()) << "read() kept the previous page's listings";
}

// ExchangeListing is filled field by field from a database row, and
// read() constructs a fresh one per listing. Anything its constructor
// leaves indeterminate is a garbage value that goes straight onto the
// wire when a column is NULL or a field is simply forgotten.
TEST(GCExchangeListTest, defaultConstructedListingIsZeroed) {
    ExchangeListing l;
    EXPECT_EQ((int64_t)0, l.listingID);
    EXPECT_EQ((int16_t)0, l.serverID);
    EXPECT_EQ(0, l.sellerRace);
    EXPECT_EQ(0, l.itemClass);
    EXPECT_EQ(0, l.itemType);
    EXPECT_EQ((int64_t)0, l.itemID);
    EXPECT_EQ(0, l.objectID);
    EXPECT_EQ(0, l.pricePoint);
    EXPECT_EQ(0, l.currency);
    EXPECT_EQ(0, l.status);
    EXPECT_EQ(0, l.taxRate);
    EXPECT_EQ(0, l.taxAmount);
    EXPECT_EQ(0, l.version);
    EXPECT_EQ(0, l.enchantLevel);
    EXPECT_EQ(0, l.grade);
    EXPECT_EQ(0, l.durability);
    EXPECT_EQ(0, l.silver);
    EXPECT_EQ(0, l.optionType1);
    EXPECT_EQ(0, l.optionType2);
    EXPECT_EQ(0, l.optionType3);
    EXPECT_EQ(0, l.optionValue1);
    EXPECT_EQ(0, l.optionValue2);
    EXPECT_EQ(0, l.optionValue3);
    EXPECT_EQ(0, l.stackCount);
    EXPECT_TRUE(l.sellerAccount.empty());
    EXPECT_TRUE(l.sellerPlayer.empty());
    EXPECT_TRUE(l.buyerAccount.empty());
    EXPECT_TRUE(l.buyerPlayer.empty());
    EXPECT_TRUE(l.createdAt.empty());
    EXPECT_TRUE(l.expireAt.empty());
    EXPECT_TRUE(l.itemName.empty());
}

// The page header's own defaults, for the same reason as
// CGExchangeList's: an empty result set is built by constructing the
// packet and setting only the counts.
TEST(GCExchangeListTest, defaultConstructedFieldsAreTheDocumentedDefaults) {
    GCExchangeList packet;
    EXPECT_EQ(1, packet.getPage());
    EXPECT_EQ(20, packet.getPageSize());
    EXPECT_EQ(0, packet.getTotal());
    EXPECT_TRUE(packet.getListings().empty());
    EXPECT_EQ(packet.getPacketSize(), writeBody(packet, 0).size());
}

TEST(GCExchangeListTest, bodyBytesMatchGolden) {
    GCExchangeList packet;
    fill(packet);
    expectGolden("GCExchangeList", 0, writeBody(packet, 0));
}

} // namespace
