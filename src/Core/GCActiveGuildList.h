//////////////////////////////////////////////////////////////////////
//
// Filename    : GCActiveGuildList.h
// Written By  :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_ACTIVE_GUILD_LIST_H__
#define __GC_ACTIVE_GUILD_LIST_H__

// include files
#include <list>

#include "GuildInfo.h"
#include "Packet.h"
#include "PacketFactory.h"

typedef list<GuildInfo*> GuildInfoList;
typedef list<GuildInfo*>::const_iterator GuildInfoListConstItor;


//////////////////////////////////////////////////////////////////////
//
// class GCActiveGuildList;
//
// Sends the client the list of guilds waiting to be registered.
//
//////////////////////////////////////////////////////////////////////

class GCActiveGuildList : public Packet {
public:
    // constructor
    GCActiveGuildList();

    // destructor
    ~GCActiveGuildList();

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_ACTIVE_GUILD_LIST;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const;

    // get packet name
    string getPacketName() const {
        return "GCActiveGuildList";
    }

    // get packet's debug string
    string toString() const;

public:
    // The count goes on the wire as a WORD.
    WORD getListNum() const {
        return m_GuildInfoList.size();
    }

    // add GuildInfo
    // Takes ownership. Refuses a guild past the count the factory max
    // budgets, so getPacketSize() can never outgrow the read buffer the
    // receiver sizes from it; the refused record is destroyed here.
    void addGuildInfo(GuildInfo* pGuildInfo) {
        if (m_GuildInfoList.size() >= GuildInfo::kMaxCount) {
            SAFE_DELETE(pGuildInfo);
            throw InvalidProtocolException("too many guild infos");
        }
        m_GuildInfoList.push_front(pGuildInfo);
    }

    // clear GuildInfoList
    void clearGuildInfoList();

    // pop front Element in GuildInfoList
    GuildInfo* popFrontGuildInfoList() {
        if (!m_GuildInfoList.empty()) {
            GuildInfo* pGuildInfo = m_GuildInfoList.front();
            m_GuildInfoList.pop_front();
            return pGuildInfo;
        }
        return NULL;
    }


private:
    GuildInfoList m_GuildInfoList;
};


//////////////////////////////////////////////////////////////////////
//
// class GCActiveGuildListFactory;
//
// Factory for GCActiveGuildList
//
//////////////////////////////////////////////////////////////////////

class GCActiveGuildListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ACTIVE_GUILD_LIST;
    static constexpr std::string_view kName = "GCActiveGuildList";
    static constexpr PacketSize_t kMaxSize{szWORD + (GuildInfo::getMaxSize() * GuildInfo::kMaxCount)};

    // create packet
    Packet* createPacket() override {
        return new GCActiveGuildList();
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
// class GCActiveGuildList;
//
//////////////////////////////////////////////////////////////////////

#endif
