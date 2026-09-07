//////////////////////////////////////////////////////////////////////////////
// Filename    : CLLoginHandler.cpp
// Written By  : Reiot
// Description :
//
// 이 패킷은 클라이언트가 아이디와 패스워드를 암호화해서
// 로그인 서버로 전송한다. 로그인 서버는 이 패킷을 받아서
// 플레이어의 아이디와 패스워드가 정확한지 DB로부터 읽어서
// 비교한 후, 로그인의 성공 여부를 전송한다.
//
// *CAUTION*
//
// 특정 아이디와 패스워드를 가진 플레이어를 검색하는 SQL 문으로 어떤 것이
// 더 효율적일까?
//
// (1) SELECT Password FROM Player WHERE ID = 'AAA' 으로 검색한 후,
//     패스워드를 비교한다.
// (2) SELECT ID FROM Player WHERE ID = 'AAA' AND Password = 'BBB' 으로
//     검색해서 리턴하는 row 가 있는지 체크한다.
//
// 이에 따라서, 인덱스를 어떻게 설정하는지가 결정되겠다.
//
// (1) - CREATE INDEX PlayerIDIndex ON Player (ID)
// (2) - CREATE INDEX PlayerIDPasswordIndex ON Player (ID , Password)
//
// 현재의 선택은 (2) 되겠다.
//
// *CAUTION*
//
// 같은 플레이어의 동시 접속을 막기 위해서 Player 테이블의 LogOn 컬럼값을
// 체크해야 한다. 만약 LogOn = 'LOGON' 일 경우, 이미 접속하고 있다고 간주
// 해야 하므로, 이런 사람은 접속을 차단해야 한다. (물론 적절한 메시지를
// 출력해줘야 한다.)
//
// 주의할 점은, 서버가 crash 될 경우 새로 띄워질때 LogOn 필드를 모두
// LOGOFF 로 초기화해줘야 한다는 점이다.
//////////////////////////////////////////////////////////////////////////////
/*

   // 넷마블의 Player table에 필요한것 정리. by sigi. 2002.10.23

   PlayerID,
   Password,	// 다른 의미.
   CurrentWorldID,
   CurrentServerGroupID,
   CurrentLoginServerID,
   SpecialEventCount,
   LogOn,
   Access,
   LoginIP,
   PayType, PayPlayDate, PayPlayHours, PayPlayFlag
   LastSlot,
   LastLoginDate,
   LoginIP


   // 넷마블에서 작업해줘야하는 것
   UPDATE Player SET Password='12345678' WHERE PlayerID='playerid';

   if (getAffectedRowCount()==0)
   {
        INSERT INTO Player (PlayerID, Password) Values ('playerid', '12345678');
   }


*/

#include "CLLogin.h"

#ifdef __LOGIN_SERVER__
#include <time.h>

#include <exception>

#include <sys/time.h>

#include "Assert1.h"
#include "GameServerGroupInfoManager.h"
#include "GameServerInfoManager.h"
#include "LCLoginError.h"
#include "LCLoginOK.h"
#include "LoginPlayer.h"
#include "PasswordHash.h"
#include "Properties.h"
#include "UserInfoManager.h"
#include "gameserver/billing/BillingPlayerManager.h"
#include "repository/LoginAccountRepository.h"
#include "types/ServerType.h"

#endif

#define SYMBOL_TEST_CLIENT '#'       // 사내테스트 버전인 경우
#define SYMBOL_NET_MARBLE_CLIENT '@' // 넷마블에서 접속하는 경우

bool isAdultByBirthday(const string& birthday);
void addLoginPlayerData(const string& ID, const string& ip, const string& SSN, const string& zipcode);

bool isBlockIP(const string& ip);

