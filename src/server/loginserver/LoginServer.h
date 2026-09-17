//////////////////////////////////////////////////////////////////////
//
// Filename    : LoginServer.h
// Written By  : reiot@ewestsoft.com
// Description : Main class for the login server
//
//////////////////////////////////////////////////////////////////////

#ifndef __LOGIN_SERVER_H__
#define __LOGIN_SERVER_H__

// Including this module makes it the login server module.
#ifndef __LOGIN_SERVER__
#define __LOGIN_SERVER__
#endif

// include files
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////
//
// class LoginServer
//
// Class representing the login server itself.
//
//////////////////////////////////////////////////////////////////////

class LoginServer {
public:
    // constructor
    LoginServer();

    // destructor
    ~LoginServer() noexcept(false);

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

#endif
