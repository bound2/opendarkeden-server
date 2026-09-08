//////////////////////////////////////////////////////////////////////////////
// Filename    : CLCreatePCHandler.cc
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLCreatePC.h"

#ifdef __LOGIN_SERVER__
#include <string.h>

#include <list>
#include <utility>

#include "Assert.h"
#include "CharacterCreation.h"
#include "GameServerInfoManager.h"
#include "LCCreatePCError.h"
#include "LCCreatePCOK.h"
#include "LoginPlayer.h"
#include "repository/LoginCharacterRepository.h"
#endif

#ifdef __THAILAND_SERVER__
// tis620 charset filter functions
int extNumberic(string srcStr);
int extEnglish(string srcStr);
int extAsciiSpecial(string srcStr);
int extTis620Normal(string srcStr);
bool isAllowString(string str);
#endif

#ifdef __CHINA_SERVER__
// GB2312(simple chinese) charset filter functions
int extNumberic(string srcStr);
int extEnglish(string srcStr);
int extGb2312Normal(string srcStr);
int extGb2312Special(string srcStr);
int extAsciiSpecial(string srcStr);
bool isAllowString(string str);
#endif


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void CLCreatePCHandler::execute(CLCreatePC* pPacket, Player* pPlayer) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __LOGIN_SERVER__

        Assert(pPacket != NULL);
    Assert(pPlayer != NULL);

    LoginPlayer* pLoginPlayer = dynamic_cast<LoginPlayer*>(pPlayer);
    LCCreatePCError lcCreatePCError;
    WorldID_t WorldID = pLoginPlayer->getWorldID();
    LoginCharacterRepository& repo = defaultLoginCharacterRepository();

    // The level-1 balance rows never change while the server runs, so one
    // cache serves every creation.
    static CreatePCBalanceCache balance;

    CreatePCRequest request;
    request.worldID = WorldID;
    request.serverGroupID = pPlayer->getServerGroupID();
    request.playerID = pLoginPlayer->getID();
    request.name = pPacket->getName();
    request.slot = pPacket->getSlot();
    request.sex = pPacket->getSex();
    request.hairStyle = pPacket->getHairStyle();
    request.hairColor = pPacket->getHairColor();
    request.skinColor = pPacket->getSkinColor();
    request.str = pPacket->getSTR();
    request.dex = pPacket->getDEX();
    request.inte = pPacket->getINT();
    request.race = pPacket->getRace();

    try {
        Outcome<CreatedCharacter, CreatePCRejection> outcome = decideCreatePC(request, repo, balance);

        if (outcome.isRejected()) {
            switch (outcome.rejection()) {
            case CreatePCRejection::ReservedName:
            case CreatePCRejection::NameTaken:
            case CreatePCRejection::SlotOccupied:
                lcCreatePCError.setErrorID(ALREADY_REGISTER_ID);
                break;

            case CreatePCRejection::DisallowedCharacters:
            case CreatePCRejection::UnknownRace:
                lcCreatePCError.setErrorID(ETC_ERROR);
                break;

            case CreatePCRejection::InvalidAttributes:
                // Attributes the creation screen cannot produce mean the
                // client is not speaking the protocol, so the connection is
                // dropped rather than answered with an error packet.
                throw InvalidProtocolException("CLCreatePCHandler::too large character attribute");
            }

            pLoginPlayer->sendPacket(&lcCreatePCError); // tell the client the creation failed
            return;
        }

        const CreatedCharacter created = std::move(outcome).events();

        // A vampire's Slayer attributes are rolled by the decision; keep the
        // packet in step with the rows that are written.
        pPacket->setSTR(created.str);
        pPacket->setDEX(created.dex);
        pPacket->setINT(created.inte);

        repo.insertSlayer(WorldID, created.slayer);

        if (created.hasOustersRow) {
            repo.insertOusters(WorldID, created.ousters);
        } else {
            repo.insertVampire(WorldID, created.vampire);
        }

        repo.insertFlagSet(WorldID, created.slayer.name, created.flagSet);

        LCCreatePCOK lcCreatePCOK;
        pLoginPlayer->sendPacket(&lcCreatePCOK);
        pLoginPlayer->setPlayerStatus(LPS_WAITING_FOR_CL_GET_PC_LIST);
    } catch (const char*) {
        // A SQL failure arrives as END_DB's const char*, already logged to
        // DBError.log (its own message dangles); the client gets the
        // failure packet with ETC_ERROR.
        lcCreatePCError.setErrorID(ETC_ERROR);
        pLoginPlayer->sendPacket(&lcCreatePCError); // tell the client the creation failed
    }

