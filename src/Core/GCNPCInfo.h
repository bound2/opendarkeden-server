//--------------------------------------------------------------------------------
//
// Filename    : GCNPCInfo.h
// Written By  : Reiot
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __GC_NPC_INFO_H__
#define __GC_NPC_INFO_H__

// include files
#include "Assert1.h"
#include "EffectInfo.h"
#include "ExtraInfo.h"
#include "GameTime.h"
#include "GearInfo.h"
#include "InventoryInfo.h"
#include "NPCInfo.h"
#include "PCSlayerInfo2.h"
#include "PCVampireInfo2.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "RideMotorcycleInfo.h"

#define FLAG_PREMIUM_ZONE 0x10 // the zone is set up as premium.
#define FLAG_PREMIUM_PLAY 0x01 // is a premium play in progress?

//--------------------------------------------------------------------------------
//
// class GCNPCInfo;
//
// When the client connects to the game server and sends the CGConnect packet, the game server
// loads the items it owns and gets ready to enter the zone. Then the PC and item information,
// and the zone information are put into GCNPCInfo and sent to the client.
//
//--------------------------------------------------------------------------------

class GCNPCInfo : public Packet {
public:
    // constructor
    GCNPCInfo();

    // destructor
    ~GCNPCInfo();

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_NPC_INFO;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        PacketSize_t size = 0;

        size += szBYTE;
        list<NPCInfo*>::const_iterator itr = m_NPCInfos.begin();
        for (; itr != m_NPCInfos.end(); itr++) {
            NPCInfo* pInfo = *itr;
            size += pInfo->getSize();
        }

        return size;
    }

    // get packet name
    string getPacketName() const {
        return "GCNPCInfo";
    }

    // get packet's debug string
    string toString() const;


    //--------------------------------------------------
    // methods
    //--------------------------------------------------
public:
    // The record count travels in one byte and the factory max budgets this
    // many records.
    static constexpr size_t kMaxNPCInfos = 255;

    // Drop every record the packet holds, destroying the ones read()
    // allocated.
    void clearNPCInfos();

    // get/set npc info
    void addNPCInfo(NPCInfo* pInfo) {
        // A record past the count byte would be written and never counted.
        // The caller owns the record either way.
        if (m_NPCInfos.size() >= kMaxNPCInfos)
            return;
        m_NPCInfos.push_back(pInfo);
    }
    NPCInfo* popNPCInfo(void) {
        if (m_NPCInfos.empty())
            return NULL;
        NPCInfo* pInfo = m_NPCInfos.front();
        m_NPCInfos.pop_front();
        return pInfo;
    }

    //--------------------------------------------------
    // data members
    //--------------------------------------------------
private:
    // Information about the NPCs present in the current zone
    list<NPCInfo*> m_NPCInfos;

    // A filler hands records the zone owns; read() allocates its own, and
    // those the packet destroys.
    bool m_OwnsNPCInfos = false;
};


//--------------------------------------------------------------------------------
//
// class GCNPCInfoFactory;
//
// Factory for GCNPCInfo
//
//--------------------------------------------------------------------------------

class GCNPCInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_NPC_INFO;
    static constexpr std::string_view kName = "GCNPCInfo";
    static constexpr PacketSize_t kMaxSize{[] {
        PacketSize_t size = 0;

        size += szBYTE;
        size += NPCInfo::getMaxSize() * GCNPCInfo::kMaxNPCInfos;

        return size;
    }()};

    // create packet
    Packet* createPacket() override {
        return new GCNPCInfo();
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
    // Define and return const static GCNPCInfoPacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
//
//--------------------------------------------------------------------------------

#endif
