//////////////////////////////////////////////////////////////////////////////
// Filename    : CGModifyNickname.h
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_MODIFY_NICKNAME_H__
#define __CG_MODIFY_NICKNAME_H__

#include "NicknameInfo.h"
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGModifyNickname;
//////////////////////////////////////////////////////////////////////////////

class CGModifyNickname : public Packet {
public:
    CGModifyNickname();
    ~CGModifyNickname();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_MODIFY_NICKNAME;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szBYTE + m_Nickname.size();
    }
    string getPacketName() const {
        return "CGModifyNickname";
    }
    string toString() const;

public:
    ObjectID_t getItemObjectID() const {
        return m_ItemObjectID;
    }
    void setItemObjectID(WORD id) {
        m_ItemObjectID = id;
    }

    string getNickname() const {
        return m_Nickname;
    }
    void setNickname(const string& name) {
        m_Nickname = name;
    }

private:
    ObjectID_t m_ItemObjectID;
    string m_Nickname;
};

//////////////////////////////////////////////////////////////////////////////
// class CGModifyNicknameFactory;
//////////////////////////////////////////////////////////////////////////////

class CGModifyNicknameFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_MODIFY_NICKNAME;
    static constexpr std::string_view kName = "CGModifyNickname";
    static constexpr PacketSize_t kMaxSize{szObjectID + szBYTE + MAX_NICKNAME_SIZE};

    Packet* createPacket() override {
        return new CGModifyNickname();
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

//////////////////////////////////////////////////////////////////////////////
// class CGModifyNicknameHandler;
//////////////////////////////////////////////////////////////////////////////

class CGModifyNicknameHandler {
public:
    static void execute(CGModifyNickname* pPacket, Player* player);
};

#endif
