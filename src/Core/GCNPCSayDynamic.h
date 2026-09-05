//////////////////////////////////////////////////////////////////////////////
// Filename    : GCNPCSayDynamic.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_NPC_SAY_DYNAMIC_H__
#define __GC_NPC_SAY_DYNAMIC_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class GCNPCSayDynamic;
// NPC 의 대사를 주변의 PC 들에게 전송한다.
//////////////////////////////////////////////////////////////////////////////

class GCNPCSayDynamic : public Packet {
public:
    GCNPCSayDynamic(){};
    ~GCNPCSayDynamic(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_NPC_SAY_DYNAMIC;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szBYTE + m_Message.size();
    }
    string getPacketName() const {
        return "GCNPCSayDynamic";
    }
    string toString() const;

    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(const ObjectID_t& creatureID) {
        m_ObjectID = creatureID;
    }

    string getMessage() const {
        return m_Message;
    }
    void setMessage(const string& msg) {
        m_Message = msg;
    }

private:
    ObjectID_t m_ObjectID; // NPC's object id
    string m_Message;      // chatting message
};


//////////////////////////////////////////////////////////////////////////////
// class GCNPCSayDynamicFactory;
//////////////////////////////////////////////////////////////////////////////


class GCNPCSayDynamicFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_NPC_SAY_DYNAMIC;
    static constexpr std::string_view kName = "GCNPCSayDynamic";
    static constexpr PacketSize_t kMaxSize{szObjectID + szBYTE + 2048};

    Packet* createPacket() override {
        return new GCNPCSayDynamic();
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
