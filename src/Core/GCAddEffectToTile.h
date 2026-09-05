//////////////////////////////////////////////////////////////////////////////
// Filename    : GCAddEffectToTile.h
// Written By  : elca@ewestsoft.com
// Description :
// ����� ���������� ������ ��Ŷ�� ���� Ŭ���� ����
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_ADD_EFFECT_TO_TILE_H__
#define __GC_ADD_EFFECT_TO_TILE_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GCAddEffectToTile;
// ���Ӽ������� Ŭ���̾�Ʈ�� �ڽ��� ����� ������ �˷��ֱ� ���� Ŭ����
//////////////////////////////////////////////////////////////////////////////

class GCAddEffectToTile : public Packet {
public:
    GCAddEffectToTile();
    ~GCAddEffectToTile() noexcept;

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_EFFECT_TO_TILE;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szCoord * 2 + szEffectID + szDuration;
    }
    string getPacketName() const {
        return "GCAddEffectToTile";
    }
    string toString() const;

public:
    EffectID_t getEffectID() const {
        return m_EffectID;
    }
    void setEffectID(EffectID_t e) {
        m_EffectID = e;
    }

    Duration_t getDuration() const {
        return m_Duration;
    }
    void setDuration(Duration_t d) {
        m_Duration = d;
    }

    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t d) {
        m_ObjectID = d;
    }

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
    Coord_t m_X;
    Coord_t m_Y;
    ObjectID_t m_ObjectID;
    EffectID_t m_EffectID;
    Duration_t m_Duration;
};

//////////////////////////////////////////////////////////////////////////////
// class GCAddEffectToTileFactory;
//////////////////////////////////////////////////////////////////////////////

class GCAddEffectToTileFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_EFFECT_TO_TILE;
    static constexpr std::string_view kName = "GCAddEffectToTile";
    static constexpr PacketSize_t kMaxSize{szObjectID + szCoord * 2 + szEffectID + szDuration};

    Packet* createPacket() override {
        return new GCAddEffectToTile();
    }
    string getPacketName() const override {
        return string(kName);
    }
    PacketID_t getPacketID() const override {
        return kPacketID;
    }
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};

#endif
