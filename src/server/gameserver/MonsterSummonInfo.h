//////////////////////////////////////////////////////////////////////////////
// Filename    : MonsterSummonInfo.h
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __MONSTER_SUMMON_INFO_H__
#define __MONSTER_SUMMON_INFO_H__

#include <list>
#include <vector>

#include "MonsterInfoTypes.h"
#include "Types.h"

struct MonsterCollection;

// Information used when summoning
struct SUMMON_INFO {
    enum ClanType {
        CLAN_TYPE_DEFAULT,      // default = 1
        CLAN_TYPE_RANDOM_EACH,  // Each one created this time differs
        CLAN_TYPE_RANDOM_GROUP, // All created this time are the same
        CLAN_TYPE_GROUP,        // All created this time are the same, with clanID given
    };

    SUMMON_INFO() {
        scanEnemy = false;
        canScanEnemy = false;
        clanType = CLAN_TYPE_DEFAULT;
        clanID = 0;
        hasItem = true;
        regenType = REGENTYPE_MAX;
        initHPPercent = 0;
    }

    bool canScanEnemy; // Does it scan for enemies (monsters) by itself?
    ClanType clanType;
    int clanID;
    bool hasItem; // Does the summoned monster carry an item?
    RegenType regenType;
    bool scanEnemy; // Does it scan for enemies (monsters) when created?
    int initHPPercent;
};

// Information used when summoning: summons several kinds at once
struct SUMMON_INFO2 : public SUMMON_INFO {
    ZoneCoord_t X;
    ZoneCoord_t Y;
    MonsterCollection* pMonsters;
};

// A number of monsters
struct MonsterCollectionInfo {
    SpriteType_t SpriteType;
    MonsterType_t MonsterType;
    int Num;

    void parseString(const string& infoString);

    string toString() const;
};

// Several kinds of monsters
struct MonsterCollection {
    list<MonsterCollectionInfo> Infos;

    void parseString(const string& collectionString);

    string toString() const;
};

// Summon step
struct MonsterSummonStep {
    vector<MonsterCollection> Collections;

    const MonsterCollection* getRandomMonsterCollection() const;

    void parseString(const string& summonStepsString);

    string toString() const;
};

// Summon information
struct MonsterSummonInfo {
    vector<MonsterSummonStep> Steps;

    const MonsterCollection* getRandomMonsterCollection(int step) const;
    bool hasNextMonsterCollection(int step) const;

    void parseString(const string& summonInfoString);

    string toString() const;
};

#endif
