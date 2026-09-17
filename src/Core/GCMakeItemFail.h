//////////////////////////////////////////////////////////////////////
//
// Filename    : GCMakeItemFail.h
// Written By  : elca@ewestsoft.com
// Description : Class definition for the packet sent when a skill succeeds
//
//////////////////////////////////////////////////////////////////////

#ifndef __GC_MAKE_ITEM_FAIL_H__
#define __GC_MAKE_ITEM_FAIL_H__

// include files
#include "Exception.h"
#include "GCChangeInventoryItemNum.h"
#include "ModifyInfo.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class GCMakeItemFail;
//
// Class the game server uses to tell the client that its own skill succeeded
//
//////////////////////////////////////////////////////////////////////

class GCMakeItemFail : public GCChangeInventoryItemNum, public ModifyInfo {
public:
    // constructor
    GCMakeItemFail();

    // destructor
    ~GCMakeItemFail();


public:
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_MAKE_ITEM_FAIL;
    }

    // get packet's body size
    // When optimizing, use the precomputed constant.
    PacketSize_t getPacketSize() const {
        return GCChangeInventoryItemNum::getPacketSize() + ModifyInfo::getPacketSize();
    }

    // get packet's name
    string getPacketName() const {
        return "GCMakeItemFail";
    }

    // get packet's debug string
    string toString() const;

private:
};


//////////////////////////////////////////////////////////////////////
//
// class GCMakeItemFailFactory;
//
// Factory for GCMakeItemFail
//
//////////////////////////////////////////////////////////////////////

class GCMakeItemFailFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MAKE_ITEM_FAIL;
    static constexpr std::string_view kName = "GCMakeItemFail";
    static constexpr PacketSize_t kMaxSize{GCChangeInventoryItemNum::getPacketMaxSize() +
                                           ModifyInfo::getPacketMaxSize()};

    // constructor
    GCMakeItemFailFactory() {}

    // destructor
    virtual ~GCMakeItemFailFactory() {}


public:
    // create packet
    Packet* createPacket() override {
        return new GCMakeItemFail();
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
    // PacketSize_t getPacketMaxSize() const  { return szSkillType + szCEffectID + szDuration + szBYTE + szBYTE*
    // m_ListNum* 2 ; }
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////

#endif
