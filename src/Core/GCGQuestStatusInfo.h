//////////////////////////////////////////////////////////////////////////////
// Filename    : GCGQuestStatusInfo.h
// Written By  : elca@ewestsoft.com
// Description :
// Class definition for the packet sent when a skill succeeds
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_GGQUEST_STATUS_INFO_H__
#define __GC_GGQUEST_STATUS_INFO_H__

#include <functional>
#include <numeric>

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "QuestStatusInfo.h"
#include "Types.h"

#define MAX_QUEST_NUM 100

//////////////////////////////////////////////////////////////////////////////
// class GCGQuestStatusInfo;
// Class the game server uses to tell the client that its own skill succeeded
//////////////////////////////////////////////////////////////////////////////

class GCGQuestStatusInfo : public Packet {
public:
    GCGQuestStatusInfo();
    ~GCGQuestStatusInfo();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return Packet::PACKET_GC_GQUEST_STATUS_INFO;
    }
    PacketSize_t getPacketSize() const {
        return accumulate(m_Infos.begin(), m_Infos.end(), szBYTE, addSize);
    }
    string getPacketName() const {
        return "GCGQuestStatusInfo";
    }
    string toString() const;

public:
    static PacketSize_t addSize(PacketSize_t tot, const QuestStatusInfo* pInfo) {
        return tot + pInfo->getSize();
    }

    // A sender keeps the records it fills the listing with; only the
    // ones read() allocates belong to the packet.
    list<QuestStatusInfo*>& getInfos() {
        return m_Infos;
    }
    const list<QuestStatusInfo*>& getInfos() const {
        return m_Infos;
    }

private:
    void clearInfos();

    list<QuestStatusInfo*> m_Infos;
    bool m_bOwnsInfos = false;
};


//////////////////////////////////////////////////////////////////////////////
// class GCGQuestStatusInfoFactory;
//////////////////////////////////////////////////////////////////////////////

class GCGQuestStatusInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_GQUEST_STATUS_INFO;
    static constexpr std::string_view kName = "GCGQuestStatusInfo";
    static constexpr PacketSize_t kMaxSize{szBYTE + QuestStatusInfo::getMaxSize() * MAX_QUEST_NUM};

    GCGQuestStatusInfoFactory() {}
    virtual ~GCGQuestStatusInfoFactory() {}

public:
    Packet* createPacket() override {
        return new GCGQuestStatusInfo();
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
