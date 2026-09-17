//////////////////////////////////////////////////////////////////////////////
// Filename    : GCExecuteElement.h
// Written By  : elca@ewestsoft.com
// Description :
// Class definition for the packet sent when a skill succeeds
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_EXECUTE_ELEMENT_H__
#define __GC_EXECUTE_ELEMENT_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GCExecuteElement;
// Class the game server uses to tell the client that its own skill succeeded
//////////////////////////////////////////////////////////////////////////////

class GCExecuteElement : public Packet {
public:
    GCExecuteElement();
    ~GCExecuteElement();

public:
    // The conditions a quest element fires under: Happen, Complete,
    // Fail, Reward.
    static constexpr BYTE kConditionMax = 4;

    void read(SocketInputStream& iStream) {
        iStream.read(m_QuestID);

        // A byte carries more values than there are conditions, so it is
        // tested before it is stored.
        BYTE condition = 0;
        iStream.read(condition);

        if (condition >= kConditionMax)
            throw InvalidProtocolException("element condition out of range");

        m_Condition = condition;

        iStream.read(m_Index);
    }
    void write(SocketOutputStream& oStream) const {
        if (m_Condition >= kConditionMax)
            throw InvalidProtocolException("element condition out of range");

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
        if (cond >= kConditionMax)
            throw InvalidProtocolException("element condition out of range");
        m_Condition = cond;
    }

    WORD getIndex() const {
        return m_Index;
    }
    void setIndex(WORD idx) {
        m_Index = idx;
    }

private:
    DWORD m_QuestID = 0;
    BYTE m_Condition = 0; // which condition it is in 0 : Happen, 1 : Complete, 2 : Fail, 3 : Reward
    WORD m_Index = 0;     // which element of that condition it is
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
