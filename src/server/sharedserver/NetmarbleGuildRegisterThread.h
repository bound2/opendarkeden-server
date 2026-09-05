/////////////////////////////////////////////////////////////////////
// Filename		: NetmarbleGuildRegisterThread.h
// Written by	: bezz@darkeden.com
// Description	:
/////////////////////////////////////////////////////////////////////

#ifndef __NETMARBLE_GUILD_REGISTER_THREAD_H__
#define __NETMARBLE_GUILD_REGISTER_THREAD_H__

#include <queue>

#include "Exception.h"
#include "ManagedThread.h"
#include "Mutex.h"
#include "Types.h"


/////////////////////////////////////////////////////////////////////
// class NetmarbleGuildRegisterThread
/////////////////////////////////////////////////////////////////////
class NetmarbleGuildRegisterThread : public ManagedThread {
public:
    // type definition
    typedef queue<GuildID_t> QueueGuildID;

public:
    // constructor & destructor
    NetmarbleGuildRegisterThread();
    ~NetmarbleGuildRegisterThread() noexcept(false);

public:
    // initialize
    void init();

    // thread main loop
    void run() override;

public:
    // Queue a guild id to be registered with Netmarble.
    void pushGuildID(GuildID_t guildID);

    // Register the guild information with Netmarble.
    void registerGuild();

private:
    // Guild ID list for register
    QueueGuildID m_GuildIDs;

    // mutex
    mutable Mutex m_Mutex;
};

// external variable declaration
extern NetmarbleGuildRegisterThread* g_pNetmarbleGuildRegisterThread;

#endif
