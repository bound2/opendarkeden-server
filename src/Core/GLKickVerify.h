//----------------------------------------------------------------------
//
// Filename    : GLKickVerify.h
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __GL_KICK_VERIFY_H__
#define __GL_KICK_VERIFY_H__

// include files
#include "DatagramPacket.h"
#include "PacketFactory.h"


//----------------------------------------------------------------------
//
// class GLKickVerify;
//
// To fix the 'already connected' problem...
// It is the result packet matching LGKickCharacter.
//
//----------------------------------------------------------------------

class GLKickVerify : public DatagramPacket {
public:
    GLKickVerify(){};
    ~GLKickVerify(){};
    // Read data from the Datagram object and initialise the packet.
    void read(Datagram& iDatagram);

    // Send the packet's binary image to the Datagram object.
    void write(Datagram& oDatagram) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GL_KICK_VERIFY;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE + szBYTE + m_PCName.size() + szuint;
    }

    // get packet name
    string getPacketName() const {
        return "GLKickVerify";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set playerID
    bool isKicked() const {
        return m_bKicked;
    }
    void setKicked(bool bKicked = true) {
        m_bKicked = bKicked;
    }

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
    // Player ID
    bool m_bKicked;

    string m_PCName;

    uint m_ID;
};


//////////////////////////////////////////////////////////////////////
//
// class GLKickVerifyFactory;
//
// Factory for GLKickVerify
//
//////////////////////////////////////////////////////////////////////

class GLKickVerifyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GL_KICK_VERIFY;
    static constexpr std::string_view kName = "GLKickVerify";
    static constexpr PacketSize_t kMaxSize{szBYTE + szBYTE + 20 + szuint};

    // create packet
    Packet* createPacket() override {
        return new GLKickVerify();
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
    // Define and return const static GLKickVerifyPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class GLKickVerifyHandler;
//
//////////////////////////////////////////////////////////////////////

class GLKickVerifyHandler {
public:
    // execute packet's handler
    static void execute(GLKickVerify* pPacket);
};

#endif
