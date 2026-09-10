//////////////////////////////////////////////////////////////////////
//
// Filename    : GCRemoveEffect.h
// Written By  : crazydog
// Description : Effect 제거.
//
//////////////////////////////////////////////////////////////////////

#ifndef __REMOVE_EFFECT_H__
#define __REMOVE_EFFECT_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCRemoveEffect;
//
// 게임서버에서 클라이언트로 자신의 변형된 데이터를 알려주기 위한 객채
// RemoveEffectrmation, SkillToObjectOK 등에 실려서 날아간다.
//
//////////////////////////////////////////////////////////////////////

class GCRemoveEffect : public Packet {
public:
    // constructor
    GCRemoveEffect();

    // destructor
    ~GCRemoveEffect();

public:
    PacketID_t getPacketID() const {
        return PACKET_GC_REMOVE_EFFECT;
    }
    string getPacketName() const {
        return "GCRemoveEffect";
    }


    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;

    // The effect list is counted in a BYTE, and the factory max budgets this
    // many ids.
    static constexpr uint kMaxCount = 255;

    // get packet's body size
    // 최적화시, 미리 계산된 정수를 사용한다.
    PacketSize_t getPacketSize() const {
        return (PacketSize_t)(szObjectID + szBYTE + szEffectID * m_EffectList.size());
    }
    static constexpr PacketSize_t getPacketMaxSize() {
        return szObjectID + szBYTE + szEffectID * kMaxCount;
    }

    // get packet's debug string
    string toString() const;

    // get ListNumber
    BYTE getListNum() const {
        return (BYTE)m_EffectList.size();
    }

    // get&set ObjectID
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t id) {
        m_ObjectID = id;
    }

    // add / delete / clear S List
    void addEffectList(EffectID_t id);

    // ClearList
    void clearList() {
        m_EffectList.clear();
    }

    // pop front Element in Status List
    WORD popFrontListElement() {
        if (m_EffectList.empty())
            throw InvalidProtocolException("no effect left");
        EffectID_t effectID = m_EffectList.front();
        m_EffectList.pop_front();
        return effectID;
    }

protected:
    ObjectID_t m_ObjectID = 0;

    // Status List
    list<EffectID_t> m_EffectList;
};

//////////////////////////////////////////////////////////////////////
//
// class GCRemoveEffectFactory;
//
// Factory for GCRemoveEffect
//
//////////////////////////////////////////////////////////////////////

class GCRemoveEffectFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_REMOVE_EFFECT;
    static constexpr std::string_view kName = "GCRemoveEffect";
    static constexpr PacketSize_t kMaxSize{GCRemoveEffect::getPacketMaxSize()};

    // constructor
    GCRemoveEffectFactory() {}

    // destructor
    virtual ~GCRemoveEffectFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCRemoveEffect();
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
