//////////////////////////////////////////////////////////////////////
//
// Filename    : wire_string_test.cpp
// Description : Pins de::wire::readString / writeString / stringWireSize,
//               the length-prefixed string field the packets are written
//               in terms of, and the WORD-prefixed readString16 /
//               writeString16 / stringWireSize16 beside them.
//
// The helper carries the bounds of ~90 packet fields, so its refusal
// points are protocol: which lengths reach a field, where a rejected
// read leaves the stream, and how many bytes a field costs. Everything
// here runs through the real socket streams.
//
//////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <string_view>

#include "Exception.h"
#include "SocketEncryptOutputStream.h"
#include "TestStreams.h"
#include "Types.h"
#include "WireString.h"

using de::wire::readString;
using de::wire::readString16;
using de::wire::StringBounds;
using de::wire::StringBounds16;
using de::wire::stringWireSize;
using de::wire::stringWireSize16;
using de::wire::writeString;
using de::wire::writeString16;

namespace {

// The bytes writeString() puts on the wire, taken straight out of a
// fresh buffer the way the packet goldens are taken.
std::vector<unsigned char> written(std::string_view value, StringBounds bounds) {
    SocketEncryptOutputStream oStream(NULL);
    oStream.setEncryptCode(0);
    writeString(oStream, value, bounds, "Field");
    const char* pBuffer = oStream.getBuffer();
    return std::vector<unsigned char>(pBuffer, pBuffer + oStream.length());
}

// Write with `writeBounds`, read back with `readBounds`, through a real
// loopback connection.
std::string roundTrip(const std::string& value, StringBounds writeBounds, StringBounds readBounds) {
    wiretest::Loopback link;
    link.setCodes(0);

    writeString(link.out(), value, writeBounds, "Field");
    link.pump(szBYTE + value.size());

    std::string out = "left over from the last read";
    readString(link.in(), out, readBounds, "Field");
    return out;
}

std::string roundTrip(const std::string& value, StringBounds bounds) {
    return roundTrip(value, bounds, bounds);
}

std::vector<unsigned char> image(unsigned prefix, const std::string& payload) {
    std::vector<unsigned char> bytes;
    bytes.push_back((unsigned char)prefix);
    bytes.insert(bytes.end(), payload.begin(), payload.end());
    return bytes;
}

} // namespace

// The field is the BYTE length and then the bytes, with nothing else
// around it - that is what keeps every adopted packet's golden identical.
TEST(WireStringTest, theFieldIsTheLengthPrefixAndThenTheBytes) {
    EXPECT_EQ(image(3, "abc"), written("abc", StringBounds{0, 20}));
    EXPECT_EQ(image(0, ""), written("", StringBounds{0, 20}));
    EXPECT_EQ(image(1, "x"), written("x", StringBounds{1, 1}));
}

// Both ends are inclusive: the bound itself is admitted, one past it is
// not, on the way out and on the way back.
TEST(WireStringTest, bothBoundsAreInclusive) {
    const StringBounds bounds{2, 4};

    EXPECT_EQ("ab", roundTrip("ab", bounds));
    EXPECT_EQ("abcd", roundTrip("abcd", bounds));

    SocketEncryptOutputStream oStream(NULL);
    EXPECT_THROW(writeString(oStream, "a", bounds, "Field"), InvalidProtocolException);
    EXPECT_THROW(writeString(oStream, "abcde", bounds, "Field"), InvalidProtocolException);

    // A length the writer would have refused is refused on read too.
    EXPECT_THROW(roundTrip("a", StringBounds{0, 10}, bounds), InvalidProtocolException);
    EXPECT_THROW(roundTrip("abcde", StringBounds{0, 10}, bounds), InvalidProtocolException);
}

// min 0 is the "may be absent" field; min 1 is the one that refuses an
// empty value. Both spellings are in the adopted set.
TEST(WireStringTest, minZeroAdmitsTheEmptyValueAndMinOneRefusesIt) {
    EXPECT_EQ("", roundTrip("", StringBounds{0, 20}));

    SocketEncryptOutputStream oStream(NULL);
    EXPECT_THROW(writeString(oStream, "", StringBounds{1, 20}, "Field"), InvalidProtocolException);

    EXPECT_THROW(roundTrip("", StringBounds{0, 20}, StringBounds{1, 20}), InvalidProtocolException);
}

// An admitted empty field assigns rather than reads, since the stream's
// read(string&, uint) refuses a zero length. The value it assigns is
// empty even when the destination already held something.
TEST(WireStringTest, anAdmittedEmptyValueClearsTheDestination) {
    wiretest::Loopback link;
    link.setCodes(0);

    writeString(link.out(), "", StringBounds{0, 20}, "Field");
    link.pump(szBYTE);

    std::string out = "still here";
    readString(link.in(), out, StringBounds{0, 20}, "Field");
    EXPECT_TRUE(out.empty());
}

