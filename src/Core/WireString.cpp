//////////////////////////////////////////////////////////////////////
//
// Filename    : WireString.cpp
// Description : Length-prefixed string fields.
//
//////////////////////////////////////////////////////////////////////

#include "WireString.h"

#include "Assert.h"
#include "Exception.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

namespace de {

namespace wire {

namespace {

std::string outOfRange(std::string_view what, size_t length, unsigned min, unsigned max) {
    return std::string(what) + " length " + std::to_string(length) + " is outside [" + std::to_string(min) + ", " +
           std::to_string(max) + "]";
}

} // namespace

void readString(SocketInputStream& iStream, std::string& out, StringBounds bounds, std::string_view what) {
    Assert(bounds.min <= bounds.max && bounds.max <= kMaxByteStringLength);

    BYTE length = 0;
    iStream.read(length);

    if (length < bounds.min || length > bounds.max)
        throw InvalidProtocolException(outOfRange(what, length, bounds.min, bounds.max));

    // read(string&, uint) refuses a zero length, so an admitted empty
    // field is an assignment rather than a read of no bytes.
    if (length == 0)
        out.clear();
    else
        iStream.read(out, length);
}

void writeString(SocketOutputStream& oStream, std::string_view value, StringBounds bounds, std::string_view what) {
    Assert(bounds.min <= bounds.max && bounds.max <= kMaxByteStringLength);

    const size_t length = value.size();

    if (length < bounds.min || length > bounds.max)
        throw InvalidProtocolException(outOfRange(what, length, bounds.min, bounds.max));

    oStream.write((BYTE)length);

    if (length != 0)
        oStream.write(value.data(), (uint)length);
}

void readString16(SocketInputStream& iStream, std::string& out, StringBounds16 bounds, std::string_view what) {
    Assert(bounds.min <= bounds.max && bounds.max <= kMaxWordStringLength);

    WORD length = 0;
    iStream.read(length);

    if (length < bounds.min || length > bounds.max)
        throw InvalidProtocolException(outOfRange(what, length, bounds.min, bounds.max));

    if (length == 0)
        out.clear();
    else
        iStream.read(out, length);
}

void writeString16(SocketOutputStream& oStream, std::string_view value, StringBounds16 bounds, std::string_view what) {
    Assert(bounds.min <= bounds.max && bounds.max <= kMaxWordStringLength);

    const size_t length = value.size();

    if (length < bounds.min || length > bounds.max)
        throw InvalidProtocolException(outOfRange(what, length, bounds.min, bounds.max));

    oStream.write((WORD)length);

    if (length != 0)
        oStream.write(value.data(), (uint)length);
}

} // namespace wire

} // namespace de
