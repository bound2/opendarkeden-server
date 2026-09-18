//////////////////////////////////////////////////////////////////////////////
// Filename    : OptionInfo.h
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __OPTIONINFO_H__
#define __OPTIONINFO_H__

#include <list>
#include <vector>

#include <unordered_map>

#include "Exception.h"
#include "Item.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// Maximum option level
//////////////////////////////////////////////////////////////////////////////
const uint OPTION_LEVEL_MAX = 100;
const uint GAMBLE_OPTION_LEVEL_MAX = 100;

class PlayerCreature;
class Slayer;
class Vampire;
class Ousters;

//////////////////////////////////////////////////////////////////////////////
// Kinds of body part an option applies to
//////////////////////////////////////////////////////////////////////////////
enum OptionClass {
    OPTION_STR = 0,       // increase STR
    OPTION_DEX,           // increase DEX
    OPTION_INT,           // increase INT
    OPTION_HP,            // increase HP
    OPTION_MP,            // increase MP
    OPTION_HP_STEAL,      // steal HP
    OPTION_MP_STEAL,      // steal MP
    OPTION_HP_REGEN,      // regenerate hp over time
    OPTION_MP_REGEN,      // regenerate mp over time
    OPTION_TOHIT,         // increase tohit
    OPTION_DEFENSE,       // increase defense
    OPTION_DAMAGE,        // increase damage
    OPTION_PROTECTION,    // increase protection
    OPTION_DURABILITY,    // increase item durability
    OPTION_POISON,        // increase poison resistence
    OPTION_ACID,          // increase acid resistence
    OPTION_CURSE,         // increase curse resistence
    OPTION_BLOOD,         // increase blood resistence
    OPTION_VISION,        // increase vision range
    OPTION_ATTACK_SPEED,  // increase attack speed
    OPTION_CRITICAL_HIT,  // increase chance of doing critical hit
    OPTION_LUCK,          // increase looting item type
    OPTION_ALL_RES,       // increase all registance
    OPTION_ALL_ATTR,      // increase all attributes(str, dex, int)
    OPTION_STR_TO_DEX,    // STR to DEX
    OPTION_STR_TO_INT,    // STR to INT
    OPTION_DEX_TO_STR,    // DEX to STR
    OPTION_DEX_TO_INT,    // DEX to INT
    OPTION_INT_TO_STR,    // INT to STR
    OPTION_INT_TO_DEX,    // INT to DEX
    OPTION_CONSUME_MP,    // decrease consume mana
    OPTION_TRANS,         // translate race language
    OPTION_MAGIC_DAMAGE,  // increase magic damage
    OPTION_PHYSIC_DAMAGE, // increase physical damage
    OPTION_GAMBLE_PRICE,  // decrease gamble price
    OPTION_POTION_PRICE,  // decrease gamble price
    OPTION_MAGIC_PRO,
    OPTION_PHYSIC_PRO,

    OPTION_MAX
};

const string OptionClass2String[] = {
    "OPTION_STR",          "OPTION_DEX",           "OPTION_INT",          "OPTION_HP",
    "OPTION_MP",           "OPTION_HP_STEAL",      "OPTION_MP_STEAL",     "OPTION_HP_REGEN",
    "OPTION_MP_REGEN",     "OPTION_TOHIT",         "OPTION_DEFENSE",      "OPTION_DAMAGE",
    "OPTION_PROTECTION",   "OPTION_DURABILITY",    "OPTION_POISON",       "OPTION_ACID",
    "OPTION_CURSE",        "OPTION_BLOOD",         "OPTION_VISION",       "OPTION_ATTACK_SPEED",
    "OPTION_CRITICAL_HIT", "OPITON_LUCK",          "OPITON_ALL_RES",      "OPITON_ALL_ATTR",
    "OPTION_STR_TO_DEX",   "OPTION_STR_TO_INT",    "OPTION_DEX_TO_STR",   "OPTION_DEX_TO_INT",
    "OPTION_INT_TO_STR",   "OPTION_INT_TO_DEX",    "OPTION_CONSUME_MP",   "OPTION_TRANS",
    "OPTION_MAGIC_DAMAGE", "OPTION_PHYSIC_DAMAGE", "OPTION_GAMBLE_PRICE", "OPTION_POTION_PRICE",
    "OPTION_MAGIC_PRO",    "OPTION_PHYSIC_PRO",

    "OPTION_MAX"};

