//////////////////////////////////////////////////////////////////////
//
// Filename    :  GCCastingSkill.h
// Written By  :  elca@ewestsoft.com
// Description :  Declaration of the GCCastingSkill packet class
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
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
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
    SkillType_t m_SkillType = 0;
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
