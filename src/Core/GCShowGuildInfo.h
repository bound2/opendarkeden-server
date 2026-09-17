//////////////////////////////////////////////////////////////////////
//
// Filename    : GCShowGuildInfo.h
// Written By  :
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_SHOW_GUILD_INFO_H__
#define __GC_SHOW_GUILD_INFO_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCShowGuildInfo;
//
// Make the client open the guild registration window.
//
//////////////////////////////////////////////////////////////////////

class GCShowGuildInfo : public Packet {
public:
    GCShowGuildInfo(){};
    ~GCShowGuildInfo(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SHOW_GUILD_INFO;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID +            // Guild ID
               szBYTE +               // Guild Name length
               m_GuildName.size() +   // Guild Name
               szGuildState +         // Guild State
               szBYTE +               // Guild Master length
               m_GuildMaster.size() + // Guild Master
               szBYTE +               // Guild Member Count
               szBYTE +               // Guild Intro length
               m_GuildIntro.size() +  // Guild Intro
               szGold;                // Guild Join Fee
    }

    // get packet name
    string getPacketName() const {
        return "GCShowGuildInfo";
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

    // get/set Guild Name
    const string& getGuildName() const {
        return m_GuildName;
    }
    void setGuildName(const string& GuildName) {
        m_GuildName = GuildName;
    }

    // get/set Guild State
    GuildState_t getGuildState() const {
        return m_GuildState;
    }
    void setGuildState(GuildState_t GuildState) {
        m_GuildState = GuildState;
    }

    // get/set Guild Master
    const string& getGuildMaster() const {
        return m_GuildMaster;
    }
    void setGuildMaster(const string& GuildMaster) {
        m_GuildMaster = GuildMaster;
    }

    // get/set Guild Member Count
    BYTE getGuildMemberCount() const {
        return m_GuildMemberCount;
    }
    void setGuildMemberCount(BYTE GuildMemberCount) {
        m_GuildMemberCount = GuildMemberCount;
    }

    // get/set Guild Intro
    const string& getGuildIntro() const {
        return m_GuildIntro;
    }
    // Truncates to the width the length byte and the factory max allow.
    void setGuildIntro(const string& GuildIntro) {
        m_GuildIntro =
            (GuildIntro.size() > GUILD_INTRO_MAX_LENGTH) ? GuildIntro.substr(0, GUILD_INTRO_MAX_LENGTH) : GuildIntro;
    }

    // get/set Guild Join Fee
    Gold_t getJoinFee() const {
        return m_JoinFee;
    }
    void setJoinFee(Gold_t JoinFee) {
        m_JoinFee = JoinFee;
    }


private:
    // Guild ID
    GuildID_t m_GuildID;

    // Guild Name
    string m_GuildName;

    // Guild State
    GuildState_t m_GuildState;

    // Guild Master
    string m_GuildMaster;

    // Guild Member Count
    BYTE m_GuildMemberCount;

    // Guild Intro
    string m_GuildIntro;

    // Guild Join Fee
    Gold_t m_JoinFee;
};


//////////////////////////////////////////////////////////////////////
//
// class GCShowGuildInfoFactory;
//
// Factory for GCShowGuildInfo
//
//////////////////////////////////////////////////////////////////////

class GCShowGuildInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SHOW_GUILD_INFO;
    static constexpr std::string_view kName = "GCShowGuildInfo";
    static constexpr PacketSize_t kMaxSize{szGuildID +    // Guild ID
                                           szBYTE +       // Guild Name length
                                           30 +           // Guild Name
                                           szGuildState + // Guild State
                                           szBYTE +       // Guild Master length
                                           20 +           // Guild Master
                                           szBYTE +       // Guild Member Count
                                           szBYTE +       // Guild Intro length
                                           256 +          // Guild Intro
                                           szGold};       // Guild Join Fee

    // create packet
    Packet* createPacket() override {
        return new GCShowGuildInfo();
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
// class GCShowGuildInfo;
//
//////////////////////////////////////////////////////////////////////

#endif
