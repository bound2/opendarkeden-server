//////////////////////////////////////////////////////////////////////////////
// Filename    : EventStar.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EVENT_STAR_H__
#define __EVENT_STAR_H__

#include "InfoClassManager.h"
#include "Item.h"
#include "ItemFactory.h"
#include "ItemInfo.h"
#include "ItemLoader.h"
#include "Mutex.h"

//////////////////////////////////////////////////////////////////////////////
// class EventStar;
//////////////////////////////////////////////////////////////////////////////

class EventStar : public Item {
public:
    EventStar();
    EventStar(ItemType_t itemType, const list<OptionType_t>& optionType, ItemNum_t Num);

public:
    virtual void create(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y,
                        ItemID_t itemID = 0);
    virtual void save(const string& ownerID, Storage storage, StorageID_t storageID, BYTE x, BYTE y);
    void tinysave(const string& field) const {
        tinysave(field.c_str());
    }
    void tinysave(const char* field) const;
    virtual string toString() const;

    static void initItemIDRegistry(void);

public:
    virtual ItemClass getItemClass() const {
        return Item::ITEM_CLASS_EVENT_STAR;
    }
    virtual string getObjectTableName() const {
        return "EventStarObject";
    }

    virtual ItemType_t getItemType() const {
        return m_ItemType;
    }
    virtual void setItemType(ItemType_t itemType) {
        m_ItemType = itemType;
    }

    virtual VolumeWidth_t getVolumeWidth() const;
    virtual VolumeHeight_t getVolumeHeight() const;
    virtual Weight_t getWeight() const;

public:
    virtual ItemNum_t getNum() const {
        return m_Num;
    }
    virtual void setNum(ItemNum_t Num) {
        m_Num = Num;
    }

    bool isStackable() const {
        return true;
    }

private:
    ItemType_t m_ItemType;
    ItemNum_t m_Num;

    static Mutex m_Mutex;             // Lock for the item ID registry
    static ItemID_t m_ItemIDRegistry; // Per-class unique item ID generator
};

//////////////////////////////////////////////////////////////////////////////
// class EventStarInfo
//////////////////////////////////////////////////////////////////////////////

class EventStarInfo : public ItemInfo {
public:
    enum ITEM_FUNCTION {
        FUNCTION_NULL = 0,
        FUNCTION_ENCHANT_OPTION = 0x01,      // Changes an option for the better.
        FUNCTION_ADD_OPTION = 0x02,          // Adds an option.
        FUNCTION_ENCHANT_RARE_OPTION = 0x04, // Raises the option of a rare item.
        FUNCTION_TRANS_KIT = 0x08,           // Changes the item's sex.
        FUNCTION_UP_GRADE = 0x10,            // Raises the item's grade.
    };

public:
    virtual Item::ItemClass getItemClass() const {
        return Item::ITEM_CLASS_EVENT_STAR;
    }
    virtual string toString() const;

public:
    virtual BYTE getFunctionFlag() const {
        return m_fFunction;
    }
    virtual BYTE isFunctionEnchantOption() const {
        return m_fFunction & FUNCTION_ENCHANT_OPTION;
    } // Changes an option for the better.
    virtual BYTE isFunctionAddOption() const {
        return m_fFunction & FUNCTION_ADD_OPTION;
    } // Adds an option.
    virtual BYTE isFunctionEnchantRareOption() const {
        return m_fFunction & FUNCTION_ENCHANT_RARE_OPTION;
    } // Changes an option for the better.
    virtual BYTE isFunctionTransKit() const {
        return m_fFunction & FUNCTION_TRANS_KIT;
    }
    virtual void setFunctionFlag(BYTE flag) {
        m_fFunction = flag;
    }

    virtual int getFunctionValue() const {
        return m_FunctionValue;
    }
    virtual void setFunctionValue(int value) {
        m_FunctionValue = value;
    }


    BYTE m_fFunction;    // Function
    int m_FunctionValue; // Value that goes with the function
};

//////////////////////////////////////////////////////////////////////////////
// class EventStarInfoManager;
//////////////////////////////////////////////////////////////////////////////

class EventStarInfoManager : public InfoClassManager {
public:
    virtual Item::ItemClass getItemClass() const {
        return Item::ITEM_CLASS_EVENT_STAR;
    }
    virtual void load();
};


//////////////////////////////////////////////////////////////////////////////
// class EventStarFactory
//////////////////////////////////////////////////////////////////////////////

class EventStarFactory : public ItemFactory {
public:
    virtual Item::ItemClass getItemClass() const {
        return Item::ITEM_CLASS_EVENT_STAR;
    }
    virtual string getItemClassName() const {
        return "EventStar";
    }

public:
    virtual Item* createItem(ItemType_t ItemType, const list<OptionType_t>& OptionType) {
        return new EventStar(ItemType, OptionType, 1);
    }
};

//////////////////////////////////////////////////////////////////////////////
// class EventStarLoader;
//////////////////////////////////////////////////////////////////////////////

class EventStarLoader : public ItemLoader {
public:
    virtual Item::ItemClass getItemClass() const {
        return Item::ITEM_CLASS_EVENT_STAR;
    }
    virtual string getItemClassName() const {
        return "EventStar";
    }

public:
    virtual void load(Creature* pCreature);
    virtual void load(Zone* pZone);
    virtual void load(StorageID_t storageID, Inventory* pInventory);
};

#endif
