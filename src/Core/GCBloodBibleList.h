//////////////////////////////////////////////////////////////////////////////
// Filename    : GCBloodBibleList.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_BLOOD_BIBLE_LIST_H__
#define __GC_BLOOD_BIBLE_LIST_H__

#include <vector>

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class GCBloodBibleList;
//////////////////////////////////////////////////////////////////////////////

class GCBloodBibleList : public Packet {
public:
    GCBloodBibleList() {}
    virtual ~GCBloodBibleList();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_BLOOD_BIBLE_LIST;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCBloodBibleList";
    }
    string toString() const;

public:
    vector<ItemType_t>& getList() {
        return m_BloodBibleList;
    }
    const vector<ItemType_t>& getList() const {
        return m_BloodBibleList;
    }

private:
    vector<ItemType_t> m_BloodBibleList;
};

//////////////////////////////////////////////////////////////////////////////
// class GCBloodBibleListFactory;
//////////////////////////////////////////////////////////////////////////////

class GCBloodBibleListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_BLOOD_BIBLE_LIST;
    static constexpr std::string_view kName = "GCBloodBibleList";
    static constexpr PacketSize_t kMaxSize{szBYTE + szItemType * 12};

    Packet* createPacket() override {
        return new GCBloodBibleList();
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
