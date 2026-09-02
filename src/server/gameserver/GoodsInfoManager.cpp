//////////////////////////////////////////////////////////////////////////////
// Filename    : GoodsInfo.cpp
// Written By  : beowulf
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GoodsInfoManager.h"

#include "Assert.h"
#include "ItemFactoryManager.h"
#include "ItemInfoManager.h"
#include "ItemUtil.h"
#include "repository/GameInfoRepository.h"

//////////////////////////////////////////////////////////////////////////////
// class GoodsInfo member methods
//////////////////////////////////////////////////////////////////////////////

GoodsInfo::GoodsInfo(){__BEGIN_TRY __END_CATCH}

GoodsInfo::~GoodsInfo(){__BEGIN_TRY __END_CATCH_NO_RETHROW}

string GoodsInfo::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "GoodsInfo (" << "Type:" << (int)m_ItemType << ",Name:" << m_Name << ",ItemClass:" << (int)m_ItemClass
        << ",ItemType:" << (int)m_ItemType << ",TimeLimit:" << (m_bTimeLimit ? "Y" : "N") << ",Hour:" << m_Hour << ")";
    return msg.toString();

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// class GoodsInfoManager member methods
//////////////////////////////////////////////////////////////////////////////

GoodsInfoManager::GoodsInfoManager()

    {__BEGIN_TRY

         __END_CATCH}

GoodsInfoManager::~GoodsInfoManager()

{
    __BEGIN_TRY

    __END_CATCH_NO_RETHROW
}

void GoodsInfoManager::init()

{
    __BEGIN_TRY

    load();

    __END_CATCH
}

void GoodsInfoManager::clear()

{
    __BEGIN_TRY

    HashMapGoodsInfoItr itr = m_GoodsInfos.begin();

    for (; itr != m_GoodsInfos.end(); itr++) {
        SAFE_DELETE(itr->second);
    }

    m_GoodsInfos.clear();

    __END_CATCH
}

void GoodsInfoManager::load()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    clear();

    vector<GoodsInfoRow> rows = defaultGameInfoRepository().loadGoods();

    for (size_t r = 0; r < rows.size(); r++) {
        GoodsInfo* pGoodsInfo = new GoodsInfo();

        pGoodsInfo->setID(rows[r].goodsID);
        pGoodsInfo->setName(rows[r].name);
        pGoodsInfo->setItemClass((Item::ItemClass)(rows[r].itemClass));
        pGoodsInfo->setItemType(rows[r].itemType);
        pGoodsInfo->setGrade(rows[r].grade);
        string optionField = rows[r].optionType;
        pGoodsInfo->setNum(rows[r].num);
        pGoodsInfo->setTimeLimit(rows[r].limited == 1); // enum( 'LIMITED'(1), 'UNLIMITED'(2) )
        pGoodsInfo->setHour(rows[r].hour);

        list<OptionType_t> optionTypes;
        setOptionTypeFromField(optionTypes, optionField);
        pGoodsInfo->setOptionTypeList(optionTypes);

        addGoodsInfo(pGoodsInfo);
    }

    __END_DEBUG
    __END_CATCH
}

void GoodsInfoManager::addGoodsInfo(GoodsInfo* pGoodsInfo)

{
    __BEGIN_TRY

    Assert(pGoodsInfo != NULL);

    HashMapGoodsInfoItr itr = m_GoodsInfos.find(pGoodsInfo->getID());

    if (itr != m_GoodsInfos.end())
        throw DuplicatedException();

    m_GoodsInfos[pGoodsInfo->getID()] = pGoodsInfo;

    __END_CATCH
}

GoodsInfo* GoodsInfoManager::getGoodsInfo(DWORD id) const

{
    __BEGIN_TRY

    HashMapGoodsInfoConstItr itr = m_GoodsInfos.find(id);

    if (itr == m_GoodsInfos.end())
        return NULL;

    return itr->second;

    __END_CATCH
}

string GoodsInfoManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GoodsInfoManager(\n";

    HashMapGoodsInfoConstItr itr = m_GoodsInfos.begin();

    for (; itr != m_GoodsInfos.end(); itr++) {
        msg << "GoodsInfos[" << itr->second->getID() << "] == ";
        msg << itr->second->getName();
        msg << "\n";
        msg << itr->second->toString() << "\n";
    }

    return msg.toString();

    __END_CATCH
}

// Global Variable definition
GoodsInfoManager* g_pGoodsInfoManager = NULL;
