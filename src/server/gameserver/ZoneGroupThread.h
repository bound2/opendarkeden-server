//////////////////////////////////////////////////////////////////////
//
// Filename    : ZoneGroupThread.h
// Written by  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

#ifndef __ZONE_THREAD_H__
#define __ZONE_THREAD_H__

#include <chrono>
#include <mutex>

#include <condition_variable>
#include <stop_token>

// include files
#include "CooperativeThread.h"
#include "Exception.h"
#include "Thread.h"
#include "Types.h"
#include "ZoneGroup.h"

//////////////////////////////////////////////////////////////////////
//
// class ZoneGroupThread;
//
// 하나의 존그룹(ZoneGroup)을 맡아서 관리하는 쓰레드로서, 존그룹에
// 종속된 PC, 존에 소속된 NPC와 MOB, 존의 각종 처리.. 등을 전담한다.
// 즉 존그룹에서는 순차 처리가 이루어지게 된다.
//
//////////////////////////////////////////////////////////////////////

class ZoneGroupThread : public Thread {
public:
    // constructor
    ZoneGroupThread(ZoneGroup* pZoneGroup);

    // destructor
    ~ZoneGroupThread() noexcept;

    void start() override;
    void stop() override;
    void join() override;
    void detach() override;

    // main method
    void run() override;

    // get debug string
    string toString() const;

    // get thread's name
    string getName() const {
        return "ZoneGroupThread";
    }

    ZoneGroup* getZoneGroup() {
        return m_pZoneGroup;
    }

private:
    void run(std::stop_token stopToken);

    ZoneGroup* m_pZoneGroup;
    std::mutex m_StopMutex;
    std::condition_variable_any m_StopCondition;

    // Keep this last: its destructor joins before the state above is torn down.
    CooperativeThread m_Worker;
};

#endif
