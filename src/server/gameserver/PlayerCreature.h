//////////////////////////////////////////////////////////////////////////////
// Filename    : PlayerCreature.h
// Written by  : excel96
// Description :
// Class that collects the parts common to the Slayer and Vampire class
// interfaces. It is abstract, so it must not be instantiated directly.
//////////////////////////////////////////////////////////////////////////////

#ifndef __PLAYER_CREATURE_H__
#define __PLAYER_CREATURE_H__

#include "Creature.h"
#include "ExpFwd.h"
#include "Garbage.h"
#include "InventorySlot.h"
#include "ObjectRegistry.h"
#include "RankBonus.h"
// #include "RankExpTable.h"
// #include "ItemNameInfo.h"
// #include "quest/Squest/QuestManager.h"
#include <atomic>
#include <bitset>
#include <vector>

#include <forward_list>
#include <unordered_map>

#include "GCMonsterKillQuestInfo.h"
#include "OptionInfo.h"
#include "repository/CharacterRace.h"

static const GuildID_t SlayerCommon = 99;
static const GuildID_t VampireCommon = 0;
static const GuildID_t OustersCommon = 66;


//////////////////////////////////////////////////////////////////////////////
// class PlayerCreature
//////////////////////////////////////////////////////////////////////////////

class Inventory;
class InventoryInfo;
class InventorySlot;
class ExtraInfo;
class Stash;
class Player;
class FlagSet;
class QuestManager;
class TimeLimitItemManager;
class GoodsInventory;
class PetInfo;
class PetItem;
class Pet;
class SMSAddressBook;
class NicknameBook;
class NicknameInfo;
class GQuestManager;
class BloodBibleSignInfo;
class Store;
// class GCMonsterKillQuestInfo;
// struct GCMonsterKillQuestInfo::QuestInfo;

typedef unordered_map<DWORD, RankBonus*> HashMapRankBonus;
typedef HashMapRankBonus::iterator HashMapRankBonusItor;
typedef HashMapRankBonus::const_iterator HashMapRankBonusConstItor;


class PlayerCreature : public Creature {
    ////////////////////////////////////////////////////////////
    // Constructor and destructor
    ////////////////////////////////////////////////////////////
public:
    PlayerCreature(ObjectID_t objectID = 0, Player* pPlayer = NULL);
    virtual ~PlayerCreature();

    virtual bool load();
    virtual void tinysave(const string& field) const;
    //	virtual void tinysave(const char* field) const  = 0;

    ////////////////////////////////////////////////////////////
    // OID registration methods
    ////////////////////////////////////////////////////////////
protected:
    // Which race table this character's rows live in.
    CharacterRace characterRace() const;

    // The experience tail the vampire and ousters rows share. Both values
    // are passed in: the goal experience because a slayer keeps one per
    // skill domain, the silver damage because a slayer has none.
    void saveExps(Exp_t goalExp, Silver_t silverDamage) const;

    virtual void registerItem(Item* pItem, ObjectRegistry& OR);

public:
    // Assign object ids to everything a freshly loaded character owns.
    virtual void registerInitObject() = 0;

    virtual void registerInventory(ObjectRegistry& OR);
    virtual void registerInitInventory(ObjectRegistry& OR);
    virtual void registerStash(void);

    virtual void registerGoodsInventory(ObjectRegistry& OR);

    //////////////////////////////////////////////////////////////
    // Time-limited item functions
    //////////////////////////////////////////////////////////////
public:
    bool wasteIfTimeLimitExpired(Item* pItem);
    virtual void checkItemTimeLimit() = 0;
    void sendTimeLimitItemInfo();
    void addTimeLimitItem(Item* pItem, DWORD time);
    void sellItem(Item* pItem);
    void deleteItemByMorph(Item* pItem);
    void updateItemTimeLimit(Item* pItem, DWORD time);
    virtual void updateEventItemTime(DWORD time) = 0;
    void loadTimeLimitItem();

    //////////////////////////////////////////////////////////////
    // Purchased goods item functions
    //////////////////////////////////////////////////////////////
public:
    void loadGoods();

