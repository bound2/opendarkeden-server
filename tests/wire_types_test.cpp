//////////////////////////////////////////////////////////////////////
//
// Filename    : wire_types_test.cpp
// Description : Pins the de::WireScalar accept/reject list and the
//               span-based buffer overloads of the socket streams.
//
// The concept is the protocol's representation rule, so its membership
// is a contract in exactly the way the golden byte fixtures are: a type
// silently joining it can change what a packet puts on the wire, and a
// type silently leaving it stops a packet compiling. The static_asserts
// below fail the BUILD, not the run, which is the point -- they are the
// same kind of check the wire layer now applies to itself.
//
//////////////////////////////////////////////////////////////////////

#include <unistd.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>
#include <type_traits>

#include "Exception.h"
#include "Packet.h"
#include "ServerSocket.h"
#include "Socket.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "Types.h"
#include "WireTypes.h"

//////////////////////////////////////////////////////////////////////
//
// The accept list.
//
// Every entry except `signed char` is a type some packet actually reads
// or writes today, established by instantiating the two templates across
// the whole tree (de-kernel plus all three servers) and listing every T
// that reached them; `signed char` is admitted so the fixed-width
// spellings form complete pairs. Adding a line here widens the
// protocol's type vocabulary and needs the same scrutiny as a new
// packet field.
//
//////////////////////////////////////////////////////////////////////
static_assert(de::WireScalar<bool>);
static_assert(de::WireScalar<char>);
static_assert(de::WireScalar<signed char>);
static_assert(de::WireScalar<unsigned char>);
static_assert(de::WireScalar<short>);
static_assert(de::WireScalar<unsigned short>);
static_assert(de::WireScalar<int>);
static_assert(de::WireScalar<unsigned int>);
static_assert(de::WireScalar<std::uint64_t>);

// The project's own spellings must land on the same types.
static_assert(de::WireScalar<BYTE> && sizeof(BYTE) == 1);
static_assert(de::WireScalar<WORD> && sizeof(WORD) == 2);
static_assert(de::WireScalar<DWORD> && sizeof(DWORD) == 4);
static_assert(de::WireScalar<uchar> && de::WireScalar<ushort> && de::WireScalar<uint>);
// ...and so must the Types.h field aliases the packets are written in.
static_assert(de::WireScalar<Coord_t> && de::WireScalar<Dir_t> && de::WireScalar<Level_t>);
static_assert(de::WireScalar<ObjectID_t> && de::WireScalar<ItemID_t> && de::WireScalar<Gold_t>);
static_assert(de::WireScalar<ItemType_t> && de::WireScalar<SkillType_t> && de::WireScalar<HP_t>);
static_assert(de::WireScalar<PacketID_t> && de::WireScalar<PacketSize_t> && de::WireScalar<SequenceSize_t>);
// cv-qualified forms are the same wire type.
static_assert(de::WireScalar<const unsigned int> && de::WireScalar<volatile BYTE>);

//////////////////////////////////////////////////////////////////////
//
// The reject list.
//
// Each of these compiled before the concept existed and would have put
// an address, an object layout, or a width the Win32 client disagrees
// with onto the wire.
//
//////////////////////////////////////////////////////////////////////
static_assert(!de::WireScalar<char*>);
static_assert(!de::WireScalar<const char*>);
static_assert(!de::WireScalar<void*>);
static_assert(!de::WireScalar<std::string>); // the CGExchangeBuy idempotency-key trap
static_assert(!de::WireScalar<float>);
static_assert(!de::WireScalar<double>);
static_assert(!de::WireScalar<char[4]>);
static_assert(!de::WireScalar<long long>); // signed 64-bit, whatever its spelling
static_assert(!de::WireScalar<wchar_t>);
static_assert(!de::WireScalar<char16_t>);
static_assert(!de::WireScalar<std::int64_t>); // no signed 64-bit field exists

// `long` / `unsigned long` / `size_t` are the platform-sized spellings
// the concept is meant to keep out: 8 bytes here, 4 on the Win32 client.
// `long` lands exactly that way. `unsigned long` and `size_t` are the
// documented exception -- on this LP64 server they ARE std::uint64_t,
// the width the Exchange listing id needs, so the concept cannot tell
// them apart from it. Assert the split that actually holds rather than
// the one we would prefer, so the caveat cannot quietly stop being true
// and so this pins the intent on a platform where the spellings differ.
static_assert(!de::WireScalar<long>);
static_assert(de::WireScalar<unsigned long> == std::is_same_v<unsigned long, std::uint64_t>);
static_assert(de::WireScalar<size_t> == std::is_same_v<size_t, std::uint64_t>);
static_assert(de::WireScalar<unsigned long long> == std::is_same_v<unsigned long long, std::uint64_t>);

