
//////////////////////////////////////////////////////////////////////////////
// Filename    : CGPetGamble.h
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_PET_GAMBLE_H__
#define __CG_PET_GAMBLE_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class CGPetGamble;
//////////////////////////////////////////////////////////////////////////////

class CGPetGamble : public Packet {
public:
    CGPetGamble();
    ~CGPetGamble();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_CG_PET_GAMBLE;
    }
    PacketSize_t getPacketSize() const {
        return 0;
    }
    string getPacketName() const {
        return "CGPetGamble";
    }
    string toString() const;

public:
private:
};

//////////////////////////////////////////////////////////////////////////////
// class CGPetGambleFactory;
//////////////////////////////////////////////////////////////////////////////

class CGPetGambleFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CG_PET_GAMBLE;
    static constexpr std::string_view kName = "CGPetGamble";
    static constexpr PacketSize_t kMaxSize{0};

    Packet* createPacket() override {
        return new CGPetGamble();
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
// class CGPetGambleHandler;
//////////////////////////////////////////////////////////////////////////////

class CGPetGambleHandler {
public:
    static void execute(CGPetGamble* pPacket, Player* player);
};

#endif
