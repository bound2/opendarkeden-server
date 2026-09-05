//////////////////////////////////////////////////////////////////////////////
// Filename    : GCOtherModifyInfo.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_OTHER_MODIFY_INFO_H__
#define __GC_OTHER_MODIFY_INFO_H__

#include "ModifyInfo.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class GCOtherModifyInfo;
//////////////////////////////////////////////////////////////////////////////

class GCOtherModifyInfo : public ModifyInfo {
public:
    GCOtherModifyInfo(){};
    ~GCOtherModifyInfo(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_OTHER_MODIFY_INFO;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + ModifyInfo::getPacketSize();
    }
    string getPacketName() const {
        return "GCOtherModifyInfo";
    }
    string toString() const;

public:
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

private:
    ObjectID_t m_ObjectID;
};


//////////////////////////////////////////////////////////////////////////////
// class GCOtherModifyInfoFactory;
//////////////////////////////////////////////////////////////////////////////

class GCOtherModifyInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_OTHER_MODIFY_INFO;
    static constexpr std::string_view kName = "GCOtherModifyInfo";
    static constexpr PacketSize_t kMaxSize{szObjectID + ModifyInfo::getPacketMaxSize()};

    Packet* createPacket() override {
        return new GCOtherModifyInfo();
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
