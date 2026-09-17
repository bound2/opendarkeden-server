//////////////////////////////////////////////////////////////////////
//
// Filename    : GCAttackArmsOK3.h
// Written By  : elca@ewestsoft.com
// Description : Packet sent to those the skill's user can see but its victim cannot
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_ATTACK_ARMS_OK_3_H__
#define __GC_ATTACK_ARMS_OK_3_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCAttackArmsOK3;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCAttackArmsOK3 : public Packet {
public:
    // constructor
    GCAttackArmsOK3();

    // destructor
    ~GCAttackArmsOK3();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_ATTACK_ARMS_OK_3;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    //	PacketSize_t getPacketSize() const  { return szObjectID + szObjectID + szbool; }
    PacketSize_t getPacketSize() const {
        return szSkillType + szObjectID + szCoord * 2;
    }

    // get packet's name
    string getPacketName() const {
        return "GCAttackArmsOK3";
    }

    // get packet's debug string
    string toString() const;

    SkillType_t getSkillType() const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t skillType) {
        m_SkillType = skillType;
    }
    // get / set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    // get / set ObjectID
    //	ObjectID_t getTargetObjectID() const  { return m_TargetObjectID; }
    //	void setTargetObjectID(ObjectID_t TargetObjectID)  { m_TargetObjectID = TargetObjectID; }
    Coord_t getTargetX() const {
        return m_X;
    }
    Coord_t getTargetY() const {
        return m_Y;
    }
    void setTargetXY(Coord_t X, Coord_t Y) {
        m_X = X;
        m_Y = Y;
    }

    // get / set success
    //	bool getSkillSuccess() const  { return m_bSuccess;}
    //	void setSkillSuccess(bool bSuccess)  { m_bSuccess = bSuccess;}

private:
    SkillType_t m_SkillType;
    // ObjectID
    ObjectID_t m_ObjectID;
    Coord_t m_X, m_Y;

    // TargetObjectID
    //	ObjectID_t m_TargetObjectID;
};


//////////////////////////////////////////////////////////////////////
//
// class GCAttackArmsOK3Factory;
//
// Factory for GCAttackArmsOK3
//
//////////////////////////////////////////////////////////////////////

class GCAttackArmsOK3Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ATTACK_ARMS_OK_3;
    static constexpr std::string_view kName = "GCAttackArmsOK3";
    static constexpr PacketSize_t kMaxSize{szSkillType + szObjectID + szCoord * 2};

    // constructor
    GCAttackArmsOK3Factory() {}

    // destructor
    virtual ~GCAttackArmsOK3Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCAttackArmsOK3();
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
