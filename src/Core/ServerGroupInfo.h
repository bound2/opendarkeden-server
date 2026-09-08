//////////////////////////////////////////////////////////////////////
//
// Filename    : ServerGroupInfo.h
// Written By  : elca@ewestsoft.com
// Description : �κ��丮 �������� ������
//
//////////////////////////////////////////////////////////////////////

#ifndef __SERVER_GROUP_INFO_H__
#define __SERVER_GROUP_INFO_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class ServerGroupInfo;
//
// ���Ӽ������� Ŭ���̾�Ʈ�� �ڽ��� ����� ������ �˷��ֱ� ���� Ŭ����
//
//////////////////////////////////////////////////////////////////////

class ServerGroupInfo {
public:
    // constructor
    ServerGroupInfo();

    // destructor
    ~ServerGroupInfo() noexcept;

public:
    // �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ��
    // �ʱ�ȭ�Ѵ�.
    void read(SocketInputStream& iStream);

    // ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
    void write(SocketOutputStream& oStream) const;

    // get packet's body size
    // ����ȭ��, �̸� ���� ������ ����Ѵ�.
    PacketSize_t getSize();

    // The list packet's factory max budgets this many entries of a
    // full-width name; LCServerList refuses one more.
    static constexpr uint kMaxCount = 37;

    static constexpr uint getMaxSize() {
        return (szServerGroupID + szBYTE + maxNameLength + szBYTE) * kMaxCount;
    }

    // get packet's debug string
    string toString() const;

    // get / set GroupID
    BYTE getGroupID() const {
        return m_GroupID;
    }
    void setGroupID(ServerGroupID_t GroupID) {
        m_GroupID = GroupID;
    }

    // get / set GroupName
    string getGroupName() const {
        return m_GroupName;
    }
    // Truncates to the width the length prefix and the factory max allow.
    void setGroupName(string GroupName) {
        m_GroupName = (GroupName.size() > maxNameLength) ? GroupName.substr(0, maxNameLength) : GroupName;
    }

    // get / set Group Stat
    BYTE getStat() const {
        return m_Stat;
    }
    void setStat(BYTE Stat) {
        m_Stat = Stat;
    }

private:
    // �׷� ���̵�
    ServerGroupID_t m_GroupID;

    // �׷� �̸�
    string m_GroupName;

    // �׷��� ����
    BYTE m_Stat;
};

#endif