    //////////////////////////////////////////////////////////////
    // Quest manager functions
    //////////////////////////////////////////////////////////////
public:
    QuestManager* getQuestManager() const {
        return m_pQuestManager;
    }
    void sendCurrentQuestInfo() const;

    virtual int getQuestLevel() const = 0;
    virtual void whenQuestLevelUpgrade();

    ////////////////////////////////////////////////////////////
    // Inventory methods
    ////////////////////////////////////////////////////////////
public:
    virtual Inventory* getInventory() const {
        return m_pInventory;
    }
    virtual void setInventory(Inventory* pInventory) {
        m_pInventory = pInventory;
    }

    virtual InventorySlot* getExtraInventorySlot() {
        return m_pExtraInventorySlot;
    }
    virtual Item* getExtraInventorySlotItem() {
        return m_pExtraInventorySlot->getItem();
    }
    virtual void deleteItemFromExtraInventorySlot() {
        m_pExtraInventorySlot->deleteItem();
    }
    virtual void addItemToExtraInventorySlot(Item* pItem) {
        m_pExtraInventorySlot->addItem(pItem);
    }

    // 2003.04.04. by Sequoia
    virtual void loadItem();

    // Rebuild the inventory and everything hanging off it for a character
    // that has just connected, and recompute its stats from the gear. Two
    // steps of it are the race's own: which overload of the item loader
    // takes the character, and whether a first-time character is given a
    // newbie set here.
    void loadItem(bool checkTimeLimit);

    // Hand this character to the item loader. The manager's overload set
    // is keyed on the concrete race, so only the race can make the call.
    virtual void loadOwnedItems() = 0;

    // Give a first-time character its starting items. A vampire is made by
    // transformation and gets none, and the two races that do give one read
    // the same flag with opposite senses, so the whole step is the race's.
    virtual void giveNewbieItems() {}

    virtual GoodsInventory* getGoodsInventory() const {
        return m_pGoodsInventory;
    }

    ////////////////////////////////////////////////////////////
    // Stash methods
    ////////////////////////////////////////////////////////////
public:
    virtual Stash* getStash(void) const {
        return m_pStash;
    }
    virtual void setStash(Stash* pStash) {
        m_pStash = pStash;
    }

    virtual BYTE getStashNum(void) const {
        return m_StashNum;
    }
    virtual void setStashNum(BYTE num) {
        m_StashNum = num;
    }
    virtual void setStashNumEx(BYTE num);

    virtual Gold_t getStashGold(void) const {
        return m_StashGold;
    }
    virtual void setStashGold(Gold_t gold) {
        m_StashGold = gold;
    }
    virtual void setStashGoldEx(Gold_t gold);
    virtual void increaseStashGoldEx(Gold_t gold);
    virtual void decreaseStashGoldEx(Gold_t gold);
    virtual bool checkStashGoldIntegrity();

    virtual bool getStashStatus(void) const {
        return m_bStashStatus;
    }
    virtual void setStashStatus(bool s) {
        m_bStashStatus = s;
    }

    virtual void deleteStash(void);


    ////////////////////////////////////////////////////////////
    // Garbage methods
    ////////////////////////////////////////////////////////////
public:
    void addItemToGarbage(Item* pItem) {
        m_Garbage.addItem(pItem);
    }
    void addItemToGarbageEx(Item* pItem) {
        m_Garbage.addItemEx(pItem, getName());
    }
    Item* popItemFromGarbage(void) {
        return m_Garbage.popItem();
    }
    void saveGarbage(void) {
        m_Garbage.save(getName());
    }
    int getGarbageSize(void) {
        return m_Garbage.size();
    }

    ////////////////////////////////////////////////////////////
    // Item search functions
    ////////////////////////////////////////////////////////////
public:
    // The colour the client should paint an item's shape in.
    Color_t getItemShapeColor(Item* pItem, OptionInfo* pOptionInfo = NULL) const;