enum OptionGroup {
    OPTION_GROUP_ATTR = 0, // Attribute-related options
    OPTION_GROUP_STAT,     // Secondary attribute-related options
    OPTION_GROUP_SPECIAL,  // Special options
    OPTION_GROUP_RESIST,   // Resistance options
    OPTION_GROUP_ETC,      // Other options
    OPTION_GROUP_NOT       // No group
};

class RareOptionUpgradeInfo {
public:
    RareOptionUpgradeInfo(int level, int grade) : m_Level(level), m_TotalGrade(grade) {
        m_UpgradeRatio[0].reserve(grade);
        m_UpgradeRatio[1].reserve(grade);
    }

    int getKey() const {
        return m_Level * 10 + m_TotalGrade;
    }

    int getRatio(int grade, bool success) const {
        Assert(grade < m_TotalGrade);
        return m_UpgradeRatio[(int)success][grade];
    }
    void setRatio(int grade, bool success, int ratio) {
        Assert(grade < m_TotalGrade);
        m_UpgradeRatio[(int)success][grade] = ratio;
    }

private:
    int m_Level;
    int m_TotalGrade;
    vector<int> m_UpgradeRatio[2];
};

class OptionClassInfo {
public:
    OptionClassInfo(OptionClass optionClass) : m_OptionClass(optionClass) {
        m_Level = 0;
        m_TotalGrade = 0;
    }

    OptionClass getOptionClass() const {
        return m_OptionClass;
    }

    OptionGroup getOptionGroup() const {
        return m_OptionGroup;
    }
    void setOptionGroup(OptionGroup optionGroup) {
        m_OptionGroup = optionGroup;
    }

    int getKey() const {
        return m_Level * 10 + m_TotalGrade;
    }

    const string& getName() const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
    }

    const string& getHName() const {
        return m_HName;
    }
    void setHName(const string& hname) {
        m_HName = hname;
    }

    int getLevel() const {
        return m_Level;
    }
    void setLevel(int level) {
        m_Level = level;
    }

    int getTotalGrade() const {
        return m_TotalGrade;
    }
    void setTotalGrade(int grade) {
        m_TotalGrade = grade;
    }

    /*
    bool			ApplyOption( PlayerCreature* pPC, int PlusPoint );
    bool			ApplyOption( Slayer* pSlayer, int PlusPoint );
    bool			ApplyOption( Vampire* pVampire, int PlusPoint );
    bool			ApplyOption( Ousters* pOusters, int PlusPoint );
    */

private:
    OptionClass m_OptionClass;
    string m_Name;
    string m_HName;
    int m_Level;      // Importance
    int m_TotalGrade; // Total number of grades of the option
    OptionGroup m_OptionGroup;
};

//////////////////////////////////////////////////////////////////////////////
// Class OptionInfo
//////////////////////////////////////////////////////////////////////////////

class OptionInfo {
    ///// Member methods /////

public:
    OptionInfo();
    ~OptionInfo();

public:
    OptionType_t getType(void) const {
        return m_OptionType;
    }
    void setType(OptionType_t type) {
        m_OptionType = type;
    }

