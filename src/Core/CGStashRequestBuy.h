////////////////////////////////////////////////////////////////////////////////
//
// Filename    : CGStashRequestBuy.h
// Written By  : 김성민
// Description :
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __CG_STASH_REQUEST_BUY_H__
#define __CG_STASH_REQUEST_BUY_H__

#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
//
// class CGStashRequestBuy;
//
////////////////////////////////////////////////////////////////////////////////

class CGStashRequestBuy : public Packet {
public:
    CGStashRequestBuy(){};
    virtual ~CGStashRequestBuy(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_STASH_REQUEST_BUY;
    }
    PacketSize_t getPacketSize() const {
        return 0;
    }
    string getPacketName() const {
        return "CGStashRequestBuy";
    }
    string toString() const;
};


////////////////////////////////////////////////////////////////////////////////
//
// class CGStashRequestBuyFactory;
//
// Factory for CGStashRequestBuy
//
////////////////////////////////////////////////////////////////////////////////

class CGStashRequestBuyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_STASH_REQUEST_BUY;
    static constexpr std::string_view kName = "CGStashRequestBuy";
    static constexpr PacketSize_t kMaxSize{0};

    Packet* createPacket() override {
        return new CGStashRequestBuy();
    }
    string getPacketName() const override {
        return string(kName);
    }
    PacketID_t getPacketID() const override {
        return kPacketID;
    }
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


////////////////////////////////////////////////////////////////////////////////
//
// class CGStashRequestBuyHandler;
//
////////////////////////////////////////////////////////////////////////////////

class CGStashRequestBuyHandler {
public:
    CGStashRequestBuyHandler(){};
    virtual ~CGStashRequestBuyHandler(){};
    static void execute(CGStashRequestBuy* pPacket, Player* player);
};

#endif