// A trivially copyable aggregate is still rejected: "memcpy-able" is not
// the question, "does the client agree on the layout" is.
struct TriviallyCopyablePair {
    unsigned int a;
    unsigned char b;
};
static_assert(std::is_trivially_copyable_v<TriviallyCopyablePair>);
static_assert(!de::WireScalar<TriviallyCopyablePair>);

// Enumerations are rejected too: the underlying type is a compiler
// choice, and the packets spell their fields with the Types.h aliases.
enum PlainEnum { kPlainEnumValue };
enum class ScopedEnum : unsigned char { kScopedEnumValue };
static_assert(!de::WireScalar<PlainEnum>);
static_assert(!de::WireScalar<ScopedEnum>);

//////////////////////////////////////////////////////////////////////
//
// A loopback pair with buffers small enough to force the ring buffer to
// wrap. tests/support/TestStreams.h has the packet-level equivalent;
// this one exists to control the buffer size, and uses the plain
// (unencrypted) streams so the bytes under test are the caller's.
//
//////////////////////////////////////////////////////////////////////
namespace {

class SmallLoopback {
public:
    explicit SmallLoopback(uint bufferSize)
        : m_pServerSocket(NULL), m_pClientSocket(NULL), m_pAcceptedSocket(NULL), m_pOut(NULL), m_pIn(NULL) {
        // A port range of its own, so a run alongside
        // TestStreams::Loopback does not fight it for ports.
        uint port = 43300;
        for (uint attempt = 0; attempt < 64; attempt++, port++) {
            try {
                m_pServerSocket = new ServerSocket(port, 1);
                break;
            } catch (...) {
                m_pServerSocket = NULL;
            }
        }
        if (m_pServerSocket == NULL)
            throw std::runtime_error("SmallLoopback: could not bind any test port");

        m_pClientSocket = new Socket("127.0.0.1", port);
        m_pClientSocket->connect();
        m_pAcceptedSocket = m_pServerSocket->accept();
        if (m_pAcceptedSocket == NULL)
            throw std::runtime_error("SmallLoopback: accept() returned NULL");

        // Non-blocking: fill() must be able to report "nothing more yet"
        // instead of parking in recv().
        m_pAcceptedSocket->setNonBlocking(true);

        m_pOut = new SocketOutputStream(m_pClientSocket, bufferSize);
        m_pIn = new SocketInputStream(m_pAcceptedSocket, bufferSize);
    }

    ~SmallLoopback() {
        delete m_pOut;
        delete m_pIn;
        closeQuietly(m_pClientSocket);
        closeQuietly(m_pAcceptedSocket);
        if (m_pServerSocket != NULL) {
            try {
                m_pServerSocket->close();
            } catch (...) {
            }
        }
        delete m_pClientSocket;
        delete m_pAcceptedSocket;
        delete m_pServerSocket;
    }

    SocketOutputStream& out() {
        return *m_pOut;
    }
    SocketInputStream& in() {
        return *m_pIn;
    }

    // Flush the writer, then fill the reader until it holds nBytes.
    void pump(uint nBytes) {
        m_pOut->flush();
        const int kMaxIdlePolls = 200; // ~1s
        int idlePolls = 0;
        while (m_pIn->length() < nBytes) {
            const uint before = m_pIn->length();
            m_pIn->fill();
            if (m_pIn->length() == before) {
                if (++idlePolls > kMaxIdlePolls) {
                    ADD_FAILURE() << "pump: stalled at " << m_pIn->length() << " of " << nBytes << " bytes";
                    return;
                }
                usleep(5000);
            } else {
                idlePolls = 0;
            }
        }
    }

private:
    static void closeQuietly(Socket* pSocket) {
        if (pSocket == NULL)
            return;
        try {
            pSocket->close();
        } catch (...) {
        }
    }

    ServerSocket* m_pServerSocket;
    Socket* m_pClientSocket;
    Socket* m_pAcceptedSocket;
    SocketOutputStream* m_pOut;
    SocketInputStream* m_pIn;
};

} // namespace

//////////////////////////////////////////////////////////////////////
// Round-trips
//////////////////////////////////////////////////////////////////////

