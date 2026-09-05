//////////////////////////////////////////////////////////////////////
//
// Filename    :  GCCastingSkill.h
// Written By  :  elca@ewestsoft.com
// Description :  Å
//
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_CASTING_SKILL_H__
#define __GC_CASTING_SKILL_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class  GCCastingSkill;
//
//////////////////////////////////////////////////////////////////////

class GCCastingSkill : public Packet {
public:
    // constructor
    GCCastingSkill();

    // destructor
    ~GCCastingSkill();


public:
    // ÀÔ·Â½ºÆ®¸²(¹öÆÛ)À¸·ÎºÎÅÍ µ¥ÀÌÅ¸¸¦ ÀÐ¾î¼­ ÆÐÅ¶À» ÃÊ±âÈ­ÇÑ´Ù.
    void read(SocketInputStream& iStream);

    // Ãâ·Â½ºÆ®¸²(¹öÆÛ)À¸·Î ÆÐÅ¶ÀÇ ¹ÙÀÌ³Ê¸® ÀÌ¹ÌÁö¸¦ º¸³½´Ù.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_CASTING_SKILL;
    }

    // get packet size
    PacketSize_t getPacketSize() const {
        return szSkillType;
    }

    // get packet's name
    string getPacketName() const {
        return "GCCastingSkill";
    }

    // get packet's debug string
    string toString() const;

    // get/set SkillType
    SkillType_t getSkillType() const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t SkillType) {
        m_SkillType = SkillType;
    }

private:
    // SkillType
    SkillType_t m_SkillType;
};


//////////////////////////////////////////////////////////////////////
//
// class  GCCastingSkillFactory;
//
// Factory for  GCCastingSkill
//
//////////////////////////////////////////////////////////////////////

class GCCastingSkillFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_CASTING_SKILL;
    static constexpr std::string_view kName = "GCCastingSkill";
    static constexpr PacketSize_t kMaxSize{szSkillType};

    // constructor
    GCCastingSkillFactory() {}

    // destructor
    virtual ~GCCastingSkillFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCCastingSkill();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get Packet Max Size
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


#endif // __GC_CASTING_SKILL_H__
