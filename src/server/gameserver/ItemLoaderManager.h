//////////////////////////////////////////////////////////////////////////////
// Filename    : ItemLoaderManager.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __ITEM_LOADER_MANAGER_H__
#define __ITEM_LOADER_MANAGER_H__

#include "Item.h"
#include "ItemLoader.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ItemLoaderManager;
//////////////////////////////////////////////////////////////////////////////

class Slayer;
class Vampire;
class Ousters;
class Zone;

class ItemLoaderManager {
public:
    ItemLoaderManager();
    ~ItemLoaderManager();

public:
    void init();

    void load(Slayer* pSlayer);
    void load(Vampire* pVampire);
    void load(Ousters* pOusters);
    void load(Zone* pZone);

    string toString() const;

private:
    // The loaders this manager creates and owns, one slot per item class.
    // init() fills the slots whose items are loaded from the database and
    // leaves the rest null.
    ItemLoader* m_pItemLoaders[Item::ITEM_CLASS_MAX] = {};
};

#endif
