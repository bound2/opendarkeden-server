//////////////////////////////////////////////////////////////////////////////
// Filename    : Vampire.h
// Written By  : Elca
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __VAMPIRE_H__
#define __VAMPIRE_H__

#include "CreatureUtil.h"
#include "ExtraInfo.h"
#include "GearInfo.h"
#include "Inventory.h"
#include "InventoryInfo.h"
#include "ModifyInfo.h"
#include "Mutex.h"
#include "OptionInfo.h"
#include "PCVampireInfo.h"
#include "PCVampireInfo2.h"
#include "PCVampireInfo3.h"
#include "PlayerCreature.h"
#include "VampireSkillInfo.h"
#include "Zone.h"
// #include "RankExpTable.h"
#include <unordered_map>

#include "GCModifyInformation.h"
#include "skill/VampireSkillSlot.h"

#define VAMPIRE_MAX_LEVEL 150     // abcd
#define VAMPIRE_MAX_RANK 50       // max rank
#define BONUS_POINTS_PER_LEVEL1 3 // ability points granted on level up
#define BONUS_POINTS_PER_LEVEL2 2 // ability points granted on level up
#define BONUS_POINTS_PER_LEVEL3 1 // ability points granted on level up

////////////////////////////////////////////////////////////////////////////////
// Buffer class that stores the stats from before a piece of equipment
// was put on or taken off.
////////////////////////////////////////////////////////////////////////////////

class VAMPIRE_RECORD {
public:
    Attr_t pSTR[3];
    Attr_t pDEX[3];
    Attr_t pINT[3];
    Rank_t Rank;
    HP_t pHP[2];
    Damage_t pDamage[2];
    Defense_t Defense;
    Protection_t Protection;
    ToHit_t ToHit;
    Speed_t AttackSpeed;
};

////////////////////////////////////////////////////////////////////////////////
//
// Class Vampire;
//
////////////////////////////////////////////////////////////////////////////////

class Vampire : public PlayerCreature {
    ////////////////////////////////////////////////////
    // Class-internal constant declarations
    ////////////////////////////////////////////////////
public:
public:
    // Wear parts
    enum WearPart {
        WEAR_NECK,
        WEAR_BODY,
        WEAR_WRIST1,
        WEAR_WRIST2,
        WEAR_FINGER1,
        WEAR_FINGER2,
        WEAR_FINGER3,
        WEAR_FINGER4,
        WEAR_EARRING1,
        WEAR_EARRING2,
        WEAR_LEFTHAND,
        WEAR_RIGHTHAND,
        WEAR_AMULET1,
        WEAR_AMULET2,
        WEAR_AMULET3,
        WEAR_AMULET4,
        WEAR_ZAP1,
        WEAR_ZAP2,
        WEAR_ZAP3,
        WEAR_ZAP4,
        WEAR_DERMIS,
        WEAR_PERSONA,
        VAMPIRE_WEAR_MAX
    };

    ////////////////////////////////////////////////////
    // Constructor / destructor
    ////////////////////////////////////////////////////
public:
    Vampire();
    virtual ~Vampire();

    ////////////////////////////////////////////////////
    // Functions inherited from Creature
    ////////////////////////////////////////////////////
public:
    virtual CreatureClass getCreatureClass() const {
        return CREATURE_CLASS_VAMPIRE;
    }
    virtual string getCreatureClassString() const {
        return "CREATURE_CLASS_VAMPIRE";
    }

    virtual void registerObject();
    virtual void registerInitObject();

    virtual bool load();
    void loadItem(bool checkTimeLimit = false);

    virtual void save() const;
    void saveSkills(void) const;
    void saveGears(void) const;

    virtual void act(const Timeval& currentTime) {}

    virtual string toString() const;

    virtual Race_t getRace() const {
        return RACE_VAMPIRE;
    }
    virtual GuildID_t getCommonGuildID() const {
        return VampireCommon;
    }


