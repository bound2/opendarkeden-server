//////////////////////////////////////////////////////////////////////////////
// Filename    : CGSilverCoating.h
// Written By  : 김성민
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_SILVER_COATING_H__
#define __CG_SILVER_COATING_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGSilverCoating
//////////////////////////////////////////////////////////////////////////////

class CGSilverCoating : public Packet {
public:
    CGSilverCoating(){};
    virtual ~CGSilverCoating(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;

    PacketID_t getPacketID() const {
        return PACKET_CG_SILVER_COATING;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }
    string getPacketName() const {
        return "CGSilverCoating";
    }
    string toString() const;

public:
    ObjectID_t getObjectID() {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

private:
    ObjectID_t m_ObjectID; // Item Object ID
};


//////////////////////////////////////////////////////////////////////////////
// class CGSilverCoatingFactory;
//////////////////////////////////////////////////////////////////////////////

class CGSilverCoatingFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_SILVER_COATING;
    static constexpr std::string_view kName = "CGSilverCoating";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    Packet* createPacket() override {
        return new CGSilverCoating();
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
// class CGSilverCoatingHandler;
//////////////////////////////////////////////////////////////////////////////

class CGSilverCoatingHandler {
public:
    static void execute(CGSilverCoating* pPacket, Player* player);
};

#endif
