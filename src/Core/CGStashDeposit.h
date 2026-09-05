////////////////////////////////////////////////////////////////////////////////
// Filename    : CGStashDeposit.h
// Written By  : 김성민
// Description :
////////////////////////////////////////////////////////////////////////////////

#ifndef __CG_STASH_DEPOSIT_H__
#define __CG_STASH_DEPOSIT_H__

#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
//
// class CGStashDeposit;
//
////////////////////////////////////////////////////////////////////////////////

class CGStashDeposit : public Packet {
public:
    CGStashDeposit(){};
    virtual ~CGStashDeposit(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_STASH_DEPOSIT;
    }
    PacketSize_t getPacketSize() const {
        return szGold;
    }
    string getPacketName() const {
        return "CGStashDeposit";
    }
    string toString() const;

public:
    Gold_t getAmount(void) const {
        return m_Amount;
    }
    void setAmount(Gold_t amount) {
        m_Amount = amount;
    }

private:
    Gold_t m_Amount;
};


////////////////////////////////////////////////////////////////////////////////
//
// class CGStashDepositFactory;
//
////////////////////////////////////////////////////////////////////////////////

class CGStashDepositFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_STASH_DEPOSIT;
    static constexpr std::string_view kName = "CGStashDeposit";
    static constexpr PacketSize_t kMaxSize{szGold};

    Packet* createPacket() override {
        return new CGStashDeposit();
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
// class CGStashDepositHandler;
//
////////////////////////////////////////////////////////////////////////////////

class CGStashDepositHandler {
public:
    // execute packet's handler
    static void execute(CGStashDeposit* pPacket, Player* player);
};

#endif