    //////////////////////////////////////////////////////////////
    // Time-limited item functions
    //////////////////////////////////////////////////////////////
public:
    void checkItemTimeLimit();
    void updateEventItemTime(DWORD time);

    ////////////////////////////////////////////////////
    // State functions (Dead or Alive)
    ////////////////////////////////////////////////////
public:
    bool isAlive() const {
        return m_HP[ATTR_CURRENT] != 0;
    }
    bool isDead() const {
        return m_HP[ATTR_CURRENT] == 0;
    }


    ////////////////////////////////////////////////////
    // Appearance functions (name, competence, sex, hair, skin)
    ////////////////////////////////////////////////////
public:
    void setName(const string& name) {
        m_Name = name;
        m_Owner = name;
    }

    BYTE getCompetence() const {
        return m_Competence;
    }
    void setCompetence(BYTE Competence) {
        m_Competence = Competence;
    }

    BYTE getCompetenceShape() const {
        return m_CompetenceShape;
    }
    void setCompetenceShape(BYTE CompetenceShape) {
        m_CompetenceShape = CompetenceShape;
    }

    Sex getSex() const {
        return m_Sex;
    }
    void setSex(Sex sex) {
        m_Sex = sex;
        m_VampireInfo.setSex(sex);
    }
    void setSex(const string& sex) {
        if (sex == Sex2String[MALE])
            setSex(MALE);
        else if (sex == Sex2String[FEMALE])
            setSex(FEMALE);
        else
            throw InvalidProtocolException("invalid sex");
    }

    Color_t getBatColor() const {
        return m_BatColor;
    }
    void setBatColor(Color_t batColor) {
        m_BatColor = batColor;
    }

    Color_t getSkinColor() const {
        return m_SkinColor;
    }
    void setSkinColor(Color_t skinColor) {
        m_SkinColor = skinColor;
    }

    ////////////////////////////////////////////////////
    // Ability functions (STR, DEX, INT)
    ////////////////////////////////////////////////////
public:
    Alignment_t getAlignment() const {
        return m_Alignment;
    }
    void setAlignment(Alignment_t Alignment) {
        m_Alignment = Alignment;
    }

    Attr_t getSTR(AttrType attrType = ATTR_CURRENT) const {
        return m_STR[attrType];
    }
    void setSTR(Attr_t attr, AttrType attrType = ATTR_CURRENT) {
        m_STR[attrType] = attr;
    }

    Attr_t getDEX(AttrType attrType = ATTR_CURRENT) const {
        return m_DEX[attrType];
    }
    void setDEX(Attr_t attr, AttrType attrType = ATTR_CURRENT) {
        m_DEX[attrType] = attr;
    }

    Attr_t getINT(AttrType attrType = ATTR_CURRENT) const {
        return m_INT[attrType];
    }
    void setINT(Attr_t attr, AttrType attrType = ATTR_CURRENT) {
        m_INT[attrType] = attr;
    }

    ////////////////////////////////////////////////////
    // Ability functions (HP)
    ////////////////////////////////////////////////////
public:
    HP_t getHP(AttrType attrType = ATTR_CURRENT) const {
        return m_HP[attrType];
    }
    void setHP(HP_t hp, AttrType attrType = ATTR_CURRENT) {
        m_HP[attrType] = hp;
    }
    void setHP(HP_t current, HP_t max) {
        m_HP[ATTR_CURRENT] = current;
        m_HP[ATTR_MAX] = max;
    }

    ////////////////////////////////////////////////////
    // Ability functions (Damage, Protect, Defense, ToHit)
    ////////////////////////////////////////////////////
public:
    Damage_t getDamage(AttrType attrType = ATTR_CURRENT) const {
        return m_Damage[attrType];
    }
    Protection_t getProtection(void) const {
        return m_Protection[ATTR_CURRENT];
    }
    Defense_t getDefense(void) const {
        return m_Defense[ATTR_CURRENT];
    }
    ToHit_t getToHit(void) const {
        return m_ToHit[ATTR_CURRENT];
    }
    Speed_t getAttackSpeed(void) const {
        return m_AttackSpeed[ATTR_CURRENT];
    }

