//////////////////////////////////////////////////////////////////////
//
// Filename    : GCKnocksTargetBackOK5.h
// Written By  : elca@ewestsoft.com
// Description : Packet sent to those the skill's user can see but its victim cannot
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_KNOCKS_TARGET_BACK_OK_5_H__
#define __GC_KNOCKS_TARGET_BACK_OK_5_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCKnocksTargetBackOK5;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCKnocksTargetBackOK5 : public Packet {
public:
    // constructor
    GCKnocksTargetBackOK5();

    // destructor
    ~GCKnocksTargetBackOK5();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_KNOCKS_TARGET_BACK_OK_5;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szObjectID + szObjectID + szbool + szSkillType + szCoord * 2 + szDir;
    }

    // get packet's name
    string getPacketName() const {
        return "GCKnocksTargetBackOK5";
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

    // get / set ObjectID
    ObjectID_t getTargetObjectID() const {
        return m_TargetObjectID;
    }
    void setTargetObjectID(ObjectID_t TargetObjectID) {
        m_TargetObjectID = TargetObjectID;
    }
    //	Coord_t getTargetX() const  { return m_X;}
    //	Coord_t getTargetY() const  { return m_Y;}
    //	void setTargetXY(Coord_t X, Coord_t Y)  { m_X = X; m_Y = Y;}

    // get / set success
    bool getSkillSuccess() const {
        return m_bSuccess;
    }
    void setSkillSuccess(bool bSuccess) {
        m_bSuccess = bSuccess;
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
    //	Coord_t m_X, m_Y;

    // TargetObjectID
    ObjectID_t m_TargetObjectID = 0;

    // bool
    bool m_bSuccess = false;

    Coord_t m_X = 0, m_Y = 0; // Coordinates it was moved to.
    Dir_t m_dir = 0;          // Direction it was pushed back in.
    SkillType_t m_SkillType = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCKnocksTargetBackOK5Factory;
//
// Factory for GCKnocksTargetBackOK5
//
//////////////////////////////////////////////////////////////////////

class GCKnocksTargetBackOK5Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_KNOCKS_TARGET_BACK_OK_5;
    static constexpr std::string_view kName = "GCKnocksTargetBackOK5";
    static constexpr PacketSize_t kMaxSize{szObjectID + szObjectID + szbool + szSkillType + szCoord * 2 + szDir};

    // constructor
    GCKnocksTargetBackOK5Factory() {}

    // destructor
    virtual ~GCKnocksTargetBackOK5Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCKnocksTargetBackOK5();
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
