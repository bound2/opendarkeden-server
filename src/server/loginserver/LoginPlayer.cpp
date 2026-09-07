//////////////////////////////////////////////////////////////////////
//
// Filename    : LoginPlyaer.cpp
// Written By  : Reiot
// Description :
//
//////////////////////////////////////////////////////////////////////

#include "LoginPlayer.h"

#include "Assert.h"
#include "GameServerInfoManager.h"
#include "GameServerManager.h"
#include "LCLoginError.h"
#include "LCLoginOK.h"
#include "LCPCList.h"
#include "LGKickCharacter.h"
#include "LogClient.h"
#include "Packet.h"
#include "PacketDispatcher.h"
#include "PacketFactoryManager.h"
#include "PacketProfile.h"
#include "PacketValidator.h"
#include "Profile.h"
#include "gameserver/billing/BillingPlayerManager.h"
#include "repository/LoginAccountRepository.h"
#include "repository/LoginCharacterRepository.h"

// by sigi. 2002.11.12
const int defaultLoginPlayerInputStreamSize = 1024;
const int defaultLoginPlayerOutputStreamSize = 4096;

static int maxIdleSec = 60 * 15; // 15 분동안 입력을 하지 않으면 자동 접속 종료된당.

// '이미 접속 중'문제를 해결하기 위한.. 시간 체크
static uint maxWaitForKickCharacter = 3;      // GameServer의 응답을 5초간 기다린다.
static uint maxWaitForKickCharacterCount = 3; // GameServer가 반응이 없으면 3회 응답을 시도한다.


// CLLoginHandler.cpp에 있는 함수다.
void addLoginPlayerData(const string& ID, const string& ip, const string& SSN, const string& zipcode);
void addLogoutPlayerData(Player* pPlayer);