    ////////////////////////////////////////////////////
    // Experience functions
    ////////////////////////////////////////////////////
public:
    //	Exp_t getExp() const  { return m_Exp; }
    //	void  setExp(Exp_t exp)  { m_Exp = exp; }

    Exp_t getGoalExp() const {
        return m_GoalExp;
    }
    void setGoalExp(Exp_t GoalExp) {
        m_GoalExp = GoalExp;
    }

    //	Exp_t getExpOffset() const  { return m_ExpOffset; }
    //	void  setExpOffset(Exp_t expOffset)  { m_ExpOffset = expOffset; }

    //	void setExp(Exp_t exp, Exp_t expOffset)  { m_Exp = exp; m_ExpOffset = expOffset; }

    Level_t getLevel() const {
        return m_Level;
    }
    void setLevel(Level_t level) {
        m_Level = level;
    }

    int getQuestLevel() const {
        return getLevel();
    }

    // Rank.


    Bonus_t getBonus() const {
        return m_Bonus;
    }
    void setBonus(Bonus_t bonus) {
        m_Bonus = bonus;
    }

    ////////////////////////////////////////////////////
    // Skill functions
    ////////////////////////////////////////////////////
public:
    void addSkill(SkillType_t SkillType);
    void addSkill(VampireSkillSlot* pSkillSlot);
    void removeCastleSkill(SkillType_t SkillType);
    void removeAllCastleSkill();
    VampireSkillSlot* hasSkill(SkillType_t SkillType) const {
        return getSkill(SkillType);
    }
    VampireSkillSlot* getSkill(SkillType_t SkillType) const;

    //	void setHotKey(BYTE pos, SkillType_t SkillType)  { m_HotKey[pos] = SkillType; }
    //	SkillType_t getHotKey(BYTE pos) const  { return m_HotKey[pos]; }


    ////////////////////////////////////////////////////
    // Item wear/unwear functions
    ////////////////////////////////////////////////////
public:
    bool isWear(WearPart Part) {
        return m_pWearItem[Part] != NULL ? true : false;
    }
    void addWearItem(WearPart Part, Item* pItem) {
        Assert(m_pWearItem[Part] != NULL);
        m_pWearItem[Part] = pItem;
    }
    void deleteWearItem(WearPart Part) {
        m_pWearItem[Part] = NULL;
    }
    Item* getWearItem(WearPart Part) {
        return m_pWearItem[Part];
    }

    void wearItem(WearPart Part);
    void wearItem(WearPart Part, Item* pItem);
    void takeOffItem(WearPart Part, bool bAddOnMouse, bool bSendModifyInfo);
    // WearPart getWearPart(Item::ItemClass IClass) const;
    // bool    changeShape(Item* pItem, Color_t color);
    bool addShape(Item::ItemClass IClass, ItemType_t IType, Color_t color);
    bool removeShape(Item::ItemClass IClass, bool bSendPacket = true);


    void destroyGears();

    bool isRealWearing(WearPart Part) const;
    bool isRealWearing(Item* pItem) const;
    bool isRealWearingEx(WearPart Part) const;
    DWORD sendRealWearingInfo(void) const;

    void getShapeInfo(DWORD& flag, Color_t color[PCVampireInfo::VAMPIRE_COLOR_MAX]) const;

