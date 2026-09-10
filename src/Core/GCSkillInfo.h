//--------------------------------------------------------------------------------
//
// Filename    : GCSkillInfo.h
// Written By  : Reiot
// Description :
//
//--------------------------------------------------------------------------------

#ifndef __GC_SKILL_INFO_H__
#define __GC_SKILL_INFO_H__

// include files
#include "Assert1.h"
#include "PCSkillInfo.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "SlayerSkillInfo.h"
#include "VampireSkillInfo.h"

//--------------------------------------------------------------------------------
//
// class GCSkillInfo;
//
// After the client loads core info and sends CGConnect, it prepares to enter the
// world by loading all core data. The PC skill information is bundled and sent
// to the client via GCSkillInfo.
//
//--------------------------------------------------------------------------------

class GCSkillInfo : public Packet {
public:
    // constructor
    GCSkillInfo();

    // destructor
    ~GCSkillInfo();

    // Initialize packet by reading data from the incoming stream.
    void read(SocketInputStream& iStream);

    // Serialize packet data to the outgoing stream.
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_GC_SKILL_INFO;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const;

    // get packet name
    string getPacketName() const {
        return "GCSkillInfo";
    }

    // get packet's debug string
    string toString() const;

    //--------------------------------------------------
    // methods
    //--------------------------------------------------
public:
    // A book carries one record per skill domain: a slayer fills one for
    // each domain it may learn, a vampire and an ousters one apiece.
    static constexpr size_t kMaxRecords = SKILL_DOMAIN_MAX;

    // read() builds a record for these three and refuses every other.
    static bool isKnownPCType(BYTE PCType) {
        return PCType == PC_SLAYER || PCType == PC_VAMPIRE || PCType == PC_OUSTERS;
    }

    // get / set PCType
    BYTE getPCType() const {
        return m_PCType;
    }
    void setPCType(BYTE PCType) {
        if (!isKnownPCType(PCType))
            throw InvalidProtocolException("unknown pc type");
        m_PCType = PCType;
    }

    // add / delete / clear Skill List
    void addListElement(PCSkillInfo* pPCSkillInfo) {
        if (m_pPCSkillInfoList.size() >= kMaxRecords)
            throw InvalidProtocolException("too many skill records");
        m_pPCSkillInfoList.push_back(pPCSkillInfo);
    }

    // ClearList
    void clearList();

    // pop front Element in Status List
    PCSkillInfo* popFrontListElement() {
        PCSkillInfo* TempPCSkillInfo = m_pPCSkillInfoList.front();
        m_pPCSkillInfoList.pop_front();
        return TempPCSkillInfo;
    }

private:
    BYTE m_PCType = PC_SLAYER;

    //---------------------------------------------------------
    // PC Skill Information
    // Holds either SlayerSkillInfo or VampireSkillInfo objects.
    //---------------------------------------------------------
    list<PCSkillInfo*> m_pPCSkillInfoList;
};


//--------------------------------------------------------------------------------
//
// class GCSkillInfoFactory;
//
// Factory for GCSkillInfo
//
//--------------------------------------------------------------------------------

class GCSkillInfoFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_SKILL_INFO;
    static constexpr std::string_view kName = "GCSkillInfo";
    // The pc type, the record count and one record per domain. The
    // slayer record is the widest of the three races.
    static constexpr PacketSize_t kMaxSize{szBYTE + szBYTE + GCSkillInfo::kMaxRecords * SlayerSkillInfo::getMaxSize()};

    // create packet
    Packet* createPacket() override {
        return new GCSkillInfo();
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
    // Use const static GCSkillInfoPacketMaxSize when possible.
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
//
//--------------------------------------------------------------------------------

#endif
