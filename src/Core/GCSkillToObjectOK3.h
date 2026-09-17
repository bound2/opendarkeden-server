//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSkillToObjectOK3.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
// 				Packet received by those who can see the one who used the skill (the target cannot be seen)
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_SKILL_TO_OBJECT_OK_3_H__
#define __GC_SKILL_TO_OBJECT_OK_3_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCSkillToObjectOK3;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCSkillToObjectOK3 : public Packet {
public:
    // constructor
    GCSkillToObjectOK3();

    // destructor
    ~GCSkillToObjectOK3();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SKILL_TO_OBJECT_OK_3;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szObjectID + szSkillType + szCoord * 2 + szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "GCSkillToObjectOK3";
    }

    // get packet's debug string
    string toString() const;

    // get / set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    // get / set SkillType
    SkillType_t getSkillType() const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t SkillType) {
        m_SkillType = SkillType;
    }

    // get / set Target X,Y
    Coord_t getTargetX() const {
        return m_TargetX;
    }
    Coord_t getTargetY() const {
        return m_TargetY;
    }
    void setTargetXY(Coord_t X, Coord_t Y) {
        m_TargetX = X;
        m_TargetY = Y;
    }


    // get / set CEffectID
    //	CEffectID_t getCEffectID() const  { return m_CEffectID; }
    //	void setCEffectID(CEffectID_t e)  { m_CEffectID = e; }

    BYTE getGrade() const {
        return m_Grade;
    }
    void setGrade(BYTE grade) {
        m_Grade = grade;
    }

private:
    // ObjectID
    ObjectID_t m_ObjectID;

    // SkillType
    SkillType_t m_SkillType;

    // TargetObjectID
    Coord_t m_TargetX, m_TargetY;


    // CEffectID
    //	CEffectID_t m_CEffectID;

    BYTE m_Grade;
};


//////////////////////////////////////////////////////////////////////
//
// class GCSkillToObjectOK3Factory;
//
// Factory for GCSkillToObjectOK3
//
//////////////////////////////////////////////////////////////////////

class GCSkillToObjectOK3Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SKILL_TO_OBJECT_OK_3;
    static constexpr std::string_view kName = "GCSkillToObjectOK3";
    static constexpr PacketSize_t kMaxSize{szObjectID + szSkillType + szCoord * 2 + szBYTE};

    // constructor
    GCSkillToObjectOK3Factory() {}

    // destructor
    virtual ~GCSkillToObjectOK3Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCSkillToObjectOK3();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
