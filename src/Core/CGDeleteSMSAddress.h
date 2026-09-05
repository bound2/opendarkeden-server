//////////////////////////////////////////////////////////////////////////////
// Filename    : CGDeleteSMSAddress.h
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_DELETE_SMS_ADDRESS_H__
#define __CG_DELETE_SMS_ADDRESS_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGDeleteSMSAddress;
//////////////////////////////////////////////////////////////////////////////

class CGDeleteSMSAddress : public Packet {
public:
    CGDeleteSMSAddress();
    ~CGDeleteSMSAddress();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_DELETE_SMS_ADDRESS;
    }
    PacketSize_t getPacketSize() const {
        return szDWORD;
    }
    string getPacketName() const {
        return "CGDeleteSMSAddress";
    }
    string toString() const;

public:
    DWORD getElementID() const {
        return m_ElementID;
    }
    void setElementID(DWORD eID) {
        m_ElementID = eID;
    }

private:
    DWORD m_ElementID;
};

//////////////////////////////////////////////////////////////////////////////
// class CGDeleteSMSAddressFactory;
//////////////////////////////////////////////////////////////////////////////

class CGDeleteSMSAddressFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_DELETE_SMS_ADDRESS;
    static constexpr std::string_view kName = "CGDeleteSMSAddress";
    static constexpr PacketSize_t kMaxSize{szDWORD};

    Packet* createPacket() override {
        return new CGDeleteSMSAddress();
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

//////////////////////////////////////////////////////////////////////////////
// class CGDeleteSMSAddressHandler;
//////////////////////////////////////////////////////////////////////////////

class CGDeleteSMSAddressHandler {
public:
    static void execute(CGDeleteSMSAddress* pPacket, Player* player);
};

#endif
