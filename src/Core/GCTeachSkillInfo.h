//////////////////////////////////////////////////////////////////////////////
// Filename    : GCTeachSkillInfo.h
// Description :
// The first packet sent to the player when an NPC is about to teach a skill.
// It describes the range of skills the NPC can teach and is the
// packet used for that.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_TEACH_SKILL_INFO_H__
#define __GC_TEACH_SKILL_INFO_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class GCTeachSkillInfo;
//////////////////////////////////////////////////////////////////////////////

class GCTeachSkillInfo : public Packet {
public:
    GCTeachSkillInfo(){};
    ~GCTeachSkillInfo(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_TEACH_SKILL_INFO;
    }
    PacketSize_t getPacketSize() const {
        return szSkillDomainType + szSkillLevel;
    }
    string getPacketName() const {
        return "GCTeachSkillInfo";
    }
    string toString() const;

public:
    SkillDomainType_t getDomainType(void) const {
        return m_DomainType;
    }
    void setDomainType(SkillDomainType_t type) {
        m_DomainType = type;
    }

    SkillLevel_t getTargetLevel(void) const {
        return m_TargetLevel;
    }
    void setTargetLevel(SkillLevel_t level) {
        m_TargetLevel = level;
    }

private:
    SkillDomainType_t m_DomainType = 0; // Domain type of the skill the NPC teaches
    SkillLevel_t m_TargetLevel = 0;     // The skill level the player is about to learn
};

//////////////////////////////////////////////////////////////////////////////
// class GCTeachSkillInfoFactory;
//////////////////////////////////////////////////////////////////////////////

class GCTeachSkillInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_TEACH_SKILL_INFO;
    static constexpr std::string_view kName = "GCTeachSkillInfo";
    static constexpr PacketSize_t kMaxSize{szSkillDomainType + szSkillLevel};

    Packet* createPacket() override {
        return new GCTeachSkillInfo();
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
