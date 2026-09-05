//////////////////////////////////////////////////////////////////////
//
// Filename    : WireTypes.h
// Description : Representation rules for the packet wire format.
//
// SocketInputStream::read<T> and SocketOutputStream::write<T> copy
// sizeof(T) raw bytes out of / into the socket ring buffer. Whatever
// compiles therefore *defines* the protocol: before this header, a
// pointer, a padded class, a std::string or a platform-sized integer
// all compiled silently and put their in-memory image on the wire.
//
// WireScalar states, once, which types are allowed to do that. It is
// an explicit accept list rather than a property test (trivially
// copyable, is_arithmetic, ...) because the question is not "can this
// be memcpy'd" but "does the deployed client agree on its width and
// layout". tests/wire_types_test.cpp pins the list.
//
//////////////////////////////////////////////////////////////////////

#ifndef __WIRE_TYPES_H__
#define __WIRE_TYPES_H__

#include <bit>
#include <cstdint>

#include <type_traits>

//////////////////////////////////////////////////////////////////////
//
// Byte order
//
// No packet swaps bytes and none ever has: the wire format IS the
// in-memory object representation of the fields below. Both deployed
// sides are little-endian x86 (the Linux server, the Win32 client), so
// state that here once instead of leaving it implicit in ~460 packet
// codecs. If the server is ever built for a big-endian target this
// assertion fires at compile time, which is the cheap failure; the
// expensive one is a live session silently reading every multi-byte
// field backwards. Adding byte swapping is a protocol change and is
// deliberately NOT done here.
//
//////////////////////////////////////////////////////////////////////
static_assert(std::endian::native == std::endian::little,
              "DarkEden's wire format is the little-endian in-memory representation of its packet "
              "fields; a big-endian build would need byte swapping in the codecs, which is a "
              "protocol change the client must ship too.");

namespace de {

namespace wire {

//////////////////////////////////////////////////////////////////////
//
// The exact set of types the protocol puts on the wire.
//
// Widths are the ones both sides agree on: the server is LP64 Linux,
// the client is ILP32 Win32. bool/char/short/int and their unsigned
// forms are 1/1/2/4 bytes on both, so the Types.h aliases built on
// them (BYTE = unsigned char, WORD = unsigned short, DWORD = unsigned
// int, and every *_t typedef of those) are admitted through them.
//
// Deliberately NOT admitted:
//   * pointers, arrays, class types (std::string included) -- these
//     would put an address or an object layout on the wire;
//   * float/double -- no packet writes one, and their representation
//     is not part of this protocol;
//   * signed 64-bit in any spelling (`long` on this server, `long long`,
//     std::int64_t) -- 8 bytes here, 4 on the client, and no packet has
//     a signed 64-bit field;
//   * `unsigned long` / `size_t` *as a platform-sized type* -- but see
//     the uint64_t entry below: on this LP64 server they ARE that
//     fixed-width type, so the concept cannot separate them from it;
//   * char8_t/char16_t/char32_t, wchar_t -- unused; spell a 64-bit
//     field std::uint64_t.
//
//////////////////////////////////////////////////////////////////////
template <typename T> inline constexpr bool isWireScalar = false;

template <> inline constexpr bool isWireScalar<bool> = true;
template <> inline constexpr bool isWireScalar<char> = true;
// signed char is admitted for completeness as std::int8_t, so the
// fixed-width spellings form a complete pair set; no packet writes one
// today. Every other entry below is a type some packet actually uses.
template <> inline constexpr bool isWireScalar<signed char> = true;    // std::int8_t
template <> inline constexpr bool isWireScalar<unsigned char> = true;  // BYTE, std::uint8_t
template <> inline constexpr bool isWireScalar<short> = true;          // std::int16_t
template <> inline constexpr bool isWireScalar<unsigned short> = true; // WORD, std::uint16_t
template <> inline constexpr bool isWireScalar<int> = true;            // std::int32_t
template <> inline constexpr bool isWireScalar<unsigned int> = true;   // DWORD, std::uint32_t

// The one 64-bit field on the wire: the Exchange listing id, written as
// `oStream.write((uint64_t)m_ListingID)` by CGExchangeBuy / GCExchangeBuy /
// GCExchangeList and read back into a uint64_t. It has to stay admitted or
// those three packets change width.
//
// CAVEAT, and the reason this is a separate entry rather than "64-bit
// integers are fine": on the LP64 server std::uint64_t IS `unsigned long`,
// so admitting it admits that spelling too, and a stray write(size_t)
// compiles here while it would be 4 bytes on the ILP32 client. Signed
// 64-bit is NOT admitted, which is what keeps plain `long` out; there is no
// signed 64-bit field on the wire. On a platform where uint64_t is
// `unsigned long long` (the Win32 client) `unsigned long` stays rejected,
// as it should be.
template <> inline constexpr bool isWireScalar<std::uint64_t> = true;

// The widths the accept list above assumes. A toolchain that disagrees
// would silently resize packet fields.
static_assert(sizeof(bool) == 1 && sizeof(char) == 1 && sizeof(short) == 2 && sizeof(int) == 4 &&
                  sizeof(std::uint64_t) == 8,
              "a wire scalar changed width: every packet using it changed size");

} // namespace wire

// Types SocketInputStream::read<T> / SocketOutputStream::write<T> may
// copy raw between an object and the wire.
template <typename T>
concept WireScalar = wire::isWireScalar<std::remove_cv_t<T>>;

} // namespace de

#endif
