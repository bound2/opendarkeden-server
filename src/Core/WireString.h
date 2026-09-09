//////////////////////////////////////////////////////////////////////
//
// Filename    : WireString.h
// Description : Length-prefixed string fields.
//
// A string on the wire is a BYTE length followed by that many bytes.
// Each packet spelled the sequence out by hand -- read the prefix,
// compare it against a literal, read the bytes -- so a field's bounds
// were repeated in read(), in write() and again in getPacketSize(),
// and drifted between them.
//
// readString() / writeString() carry that sequence, with the bounds
// stated once per field, and stringWireSize() is the same field's
// contribution to getPacketSize(). A bare
//
//     BYTE sz;
//     iStream.read(sz);
//     iStream.read(m_X, sz);
//
// sequence is the legacy shape: new and touched string fields use
// these instead.
//
//////////////////////////////////////////////////////////////////////

#ifndef __WIRE_STRING_H__
#define __WIRE_STRING_H__

#include <string>

#include <string_view>

#include "Types.h"

class SocketInputStream;
class SocketOutputStream;

namespace de {

namespace wire {

// The longest string a BYTE prefix can describe.
inline constexpr unsigned kMaxByteStringLength = 255;

// The lengths a field accepts, both ends inclusive. min 1 refuses an
// empty value, min 0 admits it; max is the field's cap and may not
// pass what the prefix can carry.
struct StringBounds {
    unsigned min = 0;
    unsigned max = kMaxByteStringLength;
};

// Read a BYTE prefix, then that many bytes into `out`. A length
// outside the bounds is an InvalidProtocolException naming `what`,
// thrown once the prefix has been consumed -- the stream is left where
// the hand-written sequences left it. A zero length clears `out`
// instead of reading.
void readString(SocketInputStream& iStream, std::string& out, StringBounds bounds, std::string_view what);

// Write the BYTE prefix and the bytes. A size outside the bounds is an
// InvalidProtocolException naming `what`, thrown before the field's
// first byte reaches the stream.
void writeString(SocketOutputStream& oStream, std::string_view value, StringBounds bounds, std::string_view what);

// What such a field occupies on the wire.
constexpr uint stringWireSize(std::string_view value) {
    return szBYTE + (uint)value.size();
}

} // namespace wire

} // namespace de

#endif
