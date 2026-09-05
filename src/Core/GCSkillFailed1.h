//////////////////////////////////////////////////////////////////////////////
// Filename    :  GCSkillFailed1.h
// Written By  :  elca@ewestsoft.com
// Description :  Å
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_SKILL_FAILED_1_H__
#define __GC_SKILL_FAILED_1_H__

#include "Exception.h"
#include "ModifyInfo.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class  GCSkillFailed1;
//////////////////////////////////////////////////////////////////////////////

class GCSkillFailed1 : public ModifyInfo {
public:
    GCSkillFailed1();
    ~GCSkillFailed1();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_SKILL_FAILED_1;
    }
    PacketSize_t getPacketSize() const {
        return szSkillType + szBYTE + ModifyInfo::getPacketSize();
    }
    string getPacketName() const {
        return "GCSkillFailed1";
    }
    string toString() const;

public:
    SkillType_t getSkillType(void) const {
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
    SkillType_t m_SkillType;
    BYTE m_Grade;
};


//////////////////////////////////////////////////////////////////////
//
// class  GCSkillFailed1Factory;
//
// Factory for  GCSkillFailed1
//
//////////////////////////////////////////////////////////////////////

class GCSkillFailed1Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SKILL_FAILED_1;
    static constexpr std::string_view kName = "GCSkillFailed1";
    static constexpr PacketSize_t kMaxSize{szSkillType + szBYTE + ModifyInfo::getPacketMaxSize()};

    GCSkillFailed1Factory() {}
    virtual ~GCSkillFailed1Factory() {}

public:
    Packet* createPacket() override {
        return new GCSkillFailed1();
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


#endif // __GC_SKILL_FAILED_1_H__
