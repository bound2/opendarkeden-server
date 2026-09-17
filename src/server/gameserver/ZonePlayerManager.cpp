//
// Filename    : ZonePlayerManager.cpp
// Written by  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "ZonePlayerManager.h"

#include <stdio.h>

#include <algorithm>

#include "Assert.h"
#include "CGLogout.h"
#include "GamePlayer.h"
#include "IncomingPlayerManager.h"
#include "LogClient.h"
#include "PaySystem.h"
#include "PlayerCreature.h"
#include "PlayerMailbox.h"
#include "Profile.h"
#include "ResurrectLocationManager.h"
#include "Slayer.h"
#include "Socket.h"
#include "SocketAPI.h"
#include "StringStream.h"
#include "Thread.h"
#include "Zone.h"
#include "ZoneInfoManager.h"
#include "ZoneUtil.h"
// #include "UserGateway.h"
#include "BroadcastFilter.h"
#include "DefaultOptionSetInfo.h"
#include "GCKickMessage.h"
#include "GCSystemMessage.h"
#include "Party.h"
#include "StringPool.h"
#include "VariableManager.h"
#include "ZoneGroup.h"
#include "repository/MessageRepository.h"


#ifndef __FULL_PROFILE__
#undef beginProfileEx
#define beginProfileEx(name) ((void)0)
#undef endProfileEx
#define endProfileEx(name) ((void)0)
#endif

bool checkZonePlayerManager(GamePlayer* pGamePlayer, ZonePlayerManager* pZPM, const string& str);