#ifdef __LOGIN_SERVER__
namespace {

// Rewrites the account's stored password as a fresh argon2id hash. A hashing
// failure is logged and otherwise ignored: the password was already accepted
// against the stored value, and the next login retries. A SQL failure leaves
// as END_DB's const char*.
void storePasswordHash(const string& ID, const string& password) {
    string hashed;
    try {
        hashed = de::password::hash(password);
    } catch (const std::exception& e) {
        filelog("loginfail.txt", "Password rehash failed, PlayerID : %s : %s", ID.c_str(), e.what());
        return;
    }
    defaultLoginAccountRepository().updatePassword(hashed, ID);
}

// A hash of a throwaway string under the current parameters. It is verified
// in place of a missing row so that an unknown account costs the same time
// as a wrong password and the two cannot be told apart by reply latency.
constexpr const char* kDecoyHash =
    "$argon2id$v=19$m=65536,t=3,p=1$lCuWLGlkLo6TwTznmqikVg$73WRQ/vIrr1aYRbsplMBD4KT9aNOiTeTJr3EjQRIVS8";

// Checks the password against the Player row. Returns false for an unknown
// account as well as a wrong password, so the caller answers both alike. A
// legacy plaintext row, or a hash under older parameters, is rewritten as a
// current hash on success.
bool checkStoredPassword(const string& ID, const string& password) {
    string stored;
    if (!defaultLoginAccountRepository().loadPasswordHash(ID, stored)) {
        de::password::verify(kDecoyHash, password);
        return false;
    }

    const de::password::Verify verdict = de::password::verify(stored, password);
    if (verdict == de::password::Verify::Rejected)
        return false;
    if (verdict == de::password::Verify::AcceptedRehash)
        storePasswordHash(ID, password);
    return true;
}

} // namespace
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CLLoginHandler::execute(CLLogin* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    // cout << pPacket->toString().c_str() << endl;

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);

    // 좌우 공백 제거. by sigi. 2002.12.6
    pPacket->setID(trim(pPacket->getID()));

    string connectIP = pLoginPlayer->getSocket()->getHost();
    string connectMAC = pPacket->getMacAddress();
    string ID = pPacket->getID();

    // MAC address setting
    pLoginPlayer->setMacAddress(pPacket->getRareMacAddress());

    bool bFreePass = false; // by sigi. 2002.10.23j

    // web login
    bool bWebLogin = pPacket->isWebLogin();
    //	static bool bWebLogin = g_pConfig->getPropertyInt("WebLogin") != 0;

    // set web login player
    if (bWebLogin)
        pLoginPlayer->setWebLogin();

    //	cout << pPacket->toString() << endl;

    if (isBlockIP(connectIP)) {
        LCLoginError lcLoginError;
        lcLoginError.setErrorID(IP_DENYED);
        pLoginPlayer->sendPacket(&lcLoginError);

        filelog("loginfail.txt", "Error Code: IP_DENYED, 1, PlayerID : %s", pPacket->getID().c_str());
        return;
    }

    // 사내테스트 버전에서는 '#sigi'  <-- 이런 식으로 계정이 들어온다.
    if (ID[0] == SYMBOL_TEST_CLIENT) {
        ID = ID.c_str() + 1;
        pPacket->setID(ID);

        // 웹 로그인 체크
        if (bWebLogin) {
            // cout << "WebLogin" << endl;

            if (!checkWebLogin(pPacket, pPlayer)) {
                return;
            }
            // else
            // cout << "Web Login OK" << endl;
        } else {
            // cout << "not WebLogin" << endl;

            // 넷마블에서 접속하는 경우
            // by sigi. 2002.10.23
            if (!checkNetMarbleClient(pPacket, pPlayer)) {
                return;
            }
        }

        bFreePass = pLoginPlayer->isFreePass();
        if (!bWebLogin && bFreePass) {
            // 웹로그인이 아닌 FreePass 는 넷마블 사용자로 ID 앞에 예약문자가 하나더 있다.
            ID = ID.c_str() + 1;
            pPacket->setID(ID);
        }

        // The test client's login is recorded.
        defaultLoginAccountRepository().insertTestClientUser(ID, connectIP);
    }
    // 넷마블에서 접속하는 경우
    else {
        // 웹 로그인 체크
        if (bWebLogin) {
            // cout << "WebLogin" << endl;

            if (!checkWebLogin(pPacket, pPlayer)) {
                return;
            }
            // else
            // cout << "Web Login OK" << endl;
        } else {
            // cout << "not WebLogin" << endl;

            // by sigi. 2002.10.23
            if (!checkNetMarbleClient(pPacket, pPlayer)) {
                return;
            }
        }

        bFreePass = pLoginPlayer->isFreePass();

        /*
        if (bFreePass)
        {
            ID = ID.c_str()+1;
            pPacket->setID(ID);
        }
        */
    }

    string PASSWORD = pPacket->getPassword();
    string SSN = "";
    ServerGroupID_t CurrentServerGroupID = 0;
    string logon = "";
    string access = "";
    string zipcode = "";

    string lastIP = "";
    string lastMacAddress = "";

    // 빌링~ by sigi. 2002.5.31
    PayType payType;
    string payPlayDate;
    string familyPayPlayDate;
    uint payPlayHours;
    uint payPlayFlag;
    bool bAdult = false;

    try {
        ////////////////////////////////////////////////////////////
        // ID랑 PASSWORD에 이상한 문자가 들어있으면
        // 못 들어오게 막는다.
        ////////////////////////////////////////////////////////////
        bool bError = false;

        // Only the ID reaches SQL text; the password is verified in C++.
        if (ID.find_first_of("'\\", 0) < ID.size())
            bError = true;

        if (bError) {
            //			cout << "이상한 글자" << endl;
            // cout << "Error" << endl;
            LCLoginError lcLoginError;
            lcLoginError.setErrorID(INVALID_ID_PASSWORD);
            pLoginPlayer->sendPacket(&lcLoginError);

            filelog("loginfail.txt", "Error Code: INVALID_ID_PASSWORD, 2, PlayerID : %s", pPacket->getID().c_str());
            return;
        }

        // The account row, in the projection the login kind reads; a web
        // login and a NetMarble free pass skip the password.
        LoginAccountRepository& repo = defaultLoginAccountRepository();
        LoginAccountRow account;
        bool bFound = false;
        bool bPasswordOK = true;

        if (bWebLogin) {
            bFound = repo.loadAccountForWebLogin(ID, account);
        } else if (bFreePass) {
            bFound = repo.loadAccountForFreePass(ID, account);
        } else {
            bPasswordOK = checkStoredPassword(ID, PASSWORD);

            bFound = repo.loadAccount(ID, account);
        }

        // An unknown ID and a wrong password get the same answer.
        bool bNoPlayer = ((!bFound || !bPasswordOK) && !bFreePass);

        // 쿼리 결과 ROW 의 개수가 0 이라는 뜻은
        // invalid ID or Password 라는 뜻이다.
        if (bNoPlayer) {
            // cout << "no Result : " << ID.c_str() << endl;
            //			cout << "플레이어 없음" << endl;
            LCLoginError lcLoginError;
            lcLoginError.setErrorID(INVALID_ID_PASSWORD);
            pLoginPlayer->sendPacket(&lcLoginError);
            filelog("loginfail.txt", "Error Code: INVALID_ID_PASSWORD, 3, PlayerID : %s", pPacket->getID().c_str());

            // 실패 회수가 3보다 클 경우, 연결을 종료한다.
            uint nFailed = pLoginPlayer->getFailureCount();

            //			cout << "실패 회수 " << nFailed << endl;

            if (nFailed > 3) {
                throw DisconnectException("too many failure");
            }

            pLoginPlayer->setFailureCount(nFailed);
            pLoginPlayer->setPlayerStatus(LPS_BEGIN_SESSION);

            return;
        } else {
            if (bWebLogin) {
                pPacket->setID(account.playerID);
                ID = pPacket->getID();
                SSN = account.ssn;
                CurrentServerGroupID = account.currentServerGroupID;
                logon = account.logOn;
                access = account.access;
                zipcode = "000-000";
                lastIP = account.loginIP;
                payType = (PayType)account.payType;
                payPlayDate = account.payPlayDate;
                payPlayHours = account.payPlayHours;
                payPlayFlag = account.payPlayFlag;
                familyPayPlayDate = account.familyPayPlayDate;
            } else if (bFreePass) {
                if (!bFound) {
                    // A NetMarble account that checkFreePass did not create.
                    LCLoginError lcLoginError;
                    lcLoginError.setErrorID(ETC_ERROR);
                    pLoginPlayer->sendPacket(&lcLoginError);
                    pLoginPlayer->setPlayerStatus(LPS_BEGIN_SESSION);
                    filelog("loginfail.txt", "Error Code: ETC_ERROR, 4, PlayerID : %s", pPacket->getID().c_str());
                } else {
                    pPacket->setID(account.playerID);
                    ID = pPacket->getID();
                    CurrentServerGroupID = account.currentServerGroupID;
                    logon = account.logOn;
                    access = account.access;
                    zipcode = "000-000";
                    lastIP = account.loginIP;
                    payType = (PayType)account.payType;
                    payPlayDate = account.payPlayDate;
                    payPlayHours = account.payPlayHours;
                    payPlayFlag = account.payPlayFlag;
                    familyPayPlayDate = account.familyPayPlayDate;
                }
            } else {
                pPacket->setID(account.playerID);
                ID = pPacket->getID();
                SSN = account.ssn;
                CurrentServerGroupID = account.currentServerGroupID;
                logon = account.logOn;
                access = account.access;
                zipcode = account.zipCode;
                lastIP = account.loginIP;
                payType = (PayType)account.payType;
                payPlayDate = account.payPlayDate;
                payPlayHours = account.payPlayHours;
                payPlayFlag = account.payPlayFlag;
                familyPayPlayDate = account.familyPayPlayDate;
            }

            pLoginPlayer->setServerGroupID(CurrentServerGroupID);

            //			if (access == "DENY" || access == "WAIT")
            if (access != "ALLOW") {
                LCLoginError lcLoginError;
                lcLoginError.setErrorID(ETC_ERROR);
                pLoginPlayer->sendPacket(&lcLoginError);
                pLoginPlayer->setPlayerStatus(LPS_BEGIN_SESSION);
                filelog("loginfail.txt", "Error Code: ETC_ERROR, 5, PlayerID : %s", pPacket->getID().c_str());
                return;
            }

#ifdef __THAILAND_SERVER__
            // add by inthesky for THAILAND child guard rule
            bool bChildGuardArea = onChildGuardTimeArea(g_pConfig->getPropertyInt("CHILDGUARD_START_TIME"),g_pConf

			cout << "Global ChildGuard Policy : " << g_pConfig->getProperty("CHILDGUARD") << endl;
			cout << "ChildGuard Start Time : " << (int)g_pConfig->getPropertyInt("CHILDGUARD_START_TIME") << endl;
			cout << "ChildGuard End Time : " << (int)g_pConfig->getPropertyInt("CHILDGUARD_END_TIME") << endl;

			if(bChildGuardArea)     cout << "ChildGuard System : RUN" << endl;
			else                    cout << "ChildGuard System : STOP" << endl;

			if(bPermission) cout << "(" << ID << ") Permission : ALLOW" << endl;
			else            cout << "(" << ID << ") Permission : DENY" << endl;

			if (!bPermission && bChildGuardArea )
			{
                LCLoginError lcLoginError;
                lcLoginError.setErrorID(CHILDGUARD_DENYED);
                pLoginPlayer->sendPacket(&lcLoginError);
                pLoginPlayer->setPlayerStatus(LPS_BEGIN_SESSION);

                return;

			}
#endif


#ifdef __PAY_SYSTEM_LOGIN__
			// 빌링 by sigi. 2002.5.31
			if (!pLoginPlayer->loginPayPlay(payType, 
											payPlayDate, payPlayHours, payPlayFlag,
											connectIP, ID))
			{
                // 돈 안 낸 계정이다. 냥~~
                LCLoginError lcLoginError;
                lcLoginError.setErrorID(NOT_PAY_ACCOUNT);
                pLoginPlayer->sendPacket(&lcLoginError);
                pLoginPlayer->setPlayerStatus(LPS_BEGIN_SESSION);
                filelog("loginfail.txt", "Error Code: NOT_PAY_ACCOUNT, 6, PlayerID : %s", pPacket->getID().c_str());
                return;
			}
#elif defined(__PAY_SYSTEM_FREE_LIMIT__)
            // 빌링 by sigi. 2002.11.21
            if (pLoginPlayer->loginPayPlay(payType, payPlayDate, payPlayHours, payPlayFlag, connectIP, ID)) {
                // 일단 그냥 들어가둔다.
            }
#else // elif defined(__PAY_SYSTEM_ZONE__)
            pLoginPlayer->setPayPlayValue(payType, payPlayDate, payPlayHours, payPlayFlag, familyPayPlayDate);
#endif

			// 이미 게임 중에 접속되어 있다면, 접속할 수 없다.
			bool bSameIP = false;
			if (logon == "LOGON" || 
				logon == "GAME")
			{
                // LOGON상태라면 같은 IP에서 접속했으면 접속 가능
                // if (logon=="LOGON" && connectIP==lastIP)
                //{
                //}
                // (!) IP가 다르면 재접불가다.
                // else

                // LOGON상태에서는 재접 불가하다.
                // IP 접속지가 다르면.. GAME이라도 접속불가다.
                if (logon == "LOGON" || connectIP != lastIP) // || !pPacket->checkMacAddress(lastMacAddress))
                {
                    LCLoginError lcLoginError;
                    // lcLoginError.setMessage("already connected");
                    lcLoginError.setErrorID(ALREADY_CONNECTED);
                    pLoginPlayer->sendPacket(&lcLoginError);
                    filelog("loginfail.txt", "Error Code: ALREADY_CONNECTED, 7, PlayerID : %s",
                            pPacket->getID().c_str());

                    // 실패 회수가 3보다 클 경우, 연결을 종료한다.
                    uint nFailed = pLoginPlayer->getFailureCount();

                    if (nFailed > 3) {
                        throw DisconnectException("too many failure");
                    }

                    pLoginPlayer->setFailureCount(nFailed);
                    pLoginPlayer->setPlayerStatus(LPS_BEGIN_SESSION);
                    return;

                    // bSameIP = false;
                }
                // GAME이고 IP가 같은 경우
                else {
                    bSameIP = true;
                }
			}

			// -- 돈 복사 때문에 잠시 봉인
			// '이미 접속 중'인데..
			// 강제 접속 해제를 시키길 기다리는 상태로 설정한다.
			if (bSameIP)
			{
                if (!bFreePass || bWebLogin) // by sigi. 2002.10.23
                {
                    // 한국
                    if (strstr(SSN.c_str(), "-") != NULL) {
                        bAdult = isAdultByBirthday(SSN.substr(0, 6));
                    }
                    // 중국
                    else {
#ifdef __CHINA_SERVER__
                        // 중국은 무조건 성인
                        bAdult = true;
#else
                        if (SSN.size() == 15) {
                            bAdult = isAdultByBirthday(SSN.substr(6, 12));
                        } else if (SSN.size() == 18) {
                            bAdult = isAdultByBirthday(SSN.substr(8, 14));
                        } else {
                            // 이런 경우는 없다고 하는데 -_-a
                            bAdult = false;
                        }
#endif
                    }
                }

                // 일단 PlayerID를 저장해둔다.
                pLoginPlayer->setID(ID);
                pLoginPlayer->setSSN(SSN);
                pLoginPlayer->setZipcode(zipcode);

                // 이 경우는 GameServer의 응답을 받아야지 LCLoginOK를 보내기 때문에
                // 일단, 쿼리 결과를 이용해서 값을 저장해둔다.
                pLoginPlayer->setAdult(bAdult);

                pLoginPlayer->sendLGKickCharacter();

                return;
			}

			//if (logon == "LOGOFF" || bSameIP)
			if (logon == "LOGOFF"
				|| logon=="LOGON")
			{
                __BEGIN_DEBUG

                // Only a LOGOFF row flips to LOGON; a row that did not
                // change is held by another session.
                bool bLoggedOn = repo.markLoggedOn(connectIP, g_pConfig->getPropertyInt("LoginServerID"), ID);
                int affectedRowCount = bLoggedOn ? 1 : 0;

                if (affectedRowCount == 0) {
                    // Another login server may already hold the account;
                    // for now every such login is refused.
                    //{
                    LCLoginError lcLoginError;
                    // lcLoginError.setMessage("already connected");
                    lcLoginError.setErrorID(ALREADY_CONNECTED);
                    pLoginPlayer->sendPacket(&lcLoginError);
                    filelog("loginfail.txt", "Error Code: ALREADY_CONNECTED, 8, PlayerID : %s",
                            pPacket->getID().c_str());

                    pLoginPlayer->setPlayerStatus(LPS_BEGIN_SESSION);

                    return;
                    //}

                    // LogOn상태로 login이 허용된 상태
                }
                /*
                }
                */

                __END_DEBUG

                // 일단 인증이 되었으므로, 아이디를 로그인 플레이어 객체에 저장한다.
                pLoginPlayer->setID(ID);

                // loginserver 에서 billing 부분 빼기로 한다.
                // 애드빌 요청. by bezz 2003.04.22
                // #ifdef __CONNECT_BILLING_SYSTEM__
                //  by sigi. 2002.11.21
                // pLoginPlayer->setBillingSession();

                // pLoginPlayer->sendBillingLogin();
                // #endif


                // 로그인 전에는 무조건 게임서버에 이 패킷 보내서
                // 접속해 있지만 DB 에 잘못 기록되 중복 로긴이 되는 것을 막는다
                //				pLoginPlayer->sendLGKickCharacter();

                // 로그인 성공을 알려준다.
                LCLoginOK lcLoginOK;
                lcLoginOK.setFamily(false);

                if (!bFreePass || bWebLogin) // by sigi. 2002.10.23
                {
                    // 한국
                    if (strstr(SSN.c_str(), "-") != NULL) {
                        bAdult = isAdultByBirthday(SSN.substr(0, 6));
                    }
                    // 중국
                    else {
#ifdef __CHINA_SERVER__
                        // 중국은 무조건 성인
                        bAdult = true;
#else
                        if (SSN.size() == 15) {
                            bAdult = isAdultByBirthday(SSN.substr(6, 12));
                        } else if (SSN.size() == 18) {
                            bAdult = isAdultByBirthday(SSN.substr(8, 14));
                        } else {
                            // 이런 경우는 없다고 하는데 -_-a
                            bAdult = false;
                        }
#endif
                    }
                }

                if (g_pConfig->getPropertyInt("IsNetMarble") == 1)
                    bAdult = pPacket->isAdult();

                lcLoginOK.setAdult(bAdult);

#ifndef __CONNECT_BILLING_SYSTEM__

#ifdef __CHINA_SERVER__
                lcLoginOK.setLastDays(0xffff);
#else

                //				if (pLoginPlayer->getPayType() == 0 || pLoginPlayer->isFamilyPayAvailable() )
                if (pLoginPlayer->getPayType() == 0)
                    lcLoginOK.setLastDays(0xfffe);
                else {
                    int lastDays = VSDateTime::currentDateTime().daysTo(pLoginPlayer->getPayPlayAvailableDateTime());
                    int lastSecs = VSDateTime::currentDateTime().secsTo(pLoginPlayer->getPayPlayAvailableDateTime());

                    int familyLastDays =
                        VSDateTime::currentDateTime().daysTo(pLoginPlayer->getFamilyPayPlayAvailableDateTime());
                    int familyLastSecs =
                        VSDateTime::currentDateTime().secsTo(pLoginPlayer->getFamilyPayPlayAvailableDateTime());

                    if (lastSecs < 0 && familyLastSecs < 0) {
                        if (pLoginPlayer->getPayPlayAvailableHours() > 0)
                            lcLoginOK.setLastDays(0xfffe);
                        else
                            lcLoginOK.setLastDays(0xfffe);
                    } else {
                        if (lastSecs < familyLastSecs) {
                            lcLoginOK.setFamily(true);
                            lcLoginOK.setLastDays(familyLastDays);

                            cout << "Family 요금제" << endl;
                        } else {
                            lcLoginOK.setFamily(false);
                            lcLoginOK.setLastDays(lastDays);
                            cout << "Premium 요금제" << endl;
                        }
                    }
                }

                // cout << lcLoginOK.getLastDays() << "일 남았습니다." << endl;
                if (lcLoginOK.getLastDays() > 1000)
                    filelog("PayPlayDateLog.txt", "UserID : %s , LastDays : %ld", ID.c_str(), lcLoginOK.getLastDays());

                {
                    // The comeback event: an account that has not yet
                    // received its premium week gets seven days of pay-play
                    // and is told so.
                    if (repo.hasUnclaimedPremiumEvent(pLoginPlayer->getID())) {
                        repo.extendPayPlayByWeek(pLoginPlayer->getID());
                        repo.markPremiumEventReceived(pLoginPlayer->getID());
                        lcLoginOK.setLastDays(0xfffd);
                    }
                }
#endif
#endif

#ifdef __NETMARBLE_SERVER__
                // 넷마블 사용자 약관 동의 여부 체크
                if (repo.hasPrivateAgreementRemaining(pLoginPlayer->getID())) {
                    pLoginPlayer->setAgree(false);
                    cout << "false - " << pLoginPlayer->getID() << endl;
                } else {
                    pLoginPlayer->setAgree(true);
                    cout << "true - " << pLoginPlayer->getID() << endl;
                }
#endif
                /*
                // 서버 그룹 이름을 셋팅한다.
                // 서버 아이디로 서버의 그룹아이디를 찾은 다음 서버 그룹 정보를 찾는다.
                lcLoginOK.setGroupName(g_pGameServerGroupInfoManager->getGameServerGroupInfo(pLoginPlayer->getServerGroupID())->getGroupName());
                lcLoginOK.setStat(SERVER_FREE);

                UserInfo * pUserInfo = g_pUserInfoManager->getUserInfo(pLoginPlayer->getServerGroupID());
                BYTE UserModify = 0;

                if(CurrentServerGroupID == 0 || CurrentServerGroupID == 1 || CurrentServerGroupID == 2 ||
CurrentServerGroupID == 7 ) {
//					UserModify = 200;
                }

                if (pUserInfo->getUserNum() < 100 + UserModify )
                {
                    lcLoginOK.setStat(SERVER_FREE);
                }
                else if (pUserInfo->getUserNum() < 250 + UserModify )
                {
                    lcLoginOK.setStat(SERVER_NORMAL);
                }
                else if (pUserInfo->getUserNum() < 400 + UserModify )
                {
                    lcLoginOK.setStat(SERVER_BUSY);
                }
                else if (pUserInfo->getUserNum() < 500 + UserModify )
                {
                    lcLoginOK.setStat(SERVER_VERY_BUSY);
                }
                else if (pUserInfo->getUserNum() >= 500 + UserModify )
                {
                    lcLoginOK.setStat(SERVER_FULL);
                }
                else
                {
                    lcLoginOK.setStat(SERVER_DOWN);
                }

                */
                pLoginPlayer->sendPacket(&lcLoginOK);
                pLoginPlayer->setPlayerStatus(LPS_WAITING_FOR_CL_GET_PC_LIST);
			}
        } // end of if (bNoPlayer) else
    } catch (const char*) {
        // A SQL failure arrives as END_DB's const char*, already logged to
        // DBError.log (its own message dangles); rethrown as an Error.
        throw Error("CLLoginHandler : SQL error, see DBError.log");
    }

    // 다른 곳에서도 필요한 코드라서. 함수로 뺏당. by sigi. 2002.5.8
    addLoginPlayerData(ID, connectIP, SSN, zipcode);

#endif

    __END_DEBUG_EX __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
//
// YYMMDD 로 성인 판별
//
//////////////////////////////////////////////////////////////////////////////
bool isAdultByBirthday(const string& birthday) {
    StringStream AdultSSN;

    time_t daytime = time(0);
    tm Timec;
    localtime_r(&daytime, &Timec);

    AdultSSN << Timec.tm_year - 18;
    // tm_mon - range 0 to 11
    if ((Timec.tm_mon + 1) < 10)
        AdultSSN << "0";
    AdultSSN << (Timec.tm_mon + 1);
    if (Timec.tm_mday < 10)
        AdultSSN << "0";
    AdultSSN << Timec.tm_mday;

    // cout << "SSN = " << birthday.c_str() << " ADULTSSN = " << AdultSSN.toString().c_str() << endl;

    // 성인인지 아닌지 주민등록 번호 체크
    if (atoi(birthday.c_str()) <= atoi(AdultSSN.toString().c_str())) {
        // cout << "어른" << endl;
        return true;
    }

    // cout << "애들" << endl;
    return false;
}


//////////////////////////////////////////////////////////////////////////////
//
// add LoginPlayerdata
//
// 접속자 통계를 위해서
// UserInfo DB의 LoginPlayerData에 Login한 사용자를 추가한다.
//
//////////////////////////////////////////////////////////////////////////////
void addLoginPlayerData(const string& ID, const string& ip, const string& SSN, const string& zipcode) {
#ifdef __LOGIN_SERVER__
    // The per-login statistics row: account, address, and the current
    // date and time as two texts. SSN and zipcode are no longer recorded.
    // A SQL failure leaves as END_DB's const char*.
    string currentDT = VSDateTime::currentDateTime().toDateTime();

    defaultLoginAccountRepository().insertLoginRecord(ID, ip, currentDT.substr(0, 10), currentDT.substr(11));
#endif
}

bool CLLoginHandler::checkNetMarbleClient(CLLogin* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX
#ifdef __LOGIN_SERVER__

        bool isNetmarble = pPacket->isNetmarble();

    if (isNetmarble) // by sigi. 2002.10.23
    {
        LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);

        bool bFreePass = checkFreePass(pPacket, pPlayer);

        if (!bFreePass) {
            LCLoginError lcLoginError;
            lcLoginError.setErrorID(INVALID_ID_PASSWORD);
            pLoginPlayer->sendPacket(&lcLoginError);
            filelog("loginfail.txt", "Error Code: INVALID_ID_PASSWORD, 9, PlayerID : %s", pPacket->getID().c_str());

            return false;
        }

        // 일부 체크에서.. FreePass로 넘어가게 된다.
        pLoginPlayer->setFreePass(true);


        // cout << "NetMarble Login OK" << endl;
    }

#endif
    __END_DEBUG_EX __END_CATCH

        return true;
}


