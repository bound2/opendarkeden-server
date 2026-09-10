//////////////////////////////////////////////////////////////////////
//
// Filename    : GCTimeLimitItemInfo.h
// Written By  :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_TIME_LIMIT_ITEM_INFO_H__
#define __GC_TIME_LIMIT_ITEM_INFO_H__

#include <map>
#include <optional>

#include "Packet.h"
#include "PacketFactory.h"

#define MAX_TIME_LIMIT_ITEM_INFO 100

//////////////////////////////////////////////////////////////////////
//
// class GCTimeLimitItemInfo;
//
//
//////////////////////////////////////////////////////////////////////

class GCTimeLimitItemInfo : public Packet {
public:
    typedef map<ObjectID_t, DWORD> ItemTimeLimitMap;

public:
    // The entries the listing carries. The count travels in a BYTE and
    // the factory max budgets this many.
    static constexpr uint kMaxEntryCount = MAX_TIME_LIMIT_ITEM_INFO;

    GCTimeLimitItemInfo();
    ~GCTimeLimitItemInfo();

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;

    PacketID_t getPacketID() const {
        return PACKET_GC_TIME_LIMIT_ITEM_INFO;
    }
    PacketSize_t getPacketSize() const;

    string getPacketName() const {
        return "GCTimeLimitItemInfo";
    }
    string toString() const;

public:
    // Empty for an item the packet does not carry: every value the
    // field holds is a remaining time a real item can have, so no
    // sentinel can say "absent".
    std::optional<DWORD> getTimeLimit(ObjectID_t objectID) const;
    bool hasTimeLimit(ObjectID_t objectID) const;
    void addTimeLimit(ObjectID_t objectID, DWORD time);


private:
    ItemTimeLimitMap m_TimeLimitItemInfos;
};


//////////////////////////////////////////////////////////////////////
//
// class GCTimeLimitItemInfoFactory;
//
// Factory for GCTimeLimitItemInfo
//
//////////////////////////////////////////////////////////////////////

class GCTimeLimitItemInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_TIME_LIMIT_ITEM_INFO;
    static constexpr std::string_view kName = "GCTimeLimitItemInfo";
    static constexpr PacketSize_t kMaxSize{szBYTE + GCTimeLimitItemInfo::kMaxEntryCount * (szObjectID + szDWORD)};

    Packet* createPacket() override {
        return new GCTimeLimitItemInfo();
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


//////////////////////////////////////////////////////////////////////
//
// class GCTimeLimitItemInfo;
//
//////////////////////////////////////////////////////////////////////

#endif
