//////////////////////////////////////////////////////////////////////
//
// Filename    : CGFailQuest.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_FAIL_QUEST_H__
#define __CG_FAIL_QUEST_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class CGFailQuest;
//
//////////////////////////////////////////////////////////////////////

class CGFailQuest : public Packet {
public:
    CGFailQuest(){};
    ~CGFailQuest(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_FAIL_QUEST;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE;
    }

    // get packet name
    string getPacketName() const {
        return "CGFailQuest";
    }

    // get packet's debug string
    string toString() const;

public:
    BYTE isFail() const {
        return m_bFail != 0;
    }
    void setFail(bool bFail) {
        m_bFail = (bFail) ? 1 : 0;
    }

private:
    BYTE m_bFail = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class CGFailQuestFactory;
//
// Factory for CGFailQuest
//
//////////////////////////////////////////////////////////////////////

class CGFailQuestFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_FAIL_QUEST;
    static constexpr std::string_view kName = "CGFailQuest";
    static constexpr PacketSize_t kMaxSize{szBYTE};

    // constructor
    CGFailQuestFactory() {}

    // destructor
    virtual ~CGFailQuestFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGFailQuest();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get Packet Max Size
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGFailQuestHandler;
//
//////////////////////////////////////////////////////////////////////

class CGFailQuestHandler {
public:
    // execute packet's handler
    static void execute(CGFailQuest* pCGFailQuest, Player* pPlayer);
};

#endif
