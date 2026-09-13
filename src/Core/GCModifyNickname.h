//////////////////////////////////////////////////////////////////////////////
// Filename    : GCModifyNickname.h
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_MODIFY_NICKNAME_H__
#define __GC_MODIFY_NICKNAME_H__

#include "NicknameInfo.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

class GCModifyNickname : public Packet {
public:
    GCModifyNickname();
    virtual ~GCModifyNickname();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_MODIFY_NICKNAME;
    }
    PacketSize_t getPacketSize() const {
        if (m_pNicknameInfo == NULL)
            throw InvalidProtocolException("nickname record missing");
        return szObjectID + m_pNicknameInfo->getSize();
    }
    string getPacketName() const {
        return "GCModifyNickname";
    }
    string toString() const;

public:
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }

    NicknameInfo* getNicknameInfo() const {
        return m_pNicknameInfo;
    }

    // A sender keeps the record it hands over; only the one read()
    // allocates belongs to the packet.
    void setNicknameInfo(NicknameInfo* pNicknameInfo) {
        clearNicknameInfo();
        m_pNicknameInfo = pNicknameInfo;
    }

private:
    void clearNicknameInfo() {
        if (m_bOwnsNicknameInfo)
            delete m_pNicknameInfo;

        m_pNicknameInfo = NULL;
        m_bOwnsNicknameInfo = false;
    }

    ObjectID_t m_ObjectID = 0;
    NicknameInfo* m_pNicknameInfo = NULL;
    bool m_bOwnsNicknameInfo = false;
};

class GCModifyNicknameFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MODIFY_NICKNAME;
    static constexpr std::string_view kName = "GCModifyNickname";
    static constexpr PacketSize_t kMaxSize{szObjectID + NicknameInfo::getMaxSize()};

    Packet* createPacket() override {
        return new GCModifyNickname();
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

#endif // __GC_MODIFY_NICKNAME_H__
