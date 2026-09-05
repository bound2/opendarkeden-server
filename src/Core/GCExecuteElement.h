//////////////////////////////////////////////////////////////////////////////
// Filename    : GCExecuteElement.h
// Written By  : elca@ewestsoft.com
// Description :
// 기술이 성공했을때 보내는 패킷을 위한 클래스 정의
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_EXECUTE_ELEMENT_H__
#define __GC_EXECUTE_ELEMENT_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GCExecuteElement;
// 게임서버에서 클라이언트로 자신의 기술이 성공을 알려주기 위한 클래스
//////////////////////////////////////////////////////////////////////////////

class GCExecuteElement : public Packet {
public:
    GCExecuteElement();
    ~GCExecuteElement();

public:
    void read(SocketInputStream& iStream) {
        iStream.read(m_QuestID);
        iStream.read(m_Condition);
        iStream.read(m_Index);
    }
    void write(SocketOutputStream& oStream) const {
        oStream.write(m_QuestID);
        oStream.write(m_Condition);
        oStream.write(m_Index);
    }
    PacketID_t getPacketID() const {
        return PACKET_GC_EXECUTE_ELEMENT;
    }
    PacketSize_t getPacketSize() const {
        return szDWORD + szBYTE + szWORD;
    }
    string getPacketName() const {
        return "GCExecuteElement";
    }
    string toString() const;

public:
    DWORD getQuestID() const {
        return m_QuestID;
    }
    void setQuestID(DWORD id) {
        m_QuestID = id;
    }

    BYTE getCondition() const {
        return m_Condition;
    }
    void setCondition(BYTE cond) {
        m_Condition = cond;
    }

    WORD getIndex() const {
        return m_Index;
    }
    void setIndex(WORD idx) {
        m_Index = idx;
    }

private:
    DWORD m_QuestID;
    BYTE m_Condition; // 어느 조건에 있는가 0 : Happen, 1 : Complete, 2 : Fail, 3 : Reward
    WORD m_Index;     // 해당 조건의 몇번째 element인가
};


//////////////////////////////////////////////////////////////////////////////
// class GCExecuteElementFactory;
//////////////////////////////////////////////////////////////////////////////

class GCExecuteElementFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_EXECUTE_ELEMENT;
    static constexpr std::string_view kName = "GCExecuteElement";
    static constexpr PacketSize_t kMaxSize{szDWORD + szBYTE + szWORD};

    GCExecuteElementFactory() {}
    virtual ~GCExecuteElementFactory() {}

public:
    Packet* createPacket() override {
        return new GCExecuteElement();
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
