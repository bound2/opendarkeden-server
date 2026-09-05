//////////////////////////////////////////////////////////////////////
//
// Filename    : CGModifyTaxRatio.h
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __CG_MODIFY_TAX_RATIO_H__
#define __CG_MODIFY_TAX_RATIO_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class CGModifyTaxRatio;
//
//////////////////////////////////////////////////////////////////////

class CGModifyTaxRatio : public Packet {
public:
    CGModifyTaxRatio(){};
    ~CGModifyTaxRatio(){};
    // 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
    void read(SocketInputStream& iStream);

    // 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CG_MODIFY_TAX_RATIO;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        return szuint;
    }

    // get packet name
    string getPacketName() const {
        return "CGModifyTaxRatio";
    }

    // get packet's debug string
    string toString() const;

    // get/set Ratio
    uint getRatio() const {
        return m_Ratio;
    }
    void setRatio(uint ratio) {
        m_Ratio = ratio;
    }


private:
    uint m_Ratio;
};


//////////////////////////////////////////////////////////////////////
//
// class CGModifyTaxRatioFactory;
//
// Factory for CGModifyTaxRatio
//
//////////////////////////////////////////////////////////////////////

class CGModifyTaxRatioFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_MODIFY_TAX_RATIO;
    static constexpr std::string_view kName = "CGModifyTaxRatio";
    static constexpr PacketSize_t kMaxSize{szuint};

    // constructor
    CGModifyTaxRatioFactory() {}

    // destructor
    virtual ~CGModifyTaxRatioFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new CGModifyTaxRatio();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get Packet Max Size
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
// class CGModifyTaxRatioHandler;
//
//////////////////////////////////////////////////////////////////////

class CGModifyTaxRatioHandler {
public:
    // execute packet's handler
    static void execute(CGModifyTaxRatio* pCGModifyTaxRatio, Player* pPlayer);
};

#endif
