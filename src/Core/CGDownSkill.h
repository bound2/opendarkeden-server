//////////////////////////////////////////////////////////////////////////////
// Filename    : CGDownSkill.h
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_DOWN_SKILL_H__
#define __CG_DOWN_SKILL_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class CGDownSkill;
//////////////////////////////////////////////////////////////////////////////

class CGDownSkill : public Packet {
public:
    CGDownSkill(){};
    ~CGDownSkill(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_DOWN_SKILL;
    }
    PacketSize_t getPacketSize() const {
        return szSkillType;
    }
    string getPacketName() const {
        return "CGDownSkill";
    }
    string toString() const;

public:
    SkillType_t getSkillType() const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t SkillType) {
        m_SkillType = SkillType;
    }

private:
    SkillType_t m_SkillType; // 기술의 종류
};

//////////////////////////////////////////////////////////////////////
// class CGDownSkillFactory;
//////////////////////////////////////////////////////////////////////

class CGDownSkillFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_DOWN_SKILL;
    static constexpr std::string_view kName = "CGDownSkill";
    static constexpr PacketSize_t kMaxSize{szSkillType};

    Packet* createPacket() override {
        return new CGDownSkill();
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


//////////////////////////////////////////////////////////////////////
// class CGDownSkillHandler;
//////////////////////////////////////////////////////////////////////

class CGDownSkillHandler {
public:
    static void execute(CGDownSkill* pCGDownSkill, Player* pPlayer);
};

#endif
