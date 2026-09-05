//////////////////////////////////////////////////////////////////////////////
// Filename    : GCAddEffect.h
// Written By  : elca@ewestsoft.com
// Description :
// ����� ���������� ������ ��Ŷ�� ���� Ŭ���� ����
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_ADD_EFFECT_H__
#define __GC_ADD_EFFECT_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GCAddEffect;
// ���Ӽ������� Ŭ���̾�Ʈ�� �ڽ��� ����� ������ �˷��ֱ� ���� Ŭ����
//////////////////////////////////////////////////////////////////////////////

class GCAddEffect : public Packet {
public:
    GCAddEffect();
    ~GCAddEffect() noexcept;

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_EFFECT;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szEffectID + szDuration;
    }
    string getPacketName() const {
        return "GCAddEffect";
    }
    string toString() const;

public:
    EffectID_t getEffectID() const {
        return m_EffectID;
    }
    void setEffectID(EffectID_t e) {
        m_EffectID = e;
    }

    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t o) {
        m_ObjectID = o;
    }

    Duration_t getDuration() const {
        return m_Duration;
    }
    void setDuration(Duration_t d) {
        m_Duration = d;
    }

private:
    ObjectID_t m_ObjectID;
    EffectID_t m_EffectID;
    Duration_t m_Duration;
};


//////////////////////////////////////////////////////////////////////////////
// class GCAddEffectFactory;
//////////////////////////////////////////////////////////////////////////////

class GCAddEffectFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_EFFECT;
    static constexpr std::string_view kName = "GCAddEffect";
    static constexpr PacketSize_t kMaxSize{szObjectID + szEffectID + szDuration};

    GCAddEffectFactory() {}
    virtual ~GCAddEffectFactory() {}

public:
    Packet* createPacket() override {
        return new GCAddEffect();
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
