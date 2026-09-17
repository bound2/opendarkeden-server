//////////////////////////////////////////////////////////////////////
//
// Filename    : CLReconnectLogin.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CL_RECONNECT_LOGIN_H__
#define __CL_RECONNECT_LOGIN_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class CLReconnectLogin;
//
// The connect packet the client sends to the server.
// It is used when moving between servers: the Key the previous server handed
// out is sent to the new server for authentication. It also carries the creature id to use.
//
//////////////////////////////////////////////////////////////////////

class CLReconnectLogin : public Packet {
public:
    CLReconnectLogin() : m_Key(0), m_LoginMode(LOGIN_MODE_NORMAL) {}
    virtual ~CLReconnectLogin(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CL_RECONNECT_LOGIN;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szDWORD + szBYTE; // authentication key
    }

    // get packet name
    string getPacketName() const {
        return "CLReconnectLogin";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set key
    DWORD getKey() const {
        return m_Key;
    }
    void setKey(DWORD key) {
        m_Key = key;
    }

    // Web login
    void setWebLogin() {
        m_LoginMode = LOGIN_MODE_WEBLOGIN;
    }
    bool isWebLogin() const {
        return m_LoginMode == LOGIN_MODE_WEBLOGIN;
    }

private:
    // authentication key
    DWORD m_Key;

    // Login Mode
    BYTE m_LoginMode;
};


//////////////////////////////////////////////////////////////////////
//
// class CLReconnectLoginFactory;
//
// Factory for CLReconnectLogin
//
//////////////////////////////////////////////////////////////////////

class CLReconnectLoginFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CL_RECONNECT_LOGIN;
    static constexpr std::string_view kName = "CLReconnectLogin";
    static constexpr PacketSize_t kMaxSize{szDWORD + szBYTE}; // authentication key

    // create packet
    Packet* createPacket() override {
        return new CLReconnectLogin();
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

//////////////////////////////////////////////////////////////////////
//
// class CLReconnectLoginHandler;
//
//////////////////////////////////////////////////////////////////////

class CLReconnectLoginHandler {
public:
    // execute packet's handler
    static void execute(CLReconnectLogin* pPacket, Player* pPlayer);
    static bool onChildGuardTimeArea(int pm, int am, string enable);
};

#endif
