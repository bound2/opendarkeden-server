//////////////////////////////////////////////////////////////////////////////
// Filename    : SkillInfo.h
// Written By  : beowulf
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_INFO_H__
#define __SKILL_INFO_H__

#include <bitset>
#include <list>

#include "Exception.h"
#include "Slayer.h"
#include "Types.h"

class Ousters;

enum {
    SKILL_PROPERTY_TYPE_MELEE = 0,
    SKILL_PROPERTY_TYPE_MAGIC = 1,
    SKILL_PROPERTY_TYPE_PHYSIC = 2,

    SKILL_PROPERTY_TYPE_MAX
};


//////////////////////////////////////////////////////////////////////////////
// Class SkillInfo
//////////////////////////////////////////////////////////////////////////////

class SkillInfo {
public:
    SkillInfo();
    ~SkillInfo();

public:
    uint getType() const {
        return m_Type;
    }
    void setType(uint type) {
        m_Type = type;
    }

    string getName() const {
        return m_Name;
    }
    void setName(string name) {
        m_Name = name;
    }

    uint getLevel() const {
        return m_Level;
    }
    void setLevel(uint level) {
        m_Level = level;
    }

    uint getMinDamage() const {
        return m_MinDamage;
    }
    void setMinDamage(uint mindam) {
        m_MinDamage = mindam;
    }

    uint getMaxDamage() const {
        return m_MaxDamage;
    }
    void setMaxDamage(uint maxdam) {
        m_MaxDamage = maxdam;
    }

    uint getMinDelay() const {
        return m_MinDelay;
    }
    void setMinDelay(uint mindelay) {
        m_MinDelay = mindelay;
    }

    uint getMaxDelay() const {
        return m_MaxDelay;
    }
    void setMaxDelay(uint maxdelay) {
        m_MaxDelay = maxdelay;
    }

    uint getMinCastTime() const {
        return m_MinCastTime;
    }
    void setMinCastTime(uint mincasttime) {
        m_MinCastTime = mincasttime;
    }

    uint getMaxCastTime() const {
        return m_MaxCastTime;
    }
    void setMaxCastTime(uint maxcasttime) {
        m_MaxCastTime = maxcasttime;
    }

    int getMinDuration() const {
        return m_MinDuration;
    }
    void setMinDuration(int minduration) {
        m_MinDuration = minduration;
    }

    int getMaxDuration() const {
        return m_MaxDuration;
    }
    void setMaxDuration(int maxduration) {
        m_MaxDuration = maxduration;
    }

    uint getConsumeMP() const {
        return m_ConsumeMP;
    }
    void setConsumeMP(uint consumemp) {
        m_ConsumeMP = consumemp;
    }

    uint getRange() const {
        return m_MaxRange;
    }

    uint getMaxRange() const {
        return m_MaxRange;
    }
    void setMaxRange(uint range) {
        m_MaxRange = range;
    }

    uint getMinRange() const {
        return m_MinRange;
    }
    void setMinRange(uint range) {
        m_MinRange = range;
    }

    uint getTarget() const {
        return m_Target;
    }
    void setTarget(uint target) {
        m_Target = target;
    }

    // For the new version (the merged version).
    uint getSubSkill() const {
        return m_SubSkill;
    }
    void setSubSkill(uint subskill) {
        m_SubSkill = subskill;
    }

    uint getPoint() const {
        return m_Point;
    }
    void setPoint(uint point) {
        m_Point = point;
    }

    BYTE getDomainType() const {
        return m_Domain;
    }
    void setDomainType(uint domain) {
        m_Domain = domain;
    }

    int getMagicDomain(void) const {
        return m_MagicDomain;
    }
    void setMagicDomain(int magic) {
        m_MagicDomain = magic;
    }

    int getElementalDomain(void) const {
        return m_ElementalDomain;
    }
    void setElementalDomain(int elemental) {
        m_ElementalDomain = elemental;
    }

