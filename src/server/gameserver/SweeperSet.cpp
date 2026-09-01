#include "SweeperSet.h"

#include "CorpseItemPosition.h"
#include "EffectKeepSweeper.h"
#include "GCAddEffect.h"
#include "GlobalItemPosition.h"
#include "GlobalItemPositionLoader.h"
#include "Item.h"
#include "ItemFactoryManager.h"
#include "MonsterCorpse.h"
#include "SweeperBonus.h"
#include "SweeperBonusManager.h"
#include "Utility.h"
#include "Zone.h"
#include "ZoneUtil.h"
#include "repository/WarInfoRepository.h"

MonsterCorpse* SweeperSet::getSweeperSafes(uint itemType) {
    map<uint, MonsterCorpse*>::iterator itr = m_SweeperSafes.find(itemType);
    if (itr == m_SweeperSafes.end())
        return NULL;
    else
        return itr->second;
}

bool SweeperSet::isSafe(MonsterCorpse* pCorpse) const {
    map<uint, MonsterCorpse*>::const_iterator itr = m_SweeperSafes.begin();
    map<uint, MonsterCorpse*>::const_iterator endItr = m_SweeperSafes.end();

    for (; itr != endItr; itr++) {
        if (itr->second == pCorpse)
            return true;
    }

    return false;
}

int SweeperSet::getType(MonsterCorpse* pCorpse) const {
    map<uint, MonsterCorpse*>::const_iterator itr = m_SweeperSafes.begin();
    map<uint, MonsterCorpse*>::const_iterator endItr = m_SweeperSafes.end();

    for (; itr != endItr; itr++) {
        if (itr->second == pCorpse)
            return itr->first;
    }

    return -1;
}

SweeperSetManager::~SweeperSetManager() {
    map<uint, SweeperSet*>::iterator itr = m_SweeperSets.begin();
    map<uint, SweeperSet*>::iterator endItr = m_SweeperSets.end();

    for (; itr != endItr; ++itr) {
        SAFE_DELETE(itr->second);
    }

    m_SweeperSets.clear();
}

