/////////////////////////////////////////////////////////////////////////////
//
// filename		: SBillingPlayerManager.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __SBILLING_PLAYER_MANAGER__
#define __SBILLING_PLAYER_MANAGER__

// include files
#include "CBillingInfo.h"
#include "Exception.h"
#include "Mutex.h"
#include "PayUser.h"
#include "Socket.h"
#include "Thread.h"
#include "Types.h"

class SBillingPlayer;

/////////////////////////////////////////////////////
// class SBillingPlayerManager
//
// 중국 빌링 서버와 통신을 전담하는 쓰레드
/////////////////////////////////////////////////////
class SBillingPlayerManager : public Thread {
public:
    SBillingPlayerManager();
    ~SBillingPlayerManager() noexcept(false);

public:
    void init() {}
    void stop();
    void run();

    // 빌링 관련 패킷 보내기 함수
    bool sendLogin(PayUser* pPayUser, int i);
    void sendIntervalValidation(int i);
    bool sendMinusPoint(PayUser* pPayUser, int i);
    void sendMinusMinute(PayUser* pPayUser, int i);
    void sendLogout(PayUser* pPayUser, int i);

    int getVersionNumber() const;
    int getMinusIntervalInt() const;
    string getMinusInterval() const;

private:
    SBillingPlayer** m_pSBillingPlayer;
    int m_SBillingPlayers;
    mutable Mutex* m_Mutex;
};

// global variable declaration
extern SBillingPlayerManager* g_pSBillingPlayerManager;

#endif
