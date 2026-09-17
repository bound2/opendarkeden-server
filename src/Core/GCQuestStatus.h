//////////////////////////////////////////////////////////////////////////////
// Filename    : GCQuestStatus.h
// Written By  : elca@ewestsoft.com
// Description :
// Class definition for the packet sent when a skill succeeds
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_QUEST_STATUS_H__
#define __GC_QUEST_STATUS_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GCQuestStatus;
// Class the game server uses to tell the client that its own skill succeeded
//////////////////////////////////////////////////////////////////////////////

class GCQuestStatus : public Packet {
public:
    GCQuestStatus();
    ~GCQuestStatus();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_QUEST_STATUS;
    }
    PacketSize_t getPacketSize() const {
        return szWORD + szWORD + szDWORD;
    }
    string getPacketName() const {
        return "GCQuestStatus";
    }
    string toString() const;

public:
    WORD getQuestID() const {
        return m_QuestID;
    }
    void setQuestID(WORD e) {
        m_QuestID = e;
    }

    WORD getCurrentNum() const {
        return m_CurrentNum;
    }
    void setCurrentNum(WORD n) {
        m_CurrentNum = n;
    }

    DWORD getRemainTime() const {
        return m_Time;
    }
    void setRemainTime(DWORD d) {
        m_Time = d;
    }

private:
    WORD m_QuestID = 0;
    WORD m_CurrentNum = 0;
    DWORD m_Time = 0;
};


//////////////////////////////////////////////////////////////////////////////
// class GCQuestStatusFactory;
//////////////////////////////////////////////////////////////////////////////

class GCQuestStatusFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_QUEST_STATUS;
    static constexpr std::string_view kName = "GCQuestStatus";
    static constexpr PacketSize_t kMaxSize{szWORD + szWORD + szDWORD};

    GCQuestStatusFactory() {}
    virtual ~GCQuestStatusFactory() {}

public:
    Packet* createPacket() override {
        return new GCQuestStatus();
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
