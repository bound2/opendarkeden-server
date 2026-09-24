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

    // Sends the castle symbols of castleZoneID's shrine sets back to their
    // guard shrines, from any thread: each return is posted to whoever holds
    // the symbol (de::war::postItemReturn), which takes it out under its own
    // lock and hands it to returnCastleSymbol(Zone*, CastleSymbol*). True when
    // a return was posted for some symbol.
    bool returnAllCastleSymbol(ZoneID_t castleZoneID) const;
    bool postCastleSymbolReturn(ShrineID_t shrineID) const;
    // Sends shrineID's symbol back from the calling zone thread, which holds
    // it: a symbol just laid on a shrine in this thread's zone.
    bool returnCastleSymbol(ShrineID_t shrineID) const;
    // Moves a symbol taken out of pZone, which the calling thread owns, into
    // its guard shrine (Zone::transportItemToCorpse).
    bool returnCastleSymbol(Zone* pZone, CastleSymbol* pCastleSymbol) const;

    ZoneID_t getGuardShrineZoneID(ZoneID_t castleZoneID) const;

    // Lift or restore the shield of the guard shrines in pZone, the guard
    // zone. The calling thread must own pZone's group: a war posts these to
    // it (de::war::postToZone).
    bool removeShrineShield(Zone* pZone);
    bool addShrineShield(Zone* pZone);

    bool putCastleSymbol(PlayerCreature* pPC, Item* pItem, MonsterCorpse* pCorpse) const;

private:
    HashMapShrineSet m_ShrineSets;
};

#endif // __SHRINE_INFO_MANAGER_H__
