//--------------------------------------------------------------------------------
//
// Filename    : CLLogin.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __CL_LOGIN_H__
#define __CL_LOGIN_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//--------------------------------------------------------------------------------
//
// class CLLogin;
//
// The first packet the client sends to the login server.
// The id and the password are encrypted.
//
//--------------------------------------------------------------------------------

// Maximum MAC ADDRESS length
#define MAX_LENGTH_MAC 6


class CLLogin : public Packet {
public:
    // The MAC bytes have no setter -- read() is what fills them -- and write()
    // emits all six either way, so a constructed instance starts them at zero.
    CLLogin() : m_bNetmarble(false), m_bAdult(false), m_cMacAddress{}, m_LoginMode(LOGIN_MODE_NORMAL) {
        m_strMacAddress = "";
    }
    virtual ~CLLogin(){};

public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CL_LOGIN;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const;

    // get packet name
    string getPacketName() const {
        return "CLLogin";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set player's id
    string getID() const {
        return m_ID;
    }
    void setID(string id) {
        m_ID = id;
    }

    // get/set player's password
    string getPassword() const {
        return m_Password;
    }
    void setPassword(string password) {
        m_Password = password;
    }

    // get/set Cpsso imformation
    bool isNetmarble() const {
        return m_bNetmarble;
    }
    void setNetmarble(bool netmarble) {
        m_bNetmarble = netmarble;
    }

    bool isAdult() const {
        return m_bAdult;
    }
    void setAdult(bool adult) {
        m_bAdult = adult;
    }

    // add - inthesky
    bool checkMacAddress(string currentMac) const;
    string getMacAddress() const {
        return m_strMacAddress;
    }

    const BYTE* getRareMacAddress() const {
        return m_cMacAddress;
    }

    void setWebLogin() {
        m_LoginMode = LOGIN_MODE_WEBLOGIN;
    }
    bool isWebLogin() const {
        return m_LoginMode == LOGIN_MODE_WEBLOGIN;
    }

private:
    // Player id
    string m_ID;

    // Player password
    string m_Password;

    // Not sent or received, but added as a member because the information is needed
    // Its size is not counted. (Holds what the netmarble Cpsso authentication carries)
    bool m_bNetmarble;
    bool m_bAdult;

    BYTE m_cMacAddress[6];
    string m_strMacAddress;

    BYTE m_LoginMode;
};


//--------------------------------------------------------------------------------
//
// class CLLoginFactory;
//
// Factory for CLLogin
//
//--------------------------------------------------------------------------------

class CLLoginFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CL_LOGIN;
    static constexpr std::string_view kName = "CLLogin";
    static constexpr PacketSize_t kMaxSize{szBYTE + 30 + szBYTE + 30 + 6 + szBYTE};

    // create packet
    Packet* createPacket() override {
        return new CLLogin();
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
    // szID + ID(<=30) + szPassword + password(<=30) + mac(6) + loginMode --
    // read() rejects longer strings; the old netmarble sso layout (szint +
    // 2048) is no longer read.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
// class CLLoginHandler;
//
//--------------------------------------------------------------------------------

class CLLoginHandler {
public:
    // execute packet's handler
    static void execute(CLLogin* pPacket, Player* pPlayer);

private:
    static bool checkFreePass(CLLogin* pPacket, Player* pPlayer);
    static bool checkNetMarbleClient(CLLogin* pPacket, Player* pPlayer);
    static bool checkWebLogin(CLLogin* pPacket, Player* pPlayer);
};

#endif
