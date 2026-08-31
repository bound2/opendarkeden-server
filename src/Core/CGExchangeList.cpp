//////////////////////////////////////////////////////////////////////////////
// Filename    : CGExchangeList.cpp
// Written By  : Exchange System
// Description : Client requests to browse exchange listings
//////////////////////////////////////////////////////////////////////////////

#include "CGExchangeList.h"


// Forward declaration of exchange service

// Out-of-line definition so the constant may also be odr-used (bound to a
// reference, e.g. std::min) by handlers and tests, not only read as a
// compile-time constant.
const PacketSize_t CGExchangeList::kMaxSellerFilter;

void CGExchangeList::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    iStream.read(m_Page);
    iStream.read(m_PageSize);
    iStream.read(m_ItemClass);
    iStream.read(m_ItemType);
    iStream.read(m_MinPrice);
    iStream.read(m_MaxPrice);

    // Read seller filter string.
    // The else-branch is not optional: read() must fully overwrite the packet's
    // state so it is a true mirror of write(). A recycled packet object whose
    // previous read left a filter behind would otherwise keep the stale value
    // when the new wire data carries a zero length.
    uint8_t len;
    iStream.read(len);
    if (len > 0) {
        // char buf[256] is safe for any uint8_t len: at len == 255 the
        // terminator lands on buf[255], the last element.
        char buf[256];
        iStream.read(buf, len);
        buf[len] = '\0';
        m_SellerFilter = buf;
    } else {
        m_SellerFilter.clear();
    }

    __END_CATCH
}

void CGExchangeList::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    oStream.write(m_Page);
    oStream.write(m_PageSize);
    oStream.write(m_ItemClass);
    oStream.write(m_ItemType);
    oStream.write(m_MinPrice);
    oStream.write(m_MaxPrice);

    // Write seller filter string with a BYTE length prefix.
    //
    // The clamp to kMaxSellerFilter is duplicated, deliberately and identically,
    // in write() and in getPacketSize(). SocketOutputStream::writePacket() emits
    // getPacketSize() into the stream header BEFORE it calls write(), so the two
    // must agree for every possible value of the field. A length byte cannot
    // carry more than 255 anyway, so an over-long string has to lose bytes; the
    // only question is whether both sides lose the same ones. Clamping in only
    // one place (or throwing from write(), with the header already on the wire)
    // desynchronises the stream for every subsequent packet on the connection.
    const uint8_t len =
        (uint8_t)(m_SellerFilter.length() > kMaxSellerFilter ? kMaxSellerFilter : m_SellerFilter.length());
    oStream.write(len);
    if (len > 0)
        oStream.write(m_SellerFilter.c_str(), len);

    __END_CATCH
}

PacketSize_t CGExchangeList::getPacketSize() const {
    PacketSize_t size = 0;
    size += sizeof(m_Page);
    size += sizeof(m_PageSize);
    size += sizeof(m_ItemClass);
    size += sizeof(m_ItemType);
    size += sizeof(m_MinPrice);
    size += sizeof(m_MaxPrice);
    // Same clamp as write(), so getPacketSize() == bytes written for ANY value.
    size += szBYTE + (m_SellerFilter.length() > kMaxSellerFilter ? kMaxSellerFilter : m_SellerFilter.length());
    return size;
}

string CGExchangeList::toString() const {
    StringStream msg;
    msg << "CGExchangeList(" << "Page:" << (int)m_Page << ",PageSize:" << (int)m_PageSize
        << ",ItemClass:" << (int)m_ItemClass << ",ItemType:" << (int)m_ItemType << ",MinPrice:" << m_MinPrice
        << ",MaxPrice:" << m_MaxPrice << ",SellerFilter:" << m_SellerFilter << ")";
    return msg.toString();
}
