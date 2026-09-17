//////////////////////////////////////////////////////////////////////
//
// Filename    : LCLoginOK.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_LOGIN_OK_H__
#define __LC_LOGIN_OK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class LCLoginOK;
//
// Packet with which the login server tells the client that login succeeded.
//
//////////////////////////////////////////////////////////////////////

class LCLoginOK : public Packet {
public:
    LCLoginOK() : m_LastDays(0xffff) {}
    ~LCLoginOK(){};

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_LOGIN_OK;
    }

    // get packet body size
    // *OPTIMIZATION HINT*
    // Define and return const static LCLoginOKPacketSize.
    PacketSize_t getPacketSize() const {
        return szBYTE + szBYTE + szBYTE + szWORD;
    }

    // get packet's name
    string getPacketName() const {
        return "LCLoginOK";
    }

    // get / set GoreLevel
    bool isAdult() const {
        return m_isAdult;
    }
    void setAdult(bool isAdult) {
        m_isAdult = isAdult;
    }

    bool isFamily() const {
        return m_bFamily;
    }
    void setFamily(bool isFamily) {
        m_bFamily = isFamily;
    }

    BYTE getStat() const {
        return m_Stat;
    }
    void setStat(BYTE Stat) {
        m_Stat = Stat;
    }

    WORD getLastDays() const {
        return m_LastDays;
    }
    void setLastDays(WORD LastDays) {
        m_LastDays = LastDays;
    }

    // get packet's debug string
    string toString() const {
        return "LCLoginOK";
    }

private:
    // Gore level: is the current player a minor?
    // true means an adult
    // false means a minor
    bool m_isAdult;

    // Is the user a Family user
    bool m_bFamily;

    // Server status
    BYTE m_Stat;

    WORD m_LastDays;
};


//////////////////////////////////////////////////////////////////////
//
// class LCLoginOKFactory;
//
// Factory for LCLoginOK
//
//////////////////////////////////////////////////////////////////////

class LCLoginOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_LOGIN_OK;
    static constexpr std::string_view kName = "LCLoginOK";
    static constexpr PacketSize_t kMaxSize{szBYTE + szBYTE + szBYTE + szWORD};

    // create packet
    Packet* createPacket() override {
        return new LCLoginOK();
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
//
//////////////////////////////////////////////////////////////////////

#endif
