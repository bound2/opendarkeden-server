//--------------------------------------------------------------------------------
//
// Filename    : WarInfo.h
// Written By  :
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __GUILD_WAR_LIST_H__
#define __GUILD_WAR_LIST_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WarInfo.h"
#include "WireString.h"

//--------------------------------------------------------------------------------
//
// class WarInfo;
//
// Information about a single war
//
//--------------------------------------------------------------------------------

class GuildWarInfo : public WarInfo {
public:
    typedef ValueList<GuildID_t> GuildIDList;

public:
    GuildWarInfo() {}
    ~GuildWarInfo() {}

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;

    PacketSize_t getSize() const {
        return WarInfo::getSize() + szZoneID + de::wire::stringWireSize(m_AttackGuildName) +
               de::wire::stringWireSize(m_DefenseGuildName) + m_GuildIDs.getPacketSize();
    }

    static constexpr PacketSize_t getMaxSize() {
        return WarInfo::getMaxSize() + szZoneID + szBYTE + 40 + szBYTE + 30 + GuildIDList::getPacketMaxSize();
    }

    // get packet's debug string
    string toString() const;

    void operator=(const GuildWarInfo& GWI) {
        m_StartTime = GWI.m_StartTime;
        m_RemainTime = GWI.m_RemainTime;
        m_CastleID = GWI.m_CastleID;
        m_DefenseGuildName = GWI.m_DefenseGuildName;
        m_AttackGuildName = GWI.m_AttackGuildName;
        m_GuildIDs = GWI.m_GuildIDs;
    }

public:
    WarType_t getWarType() const {
        return WAR_GUILD;
    }

    ZoneID_t getCastleID() const {
        return m_CastleID;
    }
    void setCastleID(ZoneID_t zid) {
        m_CastleID = zid;
    }

    const string& getAttackGuildName() const {
        return m_AttackGuildName;
    }
    void setAttackGuildName(const string& guildName) {
        m_AttackGuildName = guildName;
    }

    const string& getDefenseGuildName() const {
        return m_DefenseGuildName;
    }
    void setDefenseGuildName(const string& guildName) {
        m_DefenseGuildName = guildName;
    }

    GuildIDList& getJoinGuilds() {
        return m_GuildIDs;
    }
    void addJoinGuild(GuildID_t gid) {
        m_GuildIDs.addValue(gid);
    }

private:
    ZoneID_t m_CastleID = 0;   // Castle at war
    string m_DefenseGuildName; // Defending guild
    string m_AttackGuildName;  // Attacking guild
    GuildIDList m_GuildIDs;    // Participating guilds
};

#endif
