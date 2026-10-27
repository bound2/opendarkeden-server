//////////////////////////////////////////////////////////////////////////////
// Filename    : game_mysterious_item_test.cpp
// Description : The item type a slayer's or vampire's mysterious item falls
//               back on when none of its random draws fits the buyer: type
//               1 only for a woman's coat or trousers and a vampire woman's
//               coat, type 0 for every other class and for men.
//////////////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>

#include "Item.h"
#include "ItemUtil.h"
#include "types/CreatureTypes.h"

namespace {

TEST(GameMysteriousItem, OnlyAWomansCoatOrTrousersFallBackOnTypeOne) {
    const Item::ItemClass cut[] = {Item::ITEM_CLASS_COAT, Item::ITEM_CLASS_TROUSER, Item::ITEM_CLASS_VAMPIRE_COAT};
    for (Item::ItemClass itemClass : cut) {
        SCOPED_TRACE(itemClass);
        EXPECT_EQ(mysteriousFallbackItemType(itemClass, FEMALE), 1);
        EXPECT_EQ(mysteriousFallbackItemType(itemClass, MALE), 0);
    }

    const Item::ItemClass uncut[] = {Item::ITEM_CLASS_VAMPIRE_WEAPON, Item::ITEM_CLASS_VAMPIRE_RING,
                                     Item::ITEM_CLASS_SWORD, Item::ITEM_CLASS_HELM};
    for (Item::ItemClass itemClass : uncut) {
        SCOPED_TRACE(itemClass);
        EXPECT_EQ(mysteriousFallbackItemType(itemClass, FEMALE), 0);
        EXPECT_EQ(mysteriousFallbackItemType(itemClass, MALE), 0);
    }
}

} // namespace
