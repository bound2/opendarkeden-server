//////////////////////////////////////////////////////////////////////
//
// Filename    : GCKnocksTargetBackOK4.h
// Written By  : elca@ewestsoft.com
// Description : Packet sent to those the skill's victim can see but its user cannot
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_KNOCKS_TARGET_BACK_OK_4_H__
#define __GC_KNOCKS_TARGET_BACK_OK_4_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCKnocksTargetBackOK4;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCKnocksTargetBackOK4 : public Packet {
public:
    // constructor
    GCKnocksTargetBackOK4();

    // destructor
    ~GCKnocksTargetBackOK4();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_KNOCKS_TARGET_BACK_OK_4;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    //	PacketSize_t getPacketSize() const  { return szObjectID + szObjectID + szbool; }
    PacketSize_t getPacketSize() const {
        return szObjectID + szSkillType + szCoord * 2 + szDir;
    }

    // get packet's name
    string getPacketName() const {
        return "GCKnocksTargetBackOK4";
    }

    // get packet's debug string
    string toString() const;

    // get / set ObjectID
    //	ObjectID_t getObjectID() const  { return m_ObjectID; }
    //	void setObjectID(ObjectID_t ObjectID)  { m_ObjectID = ObjectID; }

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
    //	bool getSkillSuccess() const  { return m_bSuccess;}
    //	void setSkillSuccess(bool bSuccess)  { m_bSuccess = bSuccess;}
    //
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
    //	ObjectID_t m_ObjectID;
    //	Coord_t m_X, m_Y;

    // TargetObjectID
    ObjectID_t m_TargetObjectID = 0;

    Coord_t m_X = 0, m_Y = 0; // Coordinates it was moved to.
    Dir_t m_dir = 0;          // Direction it was pushed back in.
    SkillType_t m_SkillType = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class GCKnocksTargetBackOK4Factory;
//
// Factory for GCKnocksTargetBackOK4
//
//////////////////////////////////////////////////////////////////////

class GCKnocksTargetBackOK4Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_KNOCKS_TARGET_BACK_OK_4;
    static constexpr std::string_view kName = "GCKnocksTargetBackOK4";
    static constexpr PacketSize_t kMaxSize{szObjectID + szSkillType + szDir + szCoord * 2};

    // constructor
    GCKnocksTargetBackOK4Factory() {}

    // destructor
    virtual ~GCKnocksTargetBackOK4Factory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCKnocksTargetBackOK4();
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
