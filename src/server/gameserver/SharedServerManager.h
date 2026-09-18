//////////////////////////////////////////////////////////////////////////////
// Filename    : SharedServerManager.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SHARED_SERVER_MANANGER_H__
#define __SHARED_SERVER_MANANGER_H__

#include "Exception.h"
#include "ManagedThread.h"
#include "Mutex.h"
#include "Packet.h"
#include "Socket.h"
#include "Types.h"

class SharedServerClient;

//////////////////////////////////////////////////////////////////////////////
// class SharedServerManager;
//
// Thread dedicated to communication with the shared server.
// When the connection drops it must reconnect, and it retries until it succeeds.
//////////////////////////////////////////////////////////////////////////////

class SharedServerManager : public ManagedThread {
public:
    SharedServerManager();
    ~SharedServerManager() noexcept;

public:
    void init() {}

    void stop();

    void run();

    void sendPacket(Packet* pPacket);

private:
    SharedServerClient* m_pSharedServerClient;

    mutable Mutex m_Mutex;
};

// global variable declaration
extern SharedServerManager* g_pSharedServerManager;

#endif
