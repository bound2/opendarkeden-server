//////////////////////////////////////////////////////////////////////////////
// Filename    : GCExchangeList.h
// Written By  : Exchange System
// Description : Server sends listing list to client
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_EXCHANGE_LIST_H__
#define __GC_EXCHANGE_LIST_H__

#include <string>
#include <vector>

#include "Packet.h"
#include "PacketFactory.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// ExchangeListing structure (partial, for packet transfer)
//////////////////////////////////////////////////////////////////////////////

struct ExchangeListing {
    int64_t listingID;
    int16_t serverID;
    string sellerAccount;
    string sellerPlayer;
    uint8_t sellerRace;
    uint8_t itemClass;
    uint16_t itemType;
    int64_t itemID;
    int objectID;
    int pricePoint;
    uint8_t currency;
    uint8_t status;
    string buyerAccount;
    string buyerPlayer;
    uint8_t taxRate;
    int taxAmount;
    string createdAt;
    string expireAt;
    string soldAt;
    string cancelledAt;
    string updatedAt;
    int version;

    // Snapshot fields for UI display
    string itemName;
    uint8_t enchantLevel;
    uint16_t grade;
    int durability;
    uint16_t silver;
    uint8_t optionType1;
    uint8_t optionType2;
    uint8_t optionType3;
    uint16_t optionValue1;
    uint16_t optionValue2;
    uint16_t optionValue3;
    int stackCount;
};

//////////////////////////////////////////////////////////////////////////////
// class GCExchangeList
//////////////////////////////////////////////////////////////////////////////

class GCExchangeList : public Packet {
public:
    GCExchangeList() : m_Page(1), m_PageSize(20), m_Total(0){};
    virtual ~GCExchangeList(){};

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    void execute(Player* pPlayer); // Stub for server side

    PacketSize_t getPacketSize() const;
    PacketID_t getPacketID() const {
        return PACKET_GC_EXCHANGE_LIST;
    }
    string getPacketName() const {
        return "GCExchangeList";
    }
    string toString() const;

    // Setters
    void setListings(const vector<ExchangeListing>& listings);
    void setPage(int page) {
        m_Page = page;
    }
    void setPageSize(int pageSize) {
        m_PageSize = pageSize;
    }
    void setTotal(int total) {
        m_Total = total;
    }

    // Getters
    const vector<ExchangeListing>& getListings() const {
        return m_Listings;
    }
    int getPage() const {
        return m_Page;
    }
    int getPageSize() const {
        return m_PageSize;
    }
    int getTotal() const {
        return m_Total;
    }

private:
    vector<ExchangeListing> m_Listings;
    int m_Page;
    int m_PageSize;
    int m_Total;
};

//////////////////////////////////////////////////////////////////////////////
// class GCExchangeListFactory
//
// The server never receives this packet, so the factory exists for the
// wire-layout inventory (tests/wire-layout.txt) and to keep the packet
// comparable with the client's copy. The max size is the write() layout
// at the default page size with every string at its BYTE-length maximum.
//////////////////////////////////////////////////////////////////////////////

class GCExchangeListFactory : public PacketFactory {
public:
    Packet* createPacket() {
        return new GCExchangeList();
    }

    string getPacketName() const {
        return "GCExchangeList";
    }

    PacketID_t getPacketID() const {
        return Packet::PACKET_GC_EXCHANGE_LIST;
    }

    PacketSize_t getPacketMaxSize() const {
        const PacketSize_t kMaxString = szBYTE + 255;
        const PacketSize_t kDefaultPageSize = 20;
        const PacketSize_t listing = sizeof(int64_t)                          // listingID
                                     + sizeof(int16_t)                        // serverID
                                     + kMaxString * 2                         // sellerAccount, sellerPlayer
                                     + sizeof(uint8_t) * 2 + sizeof(uint16_t) // sellerRace, itemClass, itemType
                                     + sizeof(int64_t)                        // itemID
                                     + sizeof(int) * 2                        // objectID, pricePoint
                                     + sizeof(uint8_t) * 2                    // currency, status
                                     + kMaxString * 2                         // buyerAccount, buyerPlayer
                                     + sizeof(uint8_t) + sizeof(int)          // taxRate, taxAmount
                                     + kMaxString * 2                         // createdAt, expireAt
                                     + sizeof(int)                            // version
                                     + kMaxString                             // itemName
                                     + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(int) +
                                     sizeof(uint16_t)                             // enchant, grade, durability, silver
                                     + sizeof(uint8_t) * 3 + sizeof(uint16_t) * 3 // optionType1..3, optionValue1..3
                                     + sizeof(int);                               // stackCount
        return sizeof(int) * 3 + sizeof(uint16_t) + listing * kDefaultPageSize;
    }
};

//////////////////////////////////////////////////////////////////////////////
// class GCExchangeListHandler
//////////////////////////////////////////////////////////////////////////////

class GCExchangeListHandler {
public:
    static void execute(GCExchangeList* pPacket, Player* pPlayer);
};

#endif // __GC_EXCHANGE_LIST_H__
