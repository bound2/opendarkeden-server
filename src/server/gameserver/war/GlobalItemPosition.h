#ifndef __GLOBAL_ITEM_POSITION_H__
#define __GLOBAL_ITEM_POSITION_H__

#include "Exception.h"
#include "Types.h"

class Zone;
class Item;
class PlayerCreature;

class GlobalItemPosition {
public:
    enum PositionType {
        POS_TYPE_ZONE,      // 0
        POS_TYPE_INVENTORY, // 1
        POS_TYPE_MOUSE,     // 2
        POS_TYPE_CORPSE,    // 3
    };

protected:
    // This class cannot be created directly.
    GlobalItemPosition(PositionType type) : m_ItemPosType(type) {}

public:
    virtual ~GlobalItemPosition(){};

public:
    void setType(PositionType type) {
        m_ItemPosType = type;
    }
    PositionType getType() const {
        return m_ItemPosType;
    }

public:
    // Pull the item out.
    virtual Item* popItem(bool bLock = true) = 0;
    virtual Zone* getZone() = 0;

    // Pull the item out of pc, the player the position names, whom the
    // caller already found and owns: the zone thread running a command
    // posted to that player. Only the positions a player holds answer; the
    // others hold no player and give nothing.
    virtual Item* popItemFrom(PlayerCreature& pc) {
        return NULL;
    }

    // Make the pops take the item only if it is this one. The position is a
    // row read earlier, and by the time it is used the place it names may
    // hold another item -- a tile something else was dropped on, an
    // inventory slot filled again -- which a pop must leave alone. Without
    // this a pop takes whatever is there.
    void expectItem(int itemClass, ItemID_t itemID) {
        m_bExpectItem = true;
        m_ExpectedItemClass = itemClass;
        m_ExpectedItemID = itemID;
    }
    bool isExpectedItem(int itemClass, ItemID_t itemID) const {
        return !m_bExpectItem || (itemClass == m_ExpectedItemClass && itemID == m_ExpectedItemID);
    }

public:
    virtual string toString() const = 0;

private:
    PositionType m_ItemPosType;

    bool m_bExpectItem = false;
    int m_ExpectedItemClass = 0;
    ItemID_t m_ExpectedItemID = 0;
};

#endif // __GLOBAL_ITEM_POSITION_H__