    ////////////////////////////////////////////////////
    // Functions that adjust the stats when an item is worn or removed
    ////////////////////////////////////////////////////
public:
    void initAllStat(int numPartyMember = -1);
    void initCastleSkill();
    void sendModifyInfo(const VAMPIRE_RECORD& previous) const;
    void addModifyInfo(const VAMPIRE_RECORD& previous, ModifyInfo& pkt) const;
    void initAllStatAndSend();

private:
    void computeStatOffset(void);
    void computeItemStat(Item* pItem);
    void computeOptionStat(Item* pItem);
    void computeOptionStat(OptionType_t optionType);
    void computeOptionClassStat(OptionClass OClass, int PlusPoint);


    //////////////////////////////////////////////////////////////
    // Item lookup functions
    //////////////////////////////////////////////////////////////
public:
    virtual Item* findItemOID(ObjectID_t id) {
        return ::findItemOID(this, id);
    }
    virtual Item* findItemOID(ObjectID_t id, int& storage, int& x, int& y) {
        return ::findItemOID(this, id, storage, x, y);
    }

    virtual Item* findItemIID(ItemID_t id) {
        return ::findItemIID(this, id);
    }
    virtual Item* findItemIID(ItemID_t id, int& storage, int& x, int& y) {
        return ::findItemIID(this, id, storage, x, y);
    }

    virtual Item* findBeltOID(ObjectID_t id) {
        return ::findItemOID(this, id, Item::ITEM_CLASS_BELT);
    }
    virtual Item* findBeltOID(ObjectID_t id, int& storage, int& x, int& y) {
        return ::findItemOID(this, id, Item::ITEM_CLASS_BELT, storage, x, y);
    }

    virtual Item* findBeltIID(ItemID_t id) {
        return ::findItemIID(this, id, Item::ITEM_CLASS_BELT);
    }
    virtual Item* findBeltIID(ItemID_t id, int& storage, int& x, int& y) {
        return ::findItemIID(this, id, Item::ITEM_CLASS_BELT, storage, x, y);
    }

    ////////////////////////////////////////////////////
    // Info functions
    ////////////////////////////////////////////////////
public:
    PCVampireInfo2* getVampireInfo2();
    PCVampireInfo3 getVampireInfo3() const;
    GearInfo* getGearInfo() const;
    void sendVampireSkillInfo();


    ////////////////////////////////////////////////////
    // Miscellaneous functions
    ////////////////////////////////////////////////////
public:
    void setInMagics(const string& blob) {}

    void heartbeat(const Timeval& currentTime);

    void getVampireRecord(VAMPIRE_RECORD& record) const;

    virtual Fame_t getFame() const {
        return m_Fame;
    }
    virtual void setFame(Fame_t fame) {
        m_Fame = fame;
    }

    virtual ZoneID_t getResurrectZoneID(void) const {
        return m_ResurrectZoneID;
    }
    virtual void setResurrectZoneID(ZoneID_t id) {
        m_ResurrectZoneID = id;
    }

    Silver_t getSilverDamage() const {
        return m_SilverDamage;
    }
    void setSilverDamage(Silver_t damage) {
        m_SilverDamage = damage;
    }
    void saveSilverDamage(Silver_t damage);

    virtual Steal_t getHPStealAmount(void) const {
        return m_HPStealAmount;
    }
    virtual void setHPStealAmount(Steal_t steal) {
        m_HPStealAmount = steal;
    }

    virtual Steal_t getHPStealRatio(void) const {
        return m_HPStealRatio;
    }
    virtual void setHPStealRatio(Steal_t steal) {
        m_HPStealRatio = steal;
    }

    virtual Regen_t getHPRegen(void) const {
        return m_HPRegen;
    }
    virtual void setHPRegen(Regen_t regen) {
        m_HPRegen = regen;
    }

    virtual Luck_t getLuck(void) const {
        return m_Luck;
    }
    virtual void setLuck(Luck_t luck) {
        m_Luck = luck;
    }


    virtual int getCriticalRatio(void) const {
        return m_CriticalRatio[ATTR_CURRENT];
    }
    virtual void setCriticalRatio(int ratio) {
        m_CriticalRatio[ATTR_CURRENT] = ratio;
    }


