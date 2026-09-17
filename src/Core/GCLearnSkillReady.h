//////////////////////////////////////////////////////////////////////
//
// Filename    :  GCLearnSkillReady.h
// Written By  :  elca@ewestsoft.com
// Description :  Declaration of the GCLearnSkillReady packet class
//
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_LEARN_SKILL_READY_H__
#define __GC_LEARN_SKILL_READY_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class  GCLearnSkillReady;
//
//////////////////////////////////////////////////////////////////////

class GCLearnSkillReady : public Packet {
public:
    // constructor
    GCLearnSkillReady();

    // destructor
    ~GCLearnSkillReady();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_LEARN_SKILL_READY;
    }

    // get packet size
    PacketSize_t getPacketSize() const {
        return szSkillDomainType;
    }

    // get packet's name
    string getPacketName() const {
        return "GCLearnSkillReady";
    }

    // get packet's debug string
    string toString() const;

    SkillDomainType_t getSkillDomainType() const {
        return m_SkillDomainType;
    }
    void setSkillDomainType(SkillDomainType_t SkillDomainType) {
        m_SkillDomainType = SkillDomainType;
    }

private:
    // Skill type
    SkillDomainType_t m_SkillDomainType = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class  GCLearnSkillReadyFactory;
//
// Factory for  GCLearnSkillReady
//
//////////////////////////////////////////////////////////////////////

class GCLearnSkillReadyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_LEARN_SKILL_READY;
    static constexpr std::string_view kName = "GCLearnSkillReady";
    static constexpr PacketSize_t kMaxSize{szSkillDomainType};

    // constructor
    GCLearnSkillReadyFactory() {}

    // destructor
    virtual ~GCLearnSkillReadyFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCLearnSkillReady();
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


#endif // __GC_LEARN_SKILL_READY_H__
