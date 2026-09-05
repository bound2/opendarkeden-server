//////////////////////////////////////////////////////////////////////////////
// Filename	: GCSMSAddressList.h
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_SMS_ADDRESS_LIST_H__
#define __GC_SMS_ADDRESS_LIST_H__

#include <string>
#include <vector>

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GCSMSAddressList;
//////////////////////////////////////////////////////////////////////////////

#define MAX_ADDRESS_NUM 30

struct AddressUnit {
    DWORD ElementID;
    string CharacterName;
    string CustomName;
    string Number;

    PacketSize_t getPacketSize() const {
        return szDWORD + szBYTE + CharacterName.size() + szBYTE + CustomName.size() + szBYTE + Number.size();
    }
    static constexpr PacketSize_t getMaxPacketSize() {
        return szDWORD + szBYTE + 20 + szBYTE + 40 + szBYTE + 11;
    }

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
};

class GCSMSAddressList : public Packet {
public:
    GCSMSAddressList();
    ~GCSMSAddressList();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_SMS_ADDRESS_LIST;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCSMSAddressList";
    }
    string toString() const;

public:
    vector<AddressUnit*>& getAddresses() {
        return m_Addresses;
    }

private:
    vector<AddressUnit*> m_Addresses;
};

//////////////////////////////////////////////////////////////////////////////
// class GCSMSAddressListFactory;
//////////////////////////////////////////////////////////////////////////////

class GCSMSAddressListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SMS_ADDRESS_LIST;
    static constexpr std::string_view kName = "GCSMSAddressList";
    static constexpr PacketSize_t kMaxSize{szBYTE + AddressUnit::getMaxPacketSize() * MAX_ADDRESS_NUM};

    GCSMSAddressListFactory() {}
    virtual ~GCSMSAddressListFactory() {}

public:
    Packet* createPacket() override {
        return new GCSMSAddressList();
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

#endif
