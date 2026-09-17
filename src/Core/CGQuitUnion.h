//////////////////////////////////////////////////////////////////////
//
// Filename    : CGQuitUnion.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_QUIT_UNION_H__
#define __CG_QUIT_UNION_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class CGQuitUnion;
//
//////////////////////////////////////////////////////////////////////

class CGQuitUnion : public Packet {
public:
    enum {
        QUIT_NORMAL = 0, // Apply through the proper procedure
        QUIT_QUICK,      // Leave unilaterally
        QUIT_MAX
    };
    CGQuitUnion(){};
    ~CGQuitUnion(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_QUIT_UNION;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szGuildID + szBYTE;
    }

    // get packet name
    string getPacketName() const {
        return "CGQuitUnion";
    }

    // get packet's debug string
    string toString() const;

    // get/set GuildID
    GuildID_t getGuildID() const {
        return m_GuildID;
    }
    void setGuildID(GuildID_t GuildID) {
        m_GuildID = GuildID;
    }

    // get/set Quit Method
    BYTE getQuitMethod() const {
        return m_Method;
    }
    void setQuitMethod(BYTE Method) {
        m_Method = Method;
    }


private:
    // Guild ID
    GuildID_t m_GuildID = 0;
    BYTE m_Method = QUIT_NORMAL;
};


//////////////////////////////////////////////////////////////////////
//
// class CGQuitUnionFactory;
//
// Factory for CGQuitUnion
//
//////////////////////////////////////////////////////////////////////

class CGQuitUnionFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_QUIT_UNION;
    static constexpr std::string_view kName = "CGQuitUnion";
    static constexpr PacketSize_t kMaxSize{szGuildID + szBYTE};

    // constructor
    CGQuitUnionFactory() {}

    // destructor
    virtual ~CGQuitUnionFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGQuitUnion();
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
// class CGQuitUnionHandler;
//
//////////////////////////////////////////////////////////////////////

class CGQuitUnionHandler {
public:
    // execute packet's handler
    static void execute(CGQuitUnion* pCGQuitUnion, Player* pPlayer);
};

#endif
