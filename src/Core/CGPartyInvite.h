//////////////////////////////////////////////////////////////////////////////
// Filename    : CGPartyInvite.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_PARTY_INVITE_H__
#define __CG_PARTY_INVITE_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// 파티 가입 관련 코드
//////////////////////////////////////////////////////////////////////////////
enum {
    CG_PARTY_INVITE_REQUEST = 0,
    CG_PARTY_INVITE_CANCEL,
    CG_PARTY_INVITE_ACCEPT,
    CG_PARTY_INVITE_REJECT,
    CG_PARTY_INVITE_BUSY,

    CG_PARTY_INVITE_MAX
};

//////////////////////////////////////////////////////////////////////////////
// class CGPartyInvite
//////////////////////////////////////////////////////////////////////////////

class CGPartyInvite : public Packet {
public:
    CGPartyInvite(){};
    ~CGPartyInvite(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_PARTY_INVITE;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szBYTE;
    }
    string getPacketName() const {
        return "CGPartyInvite";
    }
    string toString() const;

public:
    ObjectID_t getTargetObjectID() const {
        return m_TargetObjectID;
    }
    void setTargetObjectID(ObjectID_t id) {
        m_TargetObjectID = id;
    }

    BYTE getCode(void) const {
        return m_Code;
    }
    void setCode(BYTE code) {
        m_Code = code;
    }

private:
    ObjectID_t m_TargetObjectID; // 상대방의 OID
    BYTE m_Code;                 // 코드
};


//////////////////////////////////////////////////////////////////////////////
// class CGPartyInviteFactory;
//////////////////////////////////////////////////////////////////////////////

class CGPartyInviteFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_PARTY_INVITE;
    static constexpr std::string_view kName = "CGPartyInvite";
    static constexpr PacketSize_t kMaxSize{szObjectID + szBYTE};

    Packet* createPacket() override {
        return new CGPartyInvite();
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
// class CGPartyInviteHandler
//////////////////////////////////////////////////////////////////////////////

class CGPartyInviteHandler {
public:
    static void execute(CGPartyInvite* pPacket, Player* player);
    static void executeError(CGPartyInvite* pPacket, Player* player, BYTE ErrorCode);
};

#endif