bool CLLoginHandler::checkFreePass(CLLogin* pPacket, Player* pPlayer)

{
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    // The NetMarble password is checked against the stored hash; an
    // account with no row is created on the spot with the hashed
    // password. A SQL failure leaves as END_DB's const char*.
    try {
        LoginAccountRepository& repo = defaultLoginAccountRepository();

        string stored;
        if (repo.loadPasswordHash(pPacket->getID(), stored)) {
            const de::password::Verify verdict = de::password::verify(stored, pPacket->getPassword());
            if (verdict != de::password::Verify::Rejected) {
                if (verdict == de::password::Verify::AcceptedRehash)
                    storePasswordHash(pPacket->getID(), pPacket->getPassword());
                return true;
            }
        } else {
            // A new NetMarble user is always admitted. SpecialEventCount
            // starts at 2, as if the event item had already been given.
            cout << "NetMarble New Player: " << pPacket->getID().c_str() << endl;

            string hashed;
            try {
                hashed = de::password::hash(pPacket->getPassword());
            } catch (const std::exception& e) {
                filelog("loginfail.txt", "Password hashing failed, PlayerID : %s : %s", pPacket->getID().c_str(),
                        e.what());
                return false;
            }

            repo.insertNetMarbleAccount(pPacket->getID(), hashed);

            return true;
        }
    } catch (Throwable& t) {
        return false;
    }

#endif

    __END_DEBUG_EX __END_CATCH

        return false;
}