#endif

    __END_DEBUG_EX __END_CATCH
}

#ifdef __THAILAND_SERVER__
int extNumberic(string srcStr) {
    unsigned char ch;
    int nNumChar = 0;

    if (srcStr.size() <= 0)
        return 0;
    else {
        for (unsigned int i = 0; i < srcStr.size(); i++) {
            ch = srcStr[i];
            if (ch >= '0' && ch <= '9') {
                if (i == 0) {
                    return -1;
                } else {
                    nNumChar++;
                }
            }
        }
    }

    return nNumChar;
}
int extEnglish(string srcStr) {
    unsigned char ch;
    int nEngChar = 0;

    if (srcStr.size() <= 0)
        return 0;
    else {
        for (unsigned int i = 0; i < srcStr.size(); i++) {
            ch = srcStr[i];
            if (ch >= 97 && ch <= 122) {
                nEngChar++;
            } else if (ch >= 65 && ch <= 90) {
                nEngChar++;
            }
        }
    }

    return nEngChar;
}
int extAsciiSpecial(string srcStr) {
    unsigned char ch;
    int nASPCChar = 0;

    if (srcStr.size() <= 0)
        return 0;
    else {
        for (int i = 0; i < srcStr.size(); i++) {
            ch = srcStr[i];

            if (ch >= 0x00 && ch <= 47) {
                nASPCChar++;
            } else if (ch >= 58 && ch <= 64) {
                nASPCChar++;
            } else if (ch >= 91 && ch <= 96) {
                nASPCChar++;
            } else if (ch >= 123 && ch <= 160) {
                nASPCChar++;
            } else if (ch >= 219 && ch <= 222) {
                nASPCChar++;
            } else if (ch >= 252) {
                nASPCChar++;
            }
        }
    }

    return nASPCChar;
}
int extTis620Normal(string srcStr) {
    unsigned char ch;
    int nNorChar = 0;

    if (srcStr.size() <= 0)
        return 0;
    else {
        for (int i = 0; i < srcStr.size(); i++) {
            ch = srcStr[i];

            if (ch >= 161 && ch <= 218) {
                nNorChar++;
            } else if (ch >= 223 && ch <= 251) {
                nNorChar++;
            }
        }
    }

    return nNorChar;
}

bool isAllowString(string str) {
    bool isAllow = true;

    if (str.size() <= 0)
        isAllow = false;
    else {
        int nEng = extEnglish(str);
        int nNum = extNumberic(str);
        int nASpc = extAsciiSpecial(str);
        int nNor = extTis620Normal(str);

        if (nNum == -1)
            isAllow = false; // 문자열의 처음이 숫자로 시작하면
        if (nEng && nNor)
            isAllow = false; // 일반문자와 영문이 섞여 있다면
        if (nASpc)
            isAllow = false; // Ascii영역에서 특수문자가 발견되면
    }

    return isAllow;
}

#endif


#ifdef __CHINA_SERVER__
// 문자열에 숫자가 몇개 있는지 찾기
// return : -1 문자열의 처음에 숫자가 있다.
// return : 0  문자열에서 숫자를 찾지 못했다.
// return : x > 0 문자열에서 1개 이상의 숫자를 찾았다.
int extNumberic(string srcStr) {
    unsigned char ch;
    int nNumChar = 0;

    if (srcStr.size() <= 0)
        return 0;
    else {
        for (unsigned int i = 0; i < srcStr.size(); i++) {
            ch = srcStr[i];
            if (ch >= '0' && ch <= '9') {
                if (i == 0) {
                    return -1;
                } else {
                    nNumChar++;
                }
            }
        }
    }

    return nNumChar;
}


