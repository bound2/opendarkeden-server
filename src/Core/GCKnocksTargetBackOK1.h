//////////////////////////////////////////////////////////////////////
//
// Filename    : GCKnocksTargetBackOK1.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_KNOCKS_TARGET_BACK_OK_1_H__
#define __GC_KNOCKS_TARGET_BACK_OK_1_H__

// include files
#include "Exception.h"
#include "ModifyInfo.h"
#include "ModifyItemInfo.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCKnocksTargetBackOK1;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCKnocksTargetBackOK1 : public ModifyInfo {
public:
    // constructor
    GCKnocksTargetBackOK1();

    // destructor
    ~GCKnocksTargetBackOK1();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_KNOCKS_TARGET_BACK_OK_1;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szSkillType + szDir + szCoord * 2 + szObjectID + szBullet + szbool + ModifyInfo::getPacketSize();
    }

    // get packet's name
    string getPacketName() const {
        return "GCKnocksTargetBackOK1";
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

    // get / set Success
    bool getSkillSuccess() const {
        return m_bSuccess;
    }
    void setSkillSuccess(bool Success) {
        m_bSuccess = Success;
    }

    // get / set Bullet Num
    Bullet_t getBullet() const {
        return m_BulletNum;
    }
    void setBulletNum(Bullet_t BulletNum) {
        m_BulletNum = BulletNum;
    }

    void setXYDir(Coord_t x, Coord_t y, Coord_t dir) {
        m_X = x;
        m_Y = y;
        m_dir = dir;
    }
    Coord_t getX() const {
        return m_X;
    }
    Coord_t getY() const {
        return m_Y;
    }
    Dir_t getDir() const {
        return m_dir;
    }

    SkillType_t getSkillType() const {
        return m_SkillType;
    }
    void setSkillType(SkillType_t s) {
        m_SkillType = s;
    }


private:
    // ObjectID
    ObjectID_t m_ObjectID = 0;

    // Bullet Num
    Bullet_t m_BulletNum = 0;

    bool m_bSuccess = false;

    Coord_t m_X = 0, m_Y = 0; // Coordinates it was moved to.
    Dir_t m_dir = 0;          // Direction it was pushed back in.
    SkillType_t m_SkillType = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCKnocksTargetBackOK1Factory;
//
// Factory for GCKnocksTargetBackOK1
//
//////////////////////////////////////////////////////////////////////

class GCKnocksTargetBackOK1Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_KNOCKS_TARGET_BACK_OK_1;
    static constexpr std::string_view kName = "GCKnocksTargetBackOK1";
    static constexpr PacketSize_t kMaxSize{szSkillType + szDir + szCoord * 2 + szObjectID + szBullet + szbool +
                                           ModifyInfo::getPacketMaxSize()};

    // constructor
    GCKnocksTargetBackOK1Factory() {}

    // destructor
    virtual ~GCKnocksTargetBackOK1Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCKnocksTargetBackOK1();
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
