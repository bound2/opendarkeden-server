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
#include <vector>

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

// The stream types are template parameters so that a test can put its
// own subclasses on the same pair of sockets -- the encrypting pair
// below is what needs it.
//
// `inBufferSize` of zero means "the same as the writer's".
// `pSendThrough`, when given, is the socket the output stream writes to
// instead of the connected one: a decorator standing in front of it,
// which the caller points at sender() once the fixture exists.
template <class OutStream, class InStream> class LoopbackOn {
public:
    explicit LoopbackOn(uint bufferSize, uint inBufferSize = 0, Socket* pSendThrough = NULL)
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
            throw std::runtime_error("LoopbackOn: could not bind any test port");

        m_pClientSocket = new Socket("127.0.0.1", port);
        m_pClientSocket->connect();
        m_pAcceptedSocket = m_pServerSocket->accept();
        if (m_pAcceptedSocket == NULL)
            throw std::runtime_error("LoopbackOn: accept() returned NULL");

        // Non-blocking: fill() must be able to report "nothing more yet"
        // instead of parking in recv().
        m_pAcceptedSocket->setNonBlocking(true);

        m_pOut = new OutStream((pSendThrough != NULL) ? pSendThrough : m_pClientSocket, bufferSize);
        m_pIn = new InStream(m_pAcceptedSocket, (inBufferSize != 0) ? inBufferSize : bufferSize);
    }

    ~LoopbackOn() {
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

    OutStream& out() {
        return *m_pOut;
    }
    InStream& in() {
        return *m_pIn;
    }

    // The two ends themselves, for tests that need to control the
    // blocking mode or the socket buffer sizes rather than only move
    // bytes across.
    Socket& sender() {
        return *m_pClientSocket;
    }
    Socket& receiver() {
        return *m_pAcceptedSocket;
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
    OutStream* m_pOut;
    InStream* m_pIn;
};

using SmallLoopback = LoopbackOn<SocketOutputStream, SocketInputStream>;

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

//////////////////////////////////////////////////////////////////////
//
// The packet size on the wire
//
// writePacket() reserves the header's size field, writes the body, then
// fills the field in with the number of bytes the body produced. These
// pin that the field is measured rather than declared -- a packet whose
// getPacketSize() disagrees with its write() must still be framed
// correctly -- and that an honest packet's bytes did not move.
//
//////////////////////////////////////////////////////////////////////
namespace {

// A packet whose declared size can be made to disagree with what write()
// emits, in either direction, by any amount.
class DriftingPacket : public Packet {
public:
    DriftingPacket(uint bodySize, int declaredDrift) : m_BodySize(bodySize), m_DeclaredDrift(declaredDrift) {}

    void read(SocketInputStream& iStream) {
        throw UnsupportedError();
    }

    void write(SocketOutputStream& oStream) const {
        for (uint i = 0; i < m_BodySize; i++)
            oStream.write((BYTE)(0xA0 + i));
    }

    PacketID_t getPacketID() const {
        return 0x4321;
    }
    PacketSize_t getPacketSize() const {
        return (PacketSize_t)((int)m_BodySize + m_DeclaredDrift);
    }
    string getPacketName() const {
        return "DriftingPacket";
    }
    string toString() const {
        return "DriftingPacket";
    }

private:
    uint m_BodySize;
    int m_DeclaredDrift;
};

// The size field as the receiver reads it: little-endian, at the four
// ring-buffer positions starting `at`, wrap included.
unsigned int sizeFieldAt(const SocketOutputStream& oStream, uint at) {
    const unsigned char* pBuffer = (const unsigned char*)oStream.getBuffer();
    const uint capacity = (uint)oStream.capacity();
    unsigned int size = 0;
    for (uint i = 0; i < szPacketSize; i++)
        size |= (unsigned int)pBuffer[(at + i) % capacity] << (8 * i);
    return size;
}

} // namespace

// A lie in either direction, on a buffer the frame fits in contiguously.
TEST(PacketFrameSize, TheHeaderCarriesTheMeasuredBodyLength) {
    const uint kBodySize = 9;

    for (int drift : {-4, 0, 7}) {
        DriftingPacket packet(kBodySize, drift);
        SocketOutputStream oStream(NULL, 128);
        oStream.writePacket(&packet);

        ASSERT_EQ(szPacketHeader + kBodySize, oStream.length()) << "drift " << drift;
        EXPECT_EQ(kBodySize, sizeFieldAt(oStream, szPacketID)) << "drift " << drift;

        const unsigned char* pBuffer = (const unsigned char*)oStream.getBuffer();
        for (uint i = 0; i < kBodySize; i++)
            EXPECT_EQ(0xA0 + i, pBuffer[szPacketHeader + i]) << "drift " << drift << ", body byte " << i;
    }
}

// An honest packet's frame is byte for byte what it always was: id,
// size, sequence, body.
TEST(PacketFrameSize, AnHonestPacketsFrameIsUnchanged) {
    DriftingPacket packet(4, 0);
    SocketOutputStream oStream(NULL, 128);
    oStream.writePacket(&packet);

    const unsigned char expected[] = {0x21, 0x43, 0x04, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xA1, 0xA2, 0xA3};
    ASSERT_EQ(sizeof(expected), oStream.length());
    EXPECT_EQ(0, memcmp(expected, oStream.getBuffer(), sizeof(expected)));
}

// The size field split across the ring buffer's wrap point.
//
// Only a flush that could not send everything leaves the head in the
// middle of the buffer -- a flush that completes resets head and tail to
// zero, and resize() moves the buffered bytes to the front -- so the
// fixture blocks the connection first: the sender is non-blocking, both
// socket buffers are pinned small, the receiver never reads, and the
// flush is given more than the link can swallow. Everything written after
// that wraps, and the tail is then walked to the exact offset where only
// the first of the size field's four bytes fits before the end.
namespace {

void expectStraddledSizeFieldCarriesBodyLength(int drift) {
    const uint kCapacity = 1u << 21;
    const uint kBlockingWrite = 1u << 20;
    const uint kBodySize = 6;

    SmallLoopback loopback(kCapacity);
    loopback.sender().setNonBlocking(true);
    loopback.sender().setSendBufferSize(4096);
    loopback.receiver().setReceiveBufferSize(4096);

    std::vector<char> filler(kBlockingWrite, 0x5A);
    ASSERT_EQ(kBlockingWrite, loopback.out().write(filler.data(), (uint)filler.size()));
    loopback.out().flush();

    ASSERT_GT(loopback.out().length(), 0u) << "the whole write left the buffer; the ring cannot wrap";
    const uint head = kBlockingWrite - loopback.out().length();
    ASSERT_GT(head, 1024u) << "the flush sent nothing; the ring cannot wrap";

    // Put the tail where writePacket's two id bytes end one byte short of
    // the buffer's end, so the size field starts at the last byte.
    const uint frameStart = kCapacity - 1 - szPacketID;
    std::vector<char> spacer(frameStart - kBlockingWrite, 0x33);
    ASSERT_EQ(spacer.size(), loopback.out().write(spacer.data(), (uint)spacer.size()));
    ASSERT_EQ(kCapacity, (uint)loopback.out().capacity()) << "the buffer grew; the tail is no longer where it was put";

    DriftingPacket packet(kBodySize, drift);
    loopback.out().writePacket(&packet);

    // One byte of the size field before the wrap, three after it.
    const uint sizeFieldStart = kCapacity - 1;
    EXPECT_EQ(kBodySize, sizeFieldAt(loopback.out(), sizeFieldStart)) << "drift " << drift;

    const unsigned char* pBuffer = (const unsigned char*)loopback.out().getBuffer();
    EXPECT_EQ(0x21, pBuffer[frameStart]);
    EXPECT_EQ(0x43, pBuffer[frameStart + 1]);
    // sequence byte, then the body, all past the wrap
    EXPECT_EQ(0x00, pBuffer[(sizeFieldStart + szPacketSize) % kCapacity]);
    for (uint i = 0; i < kBodySize; i++)
        EXPECT_EQ(0xA0 + i, pBuffer[(sizeFieldStart + szPacketSize + 1 + i) % kCapacity])
            << "drift " << drift << ", body byte " << i;

    EXPECT_EQ(kCapacity, (uint)loopback.out().capacity()) << "the buffer grew while the frame was being written";
}

} // namespace

TEST(PacketFrameSize, AStraddledSizeFieldCarriesTheMeasuredBodyLength) {
    for (int drift : {-4, 0, 7})
        expectStraddledSizeFieldCarriesBodyLength(drift);
}

//////////////////////////////////////////////////////////////////////
//
// A flush the socket cut short
//
// flush() encrypts the buffered bytes before it sends them, and a
// non-blocking socket whose peer has stopped reading takes only part of
// what it is offered. The bytes left behind are already encrypted and
// must go out as they stand: the key advances one step per byte, so a
// second pass over them would produce bytes the receiver's single pass
// cannot undo -- and, because the sender's key would then be ahead of
// the receiver's, would take everything sent afterwards with them.
//
// The streams carry the transform but production keeps it switched off,
// so the property is not visible on the wire there. The subclasses
// below run it for real, on both ends, which is what makes a second
// pass observable at all.
//
//////////////////////////////////////////////////////////////////////
namespace {

BYTE* testHashTable() {
    static BYTE hashTable[512];
    static const bool built = []() {
        for (uint i = 0; i < 512; i++)
            hashTable[i] = (BYTE)(i * 37u + 11u);
        return true;
    }();
    (void)built;
    return hashTable;
}

// The body the streams' own EncryptData carries: a per-byte XOR with
// 0xCC and with a key stream that advances one step per byte. XOR is
// its own inverse, so the receiver undoes it by running it again from
// the same key -- and two passes on the sending side do not cancel,
// because the second runs from a key that has moved on.
WORD runTestTransform(WORD key, BYTE* pHashTable, char* buf, int len) {
    for (int i = 0; i < len; i++)
        buf[i] ^= (char)0xCC;

    for (int i = 0; i < len; i++) {
        buf[i] ^= (char)pHashTable[key];
        if (++key == 512)
            key = 0;
    }

    return key;
}

class EncryptingOutputStream : public SocketOutputStream {
public:
    EncryptingOutputStream(Socket* pSocket, uint bufferSize) : SocketOutputStream(pSocket, bufferSize) {
        setKey(0, testHashTable());
    }

    WORD EncryptData(WORD key, char* buf, int len) override {
        return runTestTransform(key, m_HashTable, buf, len);
    }
};

class DecryptingInputStream : public SocketInputStream {
public:
    DecryptingInputStream(Socket* pSocket, uint bufferSize) : SocketInputStream(pSocket, bufferSize) {
        setKey(0, testHashTable());
    }

    WORD EncryptData(WORD key, char* buf, int len) override {
        return runTestTransform(key, m_HashTable, buf, len);
    }
};

// A socket that passes on an exact, scripted number of bytes before it
// starts reporting "cannot take any more" -- what a non-blocking socket
// with a full send buffer reports. It stands in front of the connected
// socket so the cut lands at a chosen position instead of wherever a
// kernel buffer happened to fill up.
class ScriptedSocket : public Socket {
public:
    ScriptedSocket() : Socket(new SocketImpl()), m_pReal(NULL), m_Allowance(0) {}

    void attach(Socket* pReal) {
        m_pReal = pReal;
    }

    // Bytes the sends may pass on from here, in total, before they
    // report zero.
    void allow(uint nBytes) {
        m_Allowance = nBytes;
    }

    uint send(const void* buf, uint len, uint flags = 0) override {
        if (m_pReal == NULL || m_Allowance == 0)
            return 0;

        const uint nOffered = (len < m_Allowance) ? len : m_Allowance;
        const uint nSent = m_pReal->send(buf, nOffered, flags);
        m_Allowance -= nSent;
        return nSent;
    }

private:
    Socket* m_pReal;
    uint m_Allowance;
};

using EncryptedLoopback = LoopbackOn<EncryptingOutputStream, DecryptingInputStream>;

// More than any of these tests write, so the socket takes everything.
const uint kSendAll = 0xFFFFFFFFu;

std::vector<char> pattern(uint length, uint seed) {
    std::vector<char> data(length);
    for (uint i = 0; i < length; i++)
        data[i] = (char)(seed * 31u + i * 7u + (i >> 3));
    return data;
}

// Read exactly nBytes back out of the receiving stream. Loopback
// delivery is not instant, so fill() is polled until it stops making
// progress, and a stall is a failure rather than a wait.
std::vector<char> receiveExactly(DecryptingInputStream& iStream, uint nBytes) {
    const int kMaxIdlePolls = 200; // ~1s
    int idlePolls = 0;

    while (iStream.length() < nBytes) {
        const uint before = iStream.length();
        iStream.fill();
        if (iStream.length() == before) {
            if (++idlePolls > kMaxIdlePolls) {
                ADD_FAILURE() << "receiveExactly: stalled at " << iStream.length() << " of " << nBytes << " bytes";
                return std::vector<char>();
            }
            usleep(5000);
        } else {
            idlePolls = 0;
        }
    }

    std::vector<char> received(nBytes);
    iStream.read(received.data(), nBytes);
    return received;
}

// Leave the writer with its contents wrapped around the end of the ring
// buffer and part of them already encrypted: most of a first write goes
// out, which puts the head at 36 of the 64 bytes, and a second write
// then runs past the end and puts the tail at 6. Returns everything
// written.
std::vector<char> writeUntilWrapped(EncryptedLoopback& loopback, ScriptedSocket& scripted) {
    const std::vector<char> first = pattern(40, 2);
    EXPECT_EQ(40u, loopback.out().write(first.data(), (uint)first.size()));

    scripted.allow(36);
    EXPECT_EQ(36u, loopback.out().flush());
    EXPECT_EQ(4u, loopback.out().length());

    const std::vector<char> second = pattern(30, 3);
    EXPECT_EQ(30u, loopback.out().write(second.data(), (uint)second.size()));
    EXPECT_EQ(64, loopback.out().capacity()) << "the buffer grew; the contents no longer wrap";
    EXPECT_EQ(34u, loopback.out().length());

    std::vector<char> all(first);
    all.insert(all.end(), second.begin(), second.end());
    return all;
}

} // namespace

// The cut in a contiguous region.
TEST(FlushShortSend, ACutInAContiguousRegionKeepsThePlaintext) {
    ScriptedSocket scripted;
    EncryptedLoopback loopback(64, 256, &scripted);
    scripted.attach(&loopback.sender());

    const std::vector<char> data = pattern(20, 1);
    ASSERT_EQ(20u, loopback.out().write(data.data(), (uint)data.size()));

    scripted.allow(8);
    EXPECT_EQ(8u, loopback.out().flush());
    ASSERT_EQ(12u, loopback.out().length()) << "the socket took everything; nothing was cut short";
    EXPECT_EQ(12u, loopback.out().encryptedLength());

    scripted.allow(kSendAll);
    EXPECT_EQ(12u, loopback.out().flush());
    EXPECT_TRUE(loopback.out().isEmpty());
    EXPECT_EQ(0u, loopback.out().encryptedLength());

    EXPECT_EQ(data, receiveExactly(loopback.in(), (uint)data.size()));
}

// The cut in the first half of a wrapped region: the head is at 36, so
// the 28 bytes up to the end of the buffer go first, and the send stops
// 10 bytes into them.
TEST(FlushShortSend, ACutInTheFirstHalfOfAWrappedRegionKeepsThePlaintext) {
    ScriptedSocket scripted;
    EncryptedLoopback loopback(64, 256, &scripted);
    scripted.attach(&loopback.sender());

    const std::vector<char> data = writeUntilWrapped(loopback, scripted);

    scripted.allow(10);
    EXPECT_EQ(10u, loopback.out().flush());
    ASSERT_EQ(24u, loopback.out().length());
    EXPECT_EQ(24u, loopback.out().encryptedLength());

    scripted.allow(kSendAll);
    EXPECT_EQ(24u, loopback.out().flush());
    EXPECT_TRUE(loopback.out().isEmpty());
    EXPECT_EQ(0u, loopback.out().encryptedLength());

    EXPECT_EQ(data, receiveExactly(loopback.in(), (uint)data.size()));
}

// The cut in the second half: 28 bytes reach the end of the buffer, the
// send carries on from the front and stops 3 bytes into the remaining 6.
TEST(FlushShortSend, ACutInTheSecondHalfOfAWrappedRegionKeepsThePlaintext) {
    ScriptedSocket scripted;
    EncryptedLoopback loopback(64, 256, &scripted);
    scripted.attach(&loopback.sender());

    const std::vector<char> data = writeUntilWrapped(loopback, scripted);

    scripted.allow(31);
    EXPECT_EQ(31u, loopback.out().flush());
    ASSERT_EQ(3u, loopback.out().length());
    EXPECT_EQ(3u, loopback.out().encryptedLength());

    scripted.allow(kSendAll);
    EXPECT_EQ(3u, loopback.out().flush());
    EXPECT_TRUE(loopback.out().isEmpty());
    EXPECT_EQ(0u, loopback.out().encryptedLength());

    EXPECT_EQ(data, receiveExactly(loopback.in(), (uint)data.size()));
}

// A buffer growth between the cut and the flush that finishes the job.
// resize() moves the buffered bytes to the front of a larger allocation
// and keeps their order, so the count of encrypted bytes -- a distance
// from the head -- still names the same bytes afterwards.
TEST(FlushShortSend, ABufferGrowthAfterACutKeepsTheEncryptedBytesEncrypted) {
    ScriptedSocket scripted;
    EncryptedLoopback loopback(64, 512, &scripted);
    scripted.attach(&loopback.sender());

    std::vector<char> data = pattern(40, 4);
    ASSERT_EQ(40u, loopback.out().write(data.data(), (uint)data.size()));

    scripted.allow(36);
    EXPECT_EQ(36u, loopback.out().flush());
    ASSERT_EQ(4u, loopback.out().length());
    ASSERT_EQ(4u, loopback.out().encryptedLength());

    // One byte more than the free space, so the write grows the buffer.
    const std::vector<char> more = pattern(59, 5);
    ASSERT_EQ(59u, loopback.out().write(more.data(), (uint)more.size()));
    data.insert(data.end(), more.begin(), more.end());

    EXPECT_GT(loopback.out().capacity(), 64) << "the write did not grow the buffer";
    EXPECT_EQ(63u, loopback.out().length());
    EXPECT_EQ(4u, loopback.out().encryptedLength()) << "the growth lost track of what was already encrypted";

    scripted.allow(kSendAll);
    EXPECT_EQ(63u, loopback.out().flush());
    EXPECT_TRUE(loopback.out().isEmpty());

    EXPECT_EQ(data, receiveExactly(loopback.in(), (uint)data.size()));
}

// Nothing to send is not a short send: an empty buffer flushes to zero
// bytes and leaves nothing encrypted behind it.
TEST(FlushShortSend, AnEmptyBufferFlushesToNothing) {
    ScriptedSocket scripted;
    EncryptedLoopback loopback(64, 256, &scripted);
    scripted.attach(&loopback.sender());

    scripted.allow(kSendAll);
    EXPECT_EQ(0u, loopback.out().flush());
    EXPECT_TRUE(loopback.out().isEmpty());
    EXPECT_EQ(0u, loopback.out().encryptedLength());
}
