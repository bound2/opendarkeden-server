//////////////////////////////////////////////////////////////////////
//
// Filename    : GCAttackArmsOK1.h
// Written By  : elca@ewestsoft.com
// Description : Packet notifying ranged-attack success to the client
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_ATTACK_ARMS_OK_1_H__
#define __GC_ATTACK_ARMS_OK_1_H__

// include files
#include "Exception.h"
#include "ModifyInfo.h"
#include "PacketFactory.h"
#include "Types.h"
// #include "ModifyItemInfo.h"

//////////////////////////////////////////////////////////////////////
//
// class GCAttackArmsOK1;
//
// Client notification that its ranged attack succeeded
//
//////////////////////////////////////////////////////////////////////

class GCAttackArmsOK1 : public ModifyInfo {
public:
    // constructor
    GCAttackArmsOK1();

    // destructor
    ~GCAttackArmsOK1();


public:
    // Initialize packet by reading data from the incoming stream.
    void read(SocketInputStream& iStream);

    // Serialize packet data to the outgoing stream.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_ATTACK_ARMS_OK_1;
    }

    // get packet's body size
    // Includes skill, object, bullet, success flag, and modification info.
    PacketSize_t getPacketSize() const {
        return szSkillType + szObjectID + szBullet + szbool + ModifyInfo::getPacketSize();
    }

    // get packet's name
    string getPacketName() const {
        return "GCAttackArmsOK1";
    }

    // get packet's debug string
    string toString() const;

    SkillType_t getSkillType() const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t skillType) {
        m_SkillType = skillType;
    }
    // get / set Success
    bool getSkillSuccess() const {
        return m_bSuccess;
    }
    void setSkillSuccess(bool Success) {
        m_bSuccess = Success;
    }

    // get / set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

    // get / set Bullet Num
    Bullet_t getBullet() const {
        return m_BulletNum;
    }
    void setBulletNum(Bullet_t BulletNum) {
        m_BulletNum = BulletNum;
    }

private:
    SkillType_t m_SkillType;
    // ObjectID
    ObjectID_t m_ObjectID;

    // Bullet Num
    Bullet_t m_BulletNum;

    // success (whether damage applied)
    bool m_bSuccess;
};


//////////////////////////////////////////////////////////////////////
//
// class GCAttackArmsOK1Factory;
//
// Factory for GCAttackArmsOK1
//
//////////////////////////////////////////////////////////////////////

class GCAttackArmsOK1Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ATTACK_ARMS_OK_1;
    static constexpr std::string_view kName = "GCAttackArmsOK1";
    static constexpr PacketSize_t kMaxSize{szSkillType + szObjectID + szBullet + szbool +
                                           ModifyInfo::getPacketMaxSize()};

    // constructor
    GCAttackArmsOK1Factory() {}

    // destructor
    virtual ~GCAttackArmsOK1Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCAttackArmsOK1();
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
