//////////////////////////////////////////////////////////////////////
//
// Filename    :  GCSelectRankBonusOK.h
// Written By  :  elca@ewestsoft.com
// Description :  Packet
//
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_SELECT_RANK_BONUS_OK_H__
#define __GC_SELECT_RANK_BONUS_OK_H__

// include files
#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class  GCSelectRankBonusOK;
//
//////////////////////////////////////////////////////////////////////

class GCSelectRankBonusOK : public Packet {
public:
    // constructor
    GCSelectRankBonusOK();

    // destructor
    ~GCSelectRankBonusOK();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SELECT_RANK_BONUS_OK;
    }

    // get packet size
    PacketSize_t getPacketSize() const {
        return szDWORD;
    }

    // get packet's name
    string getPacketName() const {
        return "GCSelectRankBonusOK";
    }

    // get packet's debug string
    string toString() const;

    // get/set m_RankBonusType
    DWORD getRankBonusType() const {
        return m_RankBonusType;
    }
    void setRankBonusType(DWORD rankBonusType) {
        m_RankBonusType = rankBonusType;
    }

private:
    // RankBonusType
    DWORD m_RankBonusType = 0;
};


//////////////////////////////////////////////////////////////////////
//
// class  GCSelectRankBonusOKFactory;
//
// Factory for  GCSelectRankBonusOK
//
//////////////////////////////////////////////////////////////////////

class GCSelectRankBonusOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SELECT_RANK_BONUS_OK;
    static constexpr std::string_view kName = "GCSelectRankBonusOK";
    static constexpr PacketSize_t kMaxSize{szDWORD};

    // constructor
    GCSelectRankBonusOKFactory() {}

    // destructor
    virtual ~GCSelectRankBonusOKFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCSelectRankBonusOK();
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


#endif
