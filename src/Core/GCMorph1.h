//--------------------------------------------------------------------------------
//
// Filename    : GCMorph1.h
// Written By  : crazydog
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __GC_MORPH1_H__
#define __GC_MORPH1_H__

// include files
#include "Assert1.h"
#include "ExtraInfo.h"
#include "GameTime.h"
#include "GearInfo.h"
#include "InventoryInfo.h"
#include "PCSlayerInfo2.h"
#include "PCVampireInfo2.h"
#include "Packet.h"
#include "PacketFactory.h"

//--------------------------------------------------------------------------------
//
// class GCMorph1;
//
//	Packet sent to the one transforming when a slayer turns into a vampire.
//--------------------------------------------------------------------------------

class GCMorph1 : public Packet {
public:
    // constructor
    GCMorph1();

    // destructor
    ~GCMorph1();

    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_MORPH_1;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        // [PCType][PCInfo][InventoryInfo][GearInfo][ExtraInfo]
        requireRecords();
        return szBYTE + m_pPCInfo->getSize() + m_pInventoryInfo->getSize() + m_pGearInfo->getSize() +
               m_pExtraInfo->getSize();
    }

    // get packet name
    string getPacketName() const {
        return "GCMorph1";
    }

    // get packet's debug string
    string toString() const;


    //--------------------------------------------------
    // methods
    //--------------------------------------------------
public:
    // The four records belong to the packet: every sender builds them
    // for it and keeps none, so each setter frees what it replaces and
    // the destructor frees what is left.
    PCInfo* getPCInfo2() const {
        return m_pPCInfo;
    }
    void setPCInfo2(PCInfo* pPCInfo) {
        if (m_pPCInfo != pPCInfo)
            delete m_pPCInfo;
        m_pPCInfo = pPCInfo;
    }

    // get/set Inventory Info
    InventoryInfo* getInventoryInfo() const {
        return m_pInventoryInfo;
    }
    void setInventoryInfo(InventoryInfo* pInventoryInfo) {
        if (m_pInventoryInfo != pInventoryInfo)
            delete m_pInventoryInfo;
        m_pInventoryInfo = pInventoryInfo;
    }

    // get/set Gear Info
    GearInfo* getGearInfo() const {
        return m_pGearInfo;
    }
    void setGearInfo(GearInfo* pGearInfo) {
        if (m_pGearInfo != pGearInfo)
            delete m_pGearInfo;
        m_pGearInfo = pGearInfo;
    }

    // get/set ExtraInfo
    ExtraInfo* getExtraInfo() const {
        return m_pExtraInfo;
    }
    void setExtraInfo(ExtraInfo* pExtraInfo) {
        if (m_pExtraInfo != pExtraInfo)
            delete m_pExtraInfo;
        m_pExtraInfo = pExtraInfo;
    }

    //--------------------------------------------------
    // data members
    //--------------------------------------------------
private:
    // write() emits all four, so a packet missing one is refused rather
    // than followed.
    void requireRecords() const {
        if (m_pPCInfo == NULL || m_pInventoryInfo == NULL || m_pGearInfo == NULL || m_pExtraInfo == NULL)
            throw InvalidProtocolException("morph record missing");
    }

    //--------------------------------------------------------------------------------
    // PC Information
    //--------------------------------------------------------------------------------
    // PCSlayerInfo2 or PCVampireInfo2 is used.
    PCInfo* m_pPCInfo;

    //--------------------------------------------------------------------------------
    // Inventory Information
    //--------------------------------------------------------------------------------
    InventoryInfo* m_pInventoryInfo;

    //--------------------------------------------------------------------------------
    // Gear Information
    //--------------------------------------------------------------------------------
    GearInfo* m_pGearInfo;

    //--------------------------------------------------------------------------------
    // Extra Information
    //--------------------------------------------------------------------------------
    ExtraInfo* m_pExtraInfo;

    // inventory
    // quick item slot
    // gear

    // Journal (PDA)
    // Quest progress information
    // Notices, event information
    // Hmm.. maybe these should be downloaded the first time the PDS is opened.. - -;
};


//--------------------------------------------------------------------------------
//
// class GCMorph1Factory;
//
// Factory for GCMorph1
//
//--------------------------------------------------------------------------------

class GCMorph1Factory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_MORPH_1;
    static constexpr std::string_view kName = "GCMorph1";
    static constexpr PacketSize_t kMaxSize{szBYTE + PCSlayerInfo2::getMaxSize() + InventoryInfo::getMaxSize() +
                                           GearInfo::getMaxSize() + ExtraInfo::getMaxSize()};

    // create packet
    Packet* createPacket() override {
        return new GCMorph1();
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
    // Define and return const static GCMorph1PacketMaxSize.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
//
//--------------------------------------------------------------------------------

#endif
