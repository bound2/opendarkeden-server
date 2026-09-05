//////////////////////////////////////////////////////////////////////////////
// Filename    : GCPartyError.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_PARTY_ERROR_H__
#define __GC_PARTY_ERROR_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// 파티 가입 관련 코드
//////////////////////////////////////////////////////////////////////////////
enum {
    // 파티에 가입시키거나 탈퇴시킬 대상이 존재하지 않는다.
    GC_PARTY_ERROR_TARGET_NOT_EXIST = 0,

    // 파티에 가입시키거나 탈퇴시킬 대상이 다른 종족이다.
    GC_PARTY_ERROR_RACE_DIFFER,

    // 안전지대가 아니다.
    GC_PARTY_ERROR_NOT_SAFE,

    // 늑대나 박쥐 상태에서는 할 수 없다.
    GC_PARTY_ERROR_NOT_NORMAL_FORM,

    // 초대 중이면서 또 초대를 하려고 한다.
    GC_TRADE_ERROR_ALREADY_INVITING,

    // 초대 중이 아니면서 초대에 대한 응답을 보내왔다.
    GC_PARTY_ERROR_NOT_INVITING,

    // 파티원을 추방할 수 있는 권한이 없다.
    GC_PARTY_ERROR_NO_AUTHORITY,

    // 알 수 없는 에러다
    GC_TRADE_ERROR_UNKNOWN,

    GC_PARTY_ERROR_MAX
};

//////////////////////////////////////////////////////////////////////////////
// class GCPartyError;
//////////////////////////////////////////////////////////////////////////////

class GCPartyError : public Packet {
public:
    GCPartyError(){};
    ~GCPartyError(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_PARTY_ERROR;
    }
    PacketSize_t getPacketSize() const {
        return szBYTE + szObjectID;
    }
    string getPacketName() const {
        return "GCPartyError";
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
// class GCPartyErrorFactory;
//////////////////////////////////////////////////////////////////////////////

class GCPartyErrorFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_PARTY_ERROR;
    static constexpr std::string_view kName = "GCPartyError";
    static constexpr PacketSize_t kMaxSize{szBYTE + szObjectID};

    Packet* createPacket() override {
        return new GCPartyError();
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
