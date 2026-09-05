////////////////////////////////////////////////////////////////////////////////
//
// Filename    : CGWithdrawPet.h
// Written By  : 김성민
// Description :
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __CG_WITHDRAW_PET_H__
#define __CG_WITHDRAW_PET_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
//
// class CGWithdrawPet;
//
////////////////////////////////////////////////////////////////////////////////

class CGWithdrawPet : public Packet {
public:
    CGWithdrawPet(){};
    virtual ~CGWithdrawPet(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_WITHDRAW_PET;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szBYTE;
    }
    string getPacketName() const {
        return "CGWithdrawPet";
    }
    string toString() const;

public:
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t objectID) {
        m_ObjectID = objectID;
    }

    BYTE getIndex(void) const {
        return m_Index;
    }
    void setIndex(BYTE index) {
        m_Index = index;
    }

private:
    ObjectID_t m_ObjectID;
    BYTE m_Index;
};


////////////////////////////////////////////////////////////////////////////////
//
// class CGWithdrawPetFactory;
//
////////////////////////////////////////////////////////////////////////////////

class CGWithdrawPetFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_WITHDRAW_PET;
    static constexpr std::string_view kName = "CGWithdrawPet";
    static constexpr PacketSize_t kMaxSize{szObjectID + szBYTE};

    Packet* createPacket() override {
        return new CGWithdrawPet();
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


////////////////////////////////////////////////////////////////////////////////
//
// class CGWithdrawPetHandler;
//
////////////////////////////////////////////////////////////////////////////////

class CGWithdrawPetHandler {
public:
    // execute packet's handler
    static void execute(CGWithdrawPet* pPacket, Player* player);
    // static void executeSlayer(CGWithdrawPet* pPacket, Player* player) ;
    // static void executeVampire(CGWithdrawPet* pPacket, Player* player) ;
};

#endif
