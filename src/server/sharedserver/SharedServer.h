//////////////////////////////////////////////////////////////////////
//
// Filename    : SharedServer.h
// Written By  : reiot@ewestsoft.com
// Description : Main class for the login server
//
//////////////////////////////////////////////////////////////////////

#ifndef __SHARED_SERVER_H__
#define __SHARED_SERVER_H__

// Including this module makes it the login server module.
#ifndef __SHARED_SERVER__
#define __SHARED_SERVER__
#endif

// include files
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class SharedServer
//
// Class representing the login server itself.
//
//////////////////////////////////////////////////////////////////////

class SharedServer {
public:
    // constructor
    SharedServer();

    // destructor
    ~SharedServer() noexcept(false);

    // intialize game server
    void init();

    // start game server
    void start();

    // Request every worker to stop, then join them while the managers they
    // use are still alive. Idempotent: main and the startup catch both call it.
    void stop();

private:
    bool m_Stopped = false;
};

// global variable declaration
extern SharedServer* g_pSharedServer;

#endif
