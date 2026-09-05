//////////////////////////////////////////////////////////////////////////////
// Filename    : CGRequestPowerPoint.h
// Written By  : bezz
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_REQUEST_POWER_POINT_H__
#define __CG_REQUEST_POWER_POINT_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGRequestPowerPoint;
//////////////////////////////////////////////////////////////////////////////

class CGRequestPowerPoint : public Packet {
public:
    CGRequestPowerPoint();
    ~CGRequestPowerPoint();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_REQUEST_POWER_POINT;
    }
    PacketSize_t getPacketSize() const {
        return szBYTE + m_CellNum.size();
    }
    string getPacketName() const {
        return "CGRequestPowerPoint";
    }
    string toString() const;

public:
    // get / set cell number
    const string& getCellNum() const {
        return m_CellNum;
    }
    void setCellNum(const string& cellnum) {
        m_CellNum = cellnum;
    }

protected:
    string m_CellNum;
};

//////////////////////////////////////////////////////////////////////////////
// class CGRequestPowerPointFactory;
//////////////////////////////////////////////////////////////////////////////

class CGRequestPowerPointFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_REQUEST_POWER_POINT;
    static constexpr std::string_view kName = "CGRequestPowerPoint";
    static constexpr PacketSize_t kMaxSize{szBYTE + 12};

    Packet* createPacket() override {
        return new CGRequestPowerPoint();
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

//////////////////////////////////////////////////////////////////////////////
// class CGRequestPowerPointHandler;
//////////////////////////////////////////////////////////////////////////////

class CGRequestPowerPointHandler {
public:
    static void execute(CGRequestPowerPoint* pCGRequestPowerPoint, Player* pPlayer);
};


#endif
