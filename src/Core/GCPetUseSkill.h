//////////////////////////////////////////////////////////////////////////////
// Filename    : GCPetUseSkill.h
// Written By  : elca@ewestsoft.com
// Description :
// Class definition for the packet sent when a skill succeeds
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
// Class the game server uses to tell the client that its own skill succeeded
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
    ObjectID_t m_Attacker = 0, m_Target = 0;
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
