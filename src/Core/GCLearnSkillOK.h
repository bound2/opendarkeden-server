//////////////////////////////////////////////////////////////////////
//
// Filename    :  GCLearnSkillOK.h
// Written By  :  elca@ewestsoft.com
// Description :  Declaration of the GCLearnSkillOK packet class
//
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_LEARN_SKILL_OK_H__
#define __GC_LEARN_SKILL_OK_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class  GCLearnSkillOK;
//
//////////////////////////////////////////////////////////////////////

class GCLearnSkillOK : public Packet {
public:
    // constructor
    GCLearnSkillOK();

    // destructor
    ~GCLearnSkillOK();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_LEARN_SKILL_OK;
    }

    // get packet size
    PacketSize_t getPacketSize() const {
        return szSkillType + szSkillDomainType;
    }

    // get packet's name
    string getPacketName() const {
        return "GCLearnSkillOK";
    }

    // get packet's debug string
    string toString() const;

    // get/set m_SkillType
    SkillType_t getSkillType() const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t SkillType) {
        m_SkillType = SkillType;
    }

    // get/set m_SkillDomainType
    SkillDomainType_t getSkillDomainType() const {
        return m_DomainType;
    }
    void setSkillDomainType(SkillDomainType_t DomainType) {
        m_DomainType = DomainType;
    }

private:
    // SkillType
    SkillType_t m_SkillType = 0;

    // DomainType
    SkillDomainType_t m_DomainType = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class  GCLearnSkillOKFactory;
//
// Factory for  GCLearnSkillOK
//
//////////////////////////////////////////////////////////////////////

class GCLearnSkillOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_LEARN_SKILL_OK;
    static constexpr std::string_view kName = "GCLearnSkillOK";
    static constexpr PacketSize_t kMaxSize{szSkillType + szSkillDomainType};

    // constructor
    GCLearnSkillOKFactory() {}

    // destructor
    virtual ~GCLearnSkillOKFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCLearnSkillOK();
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


#endif // __GC_LEARN_SKILL_OK_H__
