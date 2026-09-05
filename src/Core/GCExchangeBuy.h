//////////////////////////////////////////////////////////////////////////////
// Filename    : GCExchangeBuy.h
// Written By  : Exchange System
// Description : Server responds to buy request
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_EXCHANGE_BUY_H__
#define __GC_EXCHANGE_BUY_H__

#include <string>

#include "Packet.h"
#include "PacketFactory.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// class GCExchangeBuy
//////////////////////////////////////////////////////////////////////////////

class GCExchangeBuy : public Packet {
public:
    // Maximum wire length, in bytes, of m_Message. write(), read() and
    // getPacketSize() all clamp to this value, and the factory's max size is
    // derived from it. MUST stay equal to the client repo's constant of the
    // same name.
    static const PacketSize_t kMaxMessage = 255;

    GCExchangeBuy() : m_Success(false), m_OrderID(0){};
    virtual ~GCExchangeBuy(){};

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;

    PacketSize_t getPacketSize() const;
    PacketID_t getPacketID() const {
        return PACKET_GC_EXCHANGE_BUY;
    }
    string getPacketName() const {
        return "GCExchangeBuy";
    }
    string toString() const;

    // Setters
    void setSuccess(bool success) {
        m_Success = success;
    }
    void setMessage(const string& message) {
        m_Message = message;
    }
    void setOrderID(int64_t orderID) {
        m_OrderID = orderID;
    }

    // Getters
    bool getSuccess() const {
        return m_Success;
    }
    const string& getMessage() const {
        return m_Message;
    }
    int64_t getOrderID() const {
        return m_OrderID;
    }

private:
    bool m_Success;
    string m_Message;
    int64_t m_OrderID;
};

//////////////////////////////////////////////////////////////////////////////
// class GCExchangeBuyFactory
//
// The server never receives this packet; the factory exists for the
// wire-layout inventory (tests/wire-layout.txt) and to keep the packet
// comparable with the client's copy. write() emits a success BYTE, the
// message as a BYTE length prefix followed by the message bytes, then
// the order id; the max assumes a message at kMaxMessage, which is the
// longest write() can emit because it clamps to that constant.
//////////////////////////////////////////////////////////////////////////////

class GCExchangeBuyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_EXCHANGE_BUY;
    static constexpr std::string_view kName = "GCExchangeBuy";
    static constexpr PacketSize_t kMaxSize{szBYTE +                     // m_Success
                                           szBYTE +                     // m_Message length byte
                                           GCExchangeBuy::kMaxMessage + // m_Message body (write() clamps to this)
                                           sizeof(int64_t)};            // m_OrderID

    Packet* createPacket() override {
        return new GCExchangeBuy();
    }

    string getPacketName() const override {
        return string(kName);
    }

    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // 1 + 1 + 255 + 8 = 265. The client factory must match.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};

#endif // __GC_EXCHANGE_BUY_H__