    // The carried inventory, and the item held on the mouse cursor, as the
    // client is told to draw them.
    InventoryInfo* getInventoryInfo() const;
    ExtraInfo* getExtraInfo() const;

    virtual Item* findItemOID(ObjectID_t id) = 0;
    virtual Item* findItemOID(ObjectID_t id, int& storage, int& x, int& y) = 0;

    virtual Item* findItemIID(ItemID_t id) = 0;
    virtual Item* findItemIID(ItemID_t id, int& storage, int& x, int& y) = 0;

    virtual Item* findBeltOID(ObjectID_t id) = 0;
    virtual Item* findBeltOID(ObjectID_t id, int& storage, int& x, int& y) = 0;

    virtual Item* findBeltIID(ItemID_t id) = 0;
    virtual Item* findBeltIID(ItemID_t id, int& storage, int& x, int& y) = 0;


    ////////////////////////////////////////////////////////////
    // Flag set functions
    ////////////////////////////////////////////////////////////
public:
    FlagSet* getFlagSet(void) const {
        return m_pFlagSet;
    }
    void setFlagSet(FlagSet* pSet) {
        m_pFlagSet = pSet;
    }
    void deleteFlagSet(void);


    ////////////////////////////////////////////////////////////
    // Other functions
    ////////////////////////////////////////////////////////////
public:
    const string& getName() const {
        return m_Name;
    }

    virtual Fame_t getFame() const = 0;
    virtual void setFame(Fame_t fame) = 0;

    virtual Gold_t getGold() const {
        return m_Gold;
    }
    virtual void setGold(Gold_t gold);
    virtual void setGoldEx(Gold_t gold);
    virtual void increaseGoldEx(Gold_t gold);
    virtual void decreaseGoldEx(Gold_t gold);
    virtual bool checkGoldIntegrity();
    bool checkDBGold(Gold_t gold) {
        Gold_t temp = getGold();
        setGold(gold);
        bool ret = checkGoldIntegrity();
        setGold(temp);
        return ret;
    }


    // Damage a silver weapon has dealt and not yet healed away, which caps
    // the current HP. Only a vampire or an ousters row has the column; a
    // slayer leaves the value at zero.
    Silver_t getSilverDamage() const {
        return m_SilverDamage;
    }
    void setSilverDamage(Silver_t damage) {
        m_SilverDamage = damage;
    }
    void saveSilverDamage(Silver_t damage);

    virtual Sex getSex() const = 0;

    virtual ZoneID_t getResurrectZoneID(void) const = 0;
    virtual void setResurrectZoneID(ZoneID_t id) = 0;
    virtual void setResurrectZoneIDEx(ZoneID_t id);

    // virtual Race_t getRace() const = 0; - moved up to Creature.
    virtual GuildID_t getCommonGuildID() const = 0;

    virtual IP_t getIP(void) const;


    ////////////////////////////////////////////////////////////
    // Alignment system
    ////////////////////////////////////////////////////////////
    // enemy specific methods
    void addEnemy(const string& Name);
    void deleteEnemy(const string& Name);

    // Has this particular user already struck first?
    bool hasEnemy(const string& Name) const;
    uint getMaxEnemies() const;

    list<string>& getEnemies(void) {
        return m_Enemies;
    }

    bool isPK() {
        return m_isPK;
    }

    void setPK(bool isPK) {
        m_isPK = isPK;
    }

    void setGuildID(GuildID_t GuildID) {
        m_GuildID = GuildID;
    }
    GuildID_t getGuildID() const {
        return m_GuildID;
    }

    string getGuildName() const;
    GuildMemberRank_t getGuildMemberRank() const;

    Rank_t getRank() const;
    RankExp_t getRankExp() const;
    RankExp_t getRankGoalExp() const;

    //	virtual Rank_t getRank() const  = 0;
    void increaseRankExp(RankExp_t Point);

    WORD getRankExpSaveCount(void) const {
        return m_RankExpSaveCount;
    }
    void setRankExpSaveCount(WORD count) {
        m_RankExpSaveCount = count;
    }