    bool isMelee() const {
        return m_PropertyType.test(SKILL_PROPERTY_TYPE_MELEE);
    }
    void setMelee(bool bMelee) {
        (bMelee ? m_PropertyType.set(SKILL_PROPERTY_TYPE_MELEE) : m_PropertyType.reset(SKILL_PROPERTY_TYPE_MELEE));
    }

    bool isMagic() const {
        return m_PropertyType.test(SKILL_PROPERTY_TYPE_MAGIC);
    }
    void setMagic(bool bMagic) {
        (bMagic ? m_PropertyType.set(SKILL_PROPERTY_TYPE_MAGIC) : m_PropertyType.reset(SKILL_PROPERTY_TYPE_MAGIC));
    }

    bool isPhysic() const {
        return m_PropertyType.test(SKILL_PROPERTY_TYPE_PHYSIC);
    }
    void setPhysic(bool bPhysic) {
        (bPhysic ? m_PropertyType.set(SKILL_PROPERTY_TYPE_PHYSIC) : m_PropertyType.reset(SKILL_PROPERTY_TYPE_PHYSIC));
    }

    // Skill points required to learn this skill.
    int getSkillPoint() const {
        return m_SkillPoint;
    }
    void setSkillPoint(int skillPoint) {
        m_SkillPoint = skillPoint;
    }

    // Skill level-up points.
    int getLevelUpPoint() const {
        return m_LevelUpPoint;
    }
    void setLevelUpPoint(int levelUpPoint) {
        m_LevelUpPoint = levelUpPoint;
    }

    // Skills that must be learned before this one.
    void addRequireSkill(SkillType_t skillType) {
        m_RequireSkills.push_back(skillType);
    }
    list<SkillType_t>& getRequireSkills() {
        return m_RequireSkills;
    }

    // Skills that become learnable once this one is learned.
    void addRequiredSkill(SkillType_t skillType) {
        m_RequiredSkills.push_back(skillType);
    }
    list<SkillType_t>& getRequiredSkills() {
        return m_RequiredSkills;
    }

