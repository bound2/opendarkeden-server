////////////////////////////////////////////////////////////////////////////////
// Filename    : CGStashList.h
// Description :
// Packet the client uses to ask for the list of the items held in the
// stash.
////////////////////////////////////////////////////////////////////////////////

#ifndef __CG_STASH_LIST_H__
#define __CG_STASH_LIST_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

////////////////////////////////////////////////////////////////////////////////
//
// class CGStashList;
//
////////////////////////////////////////////////////////////////////////////////

class CGStashList : public Packet {
public:
    CGStashList(){};
    virtual ~CGStashList(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_STASH_LIST;
    }
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }
    string getPacketName() const {
        return "CGStashList";
    }
    string toString() const;

public:
    ObjectID_t getObjectID() {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t id) {
        m_ObjectID = id;
    }

private:
    ObjectID_t m_ObjectID = 0; // Object id of the player creature
};


////////////////////////////////////////////////////////////////////////////////
//
// class CGStashListFactory;
//
// Factory for CGStashList
//
////////////////////////////////////////////////////////////////////////////////

class CGStashListFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_STASH_LIST;
    static constexpr std::string_view kName = "CGStashList";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    Packet* createPacket() override {
        return new CGStashList();
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
// class CGStashListHandler;
//
////////////////////////////////////////////////////////////////////////////////

class CGStashListHandler {
public:
    static void execute(CGStashList* pPacket, Player* player);
};

#endif