void SweeperSetManager::load(int level, Zone* pZone) {
    m_SweeperSets.clear();

    m_SweeperSets[0] = new SweeperSet(SweeperSet::SWEEPER_SLAYER);
    m_SweeperSets[1] = new SweeperSet(SweeperSet::SWEEPER_VAMPIRE);
    m_SweeperSets[2] = new SweeperSet(SweeperSet::SWEEPER_OUSTERS);
    m_SweeperSets[3] = new SweeperSet(SweeperSet::SWEEPER_DEFAULT);

    vector<SweeperSetRow> sets = defaultWarInfoRepository().loadSweeperSets(pZone->getZoneID());

    for (size_t r = 0; r < sets.size(); r++) {
        const SweeperSetRow& row = sets[r];

        ItemType_t ItemType = row.itemType;

        ZoneCoord_t SlayerX = row.slayerX;
        ZoneCoord_t SlayerY = row.slayerY;
        MonsterType_t SlayerMType = row.slayerMonsterType;

        ZoneCoord_t VampireX = row.vampireX;
        ZoneCoord_t VampireY = row.vampireY;
        MonsterType_t VampireMType = row.vampireMonsterType;

        ZoneCoord_t OustersX = row.oustersX;
        ZoneCoord_t OustersY = row.oustersY;
        MonsterType_t OustersMType = row.oustersMonsterType;

        ZoneCoord_t DefaultX = row.defaultX;
        ZoneCoord_t DefaultY = row.defaultY;
        MonsterType_t DefaultMType = row.defaultMonsterType;

        string name = row.name;

        MonsterCorpse* SlayerSafe = new MonsterCorpse(SlayerMType, name, 2);
        MonsterCorpse* VampireSafe = new MonsterCorpse(VampireMType, name, 2);
        MonsterCorpse* OustersSafe = new MonsterCorpse(OustersMType, name, 2);
        MonsterCorpse* DefaultSafe = new MonsterCorpse(DefaultMType, name, 2);

        SlayerSafe->setShrine(true);
        VampireSafe->setShrine(true);
        OustersSafe->setShrine(true);
        DefaultSafe->setShrine(true);

        pZone->registerObject(SlayerSafe);
        pZone->registerObject(VampireSafe);
        pZone->registerObject(OustersSafe);
        pZone->registerObject(DefaultSafe);

        pZone->addItem(SlayerSafe, SlayerX, SlayerY, true);
        pZone->addItem(VampireSafe, VampireX, VampireY, true);
        pZone->addItem(OustersSafe, OustersX, OustersY, true);
        pZone->addItem(DefaultSafe, DefaultX, DefaultY, true);

        forbidDarkness(pZone, SlayerX, SlayerY, 1);
        forbidDarkness(pZone, VampireX, VampireY, 1);
        forbidDarkness(pZone, OustersX, OustersY, 1);
        forbidDarkness(pZone, DefaultX, DefaultY, 1);

        m_SweeperSets[0]->addSafe(ItemType, SlayerSafe);
        m_SweeperSets[1]->addSafe(ItemType, VampireSafe);
        m_SweeperSets[2]->addSafe(ItemType, OustersSafe);
        m_SweeperSets[3]->addSafe(ItemType, DefaultSafe);
    }

    vector<SweeperOwnerRow> owners = defaultWarInfoRepository().loadSweeperOwners(pZone->getZoneID());

    for (size_t r = 0; r < owners.size(); r++) {
        int type = owners[r].sweeperType;
        int race = owners[r].ownerRace;
        Assert(race < 4);
        int safeType = owners[r].sweeperSafeType;

        Item* Sweeper = g_pItemFactoryManager->createItem(Item::ITEM_CLASS_SWEEPER, type, list<OptionType_t>());
        pZone->registerObject(Sweeper);

        Assert(m_Sweepers[Sweeper->getItemType()] == NULL);
        m_Sweepers[Sweeper->getItemType()] = Sweeper;

        MonsterCorpse* TargetSafe = m_SweeperSets[race]->getSweeperSafes(safeType);

        Assert(TargetSafe != NULL);

        Sweeper->create(itos(pZone->getZoneID()), STORAGE_CORPSE, TargetSafe->getObjectID(), 0, 0);
        putSweeper(Sweeper, TargetSafe);
    }
}


int SweeperSetManager::getSafeIndex(MonsterCorpse* pSafe) const {
    map<uint, SweeperSet*>::const_iterator itr = m_SweeperSets.begin();
    map<uint, SweeperSet*>::const_iterator endItr = m_SweeperSets.end();

    for (; itr != endItr; ++itr) {
        if (itr->second->isSafe(pSafe))
            return itr->first;
    }

    return -1;
}


/*bool SweeperSetManager::isFit( Item* pSweeper, MonsterCorpse* pSafe )
{
    map<ItemType_t, SweeperSet*>::iterator itr = m_SweeperSets.find( pSweeper->getItemType() );
    if ( itr == m_SweeperSets.end() ) return false;

    SweeperSet* pSweeperSet = itr->second;
    if ( pSweeperSet->getSweeper() != pSweeper ) return false;

    return pSweeperSet->findSafeIndex( pSafe ) != -1;
}
*/

bool SweeperSetManager::putSweeper(Item* pSweeper, MonsterCorpse* pSafe) {
    Assert(pSweeper != NULL);
    Assert(pSafe != NULL);

    // Sweepr 를 넣는 사용자와 pSafe 의 종족이 같음은 위에서 확인했다고 가정한다
    if (pSweeper->getItemClass() != Item::ITEM_CLASS_SWEEPER)
        return false;
    if (pSafe->getItemClass() != Item::ITEM_CLASS_CORPSE)
        return false;
    if (getSafeIndex(pSafe) == -1)
        return false;
    if (pSafe->isFlag(Effect::EFFECT_CLASS_KEEP_SWEEPER))
        return false;

    SweeperSet* pSweeperSet = getSweeperSet(getSafeIndex(pSafe));
    if (pSweeperSet == NULL)
        return false;

    pSafe->addTreasure(pSweeper);
    pSafe->setFlag(Effect::EFFECT_CLASS_KEEP_SWEEPER);

    int safeType = pSweeperSet->getType(pSafe);
    if (safeType == -1)
        return false;

    saveSweeperOwner(pSweeper->getItemType(), safeType, getSafeIndex(pSafe));

    g_pSweeperBonusManager->getSweeperBonus(pSweeper->getItemType())->setRace(getSafeIndex(pSafe));

    EffectKeepSweeper* pEffect = new EffectKeepSweeper(pSafe);
    pEffect->setPart(pSweeper->getItemType());
    pSafe->getEffectManager().addEffect(pEffect);

    GCAddEffect gcAddEffect;
    gcAddEffect.setObjectID(pSafe->getObjectID());
    gcAddEffect.setEffectID(pEffect->getSendEffectClass());
    gcAddEffect.setDuration(65535);

    pSafe->getZone()->broadcastPacket(pSafe->getX(), pSafe->getY(), &gcAddEffect);

    return true;
}

