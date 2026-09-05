//////////////////////////////////////////////////////////////////////////////
// Filename    : GCAddNickname.h
// Written By  : elca@ewestsoft.com
// Description :
// ����� ���������� ������ ��Ŷ�� ���� Ŭ���� ����
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_ADD_NICKNAME_H__
#define __GC_ADD_NICKNAME_H__

#include "Exception.h"
#include "NicknameInfo.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GCAddNickname;
// ���Ӽ������� Ŭ���̾�Ʈ�� �ڽ��� ����� ������ �˷��ֱ� ���� Ŭ����
//////////////////////////////////////////////////////////////////////////////

class GCAddNickname : public Packet {
public:
    GCAddNickname();
    ~GCAddNickname() noexcept;

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_ADD_NICKNAME;
    }
    PacketSize_t getPacketSize() const {
        return m_NicknameInfo.getSize();
    }
    string getPacketName() const {
        return "GCAddNickname";
    }
    string toString() const;

public:
    NicknameInfo& getNicknameInfo() {
        return m_NicknameInfo;
    }

private:
    NicknameInfo m_NicknameInfo;
};


//////////////////////////////////////////////////////////////////////////////
// class GCAddNicknameFactory;
//////////////////////////////////////////////////////////////////////////////

class GCAddNicknameFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_ADD_NICKNAME;
    static constexpr std::string_view kName = "GCAddNickname";
    static constexpr PacketSize_t kMaxSize{NicknameInfo::getMaxSize()};

    GCAddNicknameFactory() {}
    virtual ~GCAddNicknameFactory() {}

public:
    Packet* createPacket() override {
        return new GCAddNickname();
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
