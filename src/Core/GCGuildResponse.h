//////////////////////////////////////////////////////////////////////////////
// Filename    : GCGuildResponse.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_GUILD_RESPONSE_H__
#define __GC_GUILD_RESPONSE_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class GCGuildResponse
//////////////////////////////////////////////////////////////////////////////

class GCGuildResponse : public Packet {
public:
    GCGuildResponse() {
        m_Code = 0;
        m_Parameter = 0;
    }
    virtual ~GCGuildResponse() {}

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;

    PacketID_t getPacketID() const {
        return PACKET_GC_GUILD_RESPONSE;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCGuildResponse";
    }
    string toString() const;

public:
    BYTE getCode(void) const {
        return m_Code;
    }
    void setCode(WORD code) {
        m_Code = code;
    }

    uint getParameter(void) const {
        return m_Parameter;
    }
    void setParameter(uint parameter) {
        m_Parameter = parameter;
    }

private:
    WORD m_Code;
    uint m_Parameter;
};


//////////////////////////////////////////////////////////////////////////////
// class GCGuildResponseFactory;
//////////////////////////////////////////////////////////////////////////////

class GCGuildResponseFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_GUILD_RESPONSE;
    static constexpr std::string_view kName = "GCGuildResponse";
    static constexpr PacketSize_t kMaxSize{szWORD + szuint};

    Packet* createPacket() override {
        return new GCGuildResponse();
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