    virtual ClanType_t getClanType(void) const {
        return m_ClanType;
    }
    virtual void setClanType(ClanType_t clan) {
        m_ClanType = clan;
    }

    //	WORD getRankExpSaveCount(void) const { return m_RankExpSaveCount; }
    //	void setRankExpSaveCount(WORD count) { m_RankExpSaveCount = count; }

    WORD getExpSaveCount(void) const {
        return m_ExpSaveCount;
    }
    void setExpSaveCount(WORD count) {
        m_ExpSaveCount = count;
    }

    WORD getFameSaveCount(void) const {
        return m_FameSaveCount;
    }
    void setFameSaveCount(WORD count) {
        m_FameSaveCount = count;
    }

    WORD getAlignmentSaveCount(void) const {
        return m_AlignmentSaveCount;
    }
    void setAlignmentSaveCount(WORD count) {
        m_AlignmentSaveCount = count;
    }

    int getMastery(uint domain) const {
        return m_Mastery[domain];
    }
    void setMastery(uint domain, int mastery) {
        m_Mastery[domain] = mastery;
    }

    //	void increaseVampExp(Exp_t Point);
    //	void increaseRankExp(RankExp_t Point);

public:
    void initPetQuestTarget();
    int getBloodBibleSignOpenNum() const;

    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////

private:
    // Competence
    BYTE m_Competence;
    BYTE m_CompetenceShape;

    // Sex, Color
    Sex m_Sex;
    Color_t m_BatColor;
    Color_t m_SkinColor;

    // Alignment
    Alignment_t m_Alignment;

    // STR/DEX/INT
    Attr_t m_STR[3];
    Attr_t m_DEX[3];
    Attr_t m_INT[3];

    // HP
    HP_t m_HP[3];

    Damage_t m_Damage[3];
    ToHit_t m_ToHit[2];
    Defense_t m_Defense[2];
    Protection_t m_Protection[2];
    Speed_t m_AttackSpeed[2];
    int m_CriticalRatio[2];

    // Exp/ExpOffset
    //	Exp_t m_Exp;
    //	Exp_t m_ExpOffset;

    // Gaol Exp
    Exp_t m_GoalExp;

    // level
    Level_t m_Level;

    // exp bonus
    Bonus_t m_Bonus;


    Fame_t m_Fame;

    ZoneCoord_t m_VisionWidth;
    ZoneCoord_t m_VisionHeight;

    // SkillSlot
    // VampireSkillSlot* m_pSkillSlot[MAX_VAMPIRE_SKILL_SLOT];
    unordered_map<SkillType_t, VampireSkillSlot*> m_SkillSlot;

    // WearItem Field
    Item* m_pWearItem[VAMPIRE_WEAR_MAX];
    bool m_pRealWearingCheck[VAMPIRE_WEAR_MAX];

    // Vampire Outlook Information
    mutable PCVampireInfo3 m_VampireInfo;

    // Hot Key Set
    //	SkillType_t m_HotKey[8];

    // mutable Thread Mutex
    mutable Mutex m_Mutex;

    // Resurrection zone
    ZoneID_t m_ResurrectZoneID;

    // Silver damage
    Silver_t m_SilverDamage;

    // HP steal
    Steal_t m_HPStealAmount;
    Steal_t m_HPStealRatio;

    // HP regeneration
    Regen_t m_HPRegen;

    // HP regeneration bonus points, gained per second
    Regen_t m_HPRegenBonus;

    Luck_t m_Luck;

    Timeval m_HPRegenTime;

    ClanType_t m_ClanType;

    // Save counts for the various experience values
    //	WORD m_RankExpSaveCount;
    WORD m_ExpSaveCount;
    WORD m_FameSaveCount;
    WORD m_AlignmentSaveCount;

    int m_Mastery[MAGIC_DOMAIN_MAX];
};


#endif // __VAMPIRE_H__
