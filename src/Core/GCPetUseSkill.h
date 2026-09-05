//////////////////////////////////////////////////////////////////////////////
// Filename    : GCPetUseSkill.h
// Written By  : elca@ewestsoft.com
// Description :
// 기술이 성공했을때 보내는 패킷을 위한 클래스 정의
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_PET_USE_SKILL_H__
#define __GC_PET_USE_SKILL_H__

#include "Assert1.h"
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GCPetUseSkill;
// 게임서버에서 클라이언트로 자신의 기술이 성공을 알려주기 위한 클래스
//////////////////////////////////////////////////////////////////////////////

class GCPetUseSkill : public Packet {
public:
    GCPetUseSkill();
    ~GCPetUseSkill();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_PET_USE_SKILL;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szObjectID;
    }
    string getPacketName() const {
        return "GCPetUseSkill";
    }
    string toString() const;

public:
    ObjectID_t getAttacker() const {
        return m_Attacker;
    }
    ObjectID_t getTarget() const {
        return m_Target;
    }

    void setAttacker(ObjectID_t Attacker) {
        m_Attacker = Attacker;
    }
    void setTarget(ObjectID_t Target) {
        m_Target = Target;
    }

private:
    ObjectID_t m_Attacker, m_Target;
};


//////////////////////////////////////////////////////////////////////////////
// class GCPetUseSkillFactory;
//////////////////////////////////////////////////////////////////////////////

class GCPetUseSkillFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_PET_USE_SKILL;
    static constexpr std::string_view kName = "GCPetUseSkill";
    static constexpr PacketSize_t kMaxSize{szObjectID + szObjectID};

    GCPetUseSkillFactory() {}
    virtual ~GCPetUseSkillFactory() {}

public:
    Packet* createPacket() override {
        return new GCPetUseSkill();
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
