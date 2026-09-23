#ifndef __CASTLE_SHRINE_INFO_MANAGER_H__
#define __CASTLE_SHRINE_INFO_MANAGER_H__

#include <unordered_map>

#include "Exception.h"
#include "ShrineInfoManager.h"
#include "Types.h"

class PlayerCreature;
class ZoneItemPosition;
class Zone;
class Item;
class CastleSymbol;
class MonsterCorpse;

class CastleShrineSet {
public:
    CastleShrineSet() {}
    ~CastleShrineSet() {}

    ItemID_t getCastleSymbolItemID() const {
        return m_ItemID;
    }
    void setCastleSymbolItemID(ItemID_t itemID) {
        m_ItemID = itemID;
    }

    string toString() const;

public:
    ShrineID_t m_ShrineID;    // Shrine ID. Must match the ItemType.
    ShrineInfo m_GuardShrine; // Shrine inside the castle
    ShrineInfo m_HolyShrine;  // Shrine in Adam's holy land
    ItemType_t m_ItemType;    // ItemType of the bible fragment
    ItemID_t m_ItemID;        // ItemID of the bible fragment
};


class CastleShrineInfoManager {
public:
    typedef unordered_map<ShrineID_t, CastleShrineSet*> HashMapShrineSet;
    typedef HashMapShrineSet::iterator HashMapShrineSetItor;
    typedef HashMapShrineSet::const_iterator HashMapShrineSetConstItor;

public:
    CastleShrineInfoManager() {}
    ~CastleShrineInfoManager();

public:
    void init();
    void load();
    void clear();

    void addAllShrineToZone();
    Item* addShrineToZone(ShrineInfo& shrineInfo, ItemType_t itemType = 0);

    void addShrineSet(CastleShrineSet* pShrineSet);
    void deleteShrineSet(ShrineID_t shrineID);
    CastleShrineSet* getShrineSet(ShrineID_t shrineID) const;
    int size() const {
        return m_ShrineSets.size();
    }

    string toString() const;

public:
    bool isMatchHolyShrine(Item* pItem, MonsterCorpse* pMonsterCorpse) const;
    bool canPickupCastleSymbol(Race_t race, CastleSymbol* pCastleSymbol) const;
    bool getMatchGuardShrinePosition(Item* pItem, ZoneItemPosition& zip) const;

    bool returnAllCastleSymbol(ZoneID_t castleZoneID) const;
    bool returnCastleSymbol(ShrineID_t shrineID, bool bLock = true) const;
    bool returnCastleSymbol(Zone* pZone, CastleSymbol* pCastleSymbol) const;

    ZoneID_t getGuardShrineZoneID(ZoneID_t castleZoneID) const;

    bool removeShrineShield(Zone* pZone);
    bool addShrineShield(Zone* pZone);
    bool addShrineShield_LOCKED(Zone* pZone);

    bool putCastleSymbol(PlayerCreature* pPC, Item* pItem, MonsterCorpse* pCorpse) const;

private:
    HashMapShrineSet m_ShrineSets;
};

#endif // __SHRINE_INFO_MANAGER_H__
