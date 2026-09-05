//////////////////////////////////////////////////////////////////////////////
// Filename    : GCNicknameVerify.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_NICKNAME_VERIFY_H__
#define __GC_NICKNAME_VERIFY_H__

#include "Packet.h"
#include "PacketFactory.h"

enum {
    NICKNAME_MODIFY_FAIL_NO_ITEM,
    NICKNAME_MODIFY_OK,

    NICKNAME_SELECT_FAIL_NO_SUCH,
    NICKNAME_SELECT_FAIL_FORCED, // GM forced rename; cannot choose the previous nickname.
    NICKNAME_SELECT_OK,

    NICKNAME_VERIFY_MAX
};

//////////////////////////////////////////////////////////////////////////////
// class GCNicknameVerify
//////////////////////////////////////////////////////////////////////////////

class GCNicknameVerify : public Packet {
public:
    GCNicknameVerify() {
        m_Code = NICKNAME_VERIFY_MAX;
        m_Parameter = 0;
    }
    virtual ~GCNicknameVerify() {}

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;

    PacketID_t getPacketID() const {
        return PACKET_GC_NICKNAME_VERIFY;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCNicknameVerify";
    }
    string toString() const;

public:
    BYTE getCode(void) const {
        return m_Code;
    }
    void setCode(BYTE code) {
        m_Code = code;
    }

    uint getParameter(void) const {
        return m_Parameter;
    }
    void setParameter(uint parameter) {
        m_Parameter = parameter;
    }

private:
    BYTE m_Code;
    uint m_Parameter;
};


//////////////////////////////////////////////////////////////////////////////
// class GCNicknameVerifyFactory;
//////////////////////////////////////////////////////////////////////////////

class GCNicknameVerifyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_NICKNAME_VERIFY;
    static constexpr std::string_view kName = "GCNicknameVerify";
    static constexpr PacketSize_t kMaxSize{szBYTE + szuint};

    Packet* createPacket() override {
        return new GCNicknameVerify();
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


#endif
