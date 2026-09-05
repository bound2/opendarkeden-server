//////////////////////////////////////////////////////////////////////////////
// Filename	: GCNicknameList.h
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_NICKNAME_LIST_H__
#define __GC_NICKNAME_LIST_H__

#include <string>
#include <vector>

#include "Exception.h"
#include "NicknameInfo.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GCNicknameList;
//////////////////////////////////////////////////////////////////////////////

#define MAX_NICKNAME_NUM 500

class GCNicknameList : public Packet {
public:
    GCNicknameList();
    ~GCNicknameList();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_NICKNAME_LIST;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCNicknameList";
    }
    string toString() const;

public:
    vector<NicknameInfo*>& getNicknames() {
        return m_Nicknames;
    }

private:
    vector<NicknameInfo*> m_Nicknames;
};

//////////////////////////////////////////////////////////////////////////////
// class GCNicknameListFactory;
//////////////////////////////////////////////////////////////////////////////

class GCNicknameListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_NICKNAME_LIST;
    static constexpr std::string_view kName = "GCNicknameList";
    static constexpr PacketSize_t kMaxSize{szBYTE + NicknameInfo::getMaxSize() * MAX_NICKNAME_NUM};

    GCNicknameListFactory() {}
    virtual ~GCNicknameListFactory() {}

public:
    Packet* createPacket() override {
        return new GCNicknameList();
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
