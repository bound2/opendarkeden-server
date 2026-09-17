//////////////////////////////////////////////////////////////////////
//
// Filename    : LCLoginError.h
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __LC_LOGIN_ERROR_H__
#define __LC_LOGIN_ERROR_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////
//
// class LCLoginError;
//
//
//////////////////////////////////////////////////////////////////////

class LCLoginError : public Packet {
public:
    LCLoginError(){};
    ~LCLoginError(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_LC_LOGIN_ERROR;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE;
    }

    // get packet's name
    string getPacketName() const {
        return "LCLoginError";
    }

    // get packet's debug string
    string toString() const;

    //	string getMessage() const  { return m_Message; }
    //	void setMessage(string message)  { m_Message = message; }
    BYTE getErrorID() const {
        return m_ErrorID;
    }
    void setErrorID(BYTE ErrorID) {
        m_ErrorID = ErrorID;
    }

private:
    // Error ID
    BYTE m_ErrorID;

    //	string m_Message;
};


//////////////////////////////////////////////////////////////////////
//
// class LCLoginErrorFactory;
//
// Factory for LCLoginError
//
//////////////////////////////////////////////////////////////////////

class LCLoginErrorFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_LC_LOGIN_ERROR;
    static constexpr std::string_view kName = "LCLoginError";
    static constexpr PacketSize_t kMaxSize{szBYTE};

    // create packet
    Packet* createPacket() override {
        return new LCLoginError();
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
