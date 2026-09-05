//////////////////////////////////////////////////////////////////////////////
// Filename    : CGAuthKey.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_AUTH_KEY_H__
#define __CG_AUTH_KEY_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGAuthKey;
// NPC 의 대사를 주변의 PC 들에게 전송한다.
//////////////////////////////////////////////////////////////////////////////

class CGAuthKey : public Packet {
public:
    CGAuthKey(){};
    ~CGAuthKey(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_AUTH_KEY;
    }
    PacketSize_t getPacketSize() const {
        return szDWORD;
    }
    string getPacketName() const {
        return "CGAuthKey";
    }
    string toString() const;

    DWORD getKey() const {
        return m_Key;
    }
    void setKey(DWORD key) {
        m_Key = key;
    }

private:
    DWORD m_Key;
};


//////////////////////////////////////////////////////////////////////////////
// class CGAuthKeyFactory;
//////////////////////////////////////////////////////////////////////////////


class CGAuthKeyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_AUTH_KEY;
    static constexpr std::string_view kName = "CGAuthKey";
    static constexpr PacketSize_t kMaxSize{szDWORD};

    Packet* createPacket() override {
        return new CGAuthKey();
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
// class CGAuthKeyHandler;
//////////////////////////////////////////////////////////////////////////////

class CGAuthKeyHandler {
public:
    static void execute(CGAuthKey* pPacket, Player* pPlayer);
};

#endif
