//////////////////////////////////////////////////////////////////////
//
// Filename    : CGSMSSend.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_SMS_SEND_H__
#define __CG_SMS_SEND_H__

// include files
#include <list>
#include <string>

#include "Packet.h"
#include "PacketFactory.h"

#define MAX_NUMBER_LENGTH 11
#define MAX_RECEVIER_NUM 5
// 40 matches the client's cap (its write() asserts size < 40); the server
// once accepted 80, which the client can never send.
#define MAX_MESSAGE_LENGTH 40

//////////////////////////////////////////////////////////////////////
//
// class CGSMSSend;
//
//////////////////////////////////////////////////////////////////////

class CGSMSSend : public Packet {
public:
    CGSMSSend(){};
    virtual ~CGSMSSend(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_SMS_SEND;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "CGSMSSend";
    }
    string toString() const;

public:
    list<string>& getNumbersList() {
        return m_Numbers;
    }

    const string& getCallerNumber() const {
        return m_CallerNumber;
    }
    void setCallerNumber(const string& Num) {
        m_CallerNumber = Num;
    }

    const string& getMessage() const {
        return m_Message;
    }
    void setMessage(const string& msg) {
        m_Message = msg;
    }

private:
    list<string> m_Numbers;
    string m_CallerNumber;
    string m_Message;
};


//////////////////////////////////////////////////////////////////////
//
// class CGSMSSendFactory;
//
// Factory for CGSMSSend
//
//////////////////////////////////////////////////////////////////////

class CGSMSSendFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_SMS_SEND;
    static constexpr std::string_view kName = "CGSMSSend";
    static constexpr PacketSize_t kMaxSize{szBYTE + (szBYTE + MAX_NUMBER_LENGTH) * MAX_RECEVIER_NUM + szBYTE +
                                           MAX_RECEVIER_NUM + szBYTE + MAX_MESSAGE_LENGTH};

    Packet* createPacket() override {
        return new CGSMSSend();
    }
    string getPacketName() const override {
        return string(kName);
    }
    PacketID_t getPacketID() const override {
        return kPacketID;
    }
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGSMSSendHandler;
//
//////////////////////////////////////////////////////////////////////

class CGSMSSendHandler {
public:
    static void execute(CGSMSSend* pPacket, Player* player);
};

#endif
