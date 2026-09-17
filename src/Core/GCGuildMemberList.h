//////////////////////////////////////////////////////////////////////
//
// Filename    : GCGuildMemberList.h
// Written By  :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_GUILD_MEMBER_LIST_H__
#define __GC_GUILD_MEMBER_LIST_H__

// include files
#include <list>

#include "GuildMemberInfo.h"
#include "Packet.h"
#include "PacketFactory.h"

typedef list<GuildMemberInfo*> GuildMemberInfoList;
typedef list<GuildMemberInfo*>::const_iterator GuildMemberInfoListConstItor;


//////////////////////////////////////////////////////////////////////
//
// class GCGuildMemberList;
//
// Sends the client the list of guilds waiting to be registered.
//
//////////////////////////////////////////////////////////////////////

class GCGuildMemberList : public Packet {
public:
    // constructor
    GCGuildMemberList();

    // destructor
    ~GCGuildMemberList();

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_GUILD_MEMBER_LIST;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const;

    // get packet name
    string getPacketName() const {
        return "GCGuildMemberList";
    }

    // get packet's debug string
    string toString() const;

public:
    BYTE getListNum() const {
        return m_GuildMemberInfoList.size();
    }

    // add GuildMemberInfoList
    // Takes ownership. Refuses a member past the count the factory max
    // budgets, so getPacketSize() can never outgrow the read buffer the
    // receiver sizes from it; the refused record is destroyed here.
    void addGuildMemberInfo(GuildMemberInfo* pGuildMemberInfo) {
        if (m_GuildMemberInfoList.size() >= GuildMemberInfo::kMaxCount) {
            SAFE_DELETE(pGuildMemberInfo);
            throw InvalidProtocolException("too many guild member infos");
        }
        m_GuildMemberInfoList.push_front(pGuildMemberInfo);
    }

    // clear GuildMemberInfoList
    void clearGuildMemberInfoList();

    // pop front Element in GuildMemberInfoList
    GuildMemberInfo* popFrontGuildMemberInfoList() {
        if (!m_GuildMemberInfoList.empty()) {
            GuildMemberInfo* pGuildMemberInfo = m_GuildMemberInfoList.front();
            m_GuildMemberInfoList.pop_front();
            return pGuildMemberInfo;
        }
        return NULL;
    }

    BYTE getType() const {
        return m_Type;
    }
    void setType(BYTE type) {
        m_Type = type;
    }

private:
    BYTE m_Type;

    GuildMemberInfoList m_GuildMemberInfoList;
};


//////////////////////////////////////////////////////////////////////
//
// class GCGuildMemberListFactory;
//
// Factory for GCGuildMemberList
//
//////////////////////////////////////////////////////////////////////

class GCGuildMemberListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_GUILD_MEMBER_LIST;
    static constexpr std::string_view kName = "GCGuildMemberList";
    static constexpr PacketSize_t kMaxSize{szBYTE + // list type
                                           szBYTE + // member count
                                           GuildMemberInfo::getMaxSize() * GuildMemberInfo::kMaxCount};

    // create packet
    Packet* createPacket() override {
        return new GCGuildMemberList();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's max body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCSystemMessagePacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class GCGuildMemberList;
//
//////////////////////////////////////////////////////////////////////

#endif
