////////////////////////////////////////////////////////////////////////////////
// Filename    : CGMouseToStash.h
// Written By  : 김성민
// Description :
////////////////////////////////////////////////////////////////////////////////

#ifndef __CG_MOUSE_TO_STASH_H__
#define __CG_MOUSE_TO_STASH_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
//
// class CGMouseToStash
//
////////////////////////////////////////////////////////////////////////////////

class CGMouseToStash : public Packet {
public:
    CGMouseToStash(){};
    ~CGMouseToStash(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_MOUSE_TO_STASH;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID + szBYTE * 2;
    }
    string getPacketName() const {
        return "CGMouseToStash";
    }
    string toString() const;

public:
    ObjectID_t getObjectID(void) {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }
    BYTE getRack(void) {
        return m_Rack;
    }
    void setRack(BYTE rack) {
        m_Rack = rack;
    }
    BYTE getIndex(void) {
        return m_Index;
    }
    void setIndex(BYTE index) {
        m_Index = index;
    }

private:
    ObjectID_t m_ObjectID;
    BYTE m_Rack;
    BYTE m_Index;
};


////////////////////////////////////////////////////////////////////////////////
//
// class CGMouseToStashFactory;
//
////////////////////////////////////////////////////////////////////////////////

class CGMouseToStashFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_MOUSE_TO_STASH;
    static constexpr std::string_view kName = "CGMouseToStash";
    static constexpr PacketSize_t kMaxSize{szObjectID + szBYTE * 2};

    Packet* createPacket() override {
        return new CGMouseToStash();
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
// class CGMouseToStashHandler;
//
////////////////////////////////////////////////////////////////////////////////

class CGMouseToStashHandler {
public:
    static void execute(CGMouseToStash* pPacket, Player* player);
    // static void executeSlayer(CGMouseToStash* pPacket, Player* player) ;
    // static void executeVampire(CGMouseToStash* pPacket, Player* player) ;
};


#endif
