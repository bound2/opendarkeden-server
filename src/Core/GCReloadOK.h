//////////////////////////////////////////////////////////////////////
//
// Filename    : GCReloadOK.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_RELOAD_OK_H__
#define __GC_RELOAD_OK_H__

// include files
#include "ModifyInfo.h"
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCReloadOK;
//
//////////////////////////////////////////////////////////////////////

class GCReloadOK : public Packet {
public:
    // Constructor
    GCReloadOK();

    // Desctructor
    ~GCReloadOK();

    // Initialize the packet by reading data from the input stream.
    void read(SocketInputStream& iStream);

    // Serialize the packet into the output stream.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_RELOAD_OK;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Use GCReloadOKPacketSize if that constant is defined.
    PacketSize_t getPacketSize() const {
        return szBullet;
    }

    // get packet name
    string getPacketName() const {
        return "GCReloadOK";
    }

    // get packet's debug string
    string toString() const;

    // get / set BulletNum
    Bullet_t getBulletNum() const {
        return m_BulletNum;
    }
    void setBulletNum(Bullet_t BulletNum) {
        m_BulletNum = BulletNum;
    }


private:
    // BulletNum
    Bullet_t m_BulletNum;
};


//////////////////////////////////////////////////////////////////////
//
// class GCReloadOKFactory;
//
// Factory for GCReloadOK
//
//////////////////////////////////////////////////////////////////////

class GCReloadOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_RELOAD_OK;
    static constexpr std::string_view kName = "GCReloadOK";
    static constexpr PacketSize_t kMaxSize{szBullet};

    // create packet
    Packet* createPacket() override {
        return new GCReloadOK();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's max body size
    // *OPTIMIZATION HINT*
    // Use GCReloadOKPacketSize if that constant is defined.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
