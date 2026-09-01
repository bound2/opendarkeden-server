#ifndef __GAME_INFO_REPOSITORY_H__
#define __GAME_INFO_REPOSITORY_H__

#include <string>
#include <vector>

// Read-only seam for the small game-info tables the gameserver loads
// once at boot (task 3.2): the skill tree (SkillTreeInfo), the rank
// bonuses (RankBonusInfo), the pet types (PetTypeInfo), the game
// server groups (GameServerGroupInfo), the blood-bible bonuses
// (BloodBibleBonusInfo) and the monster name parts (FirstNameInfo,
// MiddleNameInfo, LastNameInfo). The config round added the
// whole-table boot reads: WeatherInfo, GSStringPool, ShopTemplate,
// NicknameIndex ('LEVEL' rows), ItemMineInfo, ItemGradeRatioInfo,
// GoodsListInfo (on the dist connection), DefaultOptionSetInfo,
// DarkLightInfo, CastleSkillInfo, CastleShrineInfo and LogUserInfo.
// Every field is typed to the driver getter the inline code called
// (getInt → int, getString → std::string), so each caller's narrowing
// still happens at the caller on the same value.
//
// The MAX probes the loaders use to size their arrays are exposed as
// bools for the reason BalanceInfoRepository.h gives: MAX() over an
// empty table is one NULL row, which the inline code would have
// atoi(NULL)'d.
//
// The loginserver and sharedserver load GameServerGroupInfo with their
// own code — their own extractions.

struct SkillParentRow {
    int skillType;
    int parent;
};

struct RankBonusInfoRow {
    int type;
    std::string name;
    int rank;
    int point;
    int race;
};

struct PetTypeRow {
    int petType;
    int originalMonsterType;
    int creatureType[5];
    int foodType;
};

struct GameServerGroupRow {
    int worldID;
    int groupID;
    std::string groupName;
    int stat;
};

struct BloodBibleBonusRow {
    int type;
    std::string name;
    std::string optionList;
};

// The four name lists MonsterNameManager reads: the basic first, middle
// and last names, and the event monsters' last names.
enum MonsterNameList {
    MONSTER_NAMES_FIRST_BASIC,
    MONSTER_NAMES_MIDDLE_BASIC,
    MONSTER_NAMES_LAST_BASIC,
    MONSTER_NAMES_LAST_EVENT,
    MONSTER_NAME_LIST_MAX
};

// --- the config tables the second round added ------------------------------

struct WeatherRow {
    int month;
    int clear;
    int rainy;
    int snowy;
};

struct StringPoolRow {
    int id;
    std::string text;
};

struct ShopTemplateRow {
    int id;
    int shopType;
    int itemClass;
    int minItemType;
    int maxItemType;
    int minOptionLevel;
    int maxOptionLevel;
};

// NicknameIndex rows of NickType 'LEVEL' (LevelNickInfoManager).
struct LevelNickRow {
    int nickIndex;
    int race;
    int level10;
};

struct ItemMineRow {
    int id;
    std::string itemClass;
    int itemType;
    std::string itemOption;
};

struct ItemGradeRatioRow {
    int grade;
    int ratio;
    int gambleRatio;
    int beadRatio;
};

// GoodsListInfo rows other than Kind 'SET' (GoodsInfoManager, on the
// dist connection). limited is the enum's ordinal ("Limited+0": 1 =
// LIMITED, 2 = UNLIMITED, 3 = FOREVER).
struct GoodsInfoRow {
    int goodsID;
    std::string name;
    int itemClass;
    int itemType;
    int grade;
    std::string optionType;
    int num;
    int limited;
    int hour;
};

struct DefaultOptionSetRow {
    int type;
    std::string optionList;
};

struct DarkLightRow {
    int month;
    int hour;
    int minute;
    int darkLevel;
    int lightLevel;
};

struct CastleSkillRow {
    int skillType;
    int zoneID;
};

struct CastleShrineRow {
    int id;
    std::string name;
    int itemType;
    int guardZoneID;
    int guardX;
    int guardY;
    int guardMonsterType;
    int holyZoneID;
    int holyX;
    int holyY;
    int holyMonsterType;
};

class GameInfoRepository {
public:
    virtual ~GameInfoRepository() {}

    // The whole-table config reads of the second round, one per table.
    virtual std::vector<WeatherRow> loadWeather() = 0;
    virtual std::vector<StringPoolRow> loadStrings() = 0;
    virtual std::vector<ShopTemplateRow> loadShopTemplates() = 0;
    virtual std::vector<LevelNickRow> loadLevelNicks() = 0;
    virtual std::vector<ItemMineRow> loadItemMines() = 0;
    virtual std::vector<ItemGradeRatioRow> loadItemGradeRatios() = 0;
    virtual std::vector<GoodsInfoRow> loadGoods() = 0;
    virtual std::vector<DefaultOptionSetRow> loadDefaultOptionSets() = 0;
    virtual std::vector<DarkLightRow> loadDarkLight() = 0;
    virtual std::vector<CastleSkillRow> loadCastleSkills() = 0;
    virtual std::vector<CastleShrineRow> loadCastleShrines() = 0;
    virtual std::vector<std::string> loadLogUserNames() = 0;

    virtual bool loadMaxSkillType(int& maxSkillType) = 0;
    virtual std::vector<SkillParentRow> loadSkillTree() = 0;

    virtual bool loadMaxRankBonusType(int& maxType) = 0;
    virtual std::vector<RankBonusInfoRow> loadRankBonusInfos() = 0;

    virtual bool loadMaxPetType(int& maxPetType) = 0;
    virtual std::vector<PetTypeRow> loadPetTypes() = 0;

    virtual bool loadMaxWorldID(int& maxWorldID) = 0;
    virtual std::vector<GameServerGroupRow> loadGameServerGroups() = 0;

    virtual bool loadMaxBloodBibleBonusType(int& maxType) = 0;
    virtual std::vector<BloodBibleBonusRow> loadBloodBibleBonuses() = 0;

    // The names of one list, in the order the ORDER-BY-less SELECT
    // returns them (the manager indexes them by position — the
    // optimizer's choice, not a contract).
    virtual std::vector<std::string> loadMonsterNames(MonsterNameList list) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLGameInfoRepository.cpp. An accessor function rather than a g_p*
// extern: ratchet R1 counts those.
GameInfoRepository& defaultGameInfoRepository();

#endif
