//////////////////////////////////////////////////////////////////////
//
// Filename    : GCShowGuildMemberInfo.h
// Written By  :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_SHOW_GUILD_MEMBER_INFO_H__
#define __GC_SHOW_GUILD_MEMBER_INFO_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCShowGuildMemberInfo;
//
// Make the client open the guild registration window.
//
//////////////////////////////////////////////////////////////////////

class GCShowGuildMemberInfo : public Packet {
public:
    GCShowGuildMemberInfo(){};
    ~GCShowGuildMemberInfo(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SHOW_GUILD_MEMBER_INFO;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID +                // Guild ID
               szBYTE +                   // Name length
               m_Name.size() +            // Name
               szGuildMemberRank +        // Guild Member Rank
               szBYTE +                   // Guild Member Intro length
               m_GuildMemberIntro.size(); // Guild Member Intro
    }

    // get packet name
    string getPacketName() const {
        return "GCShowGuildMemberInfo";
    }

    // get packet's debug string
    string toString() const;

    // get/set Guild ID
    GuildID_t getGuildID() const {
        return m_GuildID;
    }
    void setGuildID(GuildID_t GuildID) {
        m_GuildID = GuildID;
    }

    // get/set Name
    const string& getName() const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
    }

    // get/set Guild Member Rank
    GuildMemberRank_t getGuildMemberRank() const {
        return m_GuildMemberRank;
    }
    void setGuildMemberRank(GuildMemberRank_t guildMemberRank) {
        m_GuildMemberRank = guildMemberRank;
    }

    // get/set Guild Member Intro
    const string& getGuildMemberIntro() const {
        return m_GuildMemberIntro;
    }
    // Truncates to the width the length byte and the factory max allow.
    void setGuildMemberIntro(const string& intro) {
        m_GuildMemberIntro = (intro.size() > GUILD_INTRO_MAX_LENGTH) ? intro.substr(0, GUILD_INTRO_MAX_LENGTH) : intro;
    }

private:
    // Guild ID
    GuildID_t m_GuildID;

    // Name
    string m_Name;

    // Guild Member Rank
    GuildMemberRank_t m_GuildMemberRank;

    // Guild Member Intro
    string m_GuildMemberIntro;
};


//////////////////////////////////////////////////////////////////////
//
// class GCShowGuildMemberInfoFactory;
//
// Factory for GCShowGuildMemberInfo
//
//////////////////////////////////////////////////////////////////////

class GCShowGuildMemberInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SHOW_GUILD_MEMBER_INFO;
    static constexpr std::string_view kName = "GCShowGuildMemberInfo";
    static constexpr PacketSize_t kMaxSize{szGuildID +         // Guild ID
                                           szBYTE +            // Name length
                                           20 +                // Name max size
                                           szGuildMemberRank + // Guild Member Rank
                                           szBYTE +            // Guild Member Intro length
                                           256};               // Guild Member Intro max size

    // create packet
    Packet* createPacket() override {
        return new GCShowGuildMemberInfo();
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
// class GCShowGuildMemberInfo;
//
//////////////////////////////////////////////////////////////////////

#endif
