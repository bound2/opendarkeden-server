//////////////////////////////////////////////////////////////////////////////
// Filename    : CGExchangeBuy.cpp
// Written By  : Exchange System
// Description : Client requests to buy an item
//////////////////////////////////////////////////////////////////////////////

#include "CGExchangeBuy.h"

#include <stdio.h>


// Out-of-line definition so the constant may also be odr-used (bound to a
// reference, e.g. std::min) by handlers and tests, not only read as a
// compile-time constant.
const PacketSize_t CGExchangeBuy::kMaxIdempotencyKey;

void CGExchangeBuy::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    uint64_t listingID;
    iStream.read(listingID);
    m_ListingID = (int64_t)listingID;

    // Read idempotency key with a BYTE length prefix.
    // NOTE: never call iStream.read(m_IdempotencyKey) — the raw template
    // overload would overwrite the std::string object itself with wire bytes.
    // The else-branch is required so read() fully overwrites the packet's
    // state, i.e. is a true mirror of write().
    uint8_t len;
    iStream.read(len);

    // This cap is below the 255 a BYTE length prefix can express, so unlike
    // the other Exchange strings the declared length has to be checked: a
    // malformed peer could declare more than write() can ever emit and make
    // read() consume past this packet into the ones queued behind it on the
    // same connection. The packet-size precheck in GamePlayer bounds the
    // declared packet size, not this byte.
    if (len > kMaxIdempotencyKey)
        throw InvalidProtocolException("CGExchangeBuy: idempotency key longer than the cap");

    if (len > 0) {
        // char buf[256] is safe for any uint8_t len: at len == 255 the
        // terminator lands on buf[255], the last element.
        char buf[256];
        iStream.read(buf, len);
        buf[len] = '\0';
        m_IdempotencyKey = buf;
    } else {
        m_IdempotencyKey.clear();
    }

    __END_CATCH
}

void CGExchangeBuy::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    oStream.write((uint64_t)m_ListingID);

    // Write idempotency key with a BYTE length prefix (the length byte is
    // always written, even when the key is empty).
    // NOTE: oStream.write(m_IdempotencyKey) would emit raw bytes with no
    // length prefix — a receiver could not frame it.
    //
    // The clamp to kMaxIdempotencyKey must be identical here and in
    // getPacketSize(), because writePacket() puts getPacketSize() on the wire
    // before calling write(); a mismatch desynchronises the stream.
    const uint8_t len =
        (uint8_t)(m_IdempotencyKey.length() > kMaxIdempotencyKey ? kMaxIdempotencyKey : m_IdempotencyKey.length());
    oStream.write(len);
    if (len > 0)
        oStream.write(m_IdempotencyKey.c_str(), len);

    __END_CATCH
}

PacketSize_t CGExchangeBuy::getPacketSize() const {
    PacketSize_t size = 0;
    size += sizeof(uint64_t); // listingID
    // Same clamp as write(), so getPacketSize() == bytes written for ANY value.
    size += szBYTE + (m_IdempotencyKey.length() > kMaxIdempotencyKey ? kMaxIdempotencyKey : m_IdempotencyKey.length());
    return size;
}

string CGExchangeBuy::toString() const {
    // StringStream has no explicitly 64-bit overload; the widest is long, which
    // only happens to be 64-bit on the LP64 build target. Format the id locally
    // rather than truncating it through (int) or depending on that coincidence.
    char listingIDBuf[24];
    snprintf(listingIDBuf, sizeof(listingIDBuf), "%lld", (long long)m_ListingID);

    StringStream msg;
    msg << "CGExchangeBuy(" << "ListingID:" << listingIDBuf << ",IdempotencyKey:" << m_IdempotencyKey << ")";
    return msg.toString();
}
