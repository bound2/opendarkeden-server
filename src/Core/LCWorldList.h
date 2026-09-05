//////////////////////////////////////////////////////////////////////
//
// Filename    : LCWorldList.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_WORLD_LIST_H__
#define __LC_WORLD_LIST_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WorldInfo.h"

//////////////////////////////////////////////////////////////////////
//
// class LCWorldList;
//
//////////////////////////////////////////////////////////////////////

class LCWorldList : public Packet {
public:
    // constructor
    // PCInfo* �迭�� ���� NULL�� �����Ѵ�.
    LCWorldList();

    // destructor
    // PCInfo* �迭�� �Ҵ�� ��ü�� �����Ѵ�.
    ~LCWorldList() noexcept;

    // �Է½�Ʈ��(����)���κ��� ����Ÿ�� �о ��Ŷ��
    // �ʱ�ȭ�Ѵ�.
    void read(SocketInputStream& iStream);

    // ��½�Ʈ��(����)���� ��Ŷ�� ���̳ʸ� �̹����� ������.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_WORLD_LIST;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const;

    // get packet's name
    string getPacketName() const {
        return "LCWorldList";
    }

    // get packet's debug string
    string toString() const;

public:
    // ���� ����
    WorldID_t getCurrentWorldID() const {
        return m_CurrentWorldID;
    }
    void setCurrentWorldID(WorldID_t WorldID) {
        m_CurrentWorldID = WorldID;
    }

    BYTE getListNum() const {
        return m_WorldInfoList.size();
    }

    // add / delete / clear S List
    void addListElement(WorldInfo* pWorldInfo) {
        m_WorldInfoList.push_back(pWorldInfo);
    }

    // ClearList
    void clearList() {
        m_WorldInfoList.clear();
    }

    // pop front Element in Status List
    WorldInfo* popFrontListElement() {
        WorldInfo* TempWorldInfo = m_WorldInfoList.front();
        m_WorldInfoList.pop_front();
        return TempWorldInfo;
    }

private:
    // ���� WorldID
    WorldID_t m_CurrentWorldID;

    // ĳ���� ����
    list<WorldInfo*> m_WorldInfoList;
};

//////////////////////////////////////////////////////////////////////
//
// class LCWorldListFactory;
//
// Factory for LCWorldList
//
//////////////////////////////////////////////////////////////////////

class LCWorldListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_WORLD_LIST;
    static constexpr std::string_view kName = "LCWorldList";
    // write() emits a BYTE ListNum between the world id and the infos
    static constexpr PacketSize_t kMaxSize{szWorldID + szBYTE + WorldInfo::getMaxSize()};

    // create packet
    Packet* createPacket() override {
        return new LCWorldList();
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
