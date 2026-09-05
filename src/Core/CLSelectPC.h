//////////////////////////////////////////////////////////////////////
//
// Filename    : CLSelectPC.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CL_SELECT_PC_H__
#define __CL_SELECT_PC_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"

class Player;

//////////////////////////////////////////////////////////////////////
//
// class CLSelectPC;
//
// 플레이할 PC 를 선택하는 패킷이다.
//
//////////////////////////////////////////////////////////////////////

class CLSelectPC : public Packet {
public:
    CLSelectPC(){};
    virtual ~CLSelectPC(){};
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CL_SELECT_PC;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szBYTE + m_PCName.size() // pc name
               + szPCType;              // pc type
    }

    // get packet's name
    string getPacketName() const {
        return "CLSelectPC";
    }

    // get packet's debug string
    string toString() const;

public:
    // get/set creature's name
    string getPCName() const {
        return m_PCName;
    }
    void setPCName(string pcName) {
        m_PCName = pcName;
    }

    // get/set pc type
    PCType getPCType() const {
        return m_PCType;
    }
    void setPCType(PCType pcType) {
        m_PCType = pcType;
    }

private:
    // Name
    string m_PCName;

    // Slayer or Vampire?
    PCType m_PCType;
};


//////////////////////////////////////////////////////////////////////
//
// class CLSelectPCFactory;
//
// Factory for CLSelectPC
//
//////////////////////////////////////////////////////////////////////

class CLSelectPCFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CL_SELECT_PC;
    static constexpr std::string_view kName = "CLSelectPC";
    static constexpr PacketSize_t kMaxSize{szBYTE + 20  // name
                                           + szPCType}; // pc type

    // create packet
    Packet* createPacket() override {
        return new CLSelectPC();
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
// class CLSelectPCHandler;
//
//////////////////////////////////////////////////////////////////////

class CLSelectPCHandler {
public:
    // execute packet's handler
    static void execute(CLSelectPC* pPacket, Player* pPlayer);
};

#endif
