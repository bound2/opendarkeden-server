//////////////////////////////////////////////////////////////////////////////
// Filename    : CharacterCreation.h
// Description : the loginserver's character-creation decision, separated
//               from the CLCreatePC handler so it can be exercised without
//               a socket or a database.
//////////////////////////////////////////////////////////////////////////////

#ifndef __CHARACTER_CREATION_H__
#define __CHARACTER_CREATION_H__

#include <string>

#include "Outcome.h"
#include "Types.h"
#include "repository/LoginCharacterRepository.h"

// Why a creation was refused. Each value maps to one LCCreatePCError code
// except InvalidAttributes, which no client can produce by playing and
// which the handler answers by dropping the connection.
enum class CreatePCRejection {
    // The name contains a reserved token (see isAvailableID).
    ReservedName,
    // A Slayer row of that name already exists in the world.
    NameTaken,
    // The account already has an ACTIVE character in that slot.
    SlotOccupied,
    // The name uses characters the regional charset filter refuses. Only
    // the Thailand and China builds have that filter, so only they
    // produce this.
    DisallowedCharacters,
    // STR/DEX/INT are outside what the race allows.
    InvalidAttributes,
    // The race byte names none of the three races.
    UnknownRace
};

// A creation request: the CLCreatePC packet's fields plus the session
// state the handler reads off the player.
struct CreatePCRequest {
    WorldID_t worldID = 0;
    ServerGroupID_t serverGroupID = 0;
    std::string playerID;
    std::string name;
    Slot slot = SLOT1;
    Sex sex = FEMALE;
    HairStyle hairStyle = HAIR_STYLE1;
    Color_t hairColor = 0;
    Color_t skinColor = 0;
    Attr_t str = 0;
    Attr_t dex = 0;
    Attr_t inte = 0;
    Race_t race = RACE_SLAYER;
};

// The rows an accepted creation writes. Every character gets the Slayer
// row; hasOustersRow says whether the second row is the Ousters one or
// the Vampire one.
struct CreatedCharacter {
    // The attributes the character is actually created with. A vampire's
    // are rolled during the decision and differ from the request's.
    Attr_t str = 0;
    Attr_t dex = 0;
    Attr_t inte = 0;

    LoginNewSlayer slayer;
    bool hasOustersRow = false;
    LoginNewVampire vampire;
    LoginNewOusters ousters;
    LoginFlagSetPreset flagSet = LOGIN_FLAGSET_SLAYER;
};

// The level-1 balance rows a creation prices its starting stats from.
// They never change while a server runs, so each is read once and kept;
// a row that is missing is retried on the next creation. One instance
// lives for the loginserver's lifetime, which is why the values are not
// keyed by world: the first world to create a character fixes them.
class CreatePCBalanceCache {
public:
    // RankEXPInfo.GoalExp at Level 1 for one rank type (0 Slayer,
    // 1 Vampire, 2 Ousters). -1 while the row has not been read.
    int rankGoalExp(LoginCharacterRepository& repository, WorldID_t worldID, int rankType);
    // VampEXPBalanceInfo.GoalExp / OustersEXPBalanceInfo.GoalExp at Level 1.
    int vampireGoalExp(LoginCharacterRepository& repository, WorldID_t worldID);
    int oustersGoalExp(LoginCharacterRepository& repository, WorldID_t worldID);
    // <STR|DEX|INT>BalanceInfo.GoalExp / .AccumExp at one level, 0 when
    // there is no such row. A level outside the cached range is read
    // straight from the repository and not kept.
    int attrGoalExp(LoginCharacterRepository& repository, WorldID_t worldID, LoginAttrTable attr, int level);
    int attrAccumExp(LoginCharacterRepository& repository, WorldID_t worldID, LoginAttrTable attr, int level);

private:
    static const int kRankTypeMax = 3;
    static const int kCachedLevels = 100;

    int m_RankGoalExp[kRankTypeMax] = {-1, -1, -1};
    int m_VampireGoalExp = -1;
    int m_OustersGoalExp = -1;
    int m_AttrGoalExp[LOGIN_ATTR_TABLE_MAX][kCachedLevels] = {};
    int m_AttrAccumExp[LOGIN_ATTR_TABLE_MAX][kCachedLevels] = {};
};

// Decide whether a character may be created, and with which rows.
//
// The repository is passed in because two of the rejections and all of
// the starting stats are database reads; the writes stay with the caller,
// so this function is a pure decision over whatever the repository
// answers and needs no database in a test.
//
// A repository that fails its query throws (the DB layer's own const
// char*); that is a server fault, not a player-facing rejection, and is
// left to the caller.
[[nodiscard]] Outcome<CreatedCharacter, CreatePCRejection>
decideCreatePC(const CreatePCRequest& request, LoginCharacterRepository& repository, CreatePCBalanceCache& balance);

// Is the name free of the reserved tokens (NONE, GM, the Korean staff
// words)? Shared with CLQueryCharacterNameHandler.
bool isAvailableID(const char* pID);

#endif
