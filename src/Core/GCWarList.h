//////////////////////////////////////////////////////////////////////////////
// Filename    : GCWarList.h
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_WAR_LIST_H__
#define __GC_WAR_LIST_H__

#include <list>

#include "Exception.h"
#include "GuildWarInfo.h"
#include "LevelWarInfo.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "RaceWarInfo.h"
#include "Types.h"

typedef list<WarInfo*> WarInfoList;
typedef WarInfoList::const_iterator WarInfoListItor;

class GCWarList : public Packet {
public:
    // The records the factory max budgets, and the widest of the three
    // shapes write() can put behind a war type byte.
    static constexpr size_t kMaxWars = 24;
    static constexpr PacketSize_t kWidestWarSize = GuildWarInfo::getMaxSize() > RaceWarInfo::getMaxSize()
                                                       ? (GuildWarInfo::getMaxSize() > LevelWarInfo::getMaxSize()
                                                              ? GuildWarInfo::getMaxSize()
                                                              : LevelWarInfo::getMaxSize())
                                                       : (RaceWarInfo::getMaxSize() > LevelWarInfo::getMaxSize()
                                                              ? RaceWarInfo::getMaxSize()
                                                              : LevelWarInfo::getMaxSize());

    GCWarList();
    virtual ~GCWarList();

    void clear();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_WAR_LIST;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCWarList";
    }
    string toString() const;

public:
    int getSize() const {
        return m_WarInfos.size();
    }
    bool isEmpty() const {
        return m_WarInfos.empty();
    }

    // The packet owns the records it holds.
    void addWarInfo(WarInfo* pWarInfo) {
        if (m_WarInfos.size() >= kMaxWars)
            throw InvalidProtocolException("too many wars");
        m_WarInfos.push_back(pWarInfo);
    }
    WarInfo* popWarInfo();

    void operator=(const GCWarList& WL);

private:
    WarInfoList m_WarInfos;
};

class GCWarListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_WAR_LIST;
    static constexpr std::string_view kName = "GCWarList";
    static constexpr PacketSize_t kMaxSize{szBYTE + (szWarType + GCWarList::kWidestWarSize) * GCWarList::kMaxWars};

    Packet* createPacket() override {
        return new GCWarList();
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


#endif // __GC_WAR_LIST_H__
