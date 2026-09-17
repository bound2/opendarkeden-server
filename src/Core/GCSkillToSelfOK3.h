//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSkillToSelfOK3.h
// Written By  : elca@ewestsoft.com
// Description : When a skill used on oneself succeeds and the user cannot be seen
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_SKILL_TO_SELF_OK_3_H__
#define __GC_SKILL_TO_SELF_OK_3_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCSkillToSelfOK3;
//
//
//////////////////////////////////////////////////////////////////////

class GCSkillToSelfOK3 : public Packet {
public:
    // constructor
    GCSkillToSelfOK3();

    // destructor
    ~GCSkillToSelfOK3();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SKILL_TO_SELF_OK_3;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szCoord * 2 + szSkillType + szDuration + szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "GCSkillToSelfOK3";
    }

    // get packet's debug string
    string toString() const;

    // get / set X,Y
    Coord_t getX() const {
        return m_X;
    }
    Coord_t getY() const {
        return m_Y;
    }
    void setXY(Coord_t X, Coord_t Y) {
        m_X = X;
        m_Y = Y;
    }


    // get / set SkillType
    SkillType_t getSkillType() const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t SkillType) {
        m_SkillType = SkillType;
    }

    // get / set Duration
    Duration_t getDuration() const {
        return m_Duration;
    }
    void setDuration(Duration_t Duration) {
        m_Duration = Duration;
    }

    BYTE getGrade() const {
        return m_Grade;
    }
    void setGrade(BYTE grade) {
        m_Grade = grade;
    }

private:
    // X,Y
    Coord_t m_X, m_Y;

    // SkillType
    SkillType_t m_SkillType;

    // Duration
    Duration_t m_Duration;

    BYTE m_Grade;
};


//////////////////////////////////////////////////////////////////////
//
// class GCSkillToSelfOK3Factory;
//
// Factory for GCSkillToSelfOK3
//
//////////////////////////////////////////////////////////////////////

class GCSkillToSelfOK3Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SKILL_TO_SELF_OK_3;
    static constexpr std::string_view kName = "GCSkillToSelfOK3";
    static constexpr PacketSize_t kMaxSize{szCoord * 2 + szSkillType + szDuration + szBYTE};

    // constructor
    GCSkillToSelfOK3Factory() {}

    // destructor
    virtual ~GCSkillToSelfOK3Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCSkillToSelfOK3();
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


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
