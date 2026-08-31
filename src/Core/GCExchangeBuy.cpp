//////////////////////////////////////////////////////////////////////////////
// Filename    : GCExchangeBuy.cpp
// Written By  : Exchange System
// Description : Server responds to buy request
//////////////////////////////////////////////////////////////////////////////

#include "GCExchangeBuy.h"

#include <stdio.h>

// Out-of-line definition so the constant may also be odr-used (bound to a
// reference, e.g. std::min) by handlers and tests, not only read as a
// compile-time constant.
const PacketSize_t GCExchangeBuy::kMaxMessage;

// Stub execute() for server side (GC packets don't execute on server)
void GCExchangeBuy::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    uint8_t success;
    iStream.read(success);
    m_Success = (success != 0);

    // Read message with a BYTE length prefix.
    // NOTE: never call iStream.read(m_Message) — the raw template overload
    // would overwrite the std::string object itself with wire bytes.
    // The else-branch is required so read() fully overwrites the packet's
    // state, i.e. is a true mirror of write().
    uint8_t len;
    iStream.read(len);
    if (len > 0) {
        // char buf[256] is safe for any uint8_t len: at len == 255 the
        // terminator lands on buf[255], the last element.
        char buf[256];
        iStream.read(buf, len);
        buf[len] = '\0';
        m_Message = buf;
    } else {
        m_Message.clear();
    }

    uint64_t orderID;
    iStream.read(orderID);
    m_OrderID = (int64_t)orderID;

    __END_CATCH
}

void GCExchangeBuy::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    oStream.write((uint8_t)m_Success);

    // Write message with a BYTE length prefix (the length byte is always
    // written, even when the message is empty).
    // NOTE: oStream.write(m_Message) would emit raw bytes with no length
    // prefix — a receiver could not frame it.
    //
    // The clamp to kMaxMessage must be identical here and in getPacketSize(),
    // because writePacket() puts getPacketSize() on the wire before calling
    // write(); a mismatch desynchronises the stream.
    const uint8_t len = (uint8_t)(m_Message.length() > kMaxMessage ? kMaxMessage : m_Message.length());
    oStream.write(len);
    if (len > 0)
        oStream.write(m_Message.c_str(), len);

    oStream.write((uint64_t)m_OrderID);

    __END_CATCH
}

PacketSize_t GCExchangeBuy::getPacketSize() const {
    PacketSize_t size = 0;
    size += szBYTE; // success
    // Same clamp as write(), so getPacketSize() == bytes written for ANY value.
    size += szBYTE + (m_Message.length() > kMaxMessage ? kMaxMessage : m_Message.length());
    size += sizeof(uint64_t); // orderID
    return size;
}

string GCExchangeBuy::toString() const {
    // StringStream has no explicitly 64-bit overload; the widest is long, which
    // only happens to be 64-bit on the LP64 build target. Format the id locally
    // rather than truncating it through (int) or depending on that coincidence.
    char orderIDBuf[24];
    snprintf(orderIDBuf, sizeof(orderIDBuf), "%lld", (long long)m_OrderID);

    StringStream msg;
    msg << "GCExchangeBuy(" << "Success:" << (m_Success ? "true" : "false") << ",Message:" << m_Message
        << ",OrderID:" << orderIDBuf << ")";
    return msg.toString();
}
