//////////////////////////////////////////////////////////////////////////////
// Filename    : GCPartyError.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_PARTY_ERROR_H__
#define __GC_PARTY_ERROR_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// Party join related codes
//////////////////////////////////////////////////////////////////////////////
enum {
    // The target to join or leave the party does not exist.
    GC_PARTY_ERROR_TARGET_NOT_EXIST = 0,

    // The target to join or leave the party is of another race.
    GC_PARTY_ERROR_RACE_DIFFER,

    // Not a safe zone.
    GC_PARTY_ERROR_NOT_SAFE,

    // Cannot be done in wolf or bat form.
    GC_PARTY_ERROR_NOT_NORMAL_FORM,

    // Trying to invite again while an invitation is in progress.
    GC_TRADE_ERROR_ALREADY_INVITING,

    // A reply to an invitation arrived although no invitation is in progress.
    GC_PARTY_ERROR_NOT_INVITING,

    // No authority to throw a party member out.
    GC_PARTY_ERROR_NO_AUTHORITY,

    // An unknown error
    GC_TRADE_ERROR_UNKNOWN,

    GC_PARTY_ERROR_MAX
};

//////////////////////////////////////////////////////////////////////////////
// class GCPartyError;
//////////////////////////////////////////////////////////////////////////////

class GCPartyError : public Packet {
public:
    GCPartyError(){};
    ~GCPartyError(){};
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_PARTY_ERROR;
    }
    PacketSize_t getPacketSize() const {
        return szBYTE + szObjectID;
    }
    string getPacketName() const {
        return "GCPartyError";
    }
    string toString() const;

public:
    BYTE getCode() const {
        return m_Code;
    }
    void setCode(BYTE code) {
        m_Code = code;
    }

    ObjectID_t getTargetObjectID(void) const {
        return m_TargetObjectID;
    }
    void setTargetObjectID(ObjectID_t id) {
        m_TargetObjectID = id;
    }

private:
    ObjectID_t m_TargetObjectID;
    BYTE m_Code; // Code
};


//////////////////////////////////////////////////////////////////////////////
// class GCPartyErrorFactory;
//////////////////////////////////////////////////////////////////////////////

class GCPartyErrorFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_PARTY_ERROR;
    static constexpr std::string_view kName = "GCPartyError";
    static constexpr PacketSize_t kMaxSize{szBYTE + szObjectID};

    Packet* createPacket() override {
        return new GCPartyError();
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
