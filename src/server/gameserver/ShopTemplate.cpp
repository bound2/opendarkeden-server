////////////////////////////////////////////////////////////////////////////////
// Filename    : ShopTemplate.cpp
// Written By  : 김성민
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "ShopTemplate.h"

#include "repository/GameInfoRepository.h"

////////////////////////////////////////////////////////////////////////////////
// global varible initialization
////////////////////////////////////////////////////////////////////////////////
ShopTemplateManager* g_pShopTemplateManager = NULL;

////////////////////////////////////////////////////////////////////////////////
// class ShopTemplate member methods
////////////////////////////////////////////////////////////////////////////////

ShopTemplate::ShopTemplate()

{
    __BEGIN_TRY

    m_ID = 0;
    m_RackType = 0;
    m_ItemClass = 0;
    m_MinItemType = 0;
    m_MaxItemType = 0;
    m_MinOptionLevel = 0;
    m_MaxOptionLevel = 0;

    __END_CATCH
}

ShopTemplate::~ShopTemplate()

    {__BEGIN_TRY __END_CATCH_NO_RETHROW}

string ShopTemplate::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ShopTemplate(" << "TemplateID : " << (int)m_ID << ",RackType : " << (int)m_RackType
        << ",ItemClass : " << (int)m_ItemClass << ",MinItemType : " << (int)m_MinItemType
        << ",MaxItemType : " << (int)m_MaxItemType << ",MinOptionLevel : " << (int)m_MinOptionLevel
        << ",MaxOptionLevel : " << (int)m_MaxOptionLevel << ")";
    return msg.toString();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// class ShopTemplateManager member methods
////////////////////////////////////////////////////////////////////////////////

ShopTemplateManager::ShopTemplateManager()

    {__BEGIN_TRY

         __END_CATCH}

ShopTemplateManager::~ShopTemplateManager()

{
    __BEGIN_TRY

    unordered_map<ShopTemplateID_t, ShopTemplate*>::iterator itr = m_Entries.begin();
    for (; itr != m_Entries.end(); itr++) {
        ShopTemplate* pTemplate = itr->second;
        SAFE_DELETE(pTemplate);
    }

    m_Entries.clear();

    __END_CATCH_NO_RETHROW
}

void ShopTemplateManager::init()

{
    __BEGIN_TRY

    load();

    __END_CATCH
}

void ShopTemplateManager::load()

{
    __BEGIN_TRY

    vector<ShopTemplateRow> rows = defaultGameInfoRepository().loadShopTemplates();

    for (size_t r = 0; r < rows.size(); r++) {
        ShopTemplate* pTemplate = new ShopTemplate();

        pTemplate->setID(rows[r].id);
        pTemplate->setShopType(rows[r].shopType);
        pTemplate->setItemClass(rows[r].itemClass);
        pTemplate->setMinItemType(rows[r].minItemType);
        pTemplate->setMaxItemType(rows[r].maxItemType);
        pTemplate->setMinOptionLevel(rows[r].minOptionLevel);
        pTemplate->setMaxOptionLevel(rows[r].maxOptionLevel);

        setTemplate(pTemplate->getID(), pTemplate);
    }

    __END_CATCH
}

ShopTemplate* ShopTemplateManager::getTemplate(ShopTemplateID_t id) const {
    __BEGIN_TRY

    unordered_map<ShopTemplateID_t, ShopTemplate*>::const_iterator itr = m_Entries.find(id);

    if (itr == m_Entries.end()) {
        cerr << "ShopTemplateManager::getTemplate() : NoSuchElementException" << endl;
        throw NoSuchElementException("template not exist.");
    }

    return itr->second;

    __END_CATCH
}

void ShopTemplateManager::setTemplate(ShopTemplateID_t id, ShopTemplate* pTemplate)

{
    __BEGIN_TRY

    unordered_map<ShopTemplateID_t, ShopTemplate*>::iterator itr = m_Entries.find(id);

    if (itr != m_Entries.end())
        throw("ShopTemplateManager::setTemplate() : Same ID already exist!");

    m_Entries[id] = pTemplate;

    __END_CATCH
}

string ShopTemplateManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "ShopTemplateManager(";
    unordered_map<ShopTemplateID_t, ShopTemplate*>::const_iterator itr = m_Entries.begin();
    for (; itr != m_Entries.end(); itr++)
        msg << "(ShopTemplate:" << (int)(itr->first) << "," << itr->second->toString() << ")";
    msg << ")";
    return msg.toString();

    __END_CATCH
}
