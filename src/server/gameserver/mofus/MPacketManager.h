/////////////////////////////////////////////////////////////////////////////
// Filename	: MPacketManager.h
/////////////////////////////////////////////////////////////////////////////

#ifndef __MPACKET_MANAGER_H__
#define __MPACKET_MANAGER_H__

// include files
#include "MPacket.h"

// forward declaration
class MPlayer;
class MPacketHandler;


// class MPacketManager
class MPacketManager {
public:
    MPacketManager();
    ~MPacketManager();

public:
    void init();

public:
    // Adds a packet creator.
    void addCreator(MPacket* pPacket);

    // Adds a packet handler.
    void addHandler(MPacketHandler* pHandler);

public:
    // Creates a new packet and returns it.
    MPacket* createPacket(MPacketID_t ID) const;

    // Runs the packet's handler.
    void execute(MPlayer* pPlayer, MPacket* pPacket);

    // Checks whether the packet has a handler.
    bool hasHandler(MPacketID_t ID) const;

    // Returns the packet's size.
    MPacketSize_t getPacketSize(MPacketID_t ID) const;

private:
    // internal implementation data
    struct IMPL;
    IMPL* m_pImpl;
};


// global variable
extern MPacketManager* g_pMPacketManager;

#endif
