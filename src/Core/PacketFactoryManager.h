//////////////////////////////////////////////////////////////////////
//
// Filename    : PacketFactoryManager.h
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __PACKET_FACTORY_MANAGER_H__
#define __PACKET_FACTORY_MANAGER_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"


//////////////////////////////////////////////////////////////////////
//
// class PacketFactoryManager
//
//////////////////////////////////////////////////////////////////////

class PacketFactoryManager {
public:
    // constructor
    PacketFactoryManager();

    // destructor
    ~PacketFactoryManager() noexcept;

    // Initialise the packet factory manager.
    // Called from the game server object's init().
    void init();

    // Add a factory object at a particular index.
    void addFactory(PacketFactory* pFactory);

    // Create a packet object from a packet id.
    Packet* createPacket(PacketID_t packetID);

    // Return the maximum size of a particular packet.
    string getPacketName(PacketID_t packetID);

    // Return the maximum size of a particular packet.
    PacketSize_t getPacketMaxSize(PacketID_t packetID);

    // get debug string
    string toString() const;

private:
    // Array of packet factories
    PacketFactory** m_Factories;

    // Size of the packet factory array
    ushort m_Size;
};

#endif