bool isBlockIP(const string& ip) {
#ifdef __LOGIN_SERVER__
    size_t i = ip.find_first_of('.', 0);
    size_t j = ip.find_first_of('.', i + 1);
    size_t k = ip.find_first_of('.', j + 1);

    /*
     * ip = 61.78.53.228
     * classA = 61
     * classB = 61.78
     * classC = 61.78.53
     */
    string classA = ip.substr(0, i);
    string classB = ip.substr(0, j);
    string classC = ip.substr(0, k);

    // Every block entry under one of the three prefixes; an entry whose
    // class is not 0, 1 or 2 blocks outright. A SQL failure leaves as
    // END_DB's const char*.
    vector<LoginIPBlockRow> blocks = defaultLoginAccountRepository().loadIPBlocks(classA, classB, classC);

    for (size_t n = 0; n < blocks.size(); n++) {
        int ipClass = blocks[n].ipClass;
        int first = blocks[n].first;
        int last = blocks[n].last;
        int index;

        switch (ipClass) {
        case 0:
            index = atoi(ip.substr(k + 1, ip.size() - k - 1).c_str());
            break;
        case 1:
            index = atoi(ip.substr(i + 1, j - i - 1).c_str());
            break;
        case 2:
            index = atoi(ip.substr(j + 1, k - j - 1).c_str());
            break;
        default:
            index = -1;
            break;
        }

        if (index < 0)
            return true;

        if (index >= first && index <= last)
            return true;
    }

    return false;
#endif
}