// Every admitted scalar survives the real streams unchanged, and each
// consumes exactly its own width -- the property the whole protocol is
// built on.
TEST(WireScalarRoundTrip, EveryAdmittedScalarKeepsItsValueAndWidth) {
    SmallLoopback loopback(4096);

    const bool wroteBool = true;
    const char wroteChar = 0x7B;
    const signed char wroteSChar = -113;
    const unsigned char wroteUChar = 0xC3;
    const short wroteShort = -21846;
    const unsigned short wroteUShort = 0xBEEF;
    const int wroteInt = -1414812757;
    const unsigned int wroteUInt = 0xDEADBEEFu;
    const std::uint64_t wroteU64 = 0x0123456789ABCDEFull;

    uint written = 0;
    written += loopback.out().write(wroteBool);
    written += loopback.out().write(wroteChar);
    written += loopback.out().write(wroteSChar);
    written += loopback.out().write(wroteUChar);
    written += loopback.out().write(wroteShort);
    written += loopback.out().write(wroteUShort);
    written += loopback.out().write(wroteInt);
    written += loopback.out().write(wroteUInt);
    written += loopback.out().write(wroteU64);
    EXPECT_EQ(1u + 1u + 1u + 1u + 2u + 2u + 4u + 4u + 8u, written);
    EXPECT_EQ(written, loopback.out().length());

    loopback.pump(written);

    bool readBool = false;
    char readChar = 0;
    signed char readSChar = 0;
    unsigned char readUChar = 0;
    short readShort = 0;
    unsigned short readUShort = 0;
    int readInt = 0;
    unsigned int readUInt = 0;
    std::uint64_t readU64 = 0;

    EXPECT_EQ(1u, loopback.in().read(readBool));
    EXPECT_EQ(1u, loopback.in().read(readChar));
    EXPECT_EQ(1u, loopback.in().read(readSChar));
    EXPECT_EQ(1u, loopback.in().read(readUChar));
    EXPECT_EQ(2u, loopback.in().read(readShort));
    EXPECT_EQ(2u, loopback.in().read(readUShort));
    EXPECT_EQ(4u, loopback.in().read(readInt));
    EXPECT_EQ(4u, loopback.in().read(readUInt));
    EXPECT_EQ(8u, loopback.in().read(readU64));

    EXPECT_EQ(wroteBool, readBool);
    EXPECT_EQ(wroteChar, readChar);
    EXPECT_EQ(wroteSChar, readSChar);
    EXPECT_EQ(wroteUChar, readUChar);
    EXPECT_EQ(wroteShort, readShort);
    EXPECT_EQ(wroteUShort, readUShort);
    EXPECT_EQ(wroteInt, readInt);
    EXPECT_EQ(wroteUInt, readUInt);
    EXPECT_EQ(wroteU64, readU64);
    EXPECT_TRUE(loopback.in().isEmpty());
}

// The little-endian layout the streams now document with a static_assert
// is also what the bytes look like: a scalar goes out least-significant
// byte first, which is what the Win32 client reads back.
TEST(WireScalarRoundTrip, ScalarBytesAreLittleEndian) {
    SocketOutputStream oStream(NULL, 64);
    oStream.write((unsigned int)0xDEADBEEFu);
    oStream.write((unsigned short)0xBEEFu);

    const unsigned char* pBuffer = (const unsigned char*)oStream.getBuffer();
    ASSERT_EQ(6u, oStream.length());
    EXPECT_EQ(0xEF, pBuffer[0]);
    EXPECT_EQ(0xBE, pBuffer[1]);
    EXPECT_EQ(0xAD, pBuffer[2]);
    EXPECT_EQ(0xDE, pBuffer[3]);
    EXPECT_EQ(0xEF, pBuffer[4]);
    EXPECT_EQ(0xBE, pBuffer[5]);
}

// The span overloads and the char*/uint pair are the same code path, so
// bytes written through one must read back through the other.
TEST(WireSpanBuffers, SpanAndPointerLengthAgree) {
    SmallLoopback loopback(4096);

    const unsigned char source[] = {0x00, 0x01, 0x7F, 0x80, 0xFE, 0xFF, 0x41, 0x42};
    EXPECT_EQ(8u, loopback.out().write(std::span<const std::byte>((const std::byte*)source, sizeof(source))));
    EXPECT_EQ(8u, loopback.out().write((const char*)source, sizeof(source)));
    loopback.pump(16);

    unsigned char viaSpan[sizeof(source)] = {};
    EXPECT_EQ(8u, loopback.in().read(std::span<std::byte>((std::byte*)viaSpan, sizeof(viaSpan))));
    EXPECT_EQ(0, memcmp(source, viaSpan, sizeof(source)));

    unsigned char viaPointer[sizeof(source)] = {};
    EXPECT_EQ(8u, loopback.in().read((char*)viaPointer, sizeof(viaPointer)));
    EXPECT_EQ(0, memcmp(source, viaPointer, sizeof(source)));
    EXPECT_TRUE(loopback.in().isEmpty());
}