// 문자열에서 영자가 몇개 있는지 찾는다
// return : 0 문자열에서 숫자를 찾지 못했다.
// return : x > 0 문자열에서 1개 이상의 숫자를 찾았다. x는 찾은 갯수
int extEnglish(string srcStr) {
    unsigned char ch;
    int nEngChar = 0;

    if (srcStr.size() <= 0)
        return 0;
    else {
        for (unsigned int i = 0; i < srcStr.size(); i++) {
            ch = srcStr[i];
            if (ch >= 97 && ch <= 122) {
                nEngChar++;
            } else if (ch >= 65 && ch <= 90) {
                nEngChar++;
            }
        }
    }

    return nEngChar;
}
// GB2312 코드셋의 문자가 몇개 있는지 있는지 찾는다.
int extGb2312Normal(string srcStr) {
    unsigned char ch;
    int nNormalChar = 0;

    if (srcStr.size() <= 0)
        return 0;
    else {
        for (unsigned int i = 0; i < srcStr.size(); i++) {
            ch = srcStr[i];
            if (ch >= 0xB0 && ch <= 0xF7) {
                unsigned char ch2 = srcStr[i + 1];
                if (ch2 > 0xA0 && ch < 0xFF) {
                    nNormalChar++;
                    i++;
                }
            } else if ((ch >= 0xA1 && ch <= 0xAF) || (ch >= 0xF8 && ch <= 0xFE)) {
                unsigned char ch2 = srcStr[i + 1];
                if (ch2 >= 0xA0 && ch <= 0xFF) {
                    i++;
                }
            }
        }
    }
    return nNormalChar;
}
int extGb2312Special(string srcStr) {
    unsigned char ch;
    int nSpecialChar = 0;

    if (srcStr.size() <= 0)
        return 0;
    else {
        for (unsigned int i = 0; i < srcStr.size(); i++) {
            ch = srcStr[i];
            if (ch >= 0xB0 && ch <= 0xF7) {
                unsigned char ch2 = srcStr[i + 1];
                if (ch2 > 0xA0 && ch < 0xFF) {
                    i++;
                }
            } else if ((ch >= 0xA1 && ch <= 0xAF) || (ch >= 0xF8 && ch <= 0xFE)) {
                unsigned char ch2 = srcStr[i + 1];
                if (ch2 >= 0xA0 && ch <= 0xFF) {
                    nSpecialChar++;
                    i++;
                }
            }
        }
    }
    return nSpecialChar;
}
int extAsciiSpecial(string srcStr) {
    unsigned char ch;
    int nASPCChar = 0;

    if (srcStr.size() <= 0)
        return 0;
    else {
        for (int i = 0; i < srcStr.size(); i++) {
            ch = srcStr[i];

            if (ch >= 0x00 && ch <= 47) {
                nASPCChar++;
            } else if (ch >= 58 && ch <= 64) {
                nASPCChar++;
            } else if (ch >= 91 && ch <= 96) {
                nASPCChar++;
            } else if (ch >= 123 && ch <= 127) {
                nASPCChar++;
            }
        }
    }

    return nASPCChar;
}
bool isAllowString(string str) {
    bool isAllow = true;

    if (str.size() <= 0)
        isAllow = false;
    else {
        int nEng = extEnglish(str);
        int nNum = extNumberic(str);
        int nNor = extGb2312Normal(str);
        int nSpc = extGb2312Special(str);
        int nASpc = extAsciiSpecial(str);

        if (nNum == -1)
            isAllow = false; // 문자열의 처음이 숫자로 시작하면
        if (nSpc)
            isAllow = false; // 특수문자가 포함되어 있다면 (gb2312안에서만 체크한
        if (nEng && nNor)
            isAllow = false; // 일반문자와 영문이 섞여 있다면
        if (nASpc)
            isAllow = false; // Ascii영역에서 특수문자가 발견되면
    }

    return isAllow;
}

#endif
