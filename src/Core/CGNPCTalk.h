//--------------------------------------------------------------------------------
//
// Filename    : CGNPCTalk.h
// Written By  : Reiot
//
//--------------------------------------------------------------------------------

#ifndef __CG_NPC_TALK_H__
#define __CG_NPC_TALK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//--------------------------------------------------------------------------------
//
// class CGNPCTalk;
//
//--------------------------------------------------------------------------------

class CGNPCTalk : public Packet {
public:
    CGNPCTalk(){};
    ~CGNPCTalk(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_NPC_TALK;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static CGNPCTalkPacketSize.
    PacketSize_t getPacketSize() const {
        return szObjectID;
    }

    // get packet name
    string getPacketName() const {
        return "CGNPCTalk";
    }

    // get packet's debug string
    string toString() const;

public:
    // get / set ObjectID
    ObjectID_t getObjectID() {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t ObjectID) {
        m_ObjectID = ObjectID;
    }

private:
    // ObjectID
    ObjectID_t m_ObjectID = 0;
};


//--------------------------------------------------------------------------------
//
// class CGNPCTalkFactory;
//
// Factory for CGNPCTalk
//
//--------------------------------------------------------------------------------

class CGNPCTalkFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_NPC_TALK;
    static constexpr std::string_view kName = "CGNPCTalk";
    static constexpr PacketSize_t kMaxSize{szObjectID};

    // create packet
    Packet* createPacket() override {
        return new CGNPCTalk();
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
    // *OPTIMIZATION HINT*
    // Define and return const static CGNPCTalkPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
// class CGNPCTalkHandler;
//
//--------------------------------------------------------------------------------

class CGNPCTalkHandler {
public:
    // execute packet's handler
    static void execute(CGNPCTalk* pPacket, Player* player);
};

#endif
