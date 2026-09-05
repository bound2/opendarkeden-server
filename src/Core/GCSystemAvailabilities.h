//////////////////////////////////////////////////////////////////////////////
// Filename    : GCSystemAvailabilities.h
// Written By  : elca@ewestsoft.com
// Description :
// 기술이 성공했을때 보내는 패킷을 위한 클래스 정의
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_SYSTEM_AVAILABILITIES_H__
#define __GC_SYSTEM_AVAILABILITIES_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GCSystemAvailabilities;
// 게임서버에서 클라이언트로 자신의 기술이 성공을 알려주기 위한 클래스
//////////////////////////////////////////////////////////////////////////////

class GCSystemAvailabilities : public Packet {
public:
    GCSystemAvailabilities();
    ~GCSystemAvailabilities();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_SYSTEM_AVAILABILITIES;
    }
    PacketSize_t getPacketSize() const {
        return szDWORD + szBYTE + szBYTE;
    }
    string getPacketName() const {
        return "GCSystemAvailabilities";
    }
    string toString() const;

public:
    DWORD getFlag() const {
        return m_Flag;
    }
    void setFlag(DWORD flag) {
        m_Flag = flag;
    }

    BYTE getOpenDegree() const {
        return m_Degree;
    }
    void setOpenDegree(BYTE deg) {
        m_Degree = deg;
    }

    BYTE getSkillLimit() const {
        return m_SkillLimit;
    }
    void setSkillLimit(BYTE lim) {
        m_SkillLimit = lim;
    }

private:
    DWORD m_Flag;
    BYTE m_Degree;
    BYTE m_SkillLimit;
};


//////////////////////////////////////////////////////////////////////////////
// class GCSystemAvailabilitiesFactory;
//////////////////////////////////////////////////////////////////////////////

class GCSystemAvailabilitiesFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SYSTEM_AVAILABILITIES;
    static constexpr std::string_view kName = "GCSystemAvailabilities";
    static constexpr PacketSize_t kMaxSize{szDWORD + szBYTE + szBYTE};

    GCSystemAvailabilitiesFactory() {}
    virtual ~GCSystemAvailabilitiesFactory() {}

public:
    Packet* createPacket() override {
        return new GCSystemAvailabilities();
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
