//////////////////////////////////////////////////////////////////////
//
// Filename    : LCRegisterPlayerOK.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_REGISTER_PLAYER_OK_H__
#define __LC_REGISTER_PLAYER_OK_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////
//
// class LCRegisterPlayerOK;
//
// Packet with which the login server tells the client that login succeeded.
//
//////////////////////////////////////////////////////////////////////

class LCRegisterPlayerOK : public Packet {
public:
    LCRegisterPlayerOK() : m_isAdult(false) {}
    ~LCRegisterPlayerOK(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_REGISTER_PLAYER_OK;
    }

    // get packet body size
    // *OPTIMIZATION HINT*
    // Define and return const static LCRegisterPlayerOKPacketSize.
    PacketSize_t getPacketSize() const {
        return de::wire::stringWireSize(m_GroupName) + szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "LCRegisterPlayerOK";
    }

    // get / set Groupname
    string getGroupName() const {
        return m_GroupName;
    }
    // Truncates to the width the length prefix and the factory max allow.
    void setGroupName(const string& GroupName) {
        m_GroupName = (GroupName.size() > maxNameLength) ? GroupName.substr(0, maxNameLength) : GroupName;
    }

    // get / set GoreLevel
    bool isAdult() const {
        return m_isAdult;
    }
    void setAdult(bool isAdult) {
        m_isAdult = isAdult;
    }


    // get packet's debug string
    string toString() const {
        return "LCRegisterPlayerOK";
    }

private:
    // Server group name.
    string m_GroupName;

    // Gore level: is the current player a minor?
    // true means an adult
    // false means a minor
    bool m_isAdult;
};


//////////////////////////////////////////////////////////////////////
//
// class LCRegisterPlayerOKFactory;
//
// Factory for LCRegisterPlayerOK
//
//////////////////////////////////////////////////////////////////////

class LCRegisterPlayerOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_REGISTER_PLAYER_OK;
    static constexpr std::string_view kName = "LCRegisterPlayerOK";
    static constexpr PacketSize_t kMaxSize{szBYTE + maxNameLength + szBYTE};

    // create packet
    Packet* createPacket() override {
        return new LCRegisterPlayerOK();
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
