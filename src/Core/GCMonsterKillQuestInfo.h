//////////////////////////////////////////////////////////////////////////////
// Filename    : GCMonsterKillQuestInfo.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_MONSTER_KILL_QUEST_INFO_H__
#define __GC_MONSTER_KILL_QUEST_INFO_H__

#include <list>

#include "GCSelectQuestID.h"
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class GCMonsterKillQuestInfo;
//////////////////////////////////////////////////////////////////////////////

class GCMonsterKillQuestInfo : public Packet {
public:
    struct QuestInfo {
        QuestID_t questID;
        SpriteType_t sType;
        WORD goal;
        DWORD timeLimit;
    };

    static constexpr int szQuestInfo = szQuestID + szSpriteType + szWORD + szDWORD;

    GCMonsterKillQuestInfo() {}
    virtual ~GCMonsterKillQuestInfo();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_MONSTER_KILL_QUEST_INFO;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCMonsterKillQuestInfo";
    }
    string toString() const;

public:
    bool empty() const {
        return m_QuestInfoList.empty();
    }
    QuestInfo* popQuestInfo() {
        QuestInfo* pQI = m_QuestInfoList.front();
        m_QuestInfoList.pop_front();
        return pQI;
    }
    void addQuestInfo(QuestInfo* pQI) {
        m_QuestInfoList.push_back(pQI);
    }

private:
    list<QuestInfo*> m_QuestInfoList;
};

//////////////////////////////////////////////////////////////////////////////
// class GCMonsterKillQuestInfoFactory;
//////////////////////////////////////////////////////////////////////////////

class GCMonsterKillQuestInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MONSTER_KILL_QUEST_INFO;
    static constexpr std::string_view kName = "GCMonsterKillQuestInfo";
    static constexpr PacketSize_t kMaxSize{szBYTE + GCMonsterKillQuestInfo::szQuestInfo * maxQuestNum};

    Packet* createPacket() override {
        return new GCMonsterKillQuestInfo();
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
