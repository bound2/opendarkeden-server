//////////////////////////////////////////////////////////////////////////////
// Filename    : CharacterCreation.cpp
// Description : the character-creation decision behind CLCreatePCHandler.
//////////////////////////////////////////////////////////////////////////////

#include "CharacterCreation.h"

#include <cstdlib>
#include <cstring>

#include "PCSlayerInfo.h"

#if defined(__THAILAND_SERVER__) || defined(__CHINA_SERVER__)
// The regional charset filter (tis620 for Thailand, gb2312 for China);
// defined with the rest of the regional code in CLCreatePCHandler.cpp.
bool isAllowString(string str);
#endif

namespace {

// Names a player may not take. Substring matches, so "GMaster" is
// refused as well.
const int kInvalidIDCount = 10;
const char* const kInvalidID[kInvalidIDCount] = {"NONE",   "관리자", "도우미", "담당자", "운영",
                                                 "기획자", "개발자", "테스터", "직원",   "GM"};

} // namespace

bool isAvailableID(const char* pID) {
    for (int i = 0; i < kInvalidIDCount; i++) {
        if (strstr(pID, kInvalidID[i]) != NULL) {
            return false;
        }
    }

    return true;
}

int CreatePCBalanceCache::rankGoalExp(LoginCharacterRepository& repository, WorldID_t worldID, int rankType) {
    if (rankType < 0 || rankType >= kRankTypeMax)
        return -1;

    if (m_RankGoalExp[rankType] == -1) {
        int goalExp = 0;
        if (repository.loadRankGoalExp(worldID, rankType, goalExp))
            m_RankGoalExp[rankType] = goalExp;
    }

    return m_RankGoalExp[rankType];
}

int CreatePCBalanceCache::vampireGoalExp(LoginCharacterRepository& repository, WorldID_t worldID) {
    if (m_VampireGoalExp == -1) {
        int goalExp = 0;
        if (repository.loadVampireGoalExp(worldID, goalExp))
            m_VampireGoalExp = goalExp;
    }

    return m_VampireGoalExp;
}

int CreatePCBalanceCache::oustersGoalExp(LoginCharacterRepository& repository, WorldID_t worldID) {
    if (m_OustersGoalExp == -1) {
        int goalExp = 0;
        if (repository.loadOustersGoalExp(worldID, goalExp))
            m_OustersGoalExp = goalExp;
    }

    return m_OustersGoalExp;
}

int CreatePCBalanceCache::attrGoalExp(LoginCharacterRepository& repository, WorldID_t worldID, LoginAttrTable attr,
                                      int level) {
    int value = 0;

    if (level < 0 || level >= kCachedLevels) {
        repository.loadAttrGoalExp(worldID, attr, level, value);
        return value;
    }

    if (m_AttrGoalExp[attr][level] == 0) {
        if (repository.loadAttrGoalExp(worldID, attr, level, value))
            m_AttrGoalExp[attr][level] = value;
    }

    return m_AttrGoalExp[attr][level];
}

int CreatePCBalanceCache::attrAccumExp(LoginCharacterRepository& repository, WorldID_t worldID, LoginAttrTable attr,
                                       int level) {
    int value = 0;

    if (level < 0 || level >= kCachedLevels) {
        repository.loadAttrAccumExp(worldID, attr, level, value);
        return value;
    }

    if (m_AttrAccumExp[attr][level] == 0) {
        if (repository.loadAttrAccumExp(worldID, attr, level, value))
            m_AttrAccumExp[attr][level] = value;
    }

    return m_AttrAccumExp[attr][level];
}

