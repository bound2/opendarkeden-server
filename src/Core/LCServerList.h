//////////////////////////////////////////////////////////////////////
//
// Filename    : LCServerList.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_SERVER_LIST_H__
#define __LC_SERVER_LIST_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "ServerGroupInfo.h"

//////////////////////////////////////////////////////////////////////
//
// class LCServerList;
//
//////////////////////////////////////////////////////////////////////

class LCServerList : public Packet {
public:
    // constructor
    // PCInfo* �迭�� ���� NULL�� �����Ѵ�.
    LCServerList();

    // destructor
    // PCInfo* �迭�� �Ҵ�� ��ü�� �����Ѵ�.
    ~LCServerList() noexcept;

    // �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ��
    // �ʱ�ȭ�Ѵ�.
    void read(SocketInputStream& iStream);

    // ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_SERVER_LIST;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const;

    // get packet's name
    string getPacketName() const {
        return "LCServerList";
    }

    // get packet's debug string
    string toString() const;

public:
    // ���� ���� �׷�
    ServerGroupID_t getCurrentServerGroupID() const {
        return m_CurrentServerGroupID;
    }
    void setCurrentServerGroupID(ServerGroupID_t ServerGroupID) {
        m_CurrentServerGroupID = ServerGroupID;
    }

    BYTE getListNum() const {
        return m_ServerGroupInfoList.size();
    }

    // add / delete / clear S List
    void addListElement(ServerGroupInfo* pServerGroupInfo) {
        m_ServerGroupInfoList.push_back(pServerGroupInfo);
    }

    // ClearList
    void clearList() {
        m_ServerGroupInfoList.clear();
    }

    // pop front Element in Status List
    ServerGroupInfo* popFrontListElement() {
        ServerGroupInfo* TempServerGroupInfo = m_ServerGroupInfoList.front();
        m_ServerGroupInfoList.pop_front();
        return TempServerGroupInfo;
    }

private:
    // ���� ���� �׷�
    ServerGroupID_t m_CurrentServerGroupID;

    // ĳ���� ����
    list<ServerGroupInfo*> m_ServerGroupInfoList;
};

//////////////////////////////////////////////////////////////////////
//
// class LCServerListFactory;
//
// Factory for LCServerList
//
//////////////////////////////////////////////////////////////////////

class LCServerListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_SERVER_LIST;
    static constexpr std::string_view kName = "LCServerList";
    // write() emits a BYTE ListNum between the group id and the infos
    static constexpr PacketSize_t kMaxSize{szServerGroupID + szBYTE + ServerGroupInfo::getMaxSize()};

    // create packet
    Packet* createPacket() override {
        return new LCServerList();
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
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
