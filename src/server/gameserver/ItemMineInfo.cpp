//////////////////////////////////////////////////////////////////////////////
// Filename    : ItemMineInfo.h
// Written By  : bezz, sequoia, dew
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "ItemMineInfo.h"

#include "ItemFactoryManager.h"
#include "ItemUtil.h"
#include "Treasure.h"
#include "Utility.h"
#include "repository/GameInfoRepository.h"

ItemMineInfo::ItemMineInfo()
    : m_ID(0){__BEGIN_TRY __END_CATCH}

      ItemMineInfo::~ItemMineInfo(){__BEGIN_TRY __END_CATCH_NO_RETHROW}

      Item
      * ItemMineInfo::getItem() {
    __BEGIN_TRY

    Item* pItem = g_pItemFactoryManager->createItem((Item::ItemClass)getItemClass(), getItemType(), getItemOptions());

    return pItem;

    __END_CATCH
}


string ItemMineInfo::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "ItemMineInfo(" << "m_ID : " << m_ID << "m_ItemClass : " << ItemClass2String[(int)m_ItemClass]
        << "m_ItemType : " << (int)m_ItemType << "m_ItemOptions : " << getOptionTypeToString(m_ItemOptions) << ")";
    return msg.toString();

    __END_CATCH
}

ItemMineInfoManager::ItemMineInfoManager(){__BEGIN_TRY __END_CATCH}

ItemMineInfoManager::~ItemMineInfoManager() {
    __BEGIN_TRY

    HashMapItemMineInfoItor itr = m_ItemMineInfos.begin();
    HashMapItemMineInfoItor endItr = m_ItemMineInfos.end();

    for (; itr != endItr; itr++) {
        ItemMineInfo* pInfo = itr->second;
        SAFE_DELETE(pInfo);
    }

    m_ItemMineInfos.clear();

    __END_CATCH_NO_RETHROW
}

void ItemMineInfoManager::load()

{
    __BEGIN_TRY

    vector<ItemMineRow> rows = defaultGameInfoRepository().loadItemMines();

    for (size_t r = 0; r < rows.size(); r++) {
        ItemMineInfo* pItemMineInfo = new ItemMineInfo();

        int id = rows[r].id;
        string itemClass = rows[r].itemClass;
        int itemType = rows[r].itemType;
        string itemOptions = rows[r].itemOption;

        list<OptionType_t> oList;
        makeOptionList(itemOptions, oList);

        pItemMineInfo->setID(id);
        pItemMineInfo->setItemClass(TreasureItemClass::getItemClassFromString(itemClass));
        pItemMineInfo->setItemType(itemType);
        pItemMineInfo->setItemOptions(oList);

        addItemMineInfo(pItemMineInfo);
    }

    __END_CATCH
}

ItemMineInfo* ItemMineInfoManager::getItemMineInfo(int id) const {
    __BEGIN_TRY

    HashMapItemMineInfoConstItor itr = m_ItemMineInfos.find(id);

    if (itr == m_ItemMineInfos.end())
        return NULL;

    return itr->second;

    __END_CATCH
}

void ItemMineInfoManager::addItemMineInfo(ItemMineInfo* pItemMineInfo)

{
    __BEGIN_TRY

    HashMapItemMineInfoItor itr = m_ItemMineInfos.find(pItemMineInfo->getID());

    if (itr != m_ItemMineInfos.end())
        throw DuplicatedException();

    m_ItemMineInfos[pItemMineInfo->getID()] = pItemMineInfo;

    __END_CATCH
}

string ItemMineInfoManager::toString() const {
    __BEGIN_TRY

    StringStream msg;

    if (m_ItemMineInfos.empty())
        msg << "EMPTY";
    else {
        HashMapItemMineInfoConstItor endItr = m_ItemMineInfos.end();
        for (HashMapItemMineInfoConstItor itr = m_ItemMineInfos.begin(); itr != endItr; itr++) {
            msg << itr->second->toString();
        }
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}

Item* ItemMineInfoManager::getRandomItem(int minID, int maxID) {
    __BEGIN_TRY

    ItemMineInfo* pItemMineInfo = getItemMineInfo(Random(minID, maxID));

    return pItemMineInfo->getItem();

    __END_CATCH
}

ItemMineInfoManager* g_pItemMineInfoManager = NULL;