    string getName(void) const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
    }

    string getHName(void) const {
        return m_HName;
    }
    void setHName(const string& hname) {
        m_HName = hname;
    }

    string getNickname(void) const {
        return m_Nickname;
    }
    void setNickname(const string& name) {
        m_Nickname = name;
    }

    OptionClass getClass(void) const {
        return m_OptionClass;
    }
    void setClass(OptionClass OClass) {
        m_OptionClass = OClass;
    }

    int getPlusPoint(void) const {
        return m_PlusPoint;
    }
    void setPlusPoint(int point) {
        m_PlusPoint = point;
    }

    int getPriceMultiplier(void) const {
        return m_PriceMultiplier;
    }
    void setPriceMultiplier(int mul) {
        m_PriceMultiplier = mul;
    }

    int getReqSTR(void) const {
        return m_ReqSTR;
    }
    void setReqSTR(int req) {
        m_ReqSTR = req;
    }

    int getReqDEX(void) const {
        return m_ReqDEX;
    }
    void setReqDEX(int req) {
        m_ReqDEX = req;
    }

    int getReqINT(void) const {
        return m_ReqINT;
    }
    void setReqINT(int req) {
        m_ReqINT = req;
    }

    int getReqSum(void) const {
        return m_ReqSum;
    }
    void setReqSum(int req) {
        m_ReqSum = req;
    }

    int getReqLevel(void) const {
        return m_ReqLevel;
    }
    void setReqLevel(int req) {
        m_ReqLevel = req;
    }

    void setReqAbility(const string& text);

    int getLevel(void) const {
        return m_Level;
    }
    void setLevel(int level) {
        m_Level = level;
    }

    Color_t getColor(void) const {
        return m_Color;
    }
    void setColor(Color_t color) {
        m_Color = color;
    }

    Ratio_t getRatio(void) const {
        return m_Ratio;
    }
    void setRatio(Ratio_t ratio) {
        m_Ratio = ratio;
    }

    int getGambleLevel(void) const {
        return m_GambleLevel;
    }
    void setGambleLevel(int level) {
        m_GambleLevel = level;
    }

    // Information needed when downgrading to the previous option grade
    OptionType_t getPreviousType(void) const {
        return m_PreviousOptionType;
    }
    void setPreviousType(OptionType_t type) {
        m_PreviousOptionType = type;
    }

    // Information needed when upgrading to the next option grade
    OptionType_t getUpgradeType(void) const {
        return m_UpgradeOptionType;
    }
    void setUpgradeType(OptionType_t type) {
        m_UpgradeOptionType = type;
    }

    Ratio_t getUpgradeRatio(void) const {
        return m_UpgradeRatio;
    }
    void setUpgradeRatio(Ratio_t ratio) {
        m_UpgradeRatio = ratio;
    }

    Ratio_t getUpgradeSecondRatio(void) const {
        return m_UpgradeSecondRatio;
    }
    void setUpgradeSecondRatio(Ratio_t ratio) {
        m_UpgradeSecondRatio = ratio;
    }

    int getUpgradeCrashPercent(void) const {
        return m_UpgradeCrashPercent;
    }
    void setUpgradeCrashPercent(int per) {
        m_UpgradeCrashPercent = per;
    }

    // Is an upgrade to the next option grade possible?
    bool isUpgradePossible() const {
        return m_UpgradeOptionType != 0 && m_UpgradeRatio != 0;
    }

    // Did the upgrade to the next option grade succeed?
    bool isUpgradeSucceed() const;

    // Did the upgrade to the next option grade succeed?
    bool isUpgradeSecondSucceed() const;

    // Did the item break because the upgrade to the next option grade failed?
    bool isUpgradeCrash() const;

    Ratio_t getNextOptionRatio(void) const {
        return m_NextOptionRatio;
    }
    void setNextOptionRatio(Ratio_t ratio) {
        m_NextOptionRatio = ratio;
    }

    int getGrade() const {
        return m_Grade;
    }
    void setGrade(int grade) {
        m_Grade = grade;
    }

    string toString() const;


    ///// Member data /////

private:
    OptionType_t m_OptionType; // Option type
    string m_Name;             // English name
    string m_HName;            // Korean name
    string m_Nickname;         // Nickname
    OptionClass m_OptionClass; // Kind of option
    int m_PlusPoint;           // Increase amount
    int m_PriceMultiplier;     // Price increase factor
    int m_ReqSTR;              // Increase in required STR
    int m_ReqDEX;              // Increase in required DEX
    int m_ReqINT;              // Increase in required INT
    int m_ReqSum;              // Increase in the required attribute total
    int m_ReqLevel;            // Increase in the required level
    int m_Level;               // Option level
    Color_t m_Color;           // Option color
    Ratio_t m_Ratio;           // Probability the option appears

    int m_GambleLevel; // Gamble level of the option

    // Information needed when upgrading to the next option grade
    OptionType_t m_PreviousOptionType; // Previous option grade's type
    OptionType_t m_UpgradeOptionType;  // Next option grade's type
    Ratio_t m_UpgradeRatio;            // Upgrade success probability = 1/m_UpgradeRatio
    Ratio_t m_UpgradeSecondRatio;      // Success probability of upgrading with Blue Drop 2
    int m_UpgradeCrashPercent;         // Probability of breaking on a failed upgrade = m_UpgradeCrashPercent%
    Ratio_t m_NextOptionRatio;         // Probability that the next option is attached

    int m_Grade; // Grade of the option within the same class
};

