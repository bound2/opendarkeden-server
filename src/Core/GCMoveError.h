//////////////////////////////////////////////////////////////////////
//
// Filename    : GCMoveError.h
// Written By  : Elca
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_MOVEError_H__
#define __GC_MOVEError_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class GCMoveError;
//
//////////////////////////////////////////////////////////////////////

class GCMoveError : public Packet {
public:
    // constructor
    GCMoveError() {}
    GCMoveError(Coord_t x, Coord_t y) : m_X(x), m_Y(y) {}
    ~GCMoveError(){};


public:
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_MOVE_ERROR;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szCoord + szCoord;
    }

    // get packet's name
    string getPacketName() const {
        return "GCMoveError";
    }

    // get packet's debug string
    string toString() const;


public:
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

    void setXY(Coord_t x, Coord_t y) {
        m_X = x;
        m_Y = y;
    }

private:
    Coord_t m_X; // 현재 X 좌표
    Coord_t m_Y; // 현재 Y 좌표
};


//////////////////////////////////////////////////////////////////////
//
// class  GCMoveErrorFactory;
//
// Factory for  GCMoveError
//
//////////////////////////////////////////////////////////////////////

class GCMoveErrorFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MOVE_ERROR;
    static constexpr std::string_view kName = "GCMoveError";
    static constexpr PacketSize_t kMaxSize{szCoord + szCoord};

    // create packet
    Packet* createPacket() override {
        return new GCMoveError();
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
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


#endif
