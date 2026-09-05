//////////////////////////////////////////////////////////////////////
//
// Filename    : CGStoreSign.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_STORE_SIGN_H__
#define __CG_STORE_SIGN_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class CGStoreSign;
//
//////////////////////////////////////////////////////////////////////

class CGStoreSign : public Packet {
public:
    CGStoreSign(){};
    virtual ~CGStoreSign(){};
    // Initialize packet by reading data from the incoming stream.
    void read(SocketInputStream& iStream);

    // Serialize packet data to the outgoing stream.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_STORE_SIGN;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE + m_Sign.size();
    }

    // get packet name
    string getPacketName() const {
        return "CGStoreSign";
    }

    // get packet's debug string
    string toString() const;

    string getSign() const {
        return m_Sign;
    }
    void setSign(const string& sign) {
        m_Sign = sign;
    }

private:
    string m_Sign;
};


//////////////////////////////////////////////////////////////////////
//
// class CGStoreSignFactory;
//
// Factory for CGStoreSign
//
//////////////////////////////////////////////////////////////////////

class CGStoreSignFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_STORE_SIGN;
    static constexpr std::string_view kName = "CGStoreSign";
    static constexpr PacketSize_t kMaxSize{szBYTE + 80};

    // constructor
    CGStoreSignFactory() {}

    // destructor
    virtual ~CGStoreSignFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGStoreSign();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get Packet Max Size
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGStoreSignHandler;
//
//////////////////////////////////////////////////////////////////////

class CGStoreSignHandler {
public:
    // execute packet's handler
    static void execute(CGStoreSign* pCGStoreSign, Player* pPlayer);
};

#endif
