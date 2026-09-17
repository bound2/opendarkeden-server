//////////////////////////////////////////////////////////////////////
//
// Filename    : CGAttack
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_ATTACK_H__
#define __CG_ATTACK_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

#ifdef __LINUX__
// #include "GCGlobalHandler.h"
#endif // __LINUX__


//////////////////////////////////////////////////////////////////////
//
// class CGAttack;
//
//////////////////////////////////////////////////////////////////////

class CGAttack : public Packet {
public:
    // constructor
    CGAttack();

    // destructor
    ~CGAttack();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_ATTACK;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szObjectID + szCoord + szCoord + szDir;
    }

    // get packet name
    string getPacketName() const {
        return "CGAttack";
    }

    // get packet's debug string
    string toString() const;

    // get/set X Coordicate
    Coord_t getX() const {
        return m_X;
    }
    void setX(Coord_t x) {
        m_X = x;
    }

    // get/set Y Coordicate
    Coord_t getY() const {
        return m_Y;
    }
    void setY(Coord_t y) {
        m_Y = y;
    }

    // get/set Direction
    Dir_t getDir() const {
        return m_Dir;
    }
    void setDir(Dir_t dir) {
        m_Dir = dir;
    }

    // get/set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

private:
    ObjectID_t m_ObjectID; // ObjectID
    Coord_t m_X;           // X coordinate
    Coord_t m_Y;           // Y coordinate
    Dir_t m_Dir;           // Direction
};


//////////////////////////////////////////////////////////////////////
//
// class CGAttackFactory;
//
// Factory for CGAttack
//
//////////////////////////////////////////////////////////////////////

class CGAttackFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_ATTACK;
    static constexpr std::string_view kName = "CGAttack";
    static constexpr PacketSize_t kMaxSize{szObjectID + szCoord + szCoord + szDir};

    // constructor
    CGAttackFactory() {}

    // destructor
    virtual ~CGAttackFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGAttack();
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
// class CGAttackHandler;
//
//////////////////////////////////////////////////////////////////////

class CGAttackHandler {
public:
    // execute packet's handler
    static void execute(CGAttack* pCGAttack, Player* player);
};

#endif
