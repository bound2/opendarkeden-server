//////////////////////////////////////////////////////////////////////////////
// Filename    : NPC.h
// Description :
// Originally there was no shop handling. Rather than deriving a shop NPC from
// this class, the shop interface was put directly into NPC.
//////////////////////////////////////////////////////////////////////////////

#ifndef __NPC_H__
#define __NPC_H__

#include "Assert.h"
#include "Creature.h"
#include "Inventory.h"
#include "Timeval.h"
#include "Zone.h"
#include "couple/PartnerWaitingManager.h"
#include "quest/TriggerManager.h"

class QuestInfoManager;
class RewardClassInfoManager;
class Script;

//////////////////////////////////////////////////////////////////////////////
// Shop types (for the Christmas event)
//////////////////////////////////////////////////////////////////////////////
enum ShopType {
    SHOPTYPE_NORMAL = 0, // Normal shop
    SHOPTYPE_EVENT,      // Event shop

    SHOPTYPE_MAX
};

//////////////////////////////////////////////////////////////////////////////
// NPC race constants.
// They are used to stop a Slayer from talking to a Vampire NPC
// or from using its shop.
//////////////////////////////////////////////////////////////////////////////
enum { NPC_RACE_SLAYER = 0, NPC_RACE_VAMPIRE, NPC_RACE_OUSTERS, NPC_RACE_MONSTER, NPC_RACE_UNKNOWN, NPC_RACE_MAX };


//////////////////////////////////////////////////////////////////////////////
// Class NPC
//////////////////////////////////////////////////////////////////////////////

class ShopRack;

class NPC : public Creature {
public:
    NPC();
    NPC(const string& name);
    virtual ~NPC();

public:
    virtual CreatureClass getCreatureClass() const {
        return CREATURE_CLASS_NPC;
    }
    virtual string getCreatureClassString() const {
        return "CREATURE_CLASS_NPC";
    }

    virtual void registerObject();

    virtual bool load();
    virtual void save() const {
        throw UnsupportedError();
    }

    virtual string toString() const;

public:
    void init();
    void act(const Timeval& currentTime);

public:
    virtual Level_t getLevel() const {
        return 1;
    }

    Race_t getRace(void) const {
        return m_Race;
    }
    void setRace(int race) {
        m_Race = race;
    }

    bool isSlayer() const {
        return (m_Race == NPC_RACE_SLAYER) ? true : false;
    }
    bool isVampire() const {
        return (m_Race == NPC_RACE_VAMPIRE) ? true : false;
    }
    bool isOusters() const {
        return (m_Race == NPC_RACE_OUSTERS) ? true : false;
    }

    SpriteType_t getSpriteType() const {
        return m_SpriteType;
    }
    void setSpriteType(SpriteType_t spriteType) {
        m_SpriteType = spriteType;
    }

    const string& getName() const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
    }

    NPCID_t getNPCID() const {
        return m_NPCID;
    }
    void setNPCID(NPCID_t NPCID) {
        m_NPCID = NPCID;
    }

    Color_t getMainColor() const {
        return m_MainColor;
    }
    void setMainColor(Color_t color) {
        m_MainColor = color;
    }

    Color_t getSubColor() const {
        return m_SubColor;
    }
    void setSubColor(Color_t color) {
        m_SubColor = color;
    }

    Inventory* getInventory() {
        return m_pInventory;
    }
    void setInventory(Inventory* pInventory) {
        m_pInventory = pInventory;
    }

    TriggerManager& getTriggerManager() {
        return m_TriggerManager;
    }
    const TriggerManager& getTriggerManager() const {
        return m_TriggerManager;
    }

    bool isShowInMinimap(void) const {
        return m_bShowInMinimap;
    }
    void setShowInMinimap(bool bShow) {
        m_bShowInMinimap = bShow;
    }

    bool isDead() const {
        return false;
    }
    bool isAlive() const {
        return true;
    }

    ////////////////////////////////////////////////////////////
    // Shop related interface.
    // Almost every function of the shop interface throws an
    // exception when the NPC is not actually a shop NPC, so
    // check that this NPC really is a shop NPC before
    // calling them.
    ////////////////////////////////////////////////////////////
