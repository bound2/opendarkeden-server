/////////////////////////////////////////////////////////////////////
// Filename		: NetmarbleGuildRegisterThread.h
// Written by	: bezz@darkeden.com
// Description	:
/////////////////////////////////////////////////////////////////////

#ifndef __NETMARBLE_GUILD_REGISTER_THREAD_H__
#define __NETMARBLE_GUILD_REGISTER_THREAD_H__

#include <queue>

#include "Exception.h"
#include "Mutex.h"
#include "Thread.h"
#include "Types.h"


/////////////////////////////////////////////////////////////////////
// class NetmarbleGuildRegisterThread
/////////////////////////////////////////////////////////////////////
class NetmarbleGuildRegisterThread : public Thread {
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
    void run();

public:
    // 넷마블에 등록할 길드 아이디를 쌓는다.
    void pushGuildID(GuildID_t guildID);

    // 길드 정보를 넷마블쪽으로 등록한다.
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
