//////////////////////////////////////////////////////////////////////////////
// Filename    : GCFlagWarStatus.h
// Written By  : elca@ewestsoft.com
// Description :
// Class definition for the packet sent when a skill succeeds
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_FLAG_WAR_STATUS_H__
#define __GC_FLAG_WAR_STATUS_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GCFlagWarStatus;
// Class the game server uses to tell the client that its own skill succeeded
//////////////////////////////////////////////////////////////////////////////

class GCFlagWarStatus : public Packet {
public:
    // The flag counts write() emits, one per race.
    static constexpr uint kRaceCount = 3;

    GCFlagWarStatus();
    ~GCFlagWarStatus();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_FLAG_WAR_STATUS;
    }
    PacketSize_t getPacketSize() const {
        return szWORD + szBYTE * kRaceCount;
    }
    string getPacketName() const {
        return "GCFlagWarStatus";
    }
    string toString() const;

public:
    WORD getTimeRemain() const {
        return m_TimeRemain;
    }
    void setTimeRemain(WORD remainTime) {
        m_TimeRemain = remainTime;
    }

    BYTE getFlagCount(Race_t race) const {
        if ((uint)race >= kRaceCount)
            throw InvalidProtocolException("flag war race out of range");
        return m_FlagCount[(int)race];
    }
    void setFlagCount(Race_t race, BYTE count) {
        if ((uint)race >= kRaceCount)
            throw InvalidProtocolException("flag war race out of range");
        m_FlagCount[(int)race] = count;
    }

private:
    WORD m_TimeRemain = 0;
    BYTE m_FlagCount[kRaceCount] = {};
};


//////////////////////////////////////////////////////////////////////////////
// class GCFlagWarStatusFactory;
//////////////////////////////////////////////////////////////////////////////

class GCFlagWarStatusFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_FLAG_WAR_STATUS;
    static constexpr std::string_view kName = "GCFlagWarStatus";
    static constexpr PacketSize_t kMaxSize{szWORD + szBYTE * GCFlagWarStatus::kRaceCount};

    GCFlagWarStatusFactory() {}
    virtual ~GCFlagWarStatusFactory() {}

public:
    Packet* createPacket() override {
        return new GCFlagWarStatus();
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
