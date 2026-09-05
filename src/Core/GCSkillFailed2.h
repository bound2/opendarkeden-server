//////////////////////////////////////////////////////////////////////
//
// Filename    :  GCSkillFailed2.h
// Written By  :  elca@ewestsoft.com
// Description :  Å
//
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_SKILL_FAILED_2_H__
#define __GC_SKILL_FAILED_2_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class  GCSkillFailed2;
//
//////////////////////////////////////////////////////////////////////

class GCSkillFailed2 : public Packet {
public:
    // constructor
    GCSkillFailed2();

    // destructor
    ~GCSkillFailed2();


public:
    // ÀÔ·Â½ºÆ®¸²(¹öÆÛ)À¸·ÎºÎÅÍ µ¥ÀÌÅ¸¸¦ ÀÐ¾î¼­ ÆÐÅ¶À» ÃÊ±âÈ­ÇÑ´Ù.
    void read(SocketInputStream& iStream);

    // Ãâ·Â½ºÆ®¸²(¹öÆÛ)À¸·Î ÆÐÅ¶ÀÇ ¹ÙÀÌ³Ê¸® ÀÌ¹ÌÁö¸¦ º¸³½´Ù.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SKILL_FAILED_2;
    }

    // get packet size
    PacketSize_t getPacketSize() const {
        return szObjectID + szObjectID + szSkillType + szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "GCSkillFailed2";
    }

    // get packet's debug string
    string toString() const;

    // get/set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    // get/set TargetObjectID
    ObjectID_t getTargetObjectID() const {
        return m_TargetObjectID;
    }
    void setTargetObjectID(ObjectID_t TargetObjectID) {
        m_TargetObjectID = TargetObjectID;
    }

    // get/set SkillType
    SkillType_t getSkillType() const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t SkillType) {
        m_SkillType = SkillType;
    }

    BYTE getGrade() const {
        return m_Grade;
    }
    void setGrade(BYTE grade) {
        m_Grade = grade;
    }

private:
    // ObjectID
    ObjectID_t m_ObjectID;

    // TaragetObjectID
    ObjectID_t m_TargetObjectID;

    // SkillType
    SkillType_t m_SkillType;

    // Grade
    BYTE m_Grade;
};


//////////////////////////////////////////////////////////////////////
//
// class  GCSkillFailed2Factory;
//
// Factory for  GCSkillFailed2
//
//////////////////////////////////////////////////////////////////////

class GCSkillFailed2Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SKILL_FAILED_2;
    static constexpr std::string_view kName = "GCSkillFailed2";
    static constexpr PacketSize_t kMaxSize{szObjectID + szObjectID + szSkillType + szBYTE};

    // constructor
    GCSkillFailed2Factory() {}

    // destructor
    virtual ~GCSkillFailed2Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCSkillFailed2();
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


#endif // __GC_SKILL_FAILED_2_H__