bool CLLoginHandler::checkWebLogin(CLLogin* pPacket, Player* pPlayer) {
    __BEGIN_TRY

#ifdef __LOGIN_SERVER__

    Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);

    // The web login key must match the one the site stored for the
    // account, and be at most five minutes old. A SQL failure leaves as
    // END_DB's const char*.
    try {
        LoginAccountRepository& repo = defaultLoginAccountRepository();

        string key;
        string createTime;
        string nowText;

        if (repo.loadWebLoginKey(pPacket->getID(), key, createTime, nowText)) {
            VSDateTime vsCreate(createTime);
            VSDateTime vsNow(nowText);

            if (key != pPacket->getPassword()) {
                LCLoginError lcLoginError;
                lcLoginError.setErrorID(INVALID_ID_PASSWORD);
                pLoginPlayer->sendPacket(&lcLoginError);
                filelog("loginfail.txt", "Error Code: INVALID_ID_PASSWORD, 10, PlayerID : %s",
                        pPacket->getID().c_str());
                filelog("keydiff.txt", "db key: %s, packet key: %s, Player ID: %s", key.c_str(),
                        pPacket->getPassword().c_str(), pPacket->getID().c_str());
                cout << "33333" << endl;
                return false;
            }

            if (vsCreate.secsTo(vsNow) > 300) {
                LCLoginError lcLoginError;
                lcLoginError.setErrorID(KEY_EXPIRED);
                pLoginPlayer->sendPacket(&lcLoginError);
                filelog("loginfail.txt", "Error Code: KEY_EXPIRED, 12, PlayerID : %s", pPacket->getID().c_str());
                return false;
            }

            pLoginPlayer->setFreePass(true);

            repo.deleteWebLoginKey(pPacket->getID());
        } else {
            LCLoginError lcLoginError;
            lcLoginError.setErrorID(NOT_FOUND_KEY);
            pLoginPlayer->sendPacket(&lcLoginError);
            filelog("loginfail.txt", "Error Code: NOT_FOUND_KEY, 11, PlayerID : %s", pPacket->getID().c_str());
            return false;
        }
    } catch (Throwable& t) {
        return false;
    }

#endif

    __END_CATCH

    return true;
}

#ifdef __THAILAND_SERVER__
bool CLLoginHandler::onChildGuardTimeArea(int pm, int am, string enable) {
    bool returnValue = false;
    tm Timem;
    time_t daytime = time(0);
    localtime_r(&daytime, &Timem);

    int Hour = Timem.tm_hour;
    int Min = Timem.tm_min;

    int timeValue = (Hour * 100) + Min;
    bool bSwitch = (enable == "ENABLE" || enable == "enable" || enable == "Enable");

    if ((timeValue >= pm && timeValue <= am) && bSwitch) {
        returnValue = true;
    } else if ((timeValue <= pm && timeValue <= am) && bSwitch) {
        if (am > 1200)
            returnValue = false;
        else
            returnValue = true;
    } else if ((timeValue <= pm && timeValue <= am) && bSwitch) {
        returnValue = false;
    } else if ((timeValue >= pm && timeValue >= am) && bSwitch) {
        if (am > 1200)
            returnValue = false;
        else
            returnValue = true;
    }


    return returnValue;
}
#endif
