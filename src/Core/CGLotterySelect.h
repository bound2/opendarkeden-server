//////////////////////////////////////////////////////////////////////
//
// Filename    : CGLotterySelect.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_LOTTERY_SELECT_H__
#define __CG_LOTTERY_SELECT_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

enum {
    TYPE_SELECT_LOTTERY = 0, // Lottery selection
    TYPE_FINISH_SCRATCH,     // Lottery scratching finished
    TYPE_OVER_ENDING,        // Ending finished

    TYPE_MAX,
};


//////////////////////////////////////////////////////////////////////
//
// class CGLotterySelect;
//
//////////////////////////////////////////////////////////////////////

class CGLotterySelect : public Packet {
public:
    CGLotterySelect(){};
    ~CGLotterySelect(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_LOTTERY_SELECT;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE + szDWORD + szDWORD;
    }

    // get packet name
    string getPacketName() const {
        return "CGLotterySelect";
    }

    // get packet's debug string
    string toString() const;

    BYTE getType() const {
        return m_Type;
    }
    void setType(BYTE type) {
        m_Type = type;
    }

    DWORD getGiftID() const {
        return m_GiftID;
    }
    void setGiftID(DWORD GiftID) {
        m_GiftID = GiftID;
    }

    DWORD getQuestLevel() const {
        return m_QuestLevel;
    }
    void setQuestLevel(DWORD QuestLevel) {
        m_QuestLevel = QuestLevel;
    }

private:
    BYTE m_Type = 0;
    DWORD m_QuestLevel = 0;
    DWORD m_GiftID = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class CGLotterySelectFactory;
//
// Factory for CGLotterySelect
//
//////////////////////////////////////////////////////////////////////

class CGLotterySelectFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_LOTTERY_SELECT;
    static constexpr std::string_view kName = "CGLotterySelect";
    static constexpr PacketSize_t kMaxSize{szBYTE + szDWORD + szDWORD};

    // constructor
    CGLotterySelectFactory() {}

    // destructor
    virtual ~CGLotterySelectFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGLotterySelect();
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
// class CGLotterySelectHandler;
//
//////////////////////////////////////////////////////////////////////

class CGLotterySelectHandler {
public:
    // execute packet's handler
    static void execute(CGLotterySelect* pCGLotterySelect, Player* pPlayer);
};

#endif