// peek() must not move the read cursor, through either entry point.
TEST(WireSpanBuffers, PeekLeavesTheCursorAlone) {
    SmallLoopback loopback(4096);

    const unsigned int value = 0x11223344u;
    loopback.out().write(value);
    loopback.pump(4);

    unsigned char peeked[4] = {};
    ASSERT_TRUE(loopback.in().peek(std::span<std::byte>((std::byte*)peeked, sizeof(peeked))));
    EXPECT_EQ(4u, loopback.in().length());
    unsigned char peekedAgain[4] = {};
    ASSERT_TRUE(loopback.in().peek((char*)peekedAgain, sizeof(peekedAgain)));
    EXPECT_EQ(0, memcmp(peeked, peekedAgain, sizeof(peeked)));
    EXPECT_EQ(0, memcmp(peeked, &value, sizeof(value)));

    unsigned int readBack = 0;
    loopback.in().read(readBack);
    EXPECT_EQ(value, readBack);

    // Asking for more than is buffered is "not yet", not an exception.
    unsigned char tooMuch[4] = {};
    EXPECT_FALSE(loopback.in().peek(std::span<std::byte>((std::byte*)tooMuch, sizeof(tooMuch))));
}

// The bounds behaviour the packet parsers depend on, unchanged by the
// span refactor: a short buffer is InsufficientDataException, an empty
// request is InvalidProtocolException.
TEST(WireSpanBuffers, BoundsExceptionsAreUnchanged) {
    SmallLoopback loopback(4096);

    loopback.out().write((unsigned char)0x5A);
    loopback.pump(1);

    // One byte buffered, four asked for.
    unsigned int tooWide = 0;
    EXPECT_THROW(loopback.in().read(tooWide), InsufficientDataException);
    unsigned char four[4] = {};
    EXPECT_THROW(loopback.in().read(std::span<std::byte>((std::byte*)four, sizeof(four))), InsufficientDataException);
    EXPECT_THROW(loopback.in().read((char*)four, sizeof(four)), InsufficientDataException);

    // A zero-length request is a protocol error, not a no-op.
    EXPECT_THROW(loopback.in().read(std::span<std::byte>((std::byte*)four, 0)), InvalidProtocolException);
    EXPECT_THROW(loopback.in().read((char*)four, 0), InvalidProtocolException);
    EXPECT_THROW(loopback.in().peek(std::span<std::byte>((std::byte*)four, 0)), InvalidProtocolException);

    // The failed reads consumed nothing.
    EXPECT_EQ(1u, loopback.in().length());
    unsigned char survived = 0;
    loopback.in().read(survived);
    EXPECT_EQ(0x5A, survived);
}

// The ring buffer's reversed-order branch, and the split copy inside it
// -- the branch the misaligned-access bug lived in. A 16-byte input
// buffer and 5-byte rounds put a multi-byte scalar across the seam.
TEST(WireRingBuffer, ScalarsSurviveTheWrapAround) {
    SmallLoopback loopback(16);

    for (unsigned int round = 0; round < 64; round++) {
        const unsigned int wroteUInt = 0xA1B2C300u + round;
        const unsigned char wroteUChar = (unsigned char)(round * 7u);

        loopback.out().write(wroteUInt);
        loopback.out().write(wroteUChar);
        loopback.pump(5);

        unsigned int readUInt = 0;
        unsigned char readUChar = 0;
        loopback.in().read(readUInt);
        loopback.in().read(readUChar);

        ASSERT_EQ(wroteUInt, readUInt) << "round " << round;
        ASSERT_EQ(wroteUChar, readUChar) << "round " << round;
        ASSERT_TRUE(loopback.in().isEmpty()) << "round " << round;
    }
}

// Same wrap, entered through the span overload with a buffer wider than
// any scalar, so the split copy runs with len > rightLen by more than a
// byte or two.
TEST(WireRingBuffer, SpanBuffersSurviveTheWrapAround) {
    SmallLoopback loopback(16);

    for (unsigned int round = 0; round < 64; round++) {
        unsigned char source[7];
        for (unsigned int i = 0; i < sizeof(source); i++)
            source[i] = (unsigned char)(round * 13u + i);

        loopback.out().write(std::span<const std::byte>((const std::byte*)source, sizeof(source)));
        loopback.pump(sizeof(source));

        unsigned char sink[sizeof(source)] = {};
        ASSERT_TRUE(loopback.in().peek(std::span<std::byte>((std::byte*)sink, sizeof(sink)))) << "round " << round;
        ASSERT_EQ(0, memcmp(source, sink, sizeof(source))) << "peek, round " << round;

        memset(sink, 0, sizeof(sink));
        ASSERT_EQ(sizeof(source), loopback.in().read(std::span<std::byte>((std::byte*)sink, sizeof(sink))))
            << "round " << round;
        ASSERT_EQ(0, memcmp(source, sink, sizeof(source))) << "read, round " << round;
        ASSERT_TRUE(loopback.in().isEmpty()) << "round " << round;
    }
}
