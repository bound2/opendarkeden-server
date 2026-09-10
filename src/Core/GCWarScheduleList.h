//////////////////////////////////////////////////////////////////////////////
// Filename    : GCWarScheduleList.h
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_WAR_SCHEDULE_LIST_H__
#define __GC_WAR_SCHEDULE_LIST_H__

#include <list>

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"
#include "WireString.h"

#define MAX_WAR_NUM 20

struct WarScheduleInfo {
    BYTE warType = 0; // 0 : 동족간 1 : 종족간
    WORD year = 0;
    BYTE month = 0;
    BYTE day = 0;
    BYTE hour = 0;
    GuildID_t challengerGuildID[5] = {};
    string challengerGuildName[5];
    GuildID_t reinforceGuildID = 0;
    string reinforceGuildName;
};

typedef list<WarScheduleInfo*> WarScheduleInfoList;
typedef WarScheduleInfoList::const_iterator WarScheduleInfoListItor;

class GCWarScheduleList : public Packet {
public:
    // The entries the factory max budgets, and the guild name width it
    // budgets for each of the six name slots.
    static constexpr size_t kMaxEntries = MAX_WAR_NUM;
    static constexpr uint kMaxGuildNameLength = 16;

    GCWarScheduleList();
    virtual ~GCWarScheduleList();

    void clearList();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_WAR_SCHEDULE_LIST;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCWarScheduleList";
    }
    string toString() const;

public:
    // The packet owns the entries it holds.
    void addWarScheduleInfo(WarScheduleInfo* warInfo) {
        if (m_WarScheduleList.size() >= kMaxEntries)
            throw InvalidProtocolException("too many war schedules");
        m_WarScheduleList.push_back(warInfo);
    }
    WarScheduleInfo* popWarScheduleInfo();

private:
    WarScheduleInfoList m_WarScheduleList;
};

class GCWarScheduleListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_WAR_SCHEDULE_LIST;
    static constexpr std::string_view kName = "GCWarScheduleList";
    static constexpr PacketSize_t kMaxSize{(szBYTE + (szBYTE + szWORD + szBYTE + szBYTE + szBYTE + szGuildID * 6 +
                                                      (szBYTE + GCWarScheduleList::kMaxGuildNameLength) * 6) *
                                                         GCWarScheduleList::kMaxEntries)};

    Packet* createPacket() override {
        return new GCWarScheduleList();
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

#endif // __GC_WAR_SCHEDULE_LIST_H__