// 255 is everything a BYTE prefix can describe, so it is the largest
// field the helper can carry.
TEST(WireStringTest, twoHundredFiftyFiveIsTheLargestField) {
    const std::string full(de::wire::kMaxByteStringLength, 'x');

    EXPECT_EQ(full, roundTrip(full, StringBounds{0, de::wire::kMaxByteStringLength}));
    EXPECT_EQ(256u, written(full, StringBounds{0, de::wire::kMaxByteStringLength}).size());

    SocketEncryptOutputStream oStream(NULL);
    EXPECT_THROW(writeString(oStream, std::string(256, 'x'), StringBounds{0, de::wire::kMaxByteStringLength}, "Field"),
                 InvalidProtocolException);
}

// Where a refused read leaves the stream. The prefix is consumed before
// the bounds are judged - the bytes it announced are still buffered,
// exactly as the hand-written sequences left them.
TEST(WireStringTest, aPrefixPastTheMaxIsRefusedAfterThePrefixIsConsumed) {
    wiretest::Loopback link;
    link.setCodes(0);

    writeString(link.out(), "0123456789", StringBounds{0, 20}, "Field");
    link.pump(szBYTE + 10);

    const uint buffered = link.in().length();

    std::string out;
    EXPECT_THROW(readString(link.in(), out, StringBounds{1, 4}, "Field"), InvalidProtocolException);
    EXPECT_EQ(buffered - szBYTE, link.in().length());
}

// A refused write emits nothing, so a packet that cannot honour its own
// bounds does not put half a field on the wire.
TEST(WireStringTest, aRefusedWriteEmitsNothing) {
    SocketEncryptOutputStream oStream(NULL);
    oStream.setEncryptCode(0);
    oStream.write((BYTE)0xAB);

    const uint before = oStream.length();

    EXPECT_THROW(writeString(oStream, "toolong", StringBounds{0, 3}, "Field"), InvalidProtocolException);
    EXPECT_EQ(before, oStream.length());

    EXPECT_THROW(writeString(oStream, "", StringBounds{1, 3}, "Field"), InvalidProtocolException);
    EXPECT_EQ(before, oStream.length());
}

// The size a field contributes to getPacketSize(). Constant-evaluated,
// because that is how the packets use it.
static_assert(stringWireSize("") == szBYTE);
static_assert(stringWireSize("abcd") == szBYTE + 4);

TEST(WireStringTest, theWireSizeIsThePrefixPlusTheBytes) {
    const std::string value = "abcdefghij";
    EXPECT_EQ(stringWireSize(value), written(value, StringBounds{0, 20}).size());
    EXPECT_EQ(stringWireSize(""), written("", StringBounds{0, 20}).size());
    EXPECT_EQ(szBYTE + de::wire::kMaxByteStringLength,
              stringWireSize(std::string(de::wire::kMaxByteStringLength, 'x')));
}

//////////////////////////////////////////////////////////////////////
// The WORD-prefixed field
//
// A few fields carry their length in a WORD, for values longer than a
// byte can count. Everything below mirrors the BYTE cases above, so the
// two prefixes are pinned to the same refusal points.
//////////////////////////////////////////////////////////////////////

namespace {

std::vector<unsigned char> written16(std::string_view value, StringBounds16 bounds) {
    SocketEncryptOutputStream oStream(NULL);
    oStream.setEncryptCode(0);
    writeString16(oStream, value, bounds, "Field");
    const char* pBuffer = oStream.getBuffer();
    return std::vector<unsigned char>(pBuffer, pBuffer + oStream.length());
}

std::string roundTrip16(const std::string& value, StringBounds16 writeBounds, StringBounds16 readBounds) {
    wiretest::Loopback link;
    link.setCodes(0);

    writeString16(link.out(), value, writeBounds, "Field");
    link.pump(szWORD + value.size());

    std::string out = "left over from the last read";
    readString16(link.in(), out, readBounds, "Field");
    return out;
}

std::string roundTrip16(const std::string& value, StringBounds16 bounds) {
    return roundTrip16(value, bounds, bounds);
}

// The prefix goes on the wire as the raw object, so the expected image
// carries the same bytes in the same order.
std::vector<unsigned char> image16(unsigned prefix, const std::string& payload) {
    const WORD length = (WORD)prefix;
    const unsigned char* pLength = (const unsigned char*)&length;

    std::vector<unsigned char> bytes(pLength, pLength + szWORD);
    bytes.insert(bytes.end(), payload.begin(), payload.end());
    return bytes;
}

} // namespace

