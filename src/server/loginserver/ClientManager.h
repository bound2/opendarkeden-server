//////////////////////////////////////////////////////////////////////
//
// Filename    : ClientManager.h
// Written by  : reiot@ewestsoft.com
// Description : Client manager for the login server
//
//////////////////////////////////////////////////////////////////////

#ifndef __LOGIN_CLIENT_MANAGER_H__
#define __LOGIN_CLIENT_MANAGER_H__

// include files
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class ClientManager;
//
// Handles the input and output of every player connected to the login server,
// and handles new client connection attempts.
//
// The client's run() runs on the main thread of the login server process,
// so it need not inherit Thread. Its structure and role are nonetheless
// similar to the other classes that do inherit Thread.
//
// *NOTES*
//
// LoginPlayerManager could be a data member of ClientManager, but it has to
// be a global variable so packet handlers can reach it easily.
//
//////////////////////////////////////////////////////////////////////

class ClientManager {
public:
    // constructor
    ClientManager();

    // destructor
    ~ClientManager() noexcept(false);

    // Initialize the client manager.
    void init();

    // Start the client manager.
    void start();

    // Stop the client manager.
    void stop();

    // The client manager's main method
    void run();
};

// external variable declaration
extern ClientManager* g_pClientManager;

#endif