Outcome<CreatedCharacter, CreatePCRejection>
decideCreatePC(const CreatePCRequest& request, LoginCharacterRepository& repository, CreatePCBalanceCache& balance) {
    typedef Outcome<CreatedCharacter, CreatePCRejection> Result;

    // Names the system uses, or that impersonate staff, are refused with
    // the same code as a taken name.
    if (!isAvailableID(request.name.c_str()))
        return Result::Rejected(CreatePCRejection::ReservedName);

#if defined(__THAILAND_SERVER__) || defined(__CHINA_SERVER__)
    // Only characters the regional charset allows may appear in a name.
    if (!isAllowString(request.name))
        return Result::Rejected(CreatePCRejection::DisallowedCharacters);
#endif

    // The name must be free and the slot empty.
    if (repository.slayerNameExists(request.worldID, request.name))
        return Result::Rejected(CreatePCRejection::NameTaken);

    if (repository.slotOccupied(request.worldID, request.playerID, Slot2String[request.slot]))
        return Result::Rejected(CreatePCRejection::SlotOccupied);

    const int rankGoalExpSlayer = balance.rankGoalExp(repository, request.worldID, 0);
    const int goalExpVampire = balance.vampireGoalExp(repository, request.worldID);
    const int goalExpOusters = balance.oustersGoalExp(repository, request.worldID);
    const int rankGoalExpVampire = balance.rankGoalExp(repository, request.worldID, 1);
    const int rankGoalExpOusters = balance.rankGoalExp(repository, request.worldID, 2);

    // Reject a character built with attributes the creation screen cannot
    // produce.
    bool invalidAttr = false;
    int nSTR = request.str;
    int nDEX = request.dex;
    int nINT = request.inte;

    if (request.race == RACE_SLAYER) {
        if (nSTR < 5 || nSTR > 20)
            invalidAttr = true;
        if (nDEX < 5 || nDEX > 20)
            invalidAttr = true;
        if (nINT < 5 || nINT > 20)
            invalidAttr = true;
        if (nSTR + nDEX + nINT > 30)
            invalidAttr = true;
    } else if (request.race == RACE_VAMPIRE) {
        // A vampire is created with a flat 20/20/20; the Slayer row it
        // also gets is rolled here, since the client never picks it.
        if (nSTR != 20 || nDEX != 20 || nINT != 20) {
            invalidAttr = true;
        } else {
            nSTR = 5 + rand() % 16; // 5~20
            nDEX = 5 + rand() % (21 - nSTR);
            nINT = 30 - nSTR - nDEX;
        }
    } else if (request.race == RACE_OUSTERS) {
        if (nSTR < 10 || nDEX < 10 || nINT < 10) {
            filelog("CreatePC.log", "Illegal PC Create [%s:%s] : %u/%u/%u", request.playerID.c_str(),
                    request.name.c_str(), nSTR, nDEX, nINT);
        }

        if (nSTR + nDEX + nINT != 45)
            invalidAttr = true;
    }

    if (invalidAttr)
        return Result::Rejected(CreatePCRejection::InvalidAttributes);

    const int nSTRGoalExp = balance.attrGoalExp(repository, request.worldID, LOGIN_ATTR_TABLE_STR, nSTR);
    const int nSTRExp = balance.attrAccumExp(repository, request.worldID, LOGIN_ATTR_TABLE_STR, nSTR - 1);
    const int nDEXGoalExp = balance.attrGoalExp(repository, request.worldID, LOGIN_ATTR_TABLE_DEX, nDEX);
    const int nDEXExp = balance.attrAccumExp(repository, request.worldID, LOGIN_ATTR_TABLE_DEX, nDEX - 1);
    const int nINTGoalExp = balance.attrGoalExp(repository, request.worldID, LOGIN_ATTR_TABLE_INT, nINT);
    const int nINTExp = balance.attrAccumExp(repository, request.worldID, LOGIN_ATTR_TABLE_INT, nINT - 1);

    // No clothing yet, so the shape carries only sex, and the Slayer
    // shape the hair style on top of it.
    DWORD slayerShape = (request.sex == MALE ? 1 : 0);
    const DWORD vampireShape = slayerShape;

    slayerShape |= (request.hairStyle << PCSlayerInfo::SLAYER_BIT_HAIRSTYLE1);

    const Color_t HelmetColor = 0;
    const Color_t JacketColor = 0;
    const Color_t PantsColor = 0;
    const Color_t WeaponColor = 0;
    const Color_t ShieldColor = 0;

    std::string race;
    switch (request.race) {
    case RACE_SLAYER:
        race = "SLAYER";
        break;
    case RACE_VAMPIRE:
        race = "VAMPIRE";
        break;
    case RACE_OUSTERS:
        race = "OUSTERS";
        break;
    default:
        return Result::Rejected(CreatePCRejection::UnknownRace);
    }

    CreatedCharacter created;
    created.str = (Attr_t)nSTR;
    created.dex = (Attr_t)nDEX;
    created.inte = (Attr_t)nINT;

    LoginNewSlayer& slayer = created.slayer;
    slayer.race = race;
    slayer.name = request.name;
    slayer.playerID = request.playerID;
    slayer.slot = Slot2String[request.slot];
    slayer.serverGroupID = (int)request.serverGroupID;
    slayer.sex = Sex2String[request.sex];
    slayer.hairStyle = HairStyle2String[request.hairStyle];
    slayer.hairColor = (int)request.hairColor;
    slayer.skinColor = (int)request.skinColor;
    slayer.str = nSTR;
    slayer.strExp = nSTRExp;
    slayer.strGoalExp = nSTRGoalExp;
    slayer.dex = nDEX;
    slayer.dexExp = nDEXExp;
    slayer.dexGoalExp = nDEXGoalExp;
    slayer.inte = nINT;
    slayer.intExp = nINTExp;
    slayer.intGoalExp = nINTGoalExp;
    slayer.rank = 1;
    slayer.rankExp = 0;
    slayer.rankGoalExp = rankGoalExpSlayer;
    slayer.hp = nSTR * 2;
    slayer.currentHP = nSTR * 2;
    slayer.mp = nINT * 2;
    slayer.currentMP = nINT * 2;
    slayer.shape = slayerShape;
    slayer.helmetColor = (int)HelmetColor;
    slayer.jacketColor = (int)JacketColor;
    slayer.pantsColor = (int)PantsColor;
    slayer.weaponColor = (int)WeaponColor;
    slayer.shieldColor = (int)ShieldColor;

    // Every character has a Slayer row; a Vampire or Ousters row
    // besides, by race.
    created.hasOustersRow = (request.race == RACE_OUSTERS);

    if (!created.hasOustersRow) {
        LoginNewVampire& vampire = created.vampire;
        vampire.name = request.name;
        vampire.playerID = request.playerID;
        vampire.slot = Slot2String[request.slot];
        vampire.serverGroupID = (int)request.serverGroupID;
        vampire.sex = Sex2String[request.sex];
        vampire.skinColor = (int)request.skinColor;
        vampire.goalExp = goalExpVampire;
        vampire.rankGoalExp = rankGoalExpVampire;
        vampire.shape = vampireShape;
    } else {
        LoginNewOusters& ousters = created.ousters;
        ousters.name = request.name;
        ousters.playerID = request.playerID;
        ousters.slot = Slot2String[request.slot];
        ousters.serverGroupID = (int)request.serverGroupID;
        ousters.str = nSTR;
        ousters.dex = nDEX;
        ousters.inte = nINT;
        ousters.goalExp = goalExpOusters;
        ousters.rankGoalExp = rankGoalExpOusters;
        ousters.hairColor = (int)request.hairColor;
    }

    created.flagSet = (request.race == RACE_SLAYER) ? LOGIN_FLAGSET_SLAYER : LOGIN_FLAGSET_OTHER;

    return Result::Ok(std::move(created));
}
