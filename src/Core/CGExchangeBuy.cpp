//////////////////////////////////////////////////////////////////////////////
// Filename    : CGExchangeBuy.cpp
// Written By  : Exchange System
// Description : Client requests to buy an item
//////////////////////////////////////////////////////////////////////////////

#include "CGExchangeBuy.h"

#include "../server/gameserver/exchange/ExchangeService.h"
#include "GCExchangeBuy.h"
#include "PlayerCreature.h"

void CGExchangeBuy::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    uint64_t listingID;
    iStream.read(listingID);
    m_ListingID = (int64_t)listingID;

    // Read idempotency key with a BYTE length prefix.
    // NOTE: never call iStream.read(m_IdempotencyKey) — the raw template
    // overload would overwrite the std::string object itself with wire bytes.
    uint8_t len;
    iStream.read(len);
    if (len > 0) {
        char buf[256];
        iStream.read(buf, len);
        buf[len] = '\0';
        m_IdempotencyKey = string(buf);
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
    uint8_t len = (uint8_t)m_IdempotencyKey.length();
    oStream.write(len);
    if (len > 0)
        oStream.write(m_IdempotencyKey.c_str(), len);

    __END_CATCH
}

PacketSize_t CGExchangeBuy::getPacketSize() const {
    PacketSize_t size = 0;
    size += sizeof(uint64_t);        // listingID
    size += sizeof(uint8_t);         // idempotency key length byte
    size += m_IdempotencyKey.size(); // idempotency key bytes
    return size;
}

string CGExchangeBuy::toString() const {
    StringStream msg;
    msg << "CGExchangeBuy(" << "ListingID:" << (int)m_ListingID << ",IdempotencyKey:" << m_IdempotencyKey << ")";
    return msg.toString();
}

void CGExchangeBuy::execute(Player* pPlayer) {
    __BEGIN_TRY

    CGExchangeBuyHandler::execute(this, pPlayer);

    __END_CATCH
}
