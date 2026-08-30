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
    GCExchangeBuy() : m_Success(false), m_OrderID(0){};
    virtual ~GCExchangeBuy(){};

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    void execute(Player* pPlayer); // Stub for server side

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
// message bytes with NO length prefix (write(string) is raw), then the
// order id; the max assumes a 255-byte message. A receiver cannot frame
// that message — recorded in docs/RESTRUCTURING.md 1.4.
//////////////////////////////////////////////////////////////////////////////

class GCExchangeBuyFactory : public PacketFactory {
public:
    Packet* createPacket() {
        return new GCExchangeBuy();
    }

    string getPacketName() const {
        return "GCExchangeBuy";
    }

    PacketID_t getPacketID() const {
        return Packet::PACKET_GC_EXCHANGE_BUY;
    }

    PacketSize_t getPacketMaxSize() const {
        return szBYTE + 255 + sizeof(int64_t);
    }
};

//////////////////////////////////////////////////////////////////////////////
// class GCExchangeBuyHandler
//////////////////////////////////////////////////////////////////////////////

class GCExchangeBuyHandler {
public:
    static void execute(GCExchangeBuy* pPacket, Player* pPlayer);
};

#endif // __GC_EXCHANGE_BUY_H__
