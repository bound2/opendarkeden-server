//////////////////////////////////////////////////////////////////////////////
// Filename    : GCPartyJoined.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_PARTY_JOINED_H__
#define __GC_PARTY_JOINED_H__

#include <string>

#include "Packet.h"
#include "PacketFactory.h"

typedef struct {
    string name;     // 10
    BYTE sex;        // 10 + 1 = 11
    BYTE hair_style; // 11 + 1 = 12
    IP_t ip;         // 12 + 4 = 16

} PARTY_MEMBER_INFO;

// 구조체 맥스 크기(14) + 이름 길이 (1)
const uint PARTY_MEMBER_INFO_MAX_SIZE = 17;

// The name width PARTY_MEMBER_INFO_MAX_SIZE budgets, and the number of
// members the packet's factory max budgets.
const uint PARTY_MEMBER_NAME_MAX_LENGTH = 10;
const uint PARTY_MEMBER_INFO_MAX_COUNT = 6;

//////////////////////////////////////////////////////////////////////////////
// class GCPartyJoined;
//////////////////////////////////////////////////////////////////////////////

class GCPartyJoined : public Packet {
public:
    GCPartyJoined();
    ~GCPartyJoined();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_PARTY_JOINED;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCPartyJoined";
    }
    string toString() const;

public:
    BYTE getMemberInfoCount(void) {
        return m_MemberCount;
    }

    // Takes ownership. Refuses a member past the count the factory max
    // budgets; the refused record is destroyed here.
    void addMemberInfo(PARTY_MEMBER_INFO* pInfo);
    PARTY_MEMBER_INFO* popMemberInfo(void);

    void clear(void);

private:
    BYTE m_MemberCount;
    list<PARTY_MEMBER_INFO*> m_MemberInfoList;
};


//////////////////////////////////////////////////////////////////////////////
// class GCPartyJoinedFactory;
//////////////////////////////////////////////////////////////////////////////

class GCPartyJoinedFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_PARTY_JOINED;
    static constexpr std::string_view kName = "GCPartyJoined";
    static constexpr PacketSize_t kMaxSize{szBYTE + PARTY_MEMBER_INFO_MAX_SIZE * PARTY_MEMBER_INFO_MAX_COUNT};

    Packet* createPacket() override {
        return new GCPartyJoined();
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
