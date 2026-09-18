#ifndef __SHRINE_INFO_MANAGER_H__
#define __SHRINE_INFO_MANAGER_H__

#include <unordered_map>

#include "Exception.h"
#include "Mutex.h"
#include "Types.h"

class PlayerCreature;
class ZoneItemPosition;
class Zone;
class Item;
class BloodBible;
class MonsterCorpse;
class GCBloodBibleStatus;

class ShrineInfo {
public:
    enum ShrineType {
        SHRINE_GUARD, // Guardian shrine inside the castle
        SHRINE_HOLY   // Holy land shrine outside the castle
    };

public:
    ShrineInfo() {}
    ~ShrineInfo() {}

public:
    MonsterType_t getMonsterType() const {
        return m_MonsterType;
    }
    void setMonsterType(MonsterType_t mt) {
        m_MonsterType = mt;
    }

    ShrineType getShrineType() const {
        return m_ShrineType;
    }
    void setShrineType(ShrineType st) {
        m_ShrineType = st;
    }

    ZoneID_t getZoneID() const {
        return m_ZoneID;
    }
    void setZoneID(const ZoneID_t zoneID) {
        m_ZoneID = zoneID;
    }

    ZoneCoord_t getX() const {
        return m_X;
    }
    void setX(ZoneCoord_t x) {
        m_X = x;
    }
    ZoneCoord_t getY() const {
        return m_Y;
    }
    void setY(ZoneCoord_t y) {
        m_Y = y;
    }

    const string& getName() const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
    }

    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t objectID) {
        m_ObjectID = objectID;
    }

    string toString() const;

private:
    MonsterType_t m_MonsterType;
    ShrineType m_ShrineType;
    ZoneID_t m_ZoneID; // Zone ID
    ZoneCoord_t m_X;
    ZoneCoord_t m_Y;
    ObjectID_t m_ObjectID;
    string m_Name; // Shrine name
};

class ShrineSet {
public:
    ShrineSet();
    ~ShrineSet();

    ItemID_t getBloodBibleItemID() const {
        return m_ItemID;
    }
    void setBloodBibleItemID(ItemID_t itemID) {
        m_ItemID = itemID;
    }

    string toString() const;

    ShrineID_t getShrineID() const {
        return m_ShrineID;
    }
    void setShrineID(ShrineID_t sid) {
        m_ShrineID = sid;
    }

    ShrineInfo& getSlayerGuardShrine() {
        return m_SlayerGuardShrine;
    }
    ShrineInfo& getVampireGuardShrine() {
        return m_VampireGuardShrine;
    }
    ShrineInfo& getOustersGuardShrine() {
        return m_OustersGuardShrine;
    }
    ShrineInfo& getHolyShrine() {
        return m_HolyShrine;
    }

    ShrineInfo& getReturnGuardShrine();

    void setSlayerGuardShrine() {
        m_OwnerRace = RACE_SLAYER;
    }
    void setVampireGuardShrine() {
        m_OwnerRace = RACE_VAMPIRE;
    }
    void setOustersGuardShrine() {
        m_OwnerRace = RACE_OUSTERS;
    }

    ItemType_t getBloodBibleItemType() const {
        return m_ItemType;
    }
    void setBloodBibleItemType(ItemType_t itemType) {
        m_ItemType = itemType;
    }

    Race_t getOwnerRace() const {
        return m_OwnerRace;
    }
    void setOwnerRace(Race_t race);

    void sendBloodBibleStatus(PlayerCreature* pPC);
    void setBloodBibleStatus(GCBloodBibleStatus* pGCBBS);
    void broadcastBloodBibleStatus();

    Item* createBloodBibleInGuardShrine();

    bool saveBloodBibleOwner();

private:
    ShrineID_t m_ShrineID;           // Shrine ID. Must be the same as ItemType.
    ShrineInfo m_SlayerGuardShrine;  // Shrine inside the Slayer castle
    ShrineInfo m_VampireGuardShrine; // Shrine inside the Vampire castle
    ShrineInfo m_OustersGuardShrine; // Shrine inside the Ousters castle
    ShrineInfo m_HolyShrine;         // Shrine in Adam's holy land
    ItemType_t m_ItemType;           // ItemType of the bible fragment
    ItemID_t m_ItemID;               // ItemID of the bible fragment
    Race_t m_OwnerRace;              // Race that held it last
    GCBloodBibleStatus* m_pGCBBS;    // Location information of the Blood Bible

    mutable Mutex m_Mutex;
};


class ShrineInfoManager {
public:
    typedef unordered_map<ShrineID_t, ShrineSet*> HashMapShrineSet;
    typedef HashMapShrineSet::iterator HashMapShrineSetItor;
    typedef HashMapShrineSet::const_iterator HashMapShrineSetConstItor;

public:
    ShrineInfoManager() {}
    ~ShrineInfoManager();

public:
    void init();
    void load();
    void reloadOwner();
    void clear();

    void addAllShrineToZone();
    void addShrineToZone(ShrineInfo& shrineInfo, ItemType_t itemType = 0);

    void addShrineSet(ShrineSet* pShrineSet);
    void deleteShrineSet(ShrineID_t shrineID);
    ShrineSet* getShrineSet(ShrineID_t shrineID) const;
    int size() const {
        return m_ShrineSets.size();
    }

    string toString() const;

public:
    bool isMatchGuardShrine(Item* pItem, MonsterCorpse* pMonsterCorpse, PlayerCreature* pPC) const;
    bool isMatchHolyShrine(Item* pItem, MonsterCorpse* pMonsterCorpse) const;
    bool isDefenderOfGuardShrine(PlayerCreature* pPC, MonsterCorpse* pShrine) const;
    bool canPickupBloodBible(Race_t race, BloodBible* pBloodBible) const;
    bool getMatchGuardShrinePosition(Item* pItem, ZoneItemPosition& zip) const;

    //	bool returnCastleBloodBible( ZoneID_t castleZoneID ) const ;
    bool returnAllBloodBible() const;
    bool returnBloodBible(ShrineID_t shrineID, bool bLock = true) const;
    bool returnBloodBible(Zone* pZone, BloodBible* pBloodBible) const;

    //	bool removeShrineShield( Zone* pZone ) ;
    bool removeShrineShield(ShrineInfo* pShrineInfo);
    // bool addShrineShield( Zone* pZone ) ;
    // bool addShrineShield_LOCKED( Zone* pZone ) ;
    bool addShrineShield(ShrineInfo& shrineInfo);

    bool putBloodBible(PlayerCreature* pPC, Item* pItem, MonsterCorpse* pCorpse) const;

public:
    void addAllShrineShield();
    bool removeAllShrineShield();

    // Save the current owner information to the DB.
    bool saveBloodBibleOwner();

    // Register the location information of the Blood Bible.
    void registerBloodBibleStatus(ItemType_t m_Part, GCBloodBibleStatus* pGCBBS);
    void sendBloodBibleStatus(PlayerCreature* pPC);

    void broadcastBloodBibleStatus();

private:
    HashMapShrineSet m_ShrineSets;
};

extern ShrineInfoManager* g_pShrineInfoManager;

#endif // __SHRINE_INFO_MANAGER_H__