    // Derive the rank from the character's level and write Rank, RankExp
    // and RankGoalExp back to the database. getLevel() is the seam that
    // carries the per-race difference: a slayer's level is its highest
    // skill-domain level, the other two races' their stored level.
    void saveInitialRank();

    virtual Alignment_t getAlignment() const = 0;
    virtual void setAlignment(Alignment_t Alignment) = 0;

    // Set the alignment and write the new value straight back to the database.
    void saveAlignment(Alignment_t alignment);

    ////////////////////////////////////////////////////////////
    // Rank bonus
    ////////////////////////////////////////////////////////////
    void loadRankBonus();
    bool hasRankBonus(RankBonus::RankBonusType type) {
        return m_RankBonusFlag.test(type);
    }
    RankBonus* getRankBonus(RankBonus::RankBonusType type) const;
    RankBonus* getRankBonusByRank(Rank_t rank) const;
    bool learnRankBonus(DWORD type);
    void clearRankBonus();
    void clearRankBonus(Rank_t rank);
    HashMapRankBonus& getRankBonuses() {
        return m_RankBonuses;
    }

    void sendRankBonusInfo();

protected:
    void addRankBonus(RankBonus* rankBonus);

public:
    virtual bool isPayPlayAvaiable();

public:
    Item* getQuestItem() const {
        return m_pQuestItem;
    }
    void setQuestItem(Item* pItem) {
        m_pQuestItem = pItem;
    }

public:
    // by sigi. 2002.12.3

    virtual void initAllStatAndSend() = 0;
    virtual void initAllStat(int numPartyMember) = 0;

    virtual void computeStatOffset(void) = 0;
    virtual void computeItemStat(Item* pItem) = 0;
    virtual void computeOptionStat(Item* pItem) = 0;
    virtual void computeOptionStat(OptionType_t optionType) = 0;
    virtual void computeOptionClassStat(OptionClass OClass, int PlusPoint) = 0;

    void heartbeat(const Timeval& currentTime);

    virtual bool canSee(Object* pObject) const;

    ////////////////////////////////////////////////////////////
    // Skill slot table
    //
    // Each race keys its own slot class by skill type. These are the
    // operations on that table that do not depend on which class it is:
    // the lookup, the two castle-skill removals and the save sweep. The
    // race supplies its slot type, and removeCastleSkillSlot its castle
    // slot type -- only a slot of that class is deleted. The bodies and
    // the explicit instantiations for the three slot types are at the
    // foot of PlayerCreature.cpp; a new slot type is added to that list.
    ////////////////////////////////////////////////////////////
protected:
    template <class SlotType>
    SlotType* findSkillSlot(const unordered_map<SkillType_t, SlotType*>& skillSlots, SkillType_t SkillType) const;

    template <class SlotType, class CastleSlotType>
    void removeCastleSkillSlot(unordered_map<SkillType_t, SlotType*>& skillSlots, SkillType_t SkillType);

    template <class SlotType> void removeAllCastleSkillSlots(unordered_map<SkillType_t, SlotType*>& skillSlots);

    template <class SlotType> void saveSkillSlots(const unordered_map<SkillType_t, SlotType*>& skillSlots) const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
protected:
    Inventory* m_pInventory;              // Inventory pointer
    InventorySlot* m_pExtraInventorySlot; // Mouse pointer

    GoodsInventory* m_pGoodsInventory; // Purchased item inventory

    Stash* m_pStash;     // Stash pointer
    BYTE m_StashNum;     // Number of stashes
    string m_Name;       // PC name
    Gold_t m_Gold = 0;   // money carried by the character
    Gold_t m_StashGold;  // Amount of money in the stash
    bool m_bStashStatus; // Whether the stash items' OIDs are registered

    Silver_t m_SilverDamage = 0; // unhealed silver damage; zero for a slayer

    Garbage m_Garbage; // Garbage

    FlagSet* m_pFlagSet; // Flag set

    // Stores the names of the people who struck first.
    // ObjectIDs could be stored, but self-defense would not be recognized after a death and return, so names are used.
    list<string> m_Enemies;

