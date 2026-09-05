//////////////////////////////////////////////////////////////////////////////
// Filename    : CGSMSAddressList.h
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_SMS_ADDRESS_LIST_H__
#define __CG_SMS_ADDRESS_LIST_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class CGSMSAddressList;
//////////////////////////////////////////////////////////////////////////////

class CGSMSAddressList : public Packet {
public:
    CGSMSAddressList(){};
    virtual ~CGSMSAddressList(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_SMS_ADDRESS_LIST;
    }
    PacketSize_t getPacketSize() const {
        return 0;
    }
    string getPacketName() const {
        return "CGSMSAddressList";
    }
    string toString() const;
};

//////////////////////////////////////////////////////////////////////
// class CGSMSAddressListFactory;
//////////////////////////////////////////////////////////////////////

class CGSMSAddressListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_SMS_ADDRESS_LIST;
    static constexpr std::string_view kName = "CGSMSAddressList";
    static constexpr PacketSize_t kMaxSize{0};

    Packet* createPacket() override {
        return new CGSMSAddressList();
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


//////////////////////////////////////////////////////////////////////
// class CGSMSAddressListHandler;
//////////////////////////////////////////////////////////////////////

class CGSMSAddressListHandler {
public:
    static void execute(CGSMSAddressList* pCGSMSAddressList, Player* pPlayer);
};

#endif
