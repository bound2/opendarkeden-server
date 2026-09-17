//////////////////////////////////////////////////////////////////////////////
// Filename    : GCAuthKey.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_AUTH_KEY_H__
#define __GC_AUTH_KEY_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class GCAuthKey;
// Sends an NPC's line to the PCs nearby.
//////////////////////////////////////////////////////////////////////////////

class GCAuthKey : public Packet {
public:
    GCAuthKey(){};
    ~GCAuthKey(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_AUTH_KEY;
    }
    PacketSize_t getPacketSize() const {
        return szDWORD;
    }
    string getPacketName() const {
        return "GCAuthKey";
    }
    string toString() const;

    DWORD getKey() const {
        return m_Key;
    }
    void setKey(DWORD key) {
        m_Key = key;
    }

private:
    DWORD m_Key = 0;
};


//////////////////////////////////////////////////////////////////////////////
// class GCAuthKeyFactory;
//////////////////////////////////////////////////////////////////////////////


class GCAuthKeyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_AUTH_KEY;
    static constexpr std::string_view kName = "GCAuthKey";
    static constexpr PacketSize_t kMaxSize{szDWORD};

    Packet* createPacket() override {
        return new GCAuthKey();
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
