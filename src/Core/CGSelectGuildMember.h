//////////////////////////////////////////////////////////////////////
//
// Filename    : CGSelectGuildMember.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_SELECT_GUILD_MEMBER_H__
#define __CG_SELECT_GUILD_MEMBER_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class CGSelectGuildMember;
//
//////////////////////////////////////////////////////////////////////

class CGSelectGuildMember : public Packet {
public:
    CGSelectGuildMember(){};
    virtual ~CGSelectGuildMember(){};
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_SELECT_GUILD_MEMBER;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID + szBYTE + m_Name.size();
    }

    // get packet name
    string getPacketName() const {
        return "CGSelectGuildMember";
    }

    // get packet's debug string
    string toString() const;

    // get/set GuildID
    GuildID_t getGuildID() const {
        return m_GuildID;
    }
    void setGuildID(GuildID_t GuildID) {
        m_GuildID = GuildID;
    }

    // get/set name
    const string& getName() const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
    }


private:
    // Guild ID
    GuildID_t m_GuildID;

    // name
    string m_Name;
};


//////////////////////////////////////////////////////////////////////
//
// class CGSelectGuildMemberFactory;
//
// Factory for CGSelectGuildMember
//
//////////////////////////////////////////////////////////////////////

class CGSelectGuildMemberFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_SELECT_GUILD_MEMBER;
    static constexpr std::string_view kName = "CGSelectGuildMember";
    static constexpr PacketSize_t kMaxSize{szGuildID + szBYTE + 20};

    // constructor
    CGSelectGuildMemberFactory() {}

    // destructor
    virtual ~CGSelectGuildMemberFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGSelectGuildMember();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get Packet Max Size
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGSelectGuildMemberHandler;
//
//////////////////////////////////////////////////////////////////////

class CGSelectGuildMemberHandler {
public:
    // execute packet's handler
    static void execute(CGSelectGuildMember* pCGSelectGuildMember, Player* pPlayer);
};

#endif
