//////////////////////////////////////////////////////////////////////
//
// Filename    : GCKnocksTargetBackOK2.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_KNOCKS_TARGET_BACK_OK_2_H__
#define __GC_KNOCKS_TARGET_BACK_OK_2_H__

// include files
#include "Exception.h"
#include "ModifyInfo.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCKnocksTargetBackOK2;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCKnocksTargetBackOK2 : public ModifyInfo {
public:
    // constructor
    GCKnocksTargetBackOK2();

    // destructor
    ~GCKnocksTargetBackOK2();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_KNOCKS_TARGET_BACK_OK_2;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szSkillType + szCoord * 2 + szDir + szObjectID + ModifyInfo::getPacketSize();
    }

    // get packet's name
    string getPacketName() const {
        return "GCKnocksTargetBackOK2";
    }

    // get packet's debug string
    string toString() const;

    // get / set CEffectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
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

    Coord_t m_X = 0, m_Y = 0; // Coordinates it was moved to.
    Dir_t m_dir = 0;          // Direction it was pushed back in.
    SkillType_t m_SkillType = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCKnocksTargetBackOK2Factory;
//
// Factory for GCKnocksTargetBackOK2
//
//////////////////////////////////////////////////////////////////////

class GCKnocksTargetBackOK2Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_KNOCKS_TARGET_BACK_OK_2;
    static constexpr std::string_view kName = "GCKnocksTargetBackOK2";
    static constexpr PacketSize_t kMaxSize{szSkillType + szCoord * 2 + szDir + szObjectID +
                                           ModifyInfo::getPacketMaxSize()};

    // constructor
    GCKnocksTargetBackOK2Factory() {}

    // destructor
    virtual ~GCKnocksTargetBackOK2Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCKnocksTargetBackOK2();
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
