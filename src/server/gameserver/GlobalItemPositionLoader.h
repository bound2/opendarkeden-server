#ifndef __GLOBAL_ITEM_POSITION_LOADER_H__
#define __GLOBAL_ITEM_POSITION_LOADER_H__

#include "GlobalItemPosition.h"
#include "Item.h"
#include "Types.h"

struct ItemPositionRow;

class GlobalItemPositionLoader {
public:
    // Where the item lies now, as its item-object row says; NULL when it has
    // no row or lies somewhere no position reaches. The caller owns it.
    GlobalItemPosition* load(Item::ItemClass itemClass, ItemID_t itemID);

    // The position a row names, as load() makes it. A row is plain values,
    // so a caller may read it on one thread and make the position on the
    // thread that owns the place it names.
    GlobalItemPosition* makeGlobalItemPosition(const ItemPositionRow& row);

    static GlobalItemPositionLoader* getInstance() {
        static GlobalItemPositionLoader globalItemPositionLoader;
        return &globalItemPositionLoader;
    }

private:
    GlobalItemPositionLoader() {}
};

#endif
