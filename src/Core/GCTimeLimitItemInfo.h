//////////////////////////////////////////////////////////////////////
//
// Filename    : GCTimeLimitItemInfo.h
// Written By  :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_TIME_LIMIT_ITEM_INFO_H__
#define __GC_TIME_LIMIT_ITEM_INFO_H__

#include <map>

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
    DWORD getTimeLimit(ObjectID_t objectID) const;
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
    static constexpr PacketSize_t kMaxSize{szBYTE + MAX_TIME_LIMIT_ITEM_INFO * (szObjectID + szDWORD)};

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