bool SweeperSetManager::returnAllSweeper() {
    bool bReturned = false;

    map<ItemType_t, Item*>::const_iterator itr = m_Sweepers.begin();
    map<ItemType_t, Item*>::const_iterator endItr = m_Sweepers.end();

    for (; itr != endItr; itr++) {
        Item* pItem = itr->second;
        Assert(pItem != NULL);

        bReturned = returnSweeper(pItem->getItemID(), false) || bReturned;
    }

    return bReturned;
}

bool SweeperSetManager::returnSweeper(ItemType_t sweeperID, bool bLock) {
    Item::ItemClass ItemClass = Item::ITEM_CLASS_SWEEPER;

    GlobalItemPosition* pItemPosition = GlobalItemPositionLoader::getInstance()->load(ItemClass, sweeperID);

    if (pItemPosition == NULL)
        return false;

    Item* pItem = pItemPosition->popItem(bLock);

    if (pItem != NULL && pItem->getItemClass() == Item::ITEM_CLASS_SWEEPER) {
        if (pItemPosition->getType() == GlobalItemPosition::POS_TYPE_CORPSE) {
            CorpseItemPosition* pCorpseItemPosition = dynamic_cast<CorpseItemPosition*>(pItemPosition);
            Assert(pCorpseItemPosition != NULL);

            Zone* pZone = pCorpseItemPosition->getZone();
            Assert(pZone != NULL);

            Item* pItem2 = pZone->getItem(pCorpseItemPosition->getCorpseObjectID());
            Assert(pItem2 != NULL);

            MonsterCorpse* pSafe = dynamic_cast<MonsterCorpse*>(pItem2);
            pSafe->addTreasure(pItem);

            return true;
        }

        Sweeper* pSweeper = dynamic_cast<Sweeper*>(pItem);
        Assert(pSweeper != NULL);

        MonsterCorpse* pDefaultSafe = getSweeperSet(3)->getSweeperSafes(pSweeper->getItemType());
        if (pDefaultSafe == NULL)
            return false;

        return putSweeper(pSweeper, pDefaultSafe);

        //		return returnSweeper( pZone, pSweeper );
    }

    return false;
}

/*
bool SweeperSetManager::returnSweeper( Zone* pZone, Sweeper* pSweeper ) const
{
    Assert( pZone != NULL );
    Assert( pSweeper != NULL );

    MonsterCorpse* pDefaultSafe = getSweeperSet( 3 )->getSweeperSafes( pSweeper->getItemType() );
    if ( pDefaultSafe == NULL ) return false;

    Zone* pTargetZone = pDefaultSafe->getZone();
    if ( pTargetZone == NULL ) return false;

    ObjectID_t CorpseObjectID = pDefaultSafe->getObjectID();

    // Default Safe 로 옮기고 소유한 종족에 대한 것도 저장해야 한다
//	pZone->transportItemToCorpse( pSweeper, pTargetZone, CorpseObjectID );

    SweeperSet* pSweeperSet = getSweeperSet( 3 );
    Assert(pSweeperSet != NULL );

    MonsterCorpse* pSafe = pSweeperSet->getSweeperSafes( pSweeperSet->getItemType() );

    putSweeper( pSweeper, pSafe );

    return true;
}
*/

void SweeperSetManager::saveSweeperOwner(uint itemType, int safeType, int ownerRace) {
    defaultWarInfoRepository().saveSweeperOwner(ownerRace, safeType, itemType);
}