public:
    int getShopType(void) const {
        return m_ShopType;
    }
    void setShopType(int type) {
        m_ShopType = type;
    }

    ShopVersion_t getShopVersion(ShopRackType_t type) const;
    void setShopVersion(ShopRackType_t type, ShopVersion_t ver);
    void increaseShopVersion(ShopRackType_t type);

    bool isExistShopItem(ShopRackType_t type, BYTE index) const;
    void insertShopItem(ShopRackType_t type, BYTE index, Item* pItem);
    void removeShopItem(ShopRackType_t type, BYTE index);
    Item* getShopItem(ShopRackType_t type, BYTE index) const;
    void clearShopItem(void);

    BYTE getFirstEmptySlot(ShopRackType_t type) const;
    BYTE getLastEmptySlot(ShopRackType_t type) const;

    bool isEmpty(ShopRackType_t type) const;
    bool isFull(ShopRackType_t type) const;

    MarketCond_t getMarketCondBuy(void) const {
        return m_MarketCondBuy;
    }
    void setMarketCondBuy(MarketCond_t cond) {
        m_MarketCondBuy = cond;
    }

    MarketCond_t getMarketCondSell(void) const {
        return m_MarketCondSell;
    }
    void setMarketCondSell(MarketCond_t cond) {
        m_MarketCondSell = cond;
    }

    virtual ClanType_t getClanType(void) const {
        return m_ClanType;
    }
    virtual void setClanType(ClanType_t clan) {
        m_ClanType = clan;
    }

    // SimpleQuest by sigi. 2002.12.3
    //	bool regenSimpleQuest() ;
    //	bool giveSimpleQuest(Creature* pPC, QuestID_t qid) ;
    //	void setSimpleQuestListScript(Script* pScript) ;
    // void sendSimpleQuestListPacket(Creature* pCreature) ;

    void setCoupleRegisterManager(PartnerWaitingManager* pPWM) {
        m_pCoupleRegisterManager = pPWM;
    }
    void setCoupleUnregisterManager(PartnerWaitingManager* pPWM) {
        m_pCoupleUnregisterManager = pPWM;
    }

    PartnerWaitingManager* getCoupleRegisterManager() const {
        return m_pCoupleRegisterManager;
    }
    PartnerWaitingManager* getCoupleUnregisterManager() const {
        return m_pCoupleUnregisterManager;
    }

    void setQuestInfoManager(QuestInfoManager* pQIM) {
        m_pQuestInfoManager = pQIM;
    }
    void setRewardClassInfoManager(RewardClassInfoManager* pRIM) {
        m_pRewardClassInfoManager = pRIM;
    }

    QuestInfoManager* getQuestInfoManager() const {
        return m_pQuestInfoManager;
    }
    RewardClassInfoManager* getRewardClassInfoManager() const {
        return m_pRewardClassInfoManager;
    }

    ZoneID_t getTaxingCastleZoneID() const {
        return m_TaxingCastleZoneID;
    }
    void setTaxingCastleZoneID(ZoneID_t castleZoneID) {
        m_TaxingCastleZoneID = castleZoneID;
    }

    int getTaxRatio(PlayerCreature* pPC) const;

protected:
    string m_Name;
    NPCID_t m_NPCID;
    int m_Race;
    SpriteType_t m_SpriteType;
    Color_t m_MainColor;
    Color_t m_SubColor;
    Timeval m_NextTurn;              // Time the next action runs
    Inventory* m_pInventory;         // The NPC's inventory
    TriggerManager m_TriggerManager; // Trigger manager
    bool m_bShowInMinimap;
    ClanType_t m_ClanType;

    // Shop interface
    int m_ShopType;
    ShopRack* m_pRack;
    MarketCond_t m_MarketCondBuy;
    MarketCond_t m_MarketCondSell;
    //	QuestBoard*  m_pQuestBoard;

    // Couple registration managers
    PartnerWaitingManager* m_pCoupleRegisterManager;
    PartnerWaitingManager* m_pCoupleUnregisterManager;

    // Quest related information
    QuestInfoManager* m_pQuestInfoManager;
    RewardClassInfoManager* m_pRewardClassInfoManager;

    ZoneID_t m_TaxingCastleZoneID;
};


#endif
