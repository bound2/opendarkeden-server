//////////////////////////////////////////////////////////////////////////////
// Filename    : CGRequestIP.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_REQUEST_IP_H__
#define __CG_REQUEST_IP_H__

#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////////////
// class CGRequestIP;
// The client asks the server for someone's IP:
// when that someone is nearby it asks by objectID,
// otherwise.. it asks by character name.
//////////////////////////////////////////////////////////////////////////////

class CGRequestIP : public Packet {
public:
    CGRequestIP();
    ~CGRequestIP();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_REQUEST_IP;
    }
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_Name);
    }
    string getPacketName() const {
        return "CGRequestIP";
    }
    string toString() const;

public:
    // The name is held to what the factory max budgets.
    static constexpr uint kMaxNameLength = 10;

    string getName() const {
        return m_Name;
    }
    void setName(const char* pName) {
        const string name(pName);
        m_Name = (name.size() > kMaxNameLength) ? name.substr(0, kMaxNameLength) : name;
    }

protected:
    string m_Name;
};

//////////////////////////////////////////////////////////////////////////////
// class CGRequestIPFactory;
//////////////////////////////////////////////////////////////////////////////

class CGRequestIPFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_REQUEST_IP;
    static constexpr std::string_view kName = "CGRequestIP";
    static constexpr PacketSize_t kMaxSize{szBYTE + CGRequestIP::kMaxNameLength};

    Packet* createPacket() override {
        return new CGRequestIP();
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
// class CGRequestIPHandler;
//////////////////////////////////////////////////////////////////////////////

class CGRequestIPHandler {
public:
    static void execute(CGRequestIP* pCGRequestIP, Player* pPlayer);
};


#endif
