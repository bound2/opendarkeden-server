//----------------------------------------------------------------------
//
// Filename    : PacketValidator
// Written By  : Reiot
// Description :
//
//----------------------------------------------------------------------

#ifndef __PACKET_ID_SET_H__
#define __PACKET_ID_SET_H__

// include files
#include "Packet.h"
#include "PlayerStatus.h"

#if defined(__LINUX__) || defined(__APPLE__)
#include <unordered_set>
typedef unordered_set<PacketID_t> PACKET_ID_SET;
#endif

//----------------------------------------------------------------------
//
// class PacketValidator;
//
// Manages a set of packets.
//
//----------------------------------------------------------------------

class PacketIDSet {
public:
    enum PacketIDSetType { PIST_NORMAL, PIST_ANY, PIST_NONE, PIST_IGNORE_EXCEPT };

public:
    // constructor
    PacketIDSet(PlayerStatus playerStatus, PacketIDSetType packetIDSetType = PIST_NORMAL);

    // destructor
    ~PacketIDSet();

    // add packet id to set
    void addPacketID(PacketID_t packetID);

    // delete packet id from set
    void deletePacketID(PacketID_t packetID);

    // has packet id?
    bool hasPacketID(PacketID_t packetID) const;

    // get player status
    PlayerStatus getPlayerStatus() const {
        return m_PlayerStatus;
    }

    // get debug string
    string toString() const;

private:
    // Packet ID Set Type
    PacketIDSetType m_PacketIDSetType;

    // Player status
    PlayerStatus m_PlayerStatus;

    // Hash set of the packets allowed
    PACKET_ID_SET m_PacketIDSet;
};

#endif
