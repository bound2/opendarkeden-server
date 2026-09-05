//////////////////////////////////////////////////////////////////////////////
// Filename    : CGTakeOutGood.h
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_TAKE_OUT_GOOD_H__
#define __CG_TAKE_OUT_GOOD_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGTakeOutGood;
//////////////////////////////////////////////////////////////////////////////

class CGTakeOutGood : public Packet {
public:
    CGTakeOutGood();
    ~CGTakeOutGood();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_TAKE_OUT_GOOD;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }
    string getPacketName() const {
        return "CGTakeOutGood";
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
    ObjectID_t m_ObjectID;
};

//////////////////////////////////////////////////////////////////////////////
// class CGTakeOutGoodFactory;
//////////////////////////////////////////////////////////////////////////////

class CGTakeOutGoodFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_TAKE_OUT_GOOD;
    static constexpr std::string_view kName = "CGTakeOutGood";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    Packet* createPacket() override {
        return new CGTakeOutGood();
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
// class CGTakeOutGoodHandler;
//////////////////////////////////////////////////////////////////////////////

class CGTakeOutGoodHandler {
public:
    static void execute(CGTakeOutGood* pPacket, Player* player);
};

#endif
