//////////////////////////////////////////////////////////////////////
//
// Filename    : GCDeleteEffectFromTile.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_DELETE_EFFECT_FROM_TILE_H__
#define __GC_DELETE_EFFECT_FROM_TILE_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCDeleteEffectFromTile;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCDeleteEffectFromTile : public Packet {
public:
    // constructor
    GCDeleteEffectFromTile();

    // destructor
    ~GCDeleteEffectFromTile();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_DELETE_EFFECT_FROM_TILE;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return szObjectID + szCoord * 2 + szEffectID;
    }

    // get packet's name
    string getPacketName() const {
        return "GCDeleteEffectFromTile";
    }

    // get packet's debug string
    string toString() const;

    // get / set EffectID
    EffectID_t getEffectID() const {
        return m_EffectID;
    }
    void setEffectID(EffectID_t e) {
        m_EffectID = e;
    }


    // get / set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t d) {
        m_ObjectID = d;
    }

    // get & set X, Y
    Coord_t getX() const {
        return m_X;
    }
    Coord_t getY() const {
        return m_Y;
    }
    void setXY(Coord_t x, Coord_t y) {
        m_X = x;
        m_Y = y;
    }

private:
    Coord_t m_X, m_Y;
    ObjectID_t m_ObjectID;

    EffectID_t m_EffectID;
};


//////////////////////////////////////////////////////////////////////
//
// class GCDeleteEffectFromTileFactory;
//
// Factory for GCDeleteEffectFromTile
//
//////////////////////////////////////////////////////////////////////

class GCDeleteEffectFromTileFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_DELETE_EFFECT_FROM_TILE;
    static constexpr std::string_view kName = "GCDeleteEffectFromTile";
    static constexpr PacketSize_t kMaxSize{szObjectID + szCoord * 2 + szEffectID};

    // constructor
    GCDeleteEffectFromTileFactory() {}

    // destructor
    virtual ~GCDeleteEffectFromTileFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCDeleteEffectFromTile();
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
    // PacketSize_t getPacketMaxSize() const  { return szSkillType + szCEffectID + szDuration + szBYTE + szBYTE*
    // m_ListNum* 2 ; }
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