    BYTE canDelete() const {
        return m_CanDelete;
    }
    void setCanDelete(BYTE canDelete) {
        m_CanDelete = canDelete;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Conditions required to cast. For Ousters.
    ////////////////////////////////////////////////////////////////////////////
    Elemental_t getRequireFire() const {
        return m_RequireFire;
    }
    Elemental_t getRequireWater() const {
        return m_RequireWater;
    }
    Elemental_t getRequireEarth() const {
        return m_RequireEarth;
    }
    Elemental_t getRequireWind() const {
        return m_RequireWind;
    }
    Elemental_t getRequireSum() const {
        return m_RequireSum;
    }

    ElementalType getRequireWristletElemental() const {
        return m_RequireWristletElemental;
    }
    ElementalType getRequireStone1Elemental() const {
        return m_RequireStone1Elemental;
    }
    ElementalType getRequireStone2Elemental() const {
        return m_RequireStone2Elemental;
    }
    ElementalType getRequireStone3Elemental() const {
        return m_RequireStone3Elemental;
    }
    ElementalType getRequireStone4Elemental() const {
        return m_RequireStone4Elemental;
    }

    void setRequireFire(Elemental_t require) {
        m_RequireFire = require;
    }
    void setRequireWater(Elemental_t require) {
        m_RequireWater = require;
    }
    void setRequireEarth(Elemental_t require) {
        m_RequireEarth = require;
    }
    void setRequireWind(Elemental_t require) {
        m_RequireWind = require;
    }
    void setRequireSum(Elemental_t require) {
        m_RequireSum = require;
    }

    void setRequireWristletElemental(ElementalType require) {
        m_RequireWristletElemental = require;
    }
    void setRequireStone1Elemental(ElementalType require) {
        m_RequireStone1Elemental = require;
    }
    void setRequireStone2Elemental(ElementalType require) {
        m_RequireStone2Elemental = require;
    }
    void setRequireStone3Elemental(ElementalType require) {
        m_RequireStone3Elemental = require;
    }
    void setRequireStone4Elemental(ElementalType require) {
        m_RequireStone4Elemental = require;
    }

    void setRequireSkill(const string& requireSkill);
    void setCondition(const string& condition);

    // toString
    string toString() const;

private:
    uint m_Type;        // Type
    string m_Name;      // Name
    uint m_Level;       // m_Level
    uint m_MinDamage;   // minimum damage
    uint m_MaxDamage;   // maximum damage
    uint m_MinDelay;    // minimum delay
    uint m_MaxDelay;    // maximum delay
    uint m_MinCastTime; // casting time, in seconds
    uint m_MaxCastTime; // maximum casting time

    // Minimum duration.
    // A skill with a single duration rather than a min/max pair is read from the minimum duration.
    // A duration of 0 means Instant, -1 means Long.
    int m_MinDuration;

    // Maximum duration.
    int m_MaxDuration;

    // MP consumption.
    uint m_ConsumeMP;

    // Maximum and minimum range; sight?
    uint m_MaxRange;
    uint m_MinRange;

    // MoveType of the targets that can be hit.
    // 0x01 : burrowing
    // 0x02 : walking
    // 0x04 : flying
    uint m_Target;

    // For the merged version.
    uint m_SubSkill;
    uint m_Point;

    BYTE m_Domain;

    // Magic domain: poison, acid, curse and so on.
    int m_MagicDomain;

    // Elemental domain.
    int m_ElementalDomain;

    // Skill points required.
    int m_SkillPoint;
    int m_LevelUpPoint;

    // Skills that must be learned before this one.
    list<SkillType_t> m_RequireSkills;

    // Skills that can be learned after this one.
    list<SkillType_t> m_RequiredSkills;

    BYTE m_CanDelete;

    // Conditions required to cast.
    // For Ousters.
    Elemental_t m_RequireFire;
    Elemental_t m_RequireWater;
    Elemental_t m_RequireEarth;
    Elemental_t m_RequireWind;
    Elemental_t m_RequireSum;

    ElementalType m_RequireWristletElemental;
    ElementalType m_RequireStone1Elemental;
    ElementalType m_RequireStone2Elemental;
    ElementalType m_RequireStone3Elemental;
    ElementalType m_RequireStone4Elemental;

    bitset<SKILL_PROPERTY_TYPE_MAX> m_PropertyType;
};

//////////////////////////////////////////////////////////////////////////////
// Class SkillInfoManager
//////////////////////////////////////////////////////////////////////////////

class SkillInfoManager {
public:
    SkillInfoManager();
    ~SkillInfoManager();

public:
    // initialize Manager
    void init();

    // void load() ;
    void load();

    // Save to DB
    void save();

    // get SkillInfo
    SkillInfo* getSkillInfo(SkillType_t SkillType) const;

    // add SkillInfo
    void addSkillInfo(SkillInfo* pSkillInfo);

    // get SkillType by LearnLevel
    // Find which skill type can be learned at the current domain level.
    // The domain level tells you.
    SkillType_t getSkillTypeByLevel(SkillDomainType_t SkillDomain, Level_t Level);

    // Return the grade that a domain level corresponds to.
    SkillGrade getGradeByDomainLevel(Level_t Level);

    // Find how far a skill can be levelled up at the current grade.
    Level_t getLimitLevelByDomainGrade(SkillGrade Grade);

    // Return the MP consumption reduction for the given INT.
    MP_t getdecreaseConsumeMP(Attr_t INT) {
        return m_decreaseConsumeMP[INT];
    }

    // toString for Debug
    string toString() const;

private:
    uint m_SkillCount;
    SkillInfo** m_SkillInfoList;
    SkillType_t m_SkillLevelMap[SKILL_DOMAIN_MAX][SLAYER_MAX_DOMAIN_LEVEL + 1];
    SkillGrade m_DomainGradeMap[SLAYER_MAX_DOMAIN_LEVEL + 1];
    Level_t m_LimitLevelMap[SKILL_GRADE_MAX + 1];
    MP_t m_decreaseConsumeMP[300 + 1];
};

#endif // __SKILL_INFO_MANAGER_H__
