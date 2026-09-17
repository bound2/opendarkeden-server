//----------------------------------------------------------------------
//
// Filename    : LGKickCharacter.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __LG_KICK_CHARACTER_H__
#define __LG_KICK_CHARACTER_H__

// include files
#include "DatagramPacket.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class LGKickCharacter;
//
// To fix the 'already connected' problem
// Packet that tries to remove a character that is already connected.
//
// The result of this packet is LGKickVerify.
//
//----------------------------------------------------------------------

class LGKickCharacter : public DatagramPacket {
public:
    LGKickCharacter(){};
    ~LGKickCharacter(){};
    // Read data from the Datagram object and initialise the packet.
    void read(Datagram& iDatagram);

    // Send the packet's binary image to the Datagram object.
    void write(Datagram& oDatagram) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LG_KICK_CHARACTER;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE + m_PCName.size() // PC name
               + szuint;
    }

    // get packet name
    string getPacketName() const {
        return "LGKickCharacter";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set pcName
    string getPCName() const {
        return m_PCName;
    }
    void setPCName(const string& pcName) {
        m_PCName = pcName;
    }

    uint getID() const {
        return m_ID;
    }
    void setID(uint id) {
        m_ID = id;
    }

private:
    // PC name
    string m_PCName;

    uint m_ID;
};


//////////////////////////////////////////////////////////////////////
//
// class LGKickCharacterFactory;
//
// Factory for LGKickCharacter
//
//////////////////////////////////////////////////////////////////////

class LGKickCharacterFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LG_KICK_CHARACTER;
    static constexpr std::string_view kName = "LGKickCharacter";
    static constexpr PacketSize_t kMaxSize{szBYTE + 20 // PC name
                                           + szuint};

    // create packet
    Packet* createPacket() override {
        return new LGKickCharacter();
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
    // Define and return const static LGKickCharacterPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class LGKickCharacterHandler;
//
//////////////////////////////////////////////////////////////////////

class LGKickCharacterHandler {
public:
    // execute packet's handler
    static void execute(LGKickCharacter* pPacket);
};

#endif