    // Records whether the character was PKed or not.
    bool m_isPK;

    // GuildID. Written on the thread that owns the player; read by the PC
    // finder's guild walk (PCFinder::getGuildPlayerNames_LOCKED) from any
    // thread, under the finder's lock only, hence atomic.
    std::atomic<GuildID_t> m_GuildID;

    // Rank Bonus map
    HashMapRankBonus m_RankBonuses;
    bitset<RankBonus::RANK_BONUS_MAX> m_RankBonusFlag;

    Rank* m_pRank;
    WORD m_RankExpSaveCount;

    //	QuestManager*	m_pQuestManager;
    QuestManager* m_pQuestManager;
    TimeLimitItemManager* m_pTimeLimitItemManager;

    Item* m_pQuestItem;
    vector<Item*> m_PetStash; // Pet stash

public:
    /////////////////////////////////////////////////////////
    // Methods related to the pet stash
    /////////////////////////////////////////////////////////
    Item* getPetStashItem(int idx);
    void addPetStashItem(int idx, Item* pPetItem);

    /////////////////////////////////////////////////////////
    // BloodBible-related bonus values
    /////////////////////////////////////////////////////////
public:
    int getConsumeMPRatio() const {
        return m_ConsumeMPRatio;
    }
    void setConsumeMPRatio(int ratio) {
        m_ConsumeMPRatio = ratio;
    }

    int getGamblePriceRatio() const {
        return m_GamblePriceRatio;
    }
    void setGamblePriceRatio(int ratio) {
        m_GamblePriceRatio = ratio;
    }

    int getPotionPriceRatio() const {
        return m_PotionPriceRatio;
    }
    void setPotionPriceRatio(int ratio) {
        m_PotionPriceRatio = ratio;
    }

    Damage_t getMagicBonusDamage() const {
        return m_MagicBonusDamage;
    }
    void setMagicBonusDamage(Damage_t damage) {
        m_MagicBonusDamage = damage;
    }

    Damage_t getPhysicBonusDamage() const {
        return m_PhysicBonusDamage;
    }
    void setPhysicBonusDamage(Damage_t damage) {
        m_PhysicBonusDamage = damage;
    }

    Damage_t getMagicDamageReduce() const {
        return m_MagicDamageReduce;
    }
    void setMagicDamageReduce(Damage_t damage) {
        m_MagicDamageReduce = damage;
    }

    Damage_t getPhysicDamageReduce() const {
        return m_PhysicDamageReduce;
    }
    void setPhysicDamageReduce(Damage_t damage) {
        m_PhysicDamageReduce = damage;
    }

protected:
    int m_ConsumeMPRatio;
    int m_GamblePriceRatio;
    int m_PotionPriceRatio;
    Damage_t m_MagicBonusDamage;
    Damage_t m_PhysicBonusDamage;

    Damage_t m_MagicDamageReduce;
    Damage_t m_PhysicDamageReduce;

    //	list<ItemNameInfo*> 	m_ItemNameInfoList;

    // Added here to record whether the user won.

public:
    DWORD getLottoRewardID() const {
        return m_LottoRewardID;
    }
    void setLottoRewardID(DWORD lottoRewardID) {
        m_LottoRewardID = lottoRewardID;
    }

    DWORD getLottoQuestLevel() const {
        return m_LottoQuestLevel;
    }
    void setLottoQuestLevel(DWORD lottoQuestLevel) {
        m_LottoQuestLevel = lottoQuestLevel;
    }

    bool isLotto() const {
        return m_bLotto;
    }
    void setLotto(bool lotto) {
        m_bLotto = lotto;
    }

protected:
    DWORD m_LottoRewardID;
    DWORD m_LottoQuestLevel;
    bool m_bLotto;

    //////////////////////////////////////////////
    // Default Option Set Info
    //////////////////////////////////////////////
public:
    void addDefaultOptionSet(DefaultOptionSetType_t type);
    void removeDefaultOptionSet(DefaultOptionSetType_t type);

protected:
    forward_list<DefaultOptionSetType_t> m_DefaultOptionSet;

public:
    PetInfo* getPetInfo() const;
    void setPetInfo(PetInfo* pPetInfo);

