//--------------------------------------------------------------------------------
//
// Filename    : CGConnectSetKey.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __CL_CONNECTSETKEY_H__
#define __CL_CONNECTSETKEY_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//--------------------------------------------------------------------------------
//
// class CGConnectSetKey;
//
// The first packet the client sends to the login server.
// The id and the password are encrypted.
//
//--------------------------------------------------------------------------------


class CGConnectSetKey : public Packet {
public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_ENCODE_KEY;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szWORD    // authentication key
               + szWORD; // Slayer or Vampire?
    }

    // get packet name
    string getPacketName() const {
        return "CGConnectSetKey";
    }

    // get packet's debug string
    string toString() const {
        return "CGConnectSetKey";
    }

public:
    // get/set key
    WORD getEncryptKey() const {
        return m_EncryptKey;
    }

    WORD getHashKey() const {
        return m_HashKey;
    }

    void setEncryptKey(WORD key) {
        m_EncryptKey = key;
    }

    void setHashKey(WORD key) {
        m_HashKey = key;
    }

private:
    WORD m_EncryptKey;

    WORD m_HashKey;
};


//--------------------------------------------------------------------------------
//
// class CLLoginFactory;
//
// Factory for CLLogin
//
//--------------------------------------------------------------------------------

class CGConnectSetKeyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_ENCODE_KEY;
    static constexpr std::string_view kName = "CGConnectSetKey";
    static constexpr PacketSize_t kMaxSize{szWORD + szWORD};

    // create packet
    Packet* createPacket() override {
        return new CGConnectSetKey();
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
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
// class CLLoginHandler;
//
//--------------------------------------------------------------------------------

class CGConnectSetKeyHandler {
public:
    // execute packet's handler
    static void execute(CGConnectSetKey* pPacket, Player* pPlayer);
};

#endif
