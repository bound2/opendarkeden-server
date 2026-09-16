//////////////////////////////////////////////////////////////////////////////
// Filename    : GCRequestedIP.h
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_REQUESTED_IP_H__
#define __GC_REQUESTED_IP_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////////////
// class GCRequestedIP;
//////////////////////////////////////////////////////////////////////////////

class GCRequestedIP : public Packet {
public:
    GCRequestedIP();
    ~GCRequestedIP();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_REQUESTED_IP;
    }
    string getPacketName() const {
        return "GCRequestedIP";
    }
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_Name) + szuint + 4;
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

    void setIP(IP_t ip) {
        m_IP = ip;
    }
    IP_t getIP() const {
        return m_IP;
    }

    void setPort(uint port) {
        m_Port = port;
    }
    uint getPort() const {
        return m_Port;
    }

protected:
    string m_Name;
    IP_t m_IP = 0;
    uint m_Port = 0;
};

//////////////////////////////////////////////////////////////////////////////
// class GCRequestedIPFactory;
//////////////////////////////////////////////////////////////////////////////

class GCRequestedIPFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_REQUESTED_IP;
    static constexpr std::string_view kName = "GCRequestedIP";
    static constexpr PacketSize_t kMaxSize{szBYTE + GCRequestedIP::kMaxNameLength + szuint + 4};

    Packet* createPacket() override {
        return new GCRequestedIP();
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
