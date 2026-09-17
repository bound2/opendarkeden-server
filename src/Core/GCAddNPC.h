//--------------------------------------------------------------------------------
//
// Filename    : GCAddNPC.h
// Written By  : Reiot
//
//--------------------------------------------------------------------------------

#ifndef __GC_ADD_NPC_H__
#define __GC_ADD_NPC_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//--------------------------------------------------------------------------------
//
// class GCAddNPC;
//
// When an NPC comes into view, its information is received in this packet.
//
//--------------------------------------------------------------------------------

class GCAddNPC : public Packet {
public:
    GCAddNPC(){};
    ~GCAddNPC(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_NPC;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCAddNPCPacketSize.
    PacketSize_t getPacketSize() const {
        return szObjectID + de::wire::stringWireSize(m_Name) + szNPCID + szSpriteType + szColor + szColor + szCoord +
               szCoord + szDir;
    }

    // get packet's name
    string getPacketName() const {
        return "GCAddNPC";
    }

    // get packet's debug string
    string toString() const;


public:
    // get/set object id
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t creatureID) {
        m_ObjectID = creatureID;
    }

    // get/set name
    string getName() const {
        return m_Name;
    }
    void setName(string name) {
        m_Name = name;
    }

    // get/set NPC id
    NPCID_t getNPCID(void) const {
        return m_NPCID;
    }
    void setNPCID(NPCID_t NPCID) {
        m_NPCID = NPCID;
    }

    // get/set sprite type
    SpriteType_t getSpriteType() const {
        return m_SpriteType;
    }
    void setSpriteType(SpriteType_t spriteType) {
        m_SpriteType = spriteType;
    }

    // get/set main color
    Color_t getMainColor() const {
        return m_MainColor;
    }
    void setMainColor(Color_t color) {
        m_MainColor = color;
    }

    // get/set sub color
    Color_t getSubColor() const {
        return m_SubColor;
    }
    void setSubColor(Color_t color) {
        m_SubColor = color;
    }

    // get/set X
    Coord_t getX() const {
        return m_X;
    }
    void setX(Coord_t x) {
        m_X = x;
    }

    // get/set Y
    Coord_t getY() const {
        return m_Y;
    }
    void setY(Coord_t y) {
        m_Y = y;
    }

    // get/set Dir
    Dir_t getDir() const {
        return m_Dir;
    }
    void setDir(Dir_t dir) {
        m_Dir = dir;
    }

private:
    // Used to tell objects apart by an id that is unique within the zone.
    ObjectID_t m_ObjectID;

    // Name of the NPC
    string m_Name;

    // Id of the NPC (not the object id.)
    NPCID_t m_NPCID;

    // Sprite type
    SpriteType_t m_SpriteType;

    // Colour information
    Color_t m_MainColor;
    Color_t m_SubColor;

    // X, Y and direction
    Coord_t m_X;
    Coord_t m_Y;
    Dir_t m_Dir;
};


//////////////////////////////////////////////////////////////////////
//
// class GCAddNPCFactory;
//
// Factory for GCAddNPC
//
//////////////////////////////////////////////////////////////////////

class GCAddNPCFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_NPC;
    static constexpr std::string_view kName = "GCAddNPC";
    static constexpr PacketSize_t kMaxSize{szObjectID + szBYTE + 40 + szNPCID + szSpriteType + szColor + szColor +
                                           szCoord + szCoord + szDir};

    // create packet
    Packet* createPacket() override {
        return new GCAddNPC();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCAddNPCPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
