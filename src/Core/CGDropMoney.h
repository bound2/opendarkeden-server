//////////////////////////////////////////////////////////////////////
//
// Filename    : CGDropMoney.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_DROP_MONEY_H__
#define __CG_DROP_MONEY_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class CGDropMoney;
//
//////////////////////////////////////////////////////////////////////

class CGDropMoney : public Packet {
public:
    // constructor
    CGDropMoney();

    // destructor
    ~CGDropMoney();

public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_DROP_MONEY;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static CGDropMoneyPacketSize.
    PacketSize_t getPacketSize() const {
        return szGold;
    }

    // get packet name
    string getPacketName() const {
        return "CGDropMoney";
    }

    // get packet's debug string
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


//////////////////////////////////////////////////////////////////////
//
// class CGDropMoneyFactory;
//
// Factory for CGDropMoney
//
//////////////////////////////////////////////////////////////////////

class CGDropMoneyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_DROP_MONEY;
    static constexpr std::string_view kName = "CGDropMoney";
    static constexpr PacketSize_t kMaxSize{szGold};

    // create packet
    Packet* createPacket() override {
        return new CGDropMoney();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's max body size
    // *OPTIMIZATION HINT*
    // Define and return const static CGDropMoneyPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGDropMoneyHandler;
//
//////////////////////////////////////////////////////////////////////

class CGDropMoneyHandler {
public:
    // execute packet's handler
    static void execute(CGDropMoney* pPacket, Player* player);
};

#endif