//////////////////////////////////////////////////////////////////////////////
// class OptionInfoSet
// Class that stores the option type per level for each option class.
//////////////////////////////////////////////////////////////////////////////

class OptionInfoSet {
public:
    OptionInfoSet();
    ~OptionInfoSet();

public:
    void addOptionType(uint level, OptionType_t type);

    void getPossibleOptionTypes(uint minLevel, uint maxLevel, vector<OptionType_t>& rOptionVector);

    void clear() {
        m_OptionTypes.clear();
    }

    string toString(void) const;

private:
    unordered_map<uint, OptionType_t> m_OptionTypes;
};

//////////////////////////////////////////////////////////////////////////////
// class PetEnchantOption
//////////////////////////////////////////////////////////////////////////////
class PetEnchantOption {
public:
    PetEnchantOption() {
        m_Type = 0;
        m_Ratio = 0;
    }
    ~PetEnchantOption() {}

    void setOptionType(OptionType_t type) {
        m_Type = type;
    }
    OptionType_t getOptionType() const {
        return m_Type;
    }

    void setRatio(int ratio) {
        m_Ratio = ratio;
    }
    int getRatio() const {
        return m_Ratio;
    }

private:
    OptionType_t m_Type;
    int m_Ratio;
};


//////////////////////////////////////////////////////////////////////////////
// class OptionInfoManager
//////////////////////////////////////////////////////////////////////////////

class OptionInfoManager {
    ///// Member methods /////

public:
    OptionInfoManager();
    ~OptionInfoManager();

public:
    void init();
    void load();

    void release();

    OptionInfo* getOptionInfo(OptionType_t OptionType);
    OptionInfo* getOptionInfo(const string& nickname);
    OptionType_t getOptionType(const string& nickname);
    string getOptionName(const list<OptionType_t>& optionTypes);

    vector<OptionType_t> getPossibleOptionVector(Item::ItemClass IClass, uint minLevel, uint maxLevel);

    void addOptionInfo(OptionInfo* pOptionInfo);

    // Gamble related
    void addGambleOption(Item::ItemClass itemClass, uint level, OptionType_t optionType);
    const vector<OptionType_t>& getPossibleGambleOptionVector(Item::ItemClass itemClass, uint level) {
        return m_GambleOptions[itemClass][level];
    }
    int getTotalGambleRatio(Item::ItemClass itemClass, uint level) {
        return m_TotalGambleRatio[itemClass][level];
    }

    // Rare enchant related
    int getRareUpgradeRatio(OptionType_t optionType, bool success);
    const OptionClassInfo* getOptionClassInfo(OptionClass oc) {
        return m_OptionClassInfos[oc];
    }

    void addPetEnchantOption(PetEnchantOption* pPetEnchantOption);
    const list<PetEnchantOption*>& getPetEnchantOptionList() const {
        return m_PetEnchantOptionList;
    }

    void setTotalPetEnchantOption(int total) {
        m_ToTalPetEnchantOption = total;
    }
    int getTotalPetEnchantOption() {
        return m_ToTalPetEnchantOption;
    }

    string toString() const;

private:
    void addOptionClassInfo(OptionClassInfo* pInfo) {
        m_OptionClassInfos[pInfo->getOptionClass()] = pInfo;
    }

    uint m_nOptionCount;
    unordered_map<OptionType_t, OptionInfo*> m_OptionInfos;
    unordered_map<string, OptionInfo*> m_NicknameOptionInfos;
    OptionInfoSet m_OptionInfoSet[OPTION_MAX];
    vector<OptionClassInfo*> m_OptionClassInfos;
    vector<OptionType_t> m_GambleOptions[Item::ITEM_CLASS_MAX][GAMBLE_OPTION_LEVEL_MAX + 1];
    int m_TotalGambleRatio[Item::ITEM_CLASS_MAX][GAMBLE_OPTION_LEVEL_MAX + 1];
    unordered_map<int, RareOptionUpgradeInfo*> m_RareEnchantInfo;

    list<PetEnchantOption*> m_PetEnchantOptionList;
    int m_ToTalPetEnchantOption;
};


//////////////////////////////////////////////////////////////////////////////
// global varibles
//////////////////////////////////////////////////////////////////////////////
extern OptionInfoManager* g_pOptionInfoManager;


#endif
