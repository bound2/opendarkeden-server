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
        return szBYTE + szuint + m_Name.size() + 4;
    }
    string toString() const;

public:
    string getName() const {
        return m_Name;
    }
    void setName(const char* pName) {
        m_Name = pName;
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
    IP_t m_IP;
    uint m_Port;
};

//////////////////////////////////////////////////////////////////////////////
// class GCRequestedIPFactory;
//////////////////////////////////////////////////////////////////////////////

class GCRequestedIPFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_REQUESTED_IP;
    static constexpr std::string_view kName = "GCRequestedIP";
    static constexpr PacketSize_t kMaxSize{szBYTE + szuint + 10 + 4};

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
