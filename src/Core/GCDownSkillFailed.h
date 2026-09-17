//////////////////////////////////////////////////////////////////////
//
// Filename    :  GCDownSkillFailed.h
// Written By  :  elca@ewestsoft.com
// Description :
//
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_DOWN_SKILL_FAILED_H__
#define __GC_DOWN_SKILL_FAILED_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class  GCDownSkillFailed;
//
//////////////////////////////////////////////////////////////////////

class GCDownSkillFailed : public Packet {
public:
    GCDownSkillFailed();
    virtual ~GCDownSkillFailed();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_DOWN_SKILL_FAILED;
    }

    // get packet size
    PacketSize_t getPacketSize() const {
        return szSkillType + szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "GCDownSkillFailed";
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
    SkillType_t m_SkillType = 0;
    BYTE m_Desc = 0; // The reason why learning the skill failed.
                     // See CGDownSkillHandler for the details.
};


//////////////////////////////////////////////////////////////////////
//
// class  GCDownSkillFailedFactory;
//
// Factory for  GCDownSkillFailed
//
//////////////////////////////////////////////////////////////////////

class GCDownSkillFailedFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_DOWN_SKILL_FAILED;
    static constexpr std::string_view kName = "GCDownSkillFailed";
    static constexpr PacketSize_t kMaxSize{szSkillType + szBYTE};

    // constructor
    GCDownSkillFailedFactory() {}

    // destructor
    virtual ~GCDownSkillFailedFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCDownSkillFailed();
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


#endif // __GC_DOWN_SKILL_FAILED_H__