//////////////////////////////////////////////////////////////////////////////
// constructor
// 하위 매니저 객체를 생성한다.
//////////////////////////////////////////////////////////////////////////////
ZonePlayerManager::ZonePlayerManager()

    : m_MinFD(-1), m_MaxFD(-1) {
    __BEGIN_TRY

    m_Mutex.setName("ZonePlayerManager");
    m_MutexBroadcast.setName("ZonePlayerManagerBroadcast");
    m_PlayerListQueue.clear();
    m_BroadcastQueue.clear();

    // fd_set 들을 0 으로 초기화한다.
    FD_ZERO(&m_ReadFDs[0]);
    FD_ZERO(&m_WriteFDs[0]);
    FD_ZERO(&m_ExceptFDs[0]);

    // m_Timeout 을 초기화한다.
    // 나중에는 이 주기 역시 옵션으로 처리하도록 하자.
    m_Timeout[0].tv_sec = 0;
    m_Timeout[0].tv_usec = 0;
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////////////
ZonePlayerManager::~ZonePlayerManager() noexcept

{
    __BEGIN_TRY

    // 플레이어 삭제는 PlayerManager 레벨에서 이루어지므로 신경쓰지 않아도 된다.

    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void ZonePlayerManager::broadcastPacket(Packet* pPacket)

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    PlayerManager::broadcastPacket(pPacket);

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void ZonePlayerManager::broadcastPacket_NOBLOCKED(Packet* pPacket)

{
    __BEGIN_TRY

    PlayerManager::broadcastPacket(pPacket);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void ZonePlayerManager::pushBroadcastPacket(Packet* pPacket, BroadcastFilter* pFilter)

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_MutexBroadcast)

    // 여기에 쓰는 패킷이 Encrypter 를 쓰지 않는 다는 전제하에 해놓은 코딩이다.
    // 만일 Encrypter 를 쓰는 패킷을 사용하려면 BroadcastQueue 를 Zone 에 두고
    // 그것을 사용해야한다.

    // 필터와 패킷을 큐에 넣는다.
    // 필터는 새로 생성한 객체(클론)를 넣는다.
    // 패킷을 스트림에 써서 큐에 넣는다.
    SocketOutputStream* pStream = new SocketOutputStream(NULL, szPacketHeader + pPacket->getPacketSize());
    pPacket->writeHeaderNBody(*pStream);

    m_BroadcastQueue.push_back(PairFilterStream(pFilter->Clone(), pStream));

    __LEAVE_CRITICAL_SECTION(m_MutexBroadcast)

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void ZonePlayerManager::flushBroadcastPacket()

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_MutexBroadcast)

    list<PairFilterStream>::iterator itr = m_BroadcastQueue.begin();
    list<PairFilterStream>::iterator endItr = m_BroadcastQueue.end();

    for (; itr != endItr; ++itr) {
        BroadcastFilter* pFilter = itr->first;
        SocketOutputStream* pStream = itr->second;

        if (pStream == NULL) {
            filelog("ZoneBug.txt", "%s : %s", "Zone::flushBroadcastPacket", "pStream이 NULL입니다.");
            continue;
        }

        for (uint i = 0; i < nMaxPlayers; ++i) {
            if (m_pPlayers[i] != NULL) {
                GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(m_pPlayers[i]);
                if (pFilter == NULL || pFilter->isSatisfy(pGamePlayer)) {
                    try {
                        pGamePlayer->sendStream(pStream);
                    } catch (Throwable& t) {
                        filelog("ZonePlayerManager.log", "broadcastPacket: %s", t.toString().c_str());
                    }
                }
            }
        }
    }

    m_BroadcastQueue.clear();

    __LEAVE_CRITICAL_SECTION(m_MutexBroadcast)

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void ZonePlayerManager::copyPlayers()

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    PlayerManager::copyPlayers();

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// call select() system call
// 상위에서 TimeoutException 을 받으면 플레이어는 처리하지 않아도 된다.
//////////////////////////////////////////////////////////////////////////////
void ZonePlayerManager::select() {
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    // m_Timeout[0] 을 m_Timeout[1] 으로 복사한다.
    m_Timeout[1].tv_sec = m_Timeout[0].tv_sec;
    m_Timeout[1].tv_usec = m_Timeout[0].tv_usec;

    // m_XXXFDs[0] 을 m_XXXFDs[1] 으로 복사한다.
    m_ReadFDs[1] = m_ReadFDs[0];
    m_WriteFDs[1] = m_WriteFDs[0];
    m_ExceptFDs[1] = m_ExceptFDs[0];

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    try {
        // 이제 m_XXXFDs[1] 을 가지고 select() 를 호출한다.
        SocketAPI::select_ex(m_MaxFD + 1, &m_ReadFDs[1], &m_WriteFDs[1], &m_ExceptFDs[1], &m_Timeout[1]);
    }
    // 주석처리 by sigi. 2002.5.14
    // do nothing
    catch (InterruptedException& ie) {
        // 시그널이 올 리가 엄찌~~
        log(LOG_GAMESERVER_ERROR, "", "", ie.toString());
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// process all players' inputs
//
// 서버 소켓의 read flag가 켜졌을 경우, 새로운 접속이 들어왔으므로
// 이를 처리하고, 다른 소켓의 read flag가 켜졌을 경우, 새로운 패킷이
// 들어왔으므로 그 플레이어의 processInput()을 호출하면 된다.
//////////////////////////////////////////////////////////////////////////////
void ZonePlayerManager::processInputs() {
    __BEGIN_TRY


    if (m_MinFD == -1 && m_MaxFD == -1) // no player exist
    {
        return;
    }


    for (int i = m_MinFD; i <= m_MaxFD; i++) {
        // ZPM에는 플레이어만 들어있으므로, 더 비교할 꺼리가 없다.
        if (FD_ISSET(i, &m_ReadFDs[1])) {
            if (m_pPlayers[i] != NULL && m_pPlayers[i] == m_pPlayers[i]) {
                GamePlayer* pTempPlayer = dynamic_cast<GamePlayer*>(m_pPlayers[i]);
                Assert(pTempPlayer != NULL);
                Assert(m_pPlayers[i] != NULL);

                if (g_pVariableManager->getVariable(PCROOM_ITEM_RATIO_BONUS) == 100 &&
                    !checkZonePlayerManager(pTempPlayer, this, "PI")) {
                    try {
                        CGLogoutHandler::execute(NULL, pTempPlayer);
                    } catch (DisconnectException& de) {
                        deletePlayer(pTempPlayer->getSocket()->getSOCKET());
                        pushOutPlayer(pTempPlayer);
                    }
                } else if (pTempPlayer->getSocket()->getSockError()) {
                    pTempPlayer->setPenaltyFlag(PENALTY_TYPE_KICKED);
                    pTempPlayer->setItemRatioBonusPoint(7);

                    try {
                        CGLogoutHandler::execute(NULL, pTempPlayer);
                    } catch (DisconnectException& de) {
                        filelog("DIFF_ZG.log", "%s ZPM+PI+SOCKERR", de.toString().c_str());
                        deletePlayer(pTempPlayer->getSocket()->getSOCKET());
                        pushOutPlayer(pTempPlayer);
                    }


                } else {
                    try {
                        pTempPlayer->processInput();
                    } catch (ConnectException& ce) {
                        pTempPlayer->setPenaltyFlag(PENALTY_TYPE_KICKED);
                        pTempPlayer->setItemRatioBonusPoint(8);

                        try {
                            CGLogoutHandler::execute(NULL, pTempPlayer);
                        } catch (DisconnectException& de) {
                            filelog("DIFF_ZG.log", "%s ZPM+PI+CE", de.toString().c_str());
                            deletePlayer(pTempPlayer->getSocket()->getSOCKET());
                            pushOutPlayer(pTempPlayer);
                        }


                    } catch (IOException& ioe) {
                        pTempPlayer->setPenaltyFlag(PENALTY_TYPE_KICKED);
                        pTempPlayer->setItemRatioBonusPoint(9);

                        try {
                            CGLogoutHandler::execute(NULL, pTempPlayer);
                        } catch (DisconnectException& de) {
                            filelog("DIFF_ZG.log", "%s ZPM+PI+IOE", de.toString().c_str());
                            deletePlayer(pTempPlayer->getSocket()->getSOCKET());
                            pushOutPlayer(pTempPlayer);
                        }
                    }
                }
            }
        }
    }


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// process all players' commands
//////////////////////////////////////////////////////////////////////////////
void ZonePlayerManager::processCommands() {
    __BEGIN_TRY
    __BEGIN_DEBUG

    // test code

    if (m_MinFD == -1 && m_MaxFD == -1) // no player exist
    {
        return;
    }


    VSDateTime currentDateTime(VSDate::currentDate(), VSTime::currentTime());

    Timeval currentTime;
    getCurrentTime(currentTime);

    for (int i = m_MinFD; i <= m_MaxFD; i++) {
        if (m_pPlayers[i] != NULL) {
            GamePlayer* pTempPlayer = dynamic_cast<GamePlayer*>(m_pPlayers[i]);
            Assert(pTempPlayer);
            Assert(m_pPlayers[i] != NULL);

            if (pTempPlayer->getSocket()->getSockError()) {
                pTempPlayer->setPenaltyFlag(PENALTY_TYPE_KICKED);
                pTempPlayer->setItemRatioBonusPoint(10);

                try {
                    CGLogoutHandler::execute(NULL, pTempPlayer);
                } catch (DisconnectException& de) {
                    filelog("DIFF_ZG.log", "%s ZPM+PC+SOCKERR", de.toString().c_str());
                    deletePlayer(pTempPlayer->getSocket()->getSOCKET());
                    pushOutPlayer(pTempPlayer);
                }


            } else {
                bool IsPayPlayEnd = false;

                // This manager owns pTempPlayer and the group mutex is held:
                // run what other threads posted for it (PlayerMailbox.h)
                // before its own packets.
                de::drainPlayerMailbox(*pTempPlayer, *this);

                try {
                    beginProfileEx("ZPM_PACKET");
                    pTempPlayer->processCommand();
                    endProfileEx("ZPM_PACKET");

                    if (g_pVariableManager->getVariable(PCROOM_ITEM_RATIO_BONUS) == 100 &&
                        !checkZonePlayerManager(pTempPlayer, this, "PC")) {
                        try {
                            CGLogoutHandler::execute(NULL, pTempPlayer);
                        } catch (DisconnectException& de) {
                            deletePlayer(pTempPlayer->getSocket()->getSOCKET());
                            pushOutPlayer(pTempPlayer);
                        }
                    }

                    // 정상적인 게임 상태에서(GPS_NORMAL)만 Pay체크를 한다.
                    // PCManager::killCreature()에서는 GPS_IGNORE_ALL로 바뀌고
                    // tile에서 지우고.. zone이동이 되므로.. 이걸로 문제가 생길 수 있다고 본다.
                    // by sigi. 2002.12.10
                    else if (pTempPlayer->getPlayerStatus() == GPS_NORMAL) {
                        // 패밀리 요금제 적용이 끝난 경우. 유료존에 있는 무료 파티원들을 무료존으로 옮겨야한다.
                        if (pTempPlayer->isFamilyFreePassEnd()) {
                            Creature* pCreature = pTempPlayer->getCreature();
                            Zone* pZone = pCreature->getZone();
                            Assert(pZone != NULL);

                            if (pZone->isPayPlay()) {
                                // 무료 사용자일 경우 아래 if 문에서 유료 체크를 하고 무료존으로 옮겨간다.
                                pTempPlayer->setPremiumPlay();
                            }
                        }

                        // 유료 사용자인 경우는 시간을 줄인다.
                        // 패밀리 요금 사용자인 경우 시간이 다되었는지 확인한다. 유무료존에 상관없이
                        if ((pTempPlayer->isPayPlaying() || pTempPlayer->isPremiumPlay() ||
                             pTempPlayer->isFamilyPayAvailable()) &&
                            !pTempPlayer->updatePayPlayTime(pTempPlayer->getID(), currentDateTime, currentTime)) {
                            Creature* pCreature = pTempPlayer->getCreature();
                            Zone* pZone = pCreature->getZone();
                            Assert(pZone != NULL);

                            // 유료 서비스 종료
                            pTempPlayer->logoutPayPlay(pTempPlayer->getID());

                            // 패밀리 요금 사용자인 경우 FamilyPayAvailable flag 을 꺼준다.
                            // 패밀리 요금 디폴트 옵션을 끊다.
                            if (pTempPlayer->isFamilyPayAvailable()) {
                                pTempPlayer->setFamilyPayAvailable(false);

                                // 파티원일 경우 Family Pay를 refresh 한다.
                                int PartyID = pCreature->getPartyID();
                                if (PartyID != 0) {
                                    g_pGlobalPartyManager->refreshFamilyPay(PartyID);
                                }

                                PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
                                Assert(pPC != NULL);

                                pPC->removeDefaultOptionSet(DEFAULT_OPTION_SET_FAMILY_PAY);
                                pPC->setFlag(Effect::EFFECT_CLASS_INIT_ALL_STAT);
                            }

                            // by sigi. 2002.12.30
                            IsPayPlayEnd = true;
                        }

                        // 패밀리 요금제 적용이 끝났다면, 다시 체크하지 않게 하기위에 타입을 바꿔준다.
                        if (pTempPlayer->isFamilyFreePassEnd()) {
                            pTempPlayer->setFamilyPayPartyType(FAMILY_PAY_PARTY_TYPE_NONE);
                        }
                    }
                } catch (ProtocolException& pe) {
                    pTempPlayer->setPenaltyFlag(PENALTY_TYPE_KICKED);
                    pTempPlayer->setItemRatioBonusPoint(11);

                    try {
                        CGLogoutHandler::execute(NULL, pTempPlayer);
                    } catch (DisconnectException& de) {
                        filelog("DIFF_ZG.log", "%s ZPM+PC+PE", de.toString().c_str());
                        deletePlayer(pTempPlayer->getSocket()->getSOCKET());
                        pushOutPlayer(pTempPlayer);
                    }

                    // by sigi. 2002.12.30
                    if (IsPayPlayEnd) {
                    } else {
                    }
                }
            }
        }
    }


    __END_DEBUG
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// process all players' outputs
//
//////////////////////////////////////////////////////////////////////
void ZonePlayerManager::processOutputs() {
    __BEGIN_TRY


    if (m_MinFD == -1 && m_MaxFD == -1) // no player exist
    {
        return;
    }


    for (int i = m_MinFD; i <= m_MaxFD; i++) {
        if (FD_ISSET(i, &m_WriteFDs[1])) {
            if (m_pPlayers[i] != NULL) {
                GamePlayer* pTempPlayer = dynamic_cast<GamePlayer*>(m_pPlayers[i]);
                Assert(pTempPlayer);
                Assert(m_pPlayers[i] != NULL);

                if (pTempPlayer->getSocket()->getSockError()) {
                    pTempPlayer->setPenaltyFlag(PENALTY_TYPE_KICKED);
                    pTempPlayer->setItemRatioBonusPoint(12);

                    try {
                        CGLogoutHandler::execute(NULL, pTempPlayer);
                    } catch (DisconnectException& de) {
                        filelog("DIFF_ZG.log", "%s ZPM+PO+SOCKERR", de.toString().c_str());
                        deletePlayer(pTempPlayer->getSocket()->getSOCKET());
                        pushOutPlayer(pTempPlayer);
                    }

                } else {
                    try {
                        pTempPlayer->processOutput();
                    } catch (ConnectException& ce) {
                        pTempPlayer->setPenaltyFlag(PENALTY_TYPE_KICKED);
                        pTempPlayer->setItemRatioBonusPoint(13);

                        try {
                            CGLogoutHandler::execute(NULL, pTempPlayer);
                        } catch (DisconnectException& de) {
                            filelog("DIFF_ZG.log", "%s ZPM+PO+CE", de.toString().c_str());
                            deletePlayer(pTempPlayer->getSocket()->getSOCKET());
                            pushOutPlayer(pTempPlayer);
                        }


                    } catch (ProtocolException& cp) {
                        pTempPlayer->setPenaltyFlag(PENALTY_TYPE_KICKED);
                        pTempPlayer->setItemRatioBonusPoint(14);

                        try {
                            CGLogoutHandler::execute(NULL, pTempPlayer);
                        } catch (DisconnectException& de) {
                            filelog("DIFF_ZG.log", "%s ZPM+PO+PE", de.toString().c_str());
                            deletePlayer(pTempPlayer->getSocket()->getSOCKET());
                            pushOutPlayer(pTempPlayer);
                        }
                    }
                }
            }
        }
    }


    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// process all players' exceptions
//
// 현재까지는 OOB 데이타를 전송할 계획은 없다.
// 따라서, 만약 OOB가 켜져 있다면 에러로 간주하고 접속을 확 짤라 버린다.
//
//////////////////////////////////////////////////////////////////////
void ZonePlayerManager::processExceptions() {
    __BEGIN_TRY


    if (m_MinFD == -1 && m_MaxFD == -1) // no player exist
    {
        return;
    }

    for (int i = m_MinFD; i <= m_MaxFD; i++) {
        if (FD_ISSET(i, &m_ExceptFDs[1])) {
            if (m_pPlayers[i] != NULL && m_pPlayers[i] == m_pPlayers[i]) {
                GamePlayer* pTempPlayer = dynamic_cast<GamePlayer*>(m_pPlayers[i]);
                Assert(pTempPlayer != NULL);
                Assert(m_pPlayers[i] != NULL);

                pTempPlayer->setPenaltyFlag(PENALTY_TYPE_KICKED);
                pTempPlayer->setItemRatioBonusPoint(15);

                try {
                    CGLogoutHandler::execute(NULL, pTempPlayer);
                } catch (DisconnectException& de) {
                    filelog("DIFF_ZG.log", "%s ZPM+PE", de.toString().c_str());
                    deletePlayer(pTempPlayer->getSocket()->getSOCKET());
                    pushOutPlayer(pTempPlayer);
                }
            }
        }
    }


    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 특정 플레이어를 매니저에 추가한다.
//////////////////////////////////////////////////////////////////////
void ZonePlayerManager::addPlayer(GamePlayer* pGamePlayer) {
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    PlayerManager::addPlayer(pGamePlayer);


    SOCKET fd = pGamePlayer->getSocket()->getSOCKET();

    // m_MinFD , m_MaxFD 를 재조정한다.
    if (m_MinFD == -1 && m_MaxFD == -1) {
        // 최초의 플레이어의 경우
        m_MinFD = m_MaxFD = fd;
    } else {
        m_MinFD = min(fd, m_MinFD);
        m_MaxFD = max(fd, m_MaxFD);
    }

    // 모든 fd_set 에 fd 비트를 on 시킨다.
    // m_XXXFDs[1] 은 다음번에 처리해주면 된다.
    FD_SET(fd, &m_ReadFDs[0]);
    FD_SET(fd, &m_WriteFDs[0]);
    FD_SET(fd, &m_ExceptFDs[0]);

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// 특정 플레이어를 매니저에 추가한다.
//////////////////////////////////////////////////////////////////////
void ZonePlayerManager::addPlayer_NOBLOCKED(GamePlayer* pGamePlayer) {
    __BEGIN_TRY

    PlayerManager::addPlayer(pGamePlayer);


    SOCKET fd = pGamePlayer->getSocket()->getSOCKET();

    // m_MinFD , m_MaxFD 를 재조정한다.
    if (m_MinFD == -1 && m_MaxFD == -1) {
        // 최초의 플레이어의 경우
        m_MinFD = m_MaxFD = fd;
    } else {
        m_MinFD = min(fd, m_MinFD);
        m_MaxFD = max(fd, m_MaxFD);
    }

    // 모든 fd_set 에 fd 비트를 on 시킨다.
    // m_XXXFDs[1] 은 다음번에 처리해주면 된다.
    FD_SET(fd, &m_ReadFDs[0]);
    FD_SET(fd, &m_WriteFDs[0]);
    FD_SET(fd, &m_ExceptFDs[0]);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// 특정 플레이어를 매니저에서 삭제한다.
//////////////////////////////////////////////////////////////////////
void ZonePlayerManager::deletePlayer_NOBLOCKED(SOCKET fd) {
    __BEGIN_TRY

    // 플레이어 포인터를 플레이어 배열에서 삭제한다.
    PlayerManager::deletePlayer(fd);

    Assert(m_pPlayers[fd] == NULL);

    // m_MinFD , m_MaxFD 를 재조정한다.
    // fd == m_MinFD && fd == m_MaxFD 인 경우는 첫번째 if 에서 처리된다.
    if (fd == m_MinFD) {
        // 앞에서부터 제일 작은 fd 를 찾는다.
        // m_MinFD 자리는 현재 NULL 이 되어 있음을 유의하라.
        int i = m_MinFD;
        for (i = m_MinFD; i <= m_MaxFD; i++) {
            if (m_pPlayers[i] != NULL) {
                m_MinFD = i;
                break;
            }
        }

        // 적절한 m_MinFD를 찾지 못했을 경우,
        // 이때에는 m_MinFD == m_MaxFD 인 경우이다.
        // 이때에는 둘 다 -1 로 설정해주자.
        if (i > m_MaxFD)
            m_MinFD = m_MaxFD = -1;

    } else if (fd == m_MaxFD) {
        // 뒤에서부터 가장 큰 fd 를 찾는다.
        int i = m_MaxFD;
        for (i = m_MaxFD; i >= m_MinFD; i--) {
            if (m_pPlayers[i] != NULL) {
                m_MaxFD = i;
                break;
            }
        }

        // 적절한 m_MinFD를 찾지 못했을 경우,
        if (i < m_MinFD) {
            filelog("ZonePlayerManagerBug.txt", "%s : %s", "ZonePlayerManager::deletePlayer_NOBLOCKED()",
                    "MinMaxFD problem");
            throw UnknownError("m_MinFD & m_MaxFD problem.");
        }
    }

    // 모든 fd_set 에 fd 비트를 off 시킨다.
    // m_XXXFDs[1]도 고쳐야 하는 이유는, 이후 처리에서 객체가 없어졌는데도
    // 처리받을 확률이 있기 때문이다.
    FD_CLR(fd, &m_ReadFDs[0]);
    FD_CLR(fd, &m_ReadFDs[1]);
    FD_CLR(fd, &m_WriteFDs[0]);
    FD_CLR(fd, &m_WriteFDs[1]);
    FD_CLR(fd, &m_ExceptFDs[0]);
    FD_CLR(fd, &m_ExceptFDs[1]);


    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 특정 플레이어를 매니저에서 삭제한다.
//////////////////////////////////////////////////////////////////////
void ZonePlayerManager::deletePlayer(SOCKET fd) {
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    try {
        // 플레이어 포인터를 플레이어 배열에서 삭제한다.
        PlayerManager::deletePlayer(fd);
    } catch (OutOfBoundException& o) {
        filelog("ZPMError.txt", "OOB: %s, Socket: %d", o.toString().c_str(), fd);
        throw;
    } catch (NoSuchElementException& n) {
        filelog("ZPMError.txt", "NSEE: %s, Socket: %d", n.toString().c_str(), fd);
        throw;
    } catch (Error& e) {
        filelog("ZPMError.txt", "Error: %s, Socket: %d", e.toString().c_str(), fd);
        throw;
    } catch (...) {
        filelog("ZPMError.txt", "난 몰라. Socket: %d", fd);
        throw;
    }

    Assert(m_pPlayers[fd] == NULL);

    // m_MinFD , m_MaxFD 를 재조정한다.
    // fd == m_MinFD && fd == m_MaxFD 인 경우는 첫번째 if 에서 처리된다.
    if (fd == m_MinFD) {
        // 앞에서부터 제일 작은 fd 를 찾는다.
        // m_MinFD 자리는 현재 NULL 이 되어 있음을 유의하라.
        int i = m_MinFD;
        for (i = m_MinFD; i <= m_MaxFD; i++) {
            if (m_pPlayers[i] != NULL) {
                m_MinFD = i;
                break;
            }
        }

        // 적절한 m_MinFD를 찾지 못했을 경우,
        // 이때에는 m_MinFD == m_MaxFD 인 경우이다.
        // 이때에는 둘 다 -1 로 설정해주자.
        if (i > m_MaxFD)
            m_MinFD = m_MaxFD = -1;

    } else if (fd == m_MaxFD) {
        // 뒤에서부터 가장 큰 fd 를 찾는다.
        int i = m_MaxFD;
        for (i = m_MaxFD; i >= m_MinFD; i--) {
            if (m_pPlayers[i] != NULL) {
                m_MaxFD = i;
                break;
            }
        }

        // 적절한 m_MinFD를 찾지 못했을 경우,
        if (i < m_MinFD) {
            filelog("ZonePlayerManagerBug.txt", "%s : %s", "ZonePlayerManager::deletePlayer()", "MinMaxFD problem");
            throw UnknownError("m_MinFD & m_MaxFD problem.");
        }
    }

    // 모든 fd_set 에 fd 비트를 off 시킨다.
    // m_XXXFDs[1]도 고쳐야 하는 이유는, 이후 처리에서 객체가 없어졌는데도
    // 처리받을 확률이 있기 때문이다.
    FD_CLR(fd, &m_ReadFDs[0]);
    FD_CLR(fd, &m_ReadFDs[1]);
    FD_CLR(fd, &m_WriteFDs[0]);
    FD_CLR(fd, &m_WriteFDs[1]);
    FD_CLR(fd, &m_ExceptFDs[0]);
    FD_CLR(fd, &m_ExceptFDs[1]);

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
Player* ZonePlayerManager::getPlayer(SOCKET fd) {
    __BEGIN_TRY

    Player* pPlayer = NULL;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    pPlayer = PlayerManager::getPlayer(fd);

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    return pPlayer;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
Player* ZonePlayerManager::getPlayerByPhoneNumber(PhoneNumber_t PhoneNumber) {
    __BEGIN_TRY
    return NULL;
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
// 전체 사용자들의 세이브를 담당하는 루틴.
// 다른 쓰레드에서 접근 할 수 있으므로 락을 걸어 줘야 하나..
// 현재 쓰레드의 Event처리를 하면서 save를 할 수 있으므로...
// 하위 save에 Lock을 걸어주고 이 루틴에서는 함수만 호출하도록 한다.
// ZonePlayerManager:: save()
//////////////////////////////////////////////////////////////////////
void ZonePlayerManager::save()

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    unsigned int i;

    for (i = 0; i < nMaxPlayers; i++) {
        if (m_pPlayers[i] != NULL) {
            Assert(m_pPlayers[i] != NULL); // by sigi

            Creature* pCreature = ((GamePlayer*)m_pPlayers[i])->getCreature();

            Assert(pCreature != NULL); // by sigi

            pCreature->save();
        }
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

void ZonePlayerManager::pushPlayer(GamePlayer* pGamePlayer)

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    m_PlayerListQueue.push_back(pGamePlayer);

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

void ZonePlayerManager::pushOutPlayer(GamePlayer* pGamePlayer)

{
    __BEGIN_TRY

    m_PlayerOutListQueue.push_back(pGamePlayer);

    __END_CATCH
}

void ZonePlayerManager::processPlayerListQueue()

{
    __BEGIN_TRY

    // PlayerQueue의 Player를 메니져에 추가한다.
    while (!m_PlayerListQueue.empty()) {
        GamePlayer* pGamePlayer = m_PlayerListQueue.front();

        if (pGamePlayer == NULL) {
            filelog("ZoneBug.txt", "%s : %s", "Zone::heartbeat(1)", "pGamePlayer가 NULL입니다.");
            continue;
        }

        addPlayer_NOBLOCKED(pGamePlayer);

        m_PlayerListQueue.pop_front();

        Creature* pCreature = pGamePlayer->getCreature();

        Assert(pCreature != NULL);

        // 새로 들어갈 Zone.. by sigi. 2002.5.11
        Zone* pZone = pCreature->getZone();
        Assert(pZone != NULL);


        pZone->addPC(pCreature, pCreature->getX(), pCreature->getY(), DOWN);
    }

    __END_CATCH
}

void ZonePlayerManager::heartbeat()

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    processPlayerListQueue();

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    // 나갈 대기열에 있는 사람을 처리 해 준다.
    // 기냥 IPM으로 Push하믄 끝이다.
    while (!m_PlayerOutListQueue.empty()) {
        GamePlayer* pGamePlayer = m_PlayerOutListQueue.front();

        m_PlayerOutListQueue.pop_front();

        Assert(pGamePlayer != NULL);

        g_pIncomingPlayerManager->pushPlayer(pGamePlayer);
    }

    // broadcast packet queue 를 처리한다.
    if (!m_BroadcastQueue.empty())
        flushBroadcastPacket();

    __END_CATCH
}

void ZonePlayerManager::deleteQueuePlayer(GamePlayer* pGamePlayer) {
    __BEGIN_TRY

    // 필요없는 lock인거 같다.
    // 제거 by sigi. 2002.5.9

    Assert(pGamePlayer != NULL);

    list<GamePlayer*>::iterator itr =
        find_if(m_PlayerOutListQueue.begin(), m_PlayerOutListQueue.end(), isSamePlayer(pGamePlayer));

    if (itr != m_PlayerOutListQueue.end()) {
        m_PlayerOutListQueue.erase(itr);
    }


    __END_CATCH
}

void ZonePlayerManager::removeFlag(Effect::EffectClass EC)

{
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    for (uint i = 0; i < nMaxPlayers; i++) {
        Player* pPlayer = m_pPlayers[i];

        if (pPlayer != NULL) {
            GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPlayer);
            Assert(pGamePlayer != NULL);

            Creature* pCreature = pGamePlayer->getCreature();
            Assert(pCreature != NULL);

            pCreature->removeFlag(EC);
        }
    }

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////
// ZonePlayerManager 에 있는 모든 사용자를 정리한다.
////////////////////////////////////////////////////////////////////////
void ZonePlayerManager::clearPlayers()

{
    __BEGIN_TRY

    // PlayerListQueue 에 있는 애들을 정리한다.
    while (!m_PlayerListQueue.empty()) {
        GamePlayer* pGamePlayer = m_PlayerListQueue.front();

        m_PlayerListQueue.pop_front();

        if (pGamePlayer != NULL) {
            try {
                pGamePlayer->disconnect();
            } catch (Throwable& t) {
                // 무시
            }

            SAFE_DELETE(pGamePlayer);
        }
    }

    // PlayerOutListQueue 에 있는 애들을 정리한다.
    while (!m_PlayerOutListQueue.empty()) {
        GamePlayer* pGamePlayer = m_PlayerOutListQueue.front();

        m_PlayerOutListQueue.pop_front();

        if (pGamePlayer != NULL) {
            try {
                pGamePlayer->disconnect();
            } catch (Throwable& t) {
                // 무시
            }

            SAFE_DELETE(pGamePlayer);
        }
    }

    if (m_MinFD == -1 && m_MaxFD == -1)
        return;

    // 플레이어를 정리한다.
    for (int i = m_MinFD; i <= m_MaxFD; i++) {
        if (m_pPlayers[i] != NULL) {
            GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(m_pPlayers[i]);

            if (pGamePlayer != NULL) {
                try {
                    pGamePlayer->disconnect();
                } catch (Throwable& t) {
                    // 무시
                }

                SAFE_DELETE(pGamePlayer);
            }
        }
    }

    __END_CATCH
}

bool checkZonePlayerManager(GamePlayer* pGamePlayer, ZonePlayerManager* pZPM, const string& str) {
    if (pGamePlayer == NULL)
        return true;

    Creature* pCreature = pGamePlayer->getCreature();
    if (pCreature == NULL)
        return true;

    Zone* pZone = pCreature->getZone();
    if (pZone == NULL)
        return true;

    ZoneGroup* pZoneGroup = pZone->getZoneGroup();
    if (pZoneGroup == NULL)
        return true;

    ZonePlayerManager* pZonePlayerManager = pZoneGroup->getZonePlayerManager();
    if (pZonePlayerManager == NULL)
        return true;

    if (pZPM != pZonePlayerManager) {
        filelog("ZPMCheck.log", "CZPM:%u GZPM:%u SOCK:%d ZID:%u NAME:%s P:%s", pZPM->getZGID(),
                pZonePlayerManager->getZGID(), pGamePlayer->getSocket()->getSOCKET(), pZone->getZoneID(),
                pCreature->getName().c_str(), str.c_str());

        return false;
    }

    return true;
}
