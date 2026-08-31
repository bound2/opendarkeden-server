//////////////////////////////////////////////////////////////////////////////
// Filename    : CGExchangeBuy.h
// Written By  : Exchange System
// Description : Client requests to buy an item
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_EXCHANGE_BUY_H__
#define __CG_EXCHANGE_BUY_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGExchangeBuy
//////////////////////////////////////////////////////////////////////////////

class CGExchangeBuy : public Packet {
public:
    // Maximum wire length, in bytes, of m_IdempotencyKey. write(), read() and
    // getPacketSize() all clamp to this value, and the factory's max size is
    // derived from it. MUST stay equal to the client repo's constant of the
    // same name.
    //
    // Deliberately 64 and not the 255 a BYTE length prefix would allow: the
    // PointLedger.IdempotencyKey column is VARCHAR(64) UNIQUE
    // (initdb/USERINFO.sql). A longer key would be silently truncated on
    // insert, so two different keys sharing a 64-byte prefix would collide on
    // the unique index and the duplicate-purchase guard would misfire.
    // Clamping on the wire keeps what the client sends, what the server reads
    // and what the database stores identical.
    static const PacketSize_t kMaxIdempotencyKey = 64;

    CGExchangeBuy() : m_ListingID(0){};
    virtual ~CGExchangeBuy(){};

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    void execute(Player* pPlayer);

    PacketSize_t getPacketSize() const;
    PacketID_t getPacketID() const {
        return PACKET_CG_EXCHANGE_BUY;
    }
    string getPacketName() const {
        return "CGExchangeBuy";
    }
    string toString() const;

    // Getters/Setters
    int64_t getListingID() const {
        return m_ListingID;
    }
    void setListingID(int64_t id) {
        m_ListingID = id;
    }

    const string& getIdempotencyKey() const {
        return m_IdempotencyKey;
    }
    void setIdempotencyKey(const string& key) {
        m_IdempotencyKey = key;
    }

private:
    int64_t m_ListingID;
    string m_IdempotencyKey;
};

//////////////////////////////////////////////////////////////////////////////
// class CGExchangeBuyFactory
//////////////////////////////////////////////////////////////////////////////

class CGExchangeBuyFactory : public PacketFactory {
public:
    Packet* createPacket() {
        return new CGExchangeBuy();
    }
    string getPacketName() const {
        return "CGExchangeBuy";
    }
    PacketID_t getPacketID() const {
        return Packet::PACKET_CG_EXCHANGE_BUY;
    }
    // 8 + 1 + 64 = 73. The client factory must match.
    PacketSize_t getPacketMaxSize() const {
        return sizeof(uint64_t) +                 // listingID
               szBYTE +                           // idempotencyKey length byte
               CGExchangeBuy::kMaxIdempotencyKey; // idempotencyKey body (write() clamps to this)
    }
};

//////////////////////////////////////////////////////////////////////////////
// class CGExchangeBuyHandler
//////////////////////////////////////////////////////////////////////////////

class GCExchangeBuy;

class CGExchangeBuyHandler {
public:
    static void execute(CGExchangeBuy* pPacket, Player* pPlayer);
};

#endif // __CG_EXCHANGE_BUY_H__
