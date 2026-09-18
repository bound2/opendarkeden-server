//////////////////////////////////////////////////////////////////////////////
// Filename    : ItemMap.h
// Description :
// Sorted map of items received from the client.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ITEM_MAP__
#define __ITEM_MAP__

#include <list>
#include <map>

#include "Item.h"

//////////////////////////////////////////////////////////////////////////////
// class ItemMap
//////////////////////////////////////////////////////////////////////////////

class Inventory;
class Item;

class ItemMap : public std::map<ulonglong, Item*> {
    // Inner type definition
public:
    typedef std::map<ulonglong, Item*> ITEM_MAP;
    typedef ITEM_MAP::iterator iterator;
    typedef ITEM_MAP::const_iterator const_iterator;

    // Member methods
public:
    ItemMap();
    ~ItemMap();

public:
    // Drop all the data.
    void clearAll(void) {
        clear();
        m_Num2x2 = 0;
        m_Num2x2Temp = 0;
    }

    // Add an item.
    bool addItem(Item* pItem);

    // Set the number of 2x2 items.
    void set2x2(int n);
    int get2x2() const {
        return m_Num2x2;
    }

protected:
    // Compute the key used to put an item into the map.
    ulonglong getKey(Item* pItem);

    // Member data
protected:
    int m_Num2x2;
    int m_Num2x2Temp; // Temporary - required by GetKey
};


#endif