    list<PetItem*>& getPetItems() {
        return m_PetItems;
    }
    const list<PetItem*>& getPetItems() const {
        return m_PetItems;
    }

    Pet* getPet() const {
        return m_pPet;
    }

protected:
    PetInfo* m_pPetInfo;
    list<PetItem*> m_PetItems;
    Pet* m_pPet;

    // Second pet quest
public:
    SpriteType_t getTargetMonsterSType() const {
        return m_TargetMonster;
    }
    virtual void initPetQuestTarget() = 0;
    GCMonsterKillQuestInfo::QuestInfo* getPetQuestInfo() const;

protected:
    SpriteType_t m_TargetMonster;
    DWORD m_TargetNum;
    DWORD m_TimeLimit;

    // SMS
public:
    SMSAddressBook* getAddressBook() const {
        return m_pSMSAddressBook;
    }
    uint getSMSCharge() const {
        return m_SMSCharge;
    }
    void setSMSCharge(uint charge) {
        m_SMSCharge = charge;
    }

    NicknameBook* getNicknameBook() const {
        return m_pNicknameBook;
    }

    NicknameInfo* getNickname() const {
        return m_pNickname;
    }
    void setNickname(NicknameInfo* pNickname) {
        m_pNickname = pNickname;
    }

    GQuestManager* getGQuestManager() const {
        return m_pGQuestManager;
    }

    BloodBibleSignInfo* getBloodBibleSign() const {
        return m_pBloodBibleSign;
    }

    virtual int getBloodBibleSignOpenNum() const = 0;
    void applyBloodBibleSign();

private:
    SMSAddressBook* m_pSMSAddressBook;
    uint m_SMSCharge;

    NicknameBook* m_pNicknameBook;
    NicknameInfo* m_pNickname;

    GQuestManager* m_pGQuestManager;
    BloodBibleSignInfo* m_pBloodBibleSign;

public:
    void setBaseLuck(Luck_t luck) {
        m_BaseLuck = luck;
    }
    Luck_t getBaseLuck() const {
        return m_BaseLuck;
    }

protected:
    Luck_t m_BaseLuck;

public:
    Store* getStore() const {
        return m_pStore;
    }

protected:
    Store* m_pStore;

public:
    // get / set PowerPoint
    int getPowerPoint() const {
        return m_PowerPoint;
    }
    void setPowerPoint(int powerpoint) {
        m_PowerPoint = powerpoint;
    }

protected:
    // Power points
    int m_PowerPoint;

public:
    bool isAdvanced() const {
        return m_bAdvanced;
    }

    Level_t getAdvancementClassLevel() const;
    Exp_t getAdvancementClassGoalExp() const;
    bool increaseAdvancementClassExp(Exp_t exp, bool bApplyExpBount = true);

protected:
    bool m_bAdvanced;
    AdvancementClass* m_pAdvancementClass;
    WORD m_AdvancementClassExpSaveCount;

public:
    Attr_t getAdvancedAttrBonus() const {
        return m_AdvancedAttrBonus;
    }
    bool putAdvancedBonusToSTR();
    bool putAdvancedBonusToDEX();
    bool putAdvancedBonusToINT();

    virtual Bonus_t getBonus() const = 0;
    virtual void setBonus(Bonus_t bonus) = 0;

protected:
    Attr_t m_AdvancedSTR;
    Attr_t m_AdvancedDEX;
    Attr_t m_AdvancedINT;
    Attr_t m_AdvancedAttrBonus;

public:
    BYTE getMasterEffectColor() const {
        return m_MasterEffectColor;
    }
    void setMasterEffectColor(BYTE color) {
        m_MasterEffectColor = color;
    }
    // add by sonic 2006.10.29
    bool canChangeMasterEffectColor();

protected:
    BYTE m_MasterEffectColor;
};

#endif