TEST(WireStringTest, theWordFieldIsTheLengthWordAndThenTheBytes) {
    EXPECT_EQ(image16(3, "abc"), written16("abc", StringBounds16{0, 20}));
    EXPECT_EQ(image16(0, ""), written16("", StringBounds16{0, 20}));
    EXPECT_EQ(image16(1, "x"), written16("x", StringBounds16{1, 1}));
}

TEST(WireStringTest, bothWordBoundsAreInclusive) {
    const StringBounds16 bounds{2, 4};

    EXPECT_EQ("ab", roundTrip16("ab", bounds));
    EXPECT_EQ("abcd", roundTrip16("abcd", bounds));

    SocketEncryptOutputStream oStream(NULL);
    EXPECT_THROW(writeString16(oStream, "a", bounds, "Field"), InvalidProtocolException);
    EXPECT_THROW(writeString16(oStream, "abcde", bounds, "Field"), InvalidProtocolException);

    EXPECT_THROW(roundTrip16("a", StringBounds16{0, 10}, bounds), InvalidProtocolException);
    EXPECT_THROW(roundTrip16("abcde", StringBounds16{0, 10}, bounds), InvalidProtocolException);
}

TEST(WireStringTest, minZeroAdmitsTheEmptyWordValueAndMinOneRefusesIt) {
    EXPECT_EQ("", roundTrip16("", StringBounds16{0, 20}));

    SocketEncryptOutputStream oStream(NULL);
    EXPECT_THROW(writeString16(oStream, "", StringBounds16{1, 20}, "Field"), InvalidProtocolException);

    EXPECT_THROW(roundTrip16("", StringBounds16{0, 20}, StringBounds16{1, 20}), InvalidProtocolException);
}

TEST(WireStringTest, anAdmittedEmptyWordValueClearsTheDestination) {
    wiretest::Loopback link;
    link.setCodes(0);

    writeString16(link.out(), "", StringBounds16{0, 20}, "Field");
    link.pump(szWORD);

    std::string out = "still here";
    readString16(link.in(), out, StringBounds16{0, 20}, "Field");
    EXPECT_TRUE(out.empty());
}

// What the WORD prefix is for: a value the BYTE prefix could not
// describe travels whole.
TEST(WireStringTest, theWordFieldCarriesPastWhatAByteCounts) {
    const std::string value(300, 'w');

    EXPECT_EQ(value, roundTrip16(value, StringBounds16{0, 1024}));
    EXPECT_EQ(szWORD + 300, written16(value, StringBounds16{0, 1024}).size());
}

// 65535 is everything a WORD prefix can describe, so it is the largest
// field readString16 / writeString16 can carry.
TEST(WireStringTest, sixtyFiveThousandFiveHundredThirtyFiveIsTheLargestWordField) {
    const std::string full(de::wire::kMaxWordStringLength, 'x');
    const StringBounds16 bounds{0, de::wire::kMaxWordStringLength};

    EXPECT_EQ(szWORD + de::wire::kMaxWordStringLength, written16(full, bounds).size());

    SocketEncryptOutputStream oStream(NULL);
    EXPECT_THROW(writeString16(oStream, std::string(65536, 'x'), bounds, "Field"), InvalidProtocolException);
}

// Where a refused read leaves the stream: the prefix is consumed, the
// bytes it announced are still buffered.
TEST(WireStringTest, aWordPrefixPastTheMaxIsRefusedAfterThePrefixIsConsumed) {
    wiretest::Loopback link;
    link.setCodes(0);

    writeString16(link.out(), "0123456789", StringBounds16{0, 20}, "Field");
    link.pump(szWORD + 10);

    const uint buffered = link.in().length();

    std::string out;
    EXPECT_THROW(readString16(link.in(), out, StringBounds16{1, 4}, "Field"), InvalidProtocolException);
    EXPECT_EQ(buffered - szWORD, link.in().length());
}

TEST(WireStringTest, aRefusedWordWriteEmitsNothing) {
    SocketEncryptOutputStream oStream(NULL);
    oStream.setEncryptCode(0);
    oStream.write((BYTE)0xAB);

    const uint before = oStream.length();

    EXPECT_THROW(writeString16(oStream, "toolong", StringBounds16{0, 3}, "Field"), InvalidProtocolException);
    EXPECT_EQ(before, oStream.length());

    EXPECT_THROW(writeString16(oStream, "", StringBounds16{1, 3}, "Field"), InvalidProtocolException);
    EXPECT_EQ(before, oStream.length());
}

static_assert(stringWireSize16("") == szWORD);
static_assert(stringWireSize16("abcd") == szWORD + 4);

TEST(WireStringTest, theWordWireSizeIsThePrefixPlusTheBytes) {
    const std::string value = "abcdefghij";
    EXPECT_EQ(stringWireSize16(value), written16(value, StringBounds16{0, 20}).size());
    EXPECT_EQ(stringWireSize16(""), written16("", StringBounds16{0, 20}).size());
}
