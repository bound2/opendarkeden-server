//////////////////////////////////////////////////////////////////////////////
// Filename    : GCExchangeBuy.cpp
// Written By  : Exchange System
// Description : Server responds to buy request
//////////////////////////////////////////////////////////////////////////////

#include "GCExchangeBuy.h"

// Stub execute() for server side (GC packets don't execute on server)
void GCExchangeBuy::execute(Player* pPlayer) {
    __BEGIN_TRY
    // Server doesn't execute GC packets
    __END_CATCH
}

void GCExchangeBuy::read(SocketInputStream& iStream) {
    __BEGIN_TRY

    uint8_t success;
    iStream.read(success);
    m_Success = (success != 0);

    // Read message with a BYTE length prefix.
    // NOTE: never call iStream.read(m_Message) — the raw template overload
    // would overwrite the std::string object itself with wire bytes.
    uint8_t len;
    iStream.read(len);
    if (len > 0) {
        char buf[256];
        iStream.read(buf, len);
        buf[len] = '\0';
        m_Message = string(buf);
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
    uint8_t len = (uint8_t)m_Message.length();
    oStream.write(len);
    if (len > 0)
        oStream.write(m_Message.c_str(), len);

    oStream.write((uint64_t)m_OrderID);

    __END_CATCH
}

PacketSize_t GCExchangeBuy::getPacketSize() const {
    PacketSize_t size = 0;
    size += sizeof(uint8_t);  // success
    size += sizeof(uint8_t);  // message length byte
    size += m_Message.size(); // message bytes
    size += sizeof(m_OrderID);
    return size;
}

string GCExchangeBuy::toString() const {
    StringStream msg;
    msg << "GCExchangeBuy(" << "Success:" << (m_Success ? "true" : "false") << ",Message:" << m_Message
        << ",OrderID:" << (int)m_OrderID << ")";
    return msg.toString();
}
