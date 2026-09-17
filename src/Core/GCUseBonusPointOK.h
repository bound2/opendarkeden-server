//////////////////////////////////////////////////////////////////////
//
// Filename    : GCUseBonusPointOK.h
// Written By  : crazydog
// Description : A vampire is allowed to use a bonus.
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_USE_BONUS_POINT_OK_H__
#define __GC_USE_BONUS_POINT_OK_H__

// include files
#include "ModifyInfo.h"
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class GCUseBonusPointOK;
//
//////////////////////////////////////////////////////////////////////

class GCUseBonusPointOK : public ModifyInfo {
public:
    // Constructor
    GCUseBonusPointOK();

    // Desctructor
    ~GCUseBonusPointOK();

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_USE_BONUS_POINT_OK;
    }

    // get packet's body size
    // *OPTIMIZATION HINT*
    // Define and return const static GCUseBonusPointOKPacketSize.
    PacketSize_t getPacketSize() const {
        return ModifyInfo::getPacketSize();
    }

    // get packet name
    string getPacketName() const {
        return "GCUseBonusPointOK";
    }

    // get packet's debug string
    string toString() const;
};


//////////////////////////////////////////////////////////////////////
//
// class GCUseBonusPointOKFactory;
//
// Factory for GCUseBonusPointOK
//
//////////////////////////////////////////////////////////////////////

class GCUseBonusPointOKFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_USE_BONUS_POINT_OK;
    static constexpr std::string_view kName = "GCUseBonusPointOK";
    static constexpr PacketSize_t kMaxSize{ModifyInfo::getPacketMaxSize()};

    // create packet
    Packet* createPacket() override {
        return new GCUseBonusPointOK();
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
    // *OPTIMIZATION HINT*
    // Define and return const static GCUseBonusPointOKPacketSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
