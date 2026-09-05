//////////////////////////////////////////////////////////////////////
//
// Filename    :  GCLearnSkillFailed.h
// Written By  :  elca@ewestsoft.com
// Description :  Å
//
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_LEARN_SKILL_FAILED_H__
#define __GC_LEARN_SKILL_FAILED_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class  GCLearnSkillFailed;
//
//////////////////////////////////////////////////////////////////////

class GCLearnSkillFailed : public Packet {
public:
    GCLearnSkillFailed();
    virtual ~GCLearnSkillFailed();


public:
    // ÀÔ·Â½ºÆ®¸²(¹öÆÛ)À¸·ÎºÎÅÍ µ¥ÀÌÅ¸¸¦ ÀÐ¾î¼­ ÆÐÅ¶À» ÃÊ±âÈ­ÇÑ´Ù.
    void read(SocketInputStream& iStream);

    // Ãâ·Â½ºÆ®¸²(¹öÆÛ)À¸·Î ÆÐÅ¶ÀÇ ¹ÙÀÌ³Ê¸® ÀÌ¹ÌÁö¸¦ º¸³½´Ù.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_LEARN_SKILL_FAILED;
    }

    // get packet size
    PacketSize_t getPacketSize() const {
        return szSkillType + szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "GCLearnSkillFailed";
    }

    // get packet's debug string
    string toString() const;

    // get/set skill type
    SkillType_t getSkillType(void) const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t SkillType) {
        m_SkillType = SkillType;
    }

    // get/set description
    BYTE getDesc(void) const {
        return m_Desc;
    }
    void setDesc(BYTE desc) {
        m_Desc = desc;
    }

private:
    SkillType_t m_SkillType;
    BYTE m_Desc; // ±â¼úÀ» ¹è¿ì´Â µ¥ ½ÇÆÐÇÑ ÀÌÀ¯ÀÌ´Ù.
                 // ÀÚ¼¼ÇÑ ³»¿ëÀº CGLearnSkillHandler¸¦ Âü°íÇÏµµ·Ï.
};


//////////////////////////////////////////////////////////////////////
//
// class  GCLearnSkillFailedFactory;
//
// Factory for  GCLearnSkillFailed
//
//////////////////////////////////////////////////////////////////////

class GCLearnSkillFailedFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_LEARN_SKILL_FAILED;
    static constexpr std::string_view kName = "GCLearnSkillFailed";
    static constexpr PacketSize_t kMaxSize{szSkillType + szBYTE};

    // constructor
    GCLearnSkillFailedFactory() {}

    // destructor
    virtual ~GCLearnSkillFailedFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCLearnSkillFailed();
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


#endif // __GC_LEARN_SKILL_FAILED_H__
