//////////////////////////////////////////////////////////////////////
//
// Filename    : GCModifyMoney.h
// Written By  :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_MODIFY_MONEY_H__
#define __GC_MODIFY_MONEY_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCModifyMoney;
//
// Make the client open the guild registration window.
//
//////////////////////////////////////////////////////////////////////

class GCModifyMoney : public Packet {
public:
    GCModifyMoney(){};
    ~GCModifyMoney(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_MODIFY_MONEY;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGold;
    }

    // get packet name
    string getPacketName() const {
        return "GCModifyMoney";
    }

    // get packet's debug string
    string toString() const;

    // get/set amount
    Gold_t getAmount() const {
        return m_Amount;
    }
    void setAmount(Gold_t amount) {
        m_Amount = amount;
    }


private:
    // Amount
    Gold_t m_Amount;
};


//////////////////////////////////////////////////////////////////////
//
// class GCModifyMoneyFactory;
//
// Factory for GCModifyMoney
//
//////////////////////////////////////////////////////////////////////

class GCModifyMoneyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MODIFY_MONEY;
    static constexpr std::string_view kName = "GCModifyMoney";
    static constexpr PacketSize_t kMaxSize{szGold};

    // create packet
    Packet* createPacket() override {
        return new GCModifyMoney();
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
    // Define and return const static GCSystemMessagePacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class GCModifyMoney;
//
//////////////////////////////////////////////////////////////////////

#endif
