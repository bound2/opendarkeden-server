//////////////////////////////////////////////////////////////////////////////
// Filename    : ItemFactoryManager.h
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __ITEM_FACTORY_MANAGER_H__
#define __ITEM_FACTORY_MANAGER_H__

#include <unordered_map>

#include "Item.h"
#include "ItemFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class ItemFactoryManager
//////////////////////////////////////////////////////////////////////////////

class ItemFactoryManager {
public:
    ItemFactoryManager();
    ~ItemFactoryManager();

public:
    void init();

    void addFactory(ItemFactory* pFactory);

    Item* createItem(Item::ItemClass IClass, ItemType_t ItemType, const list<OptionType_t>& OptionType);

    string getItemName(Item::ItemClass IClass);

    Item::ItemClass getItemClassByName(const string& ClassName);

    string toString() const;

private:
    ItemFactory** m_Factories; // Array of item factories
    ushort m_Size;             // Size of the item factory array
    unordered_map<string, Item::ItemClass> m_ItemClassMap;
};

#endif
