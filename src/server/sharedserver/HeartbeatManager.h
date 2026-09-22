//////////////////////////////////////////////////////////////////////
//
// Filename    : HeartbeatManager.h
// Written by  : reiot@ewestsoft.com
// Description : Client manager for the login server
//
//////////////////////////////////////////////////////////////////////

#ifndef __HEARTBEAT_MANAGER_H__
#define __HEARTBEAT_MANAGER_H__

// include files
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class HeartbeatManager;
//
// Handles the input and output of every player connected to the login server,
// and handles new client connection attempts.
//
// The client's run() runs on the main thread of the login server process,
// so it need not inherit Thread. Its structure and role are nonetheless
// similar to the other classes that do inherit Thread.
//
//////////////////////////////////////////////////////////////////////

class HeartbeatManager {
public:
    // constructor
    HeartbeatManager();

    // destructor
    ~HeartbeatManager() noexcept(false);

    // Initialize the client manager.
    void init();

    // Start the client manager.
    void start();

    // Stop the client manager.
    void stop();

    // The client manager's main method
    void run();
};

#endif