//////////////////////////////////////////////////////////////////////
//
// constructor
//
//////////////////////////////////////////////////////////////////////
LoginPlayer::LoginPlayer(Socket* pSocket)
    : // Player(pSocket),
      m_PlayerStatus(LPS_NONE), m_FailureCount(0) {
    __BEGIN_TRY

    Assert(pSocket != NULL);
    m_pSocket = pSocket;

    // create socket input stream
    m_pInputStream = new SocketInputStream(m_pSocket, defaultLoginPlayerInputStreamSize);

    Assert(m_pInputStream != NULL);

    // create socket output stream
    m_pOutputStream = new SocketOutputStream(m_pSocket, defaultLoginPlayerOutputStreamSize);
    Assert(m_pOutputStream != NULL);


    m_Mutex.setName("LoginPlayer");

    m_ID = "NONE";

    Assert(m_PacketHistory.empty());

    // 로그인 플레이어가 생성될 때, 현재 시간을 최종 입력 시간으로 간주한다.
    getCurrentTime(m_ExpireTime);
    m_ExpireTime.tv_sec += maxIdleSec;

    m_bSetWorldGroupID = false;
    m_WorldID = 1;
    m_ServerGroupID = 0;
    m_LastSlot = 0;

    m_isAdult = true;

    m_KickCharacterCount = 0;

    m_bFreePass = false;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// destructor
//
//////////////////////////////////////////////////////////////////////
LoginPlayer::~LoginPlayer() noexcept {
    __BEGIN_TRY

    // 그 어떤 플레이어 객체가 삭제될 때에도, 그 상태는 로그아웃이어야 한다.
    // 즉 어떤 플레이어를 접속 종료 시키려면, 그 상태를 로그아웃으로 만들어야 한다.
    Assert(m_PlayerStatus == LPS_END_SESSION);

    // delete all previous packets
    while (!m_PacketHistory.empty()) {
        Packet* pPacket = m_PacketHistory.front();
        delete pPacket;
        m_PacketHistory.pop_front();
    }

    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////
//
// '이미 접속 중'인 경우. 캐릭터의 강제 접속 해제를 위해서
// 대기하는 시간 설정.
//
//////////////////////////////////////////////////////////////////////
void LoginPlayer::setExpireTimeForKickCharacter() {
    getCurrentTime(m_ExpireTimeForKickCharacter);

    m_ExpireTimeForKickCharacter.tv_sec += maxWaitForKickCharacter;
}

//////////////////////////////////////////////////////////////////////
//
// parse packet and execute handler for the packet
//
//////////////////////////////////////////////////////////////////////
void LoginPlayer::processCommand(bool Option) {
    __BEGIN_TRY

    //	static Timeval currentTime;

    // '이미 접속 중'인 경우.. 강제 접속 해제를 시킬려고 할 때.
    if (m_PlayerStatus == LPS_WAITING_FOR_GL_KICK_VERIFY) {
        Timeval currentTime;
        getCurrentTime(currentTime);

        // timeout 체크
        if (currentTime >= m_ExpireTimeForKickCharacter) {
            // 다시 KickCharcter를 보내본다.
            sendLGKickCharacter();

            // 반응이 없는 경우 여러번 시도를 해본다.
            // 한계에 도달하면.. GameServer가 죽었다고 판단하고 LoginOK를 보낸다.
            if (++m_KickCharacterCount >= maxWaitForKickCharacterCount) {
                sendLCLoginOK();
            }
        }

        return;
    }

    try {
        // 헤더를 임시저장할 버퍼 생성
        char header[szPacketHeader];
        PacketID_t packetID;
        PacketSize_t packetSize;
        Packet* pPacket;

        // 입력버퍼에 들어있는 완전한 패킷들을 모조리 처리한다.
        while (true) {
            // 입력스트림에서 패킷헤더크기만큼 읽어본다.
            // 만약 지정한 크기만큼 스트림에서 읽을 수 없다면,
            // Insufficient 예외가 발생하고, 루프를 빠져나간다.
            if (!m_pInputStream->peek(header, szPacketHeader)) {
                // 입력이 아무 것도 없었다면, 입력제한 시간을 초과했는지 체크한다.
                Timeval currentTime;
                getCurrentTime(currentTime);
                if (currentTime >= m_ExpireTime)
                    throw DisconnectException("일정 시간동안 입력하지 않으면 접속이 종료됩니다.");
                break;
            }

            // 패킷아이디 및 패킷크기를 알아낸다.
            // 이때 패킷크기는 헤더를 포함한다.
            memcpy(&packetID, &header[0], szPacketID);
            memcpy(&packetSize, &header[szPacketID], szPacketSize);

            /*
            LOG4("RECV PACKET from %s, %s(%d) %d/%d\n",
                m_ID.c_str(),
                g_pPacketFactoryManager->getPacketName( packetID ).c_str(),
                packetID,
                szPacketHeader + packetSize,
                m_pInputStream->length()
            );

            */
            // DEBUG by tiancaiamao
            StringStream msg;
            msg << "RECV PACKET from " << m_ID << ", " << g_pPacketFactoryManager->getPacketName(packetID) << "("
                << packetID << ") " << szPacketHeader + packetSize << "/" << m_pInputStream->length() << eos;
            cout << msg.toString() << endl;

            // 패킷 아이디가 이상하면 프로토콜 에러로 간주한다.
            if (packetID >= Packet::PACKET_MAX)
                // 디버깅을 위해서 에러를 구체적으로 표시해둔다.
                throw InvalidProtocolException("too large packet id");

            try {
                // 패킷의 순서가 valid 한지 체크한다.
                if (!g_pPacketValidator->isValidPacketID(getPlayerStatus(), packetID)) {
                    // DEBUG by tiancaiamao
                    cout << "player status: " << getPlayerStatus() << " receive packet: " << packetID << endl;
                    throw InvalidProtocolException("invalid packet order");
                }

                // 패킷 크기가 너무 크면 프로토콜 에러로 간주한다.
                if (packetSize > g_pPacketFactoryManager->getPacketMaxSize(packetID))
                    throw InvalidProtocolException("too large packet size");

                // 입력버퍼내에 패킷크기만큼의 데이타가 들어있는지 확인한다.
                // 최적화시 break 를 사용하면 된다. (여기서는 일단 exception을 쓸 것이다.)
                if (m_pInputStream->length() < szPacketHeader + packetSize)
                    //	throw InsufficientDataException();
                    break;

                // 최종입력시간을 갱신한다.
                // 최종입력시간은 패킷 하나가 완전하게 도착한 시간을 의미한다.
                getCurrentTime(m_ExpireTime);
                m_ExpireTime.tv_sec += maxIdleSec;

                // 여기까지 왔다면 입력버퍼에는 완전한 패킷 하나 이상이 들어있다는 뜻이다.
                // 패킷팩토리매니저로부터 패킷아이디를 사용해서 패킷 스트럭처를 생성하면 된다.
                // 패킷아이디가 잘못될 경우는 패킷팩토리매니저에서 처리한다.
                pPacket = g_pPacketFactoryManager->createPacket(packetID);

                // 이제 이 패킷스트럭처를 초기화한다.
                // 패킷하위클래스에 정의된 read()가 virtual 메커니즘에 의해서 호출되어
                // 자동적으로 초기화된다.
                m_pInputStream->readPacket(pPacket);

                Timeval start, end;
                getCurrentTime(start);

                // 이제 이 패킷스트럭처를 가지고 패킷핸들러를 수행하면 된다.
                // 패킷아이디가 잘못될 경우는 패킷핸들러매니저에서 처리한다.
                PacketDispatcher::dispatch(pPacket, this);

                getCurrentTime(end);
                g_PacketProfileManager.addAccuTime(pPacket->getPacketName(), start, end);

                // 현재 패킷을 패킷 히스토리의 맨 뒤에 넣는다.
                m_PacketHistory.push_back(pPacket);

                // 패킷을 nPacketHistory 개만큼만 저장한다.
                while (m_PacketHistory.size() > nPacketHistory) {
                    Packet* oldPacket = m_PacketHistory.front();
                    delete oldPacket;
                    m_PacketHistory.pop_front();
                }

            } catch (IgnorePacketException&) {
                // PacketValidator 에서 패킷을 무시하라고 했으니,
                // 입력스트림에서 모두 지워버리고 실행하지 않도록 한다.

                // 패킷 크기가 너무 크면 프로토콜 에러로 간주한다.
                if (packetSize > g_pPacketFactoryManager->getPacketMaxSize(packetID))
                    throw InvalidProtocolException("too large packet size");

                // 입력버퍼내에 패킷크기만큼의 데이타가 들어있는지 확인한다.
                // 최적화시 break 를 사용하면 된다. (여기서는 일단 exception을 ?것이다.)
                if (m_pInputStream->length() < szPacketHeader + packetSize)
                    throw InsufficientDataException();

                // 데이타가 모두 도착했으면, 그 크기만큼 무시하고,
                // 다른 패킷을 처리하도록 한다....
                m_pInputStream->skip(szPacketHeader + packetSize);

                // 무시된 패킷은, expire 에 영향을 주지 않게 된다.
                // 즉 유효한 패킷만이 짤리지 않게 해준다.
                // 또한 히스토리에도 들어가지 않는다.
            }
        }

    } catch (InsufficientDataException& ide) {
        // 입력이 아무 것도 없었다면, 입력제한 시간을 초과했는지 체크한다.
        Timeval currentTime;
        getCurrentTime(currentTime);
        if (currentTime >= m_ExpireTime)
            throw DisconnectException("일정 시간동안 입력하지 않으면 접속이 종료됩니다.");

    } catch (InvalidProtocolException& ipe) {
        // 접속을 강제종료시켜야 한다. 무슨 방법으로??
        throw;

    } catch (DisconnectException& de) {
        // 패킷 처리에서 발생한 어떤 문제로 연결을 종료해야 한다.
        throw;
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// disconnect player
//--------------------------------------------------------------------------------
void LoginPlayer::disconnect(bool bDisconnected) {
    __BEGIN_TRY

    if (bDisconnected == UNDISCONNECTED) {
        // 클라이언트에게 GCDisconnect 패킷을 전송한다.
        // GCDisconnect lcDisconnect;
        // sendPacket( lcDisconnect );

        // 출력 버퍼에 남아있는 데이타를 전송한다.
        m_pOutputStream->flush();
    }

    // 소켓 연결을 닫는다.
    m_pSocket->close();

    // '이미 접속 중'인 경우, 캐릭터 강제 접속 해제를 기다리는 상황.
    if (m_PlayerStatus == LPS_WAITING_FOR_GL_KICK_VERIFY) {
        m_ID = "NONE";
    }

    // 플레이어의 상태를 로그아웃으로 만든다.
    Assert(m_PlayerStatus != LPS_END_SESSION);
    m_PlayerStatus = LPS_END_SESSION;

    // 아이디가 설정되었다는 뜻은, 로그인이 이루어졌다는 뜻이다.
    // '이미 접속 중'인 경우에..
    // 캐릭 접속 해제를 기다리는 경우는 ID가 설정될 수 있으므로 아니다
    if (m_ID != "NONE") {
        try {
            defaultLoginAccountRepository().markLoggedOff(m_ID);

#if defined(__PAY_SYSTEM_LOGIN__) || defined(__PAY_SYSTEM_FREE_LIMIT__)
            bool bClear = false;        // drop the paid-play state entirely
            bool bDecreaseTime = false; // the login server does not count play time down
            logoutPayPlay(m_ID, bClear, bDecreaseTime);
#endif
        } catch (const char*) {
            // A SQL failure arrives as END_DB's const char*, already logged
            // to DBError.log (its own message dangles); rethrown as the
            // Error the callers expect.
            throw Error("LoginPlayer::disconnect : SQL error, see DBError.log");
        }
    }

    addLogoutPlayerData(this);

    __END_CATCH
}
//--------------------------------------------------------------------------------
// disconnect player no log
// DB 에 로그를 쌓지 않게 한다.
//--------------------------------------------------------------------------------
void LoginPlayer::disconnect_nolog(bool bDisconnected) {
    __BEGIN_TRY

    if (bDisconnected == UNDISCONNECTED) {
        // 클라이언트에게 GCDisconnect 패킷을 전송한다.
        // GCDisconnect lcDisconnect;
        // sendPacket( lcDisconnect );

        // 출력 버퍼에 남아있는 데이타를 전송한다.
        m_pOutputStream->flush();
    }

    // 소켓 연결을 닫는다.
    m_pSocket->close();

    // '이미 접속 중'인 경우, 캐릭터 강제 접속 해제를 기다리는 상황.
    if (m_PlayerStatus == LPS_WAITING_FOR_GL_KICK_VERIFY) {
        m_ID = "NONE";
    }

    // 플레이어의 상태를 로그아웃으로 만든다.
    Assert(m_PlayerStatus != LPS_END_SESSION);
    m_PlayerStatus = LPS_END_SESSION;

    // 아이디가 설정되었다는 뜻은, 로그인이 이루어졌다는 뜻이다.
    // '이미 접속 중'인 경우에..
    // 캐릭 접속 해제를 기다리는 경우는 ID가 설정될 수 있으므로 아니다
    if (m_ID != "NONE") {
        try {
            defaultLoginAccountRepository().markLoggedOff(m_ID);

#if defined(__PAY_SYSTEM_LOGIN__) || defined(__PAY_SYSTEM_FREE_LIMIT__)
            bool bClear = false;        // drop the paid-play state entirely
            bool bDecreaseTime = false; // the login server does not count play time down
            logoutPayPlay(m_ID, bClear, bDecreaseTime);
#endif
        } catch (const char*) {
            // A SQL failure arrives as END_DB's const char*, already logged
            // to DBError.log (its own message dangles); rethrown as the
            // Error the callers expect.
            throw Error("LoginPlayer::disconnect : SQL error, see DBError.log");
        }
    }


    __END_CATCH
}

//--------------------------------------------------------------------------------
//
// 원래는 로그인플레이어매니저 외에는 로그인플레이어에 동시에 접속하는
// 쓰레드는 존재하지 않을 계획이었지만, 게임서버매니저가 동시에 쓰레드로
// 돌아가면서 로그인 플레이어에 접근할 가능성이 생겨버렸다. - -; 그래서,
// 아래와 같이 mutex 로 보호되는 버전을 급조했다.
//
//--------------------------------------------------------------------------------
void LoginPlayer::sendPacket(Packet* pPacket) {
    __BEGIN_TRY

    __ENTER_CRITICAL_SECTION(m_Mutex)

    // LOG4("SEND PACKET to %s : %s(%d) %d /%d\n", m_ID.c_str() , pPacket->getPacketName().c_str() ,
    // pPacket->getPacketID(), pPacket->getPacketSize(), m_pOutputStream->length() );
    Player::sendPacket(pPacket);

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// 최근 N 번째의 패킷을 리턴한다.
//
// N == 0 일 경우, 가장 최근의 패킷을 리턴하게 된다.
//
// 최대 nPacketHistory - 1 까지 지정할 수 있다.
//
//////////////////////////////////////////////////////////////////////
Packet* LoginPlayer::getOldPacket(uint prev) {
    __BEGIN_TRY

    if (prev >= nPacketHistory)
        throw OutOfBoundException();
    if (prev >= m_PacketHistory.size())
        throw NoSuchElementException();
    return m_PacketHistory[m_PacketHistory.size() - prev - 1];

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// 특정 패킷아이디를 가진 가장 최근의 패킷을 리턴한다.
//
//////////////////////////////////////////////////////////////////////
Packet* LoginPlayer::getOldPacket(PacketID_t packetID) {
    __BEGIN_TRY

    Packet* pPacket = NULL;

    deque<Packet*>::reverse_iterator ritr = m_PacketHistory.rbegin();
    for (; ritr != m_PacketHistory.rend(); ritr++) {
        if ((*ritr)->getPacketID() == packetID) {
            pPacket = (*ritr);
            break;
        }
    }

    if (pPacket == NULL)
        throw NoSuchElementException();

    return pPacket;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//
// send LGKickCharacter
//
// GameServer로 '이미 접속중'인 캐릭터를 제거해달라고 메세지를 보낸다.
//
//////////////////////////////////////////////////////////////////////////////
void LoginPlayer::sendLGKickCharacter() {
    cout << "send LGKickCharacter" << endl;

    // Game서버로 캐릭터를 제거해달라는 message를 보낸다.
    LGKickCharacter lgKickCharacter;

    string characterName = getLastCharacterName();
    int serverID, serverGroupID, worldID, lastSlot;

    string gameServerIP;
    uint gameServerPort;

    //----------------------------------------------------------------------
    // DB에서 이 player가 최근에 접속한
    // WorldID, ServerID, LastSlot을 얻어내자.
    //----------------------------------------------------------------------
    if (!isSetWorldGroupID()) {
        int currentWorldID = 0;
        int currentServerGroupID = 0;
        int currentLastSlot = 0;

        if (defaultLoginAccountRepository().loadLastLocation(getID(), currentWorldID, currentServerGroupID,
                                                             currentLastSlot)) {
            serverID = 1; // always 1 for now
            worldID = currentWorldID;
            serverGroupID = currentServerGroupID;
            lastSlot = currentLastSlot;

            setWorldID(worldID);
            setGroupID(serverGroupID);
            setLastSlot(lastSlot);
            setWorldGroupID(true); // the values are now set
        }
    } else {
        serverID = 1; // always 1 for now
        worldID = getWorldID();
        serverGroupID = getGroupID();
    }

    //----------------------------------------------------------------------
    // The character in the last slot, when the caller did not name one.
    //----------------------------------------------------------------------
    if (characterName.size() == 0) {
        string name;

        if (defaultLoginCharacterRepository().loadSlayerNameInSlot(m_WorldID, getID(), lastSlot, name)) {
            characterName = name;
            setLastCharacterName(characterName);
        } else {
            cout << "No CharacterName" << endl;

            LCLoginError lcLoginError;
            lcLoginError.setErrorID(ALREADY_CONNECTED);
            sendPacket(&lcLoginError);

            setPlayerStatus(LPS_BEGIN_SESSION);
            setID("NONE"); // keeps disconnect() from writing LOGOFF
            return;
        }
    }

    //----------------------------------------------------------------------
    // GameServer의 정보를 알아낸다.
    //
    // 해당 World 에 모든 Server 에 보낸다
    //----------------------------------------------------------------------
    for (int i = 0; i < g_pGameServerInfoManager->getMaxServerGroupID(); i++) {
        serverGroupID = i;

        try {
            cout << "World=" << worldID << ", " << "Group=" << serverGroupID << ", " << "Server=" << serverID << endl;

            GameServerInfo* pGameServerInfo =
                g_pGameServerInfoManager->getGameServerInfo(serverID, serverGroupID, worldID);

            if (pGameServerInfo != NULL) {
                gameServerIP = pGameServerInfo->getIP();
                gameServerPort = pGameServerInfo->getUDPPort();

                cout << "IP=" << gameServerIP.c_str() << ", Port=" << gameServerPort << endl;
            }
        } catch (NoSuchElementException&) {
            cout << "No GameServerInfo" << endl;
            // LoginError(이미 접속 중)
            //		LCLoginError lcLoginError;
            //		lcLoginError.setErrorID(ALREADY_CONNECTED);
            //		sendPacket(&lcLoginError);
            //		setPlayerStatus(LPS_BEGIN_SESSION);

            setID("NONE"); // disconnect에서 LOGOFF로 설정되지 않게 하기 위해서

            return;
        }

        lgKickCharacter.setID(getSocket()->getSOCKET()); // SocketFD. 검색을 위해서
        lgKickCharacter.setPCName(characterName);

        cout << "( " << gameServerIP.c_str() << ", " << gameServerPort << " )" << endl;
        g_pGameServerManager->sendPacket(gameServerIP, gameServerPort, &lgKickCharacter);
    }

    setExpireTimeForKickCharacter();
    setPlayerStatus(LPS_WAITING_FOR_GL_KICK_VERIFY);
}


//////////////////////////////////////////////////////////////////////
//
// send LCLoginOK
//
// Player table의 LogOn을 'LOGON'으로 바꾸고
// client에게 LCLoginOK를 보낸다.
// PlayerStatus는 LPS_WAITING_FOR_CL_GET_PC_LIST로 설정.
//
//////////////////////////////////////////////////////////////////////
void LoginPlayer::sendLCLoginOK() {
    try {
        string connectIP = getSocket()->getHost();

        // LogOn flips to LOGON; a row that did not change belongs to a
        // session already logged on.
        if (!defaultLoginAccountRepository().setLoggedOn(getID())) {
            filelog("MultiLogin.log", "Multiple login attempt suspected : [%s:%s]", getID().c_str(), connectIP.c_str());

            LCLoginError lcLoginError;
            lcLoginError.setErrorID(ALREADY_CONNECTED);
            sendPacket(&lcLoginError);

            setPlayerStatus(LPS_BEGIN_SESSION);
            return;
        }

        defaultLoginAccountRepository().setLoginIP(connectIP, getID());

        LCLoginOK lcLoginOK;
        lcLoginOK.setAdult(isAdult());
        lcLoginOK.setLastDays(0xffff);

        sendPacket(&lcLoginOK);

        setPlayerStatus(LPS_WAITING_FOR_CL_GET_PC_LIST);

        addLoginPlayerData(m_ID, connectIP, m_SSN, m_Zipcode);
    } catch (Throwable& t) {
        filelog("loginOKError.txt", "%s", t.toString().c_str());
        throw;
    }
}


bool LoginPlayer::sendBillingLogin() {
    __BEGIN_TRY

    if (!m_ID.empty() && m_ID != "NONE") {
        Timeval currentTime;
        getCurrentTime(currentTime);

        if (currentTime > m_BillingNextLoginRequestTime) {
            g_pBillingPlayerManager->sendPayLogin(this);

            // PayLogin 요청한 회수 기억
            m_BillingLoginRequestCount++;

            // 10초 후 다시 체크한다.
            m_BillingNextLoginRequestTime.tv_sec = currentTime.tv_sec + 10;
        }

        return true;
    }

    return false;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// get debug string
//
//////////////////////////////////////////////////////////////////////
string LoginPlayer::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "LoginPlayer(" << "ID:" << m_ID << ",SocketID:" << m_pSocket->getSOCKET() << ",Host:" << m_pSocket->getHost()
        << ")";
    return msg.toString();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//
// add LogoutPlayerdata
//
// 접속자 통계를 위해서
// UserInfo DB의 LogoutPlayerData에 Logout한 사용자를 추가한다.
//
//////////////////////////////////////////////////////////////////////////////
void addLogoutPlayerData(Player* pPlayer) {
    /*if(pPlayer->getID() != "NONE")
    {

        Statement* pStmt = NULL;

        pStmt = g_pDatabaseManager->getUserInfoConnection()->createStatement();

        // 유저 통계 관련 정보를 입력한다.
        BEGIN_DB
        {
            string ID = pPlayer->getID();
            string ip = pPlayer->getSocket()->getHost();

            // 먼저 현재 시간을 얻어낸다.
            int year, month, day, hour, minute, second;
            getCurrentTimeEx(year, month, day, hour, minute, second);
            string currentDT = VSDateTime::currentDateTime().toDateTime();

            StringStream sql;
            sql << "INSERT INTO USERINFO.LogoutPlayerData (PlayerID,IP,Date,Time) VALUES ('"
                << ID << "','" << ip << "','"
                << currentDT.substr( 0, 10 ).c_str() << "','"
                << currentDT.substr( 11 ).c_str() << "')";

            pStmt->executeQuery(sql.toString());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }*/
}

void LoginPlayer::makePCList(LCPCList& lcPCList) {
    WorldID_t WorldID = getWorldID();
    LoginCharacterRepository& repo = defaultLoginCharacterRepository();

    try {
        // Every ACTIVE Slayer row of the account is a character; its Race
        // column says which table holds the rest.
        vector<LoginSlayerListRow> slayers = repo.loadSlayerList(WorldID, getID());

        DWORD shape;
        Color_t colors[PCSlayerInfo::SLAYER_COLOR_MAX];
        Color_t colorsVamp[PCVampireInfo::VAMPIRE_COLOR_MAX];

        for (size_t n = 0; n < slayers.size(); n++) {
            const LoginSlayerListRow& s = slayers[n];
            string race = s.race;
            string name = s.name;

            if (race == "SLAYER") {
                PCSlayerInfo* pPCSlayerInfo = new PCSlayerInfo();

                pPCSlayerInfo->setName(name);
                pPCSlayerInfo->setSlot(s.slot);
                pPCSlayerInfo->setSex(s.sex);
                pPCSlayerInfo->setHairStyle(HAIR_STYLE1);
                pPCSlayerInfo->setHairColor(s.hairColor);
                pPCSlayerInfo->setSkinColor(s.skinColor);
                pPCSlayerInfo->setAdvancementLevel(s.advancementClass);
                pPCSlayerInfo->setSTR(s.str);
                pPCSlayerInfo->setSTRExp(s.strExp);
                pPCSlayerInfo->setDEX(s.dex);
                pPCSlayerInfo->setDEXExp(s.dexExp);
                pPCSlayerInfo->setINT(s.inte);
                pPCSlayerInfo->setINTExp(s.intExp);
                pPCSlayerInfo->setHP(s.hp, s.currentHP);
                pPCSlayerInfo->setMP(s.mp, s.currentMP);
                pPCSlayerInfo->setFame(s.fame);

                for (int j = 0; j < SKILL_DOMAIN_VAMPIRE; j++) {
                    pPCSlayerInfo->setSkillDomainLevel((SkillDomain)j, (SkillLevel_t)s.domainLevel[j]);
                }

                pPCSlayerInfo->setAlignment(s.alignment);

                shape = s.shape;

                colors[PCSlayerInfo::SLAYER_COLOR_HAIR] = pPCSlayerInfo->getHairColor();
                colors[PCSlayerInfo::SLAYER_COLOR_SKIN] = pPCSlayerInfo->getSkinColor();
                colors[PCSlayerInfo::SLAYER_COLOR_HELMET] = s.helmetColor;
                colors[PCSlayerInfo::SLAYER_COLOR_JACKET] = s.jacketColor;
                colors[PCSlayerInfo::SLAYER_COLOR_PANTS] = s.pantsColor;
                colors[PCSlayerInfo::SLAYER_COLOR_WEAPON] = s.weaponColor;
                colors[PCSlayerInfo::SLAYER_COLOR_SHIELD] = s.shieldColor;

                pPCSlayerInfo->setShapeInfo(shape, colors);
                pPCSlayerInfo->setRank(s.rank);

                lcPCList.setPCInfo(pPCSlayerInfo->getSlot(), pPCSlayerInfo);
            } else if (race == "VAMPIRE") {
                LoginVampireListRow v;

                if (!repo.loadVampireListRow(WorldID, getID(), name, v)) {
                    throw DisconnectException("No Vampire");
                }

                PCVampireInfo* pPCVampireInfo = new PCVampireInfo();

                pPCVampireInfo->setName(v.name);
                pPCVampireInfo->setSlot(v.slot);
                pPCVampireInfo->setSex(v.sex);
                pPCVampireInfo->setBatColor(v.batColor);
                pPCVampireInfo->setSkinColor(v.skinColor);
                pPCVampireInfo->setAdvancementLevel(v.advancementClass);
                pPCVampireInfo->setSTR(v.str);
                pPCVampireInfo->setDEX(v.dex);
                pPCVampireInfo->setINT(v.inte);
                pPCVampireInfo->setHP(v.hp, v.currentHP);
                pPCVampireInfo->setRank(v.rank);
                pPCVampireInfo->setExp(v.goalExp);
                pPCVampireInfo->setLevel(v.level);
                pPCVampireInfo->setBonus(v.bonus);
                pPCVampireInfo->setFame(v.fame);
                pPCVampireInfo->setAlignment(v.alignment);

                shape = v.shape;
                colorsVamp[0] = v.coatColor;

                pPCVampireInfo->setShapeInfo(shape, colorsVamp);

                lcPCList.setPCInfo(pPCVampireInfo->getSlot(), pPCVampireInfo);
            } else {
                LoginOustersListRow o;

                if (!repo.loadOustersListRow(WorldID, getID(), name, o)) {
                    throw DisconnectException("No Ousters");
                }

                PCOustersInfo* pPCOustersInfo = new PCOustersInfo();

                pPCOustersInfo->setName(o.name);
                pPCOustersInfo->setSlot(o.slot);
                pPCOustersInfo->setSex(o.sex);
                pPCOustersInfo->setAdvancementLevel(o.advancementClass);
                pPCOustersInfo->setSTR(o.str);
                pPCOustersInfo->setDEX(o.dex);
                pPCOustersInfo->setINT(o.inte);
                pPCOustersInfo->setHP(o.hp, o.currentHP);
                pPCOustersInfo->setRank(o.rank);
                pPCOustersInfo->setExp(o.exp);
                pPCOustersInfo->setLevel(o.level);
                pPCOustersInfo->setBonus(o.bonus);
                pPCOustersInfo->setSkillBonus(o.skillBonus);
                pPCOustersInfo->setFame(o.fame);
                pPCOustersInfo->setAlignment(o.alignment);
                pPCOustersInfo->setCoatType((OustersCoatType)o.coatType);
                pPCOustersInfo->setArmType((OustersArmType)o.armType);
                pPCOustersInfo->setCoatColor(o.coatColor);
                pPCOustersInfo->setHairColor(o.hairColor);
                pPCOustersInfo->setArmColor(o.armColor);
                pPCOustersInfo->setBootsColor(o.bootsColor);

                lcPCList.setPCInfo(pPCOustersInfo->getSlot(), pPCOustersInfo);
            }
        }
    } catch (const char*) {
        // A SQL failure arrives as END_DB's const char*, already logged to
        // DBError.log (its own message dangles); the client is dropped.
        throw DisconnectException("LoginPlayer::makePCList : SQL error, see DBError.log");
    }
}
