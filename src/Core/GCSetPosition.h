//----------------------------------------------------------------------
//
// Filename    : GCSetPosition.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __GC_SET_POSITION_H__
#define __GC_SET_POSITION_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class GCSetPosition;
//
// 게임 서버에서 플레이어의 위치를 설정해주는 패킷이다.
// 나중에 GCPatchPCInfo(가칭) 패킷에 통합될 전망이다.
//
//----------------------------------------------------------------------

class GCSetPosition : public Packet {
public:
    GCSetPosition(){};
    ~GCSetPosition(){};
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SET_POSITION;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // const static GCSetPositionPacketSize 를 정의해서 리턴하라.
    PacketSize_t getPacketSize() const {
        return szCoord + szCoord + szDir;
    }

    // get packet name
    string getPacketName() const {
        return "GCSetPosition";
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

private:
    Coord_t m_X; // X 좌표
    Coord_t m_Y; // Y 좌표
    Dir_t m_Dir; // 방향
};


//////////////////////////////////////////////////////////////////////
//
// class GCSetPositionFactory;
//
// Factory for GCSetPosition
//
//////////////////////////////////////////////////////////////////////

class GCSetPositionFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SET_POSITION;
    static constexpr std::string_view kName = "GCSetPosition";
    static constexpr PacketSize_t kMaxSize{szCoord + szCoord + szDir};

    // create packet
    Packet* createPacket() override {
        return new GCSetPosition();
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
    // const static GCSetPositionPacketSize 를 정의해서 리턴하라.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
