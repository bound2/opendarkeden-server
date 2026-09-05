//////////////////////////////////////////////////////////////////////////////
// Filename    : GCPartyInvite.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_PARTY_INVITE_H__
#define __GC_PARTY_INVITE_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// 파티 가입 관련 코드
//////////////////////////////////////////////////////////////////////////////
enum {
    GC_PARTY_INVITE_REQUEST = 0,
    GC_PARTY_INVITE_CANCEL,
    GC_PARTY_INVITE_ACCEPT,
    GC_PARTY_INVITE_REJECT,
    GC_PARTY_INVITE_BUSY,
    GC_PARTY_INVITE_ANOTHER_PARTY,
    GC_PARTY_INVITE_MEMBER_FULL,

    GC_PARTY_INVITE_MAX
};

//////////////////////////////////////////////////////////////////////////////
// class GCPartyInvite;
//////////////////////////////////////////////////////////////////////////////

class GCPartyInvite : public Packet {
public:
    GCPartyInvite(){};
    ~GCPartyInvite(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_PARTY_INVITE;
    }
    PacketSize_t getPacketSize() const {
        return szBYTE + szObjectID;
    }
    string getPacketName() const {
        return "GCPartyInvite";
    }
    string toString() const;

public:
    BYTE getCode() const {
        return m_Code;
    }
    void setCode(BYTE code) {
        m_Code = code;
    }

    ObjectID_t getTargetObjectID(void) const {
        return m_TargetObjectID;
    }
    void setTargetObjectID(ObjectID_t id) {
        m_TargetObjectID = id;
    }

private:
    ObjectID_t m_TargetObjectID;
    BYTE m_Code; // 코드
};


//////////////////////////////////////////////////////////////////////////////
// class GCPartyInviteFactory;
//////////////////////////////////////////////////////////////////////////////

class GCPartyInviteFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_PARTY_INVITE;
    static constexpr std::string_view kName = "GCPartyInvite";
    static constexpr PacketSize_t kMaxSize{szBYTE + szObjectID};

    Packet* createPacket() override {
        return new GCPartyInvite();
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
