//////////////////////////////////////////////////////////////////////////////
// Filename    : GCModifyInformation.h
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_MODIFY_INFORMATION_H__
#define __GC_MODIFY_INFORMATION_H__

#include "Exception.h"
#include "ModifyInfo.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GCModifyInformation;
//////////////////////////////////////////////////////////////////////////////

class GCModifyInformation : public ModifyInfo {
public:
    GCModifyInformation(){};
    ~GCModifyInformation(){};
    PacketID_t getPacketID() const {
        return PACKET_GC_MODIFY_INFORMATION;
    }
    string getPacketName() const {
        return "GCModifyInformation";
    }
};


//////////////////////////////////////////////////////////////////////////////
// class GCModifyInformationFactory;
//////////////////////////////////////////////////////////////////////////////

class GCModifyInformationFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MODIFY_INFORMATION;
    static constexpr std::string_view kName = "GCModifyInformation";
    static constexpr PacketSize_t kMaxSize{ModifyInfo::getPacketMaxSize()};

    Packet* createPacket() override {
        return new GCModifyInformation();
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
