//////////////////////////////////////////////////////////////////////////////
// Filename    : PriceManager.cpp
// Description :
// Class that decides the price when items are bought from or sold to a shop.
// Internally it computes from the original price held by ItemInfoManager.
//////////////////////////////////////////////////////////////////////////////

#include "PriceManager.h"

#include "Creature.h"
#include "GameContext.h"
#include "GamePlayer.h"
#include "Item.h"
#include "ItemInfoManager.h"
#include "ItemUtil.h"
#include "OptionInfo.h"
#include "Ousters.h"
#include "Slayer.h"
#include "Vampire.h"
#include "VariableManager.h"
#include "item/OustersSummonItem.h"
#include "item/Skull.h"
#include "item/SlayerPortalItem.h"
#include "item/VampirePortalItem.h"

// constants
const uint PORTAL_ITEM_CHARGE_PRICE = 5000;
const uint SUMMON_ITEM_CHARGE_PRICE = 1000;

//////////////////////////////////////////////////////////////////////////////
// getPrice()
// Determines the actual price of an item from its item info.
// The nDiscount parameter (a percentage) controls the price.
//////////////////////////////////////////////////////////////////////////////
Price_t PriceManager::getPrice(Item* pItem, MarketCond_t nDiscount, ShopRackType_t shopType,
                               Creature* pCreature) const {
    // An item that was given away for free sells for only 1.
    if (pItem->getCreateType() == Item::CREATE_TYPE_GAME)
        return (Price_t)1;
    // A time-limited quest item sells for 50.
    if (pItem->isTimeLimitItem())
        return (Price_t)50;
    if (pItem->getItemClass() == Item::ITEM_CLASS_MOON_CARD && pItem->getItemType() == 4) {
        return (Price_t)g_pVariableManager->getVariable(CROWN_PRICE);
    }

    // Get the item's original price.
    ItemInfo* pItemInfo = de::gameContext().itemInfos().getItemInfo(pItem->getItemClass(), pItem->getItemType());
    double originalPrice = pItemInfo->getPrice();
    double finalPrice = 0;

    if (pItem->getGrade() != -1) {
        double gradePercent = 80 + (5 * pItem->getGrade());
        //		originalPrice = getPercentValue( originalPrice, gradePercent );
        originalPrice *= (gradePercent / 100.0);
    }

    // A slayer portal adds the price of its current charges to the original price.
    if (pItem->getItemClass() == Item::ITEM_CLASS_SLAYER_PORTAL_ITEM) {
        SlayerPortalItem* pSlayerPortalItem = dynamic_cast<SlayerPortalItem*>(pItem);
        originalPrice += (pSlayerPortalItem->getCharge() * PORTAL_ITEM_CHARGE_PRICE);
    } else if (pItem->getItemClass() == Item::ITEM_CLASS_VAMPIRE_PORTAL_ITEM) {
        VampirePortalItem* pVampirePortalItem = dynamic_cast<VampirePortalItem*>(pItem);
        originalPrice += (pVampirePortalItem->getCharge() * PORTAL_ITEM_CHARGE_PRICE);
    } else if (pItem->getItemClass() == Item::ITEM_CLASS_OUSTERS_SUMMON_ITEM) {
        OustersSummonItem* pOustersSummonItem = dynamic_cast<OustersSummonItem*>(pItem);
        originalPrice += (pOustersSummonItem->getCharge() * SUMMON_ITEM_CHARGE_PRICE);
    }

    // If the item has options, multiply the price by the option multiplier.
    const list<OptionType_t>& optionTypes = pItem->getOptionTypeList();
    if (!optionTypes.empty()) {
        finalPrice = 0;

        // price = (original price * the option's PriceMultiplier / 100) + ..
        double priceMultiplier = 0;
        list<OptionType_t>::const_iterator itr;
        for (itr = optionTypes.begin(); itr != optionTypes.end(); itr++) {
            OptionInfo* pOptionInfo = g_pOptionInfoManager->getOptionInfo(*itr);
            Assert(pOptionInfo != NULL);
            priceMultiplier = (double)(pOptionInfo->getPriceMultiplier());
            finalPrice += (originalPrice * priceMultiplier / 100);
        }

        originalPrice = finalPrice;
    }

    // A damaged item loses price in proportion to the damage.
    double maxDurability = (double)computeMaxDurability(pItem);
    double curDurability = (double)(pItem->getDurability());

    // Some items have no durability, so handle that case.
    if (maxDurability > 1)
        finalPrice = originalPrice * curDurability / maxDurability;
    else
        finalPrice = originalPrice;

    // Adjust the price again for the shop's market condition.
    finalPrice = finalPrice * nDiscount / 100;

    // Adjust the price again for the kind of shop.
    if (shopType == SHOP_RACK_MYSTERIOUS) {
        finalPrice *= 10;
    }

    // Adjust the price again for the creature's own modifiers.
    if (pCreature != NULL) {
        if (pCreature->isSlayer()) {
            Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
            Attr_t CSTR = pSlayer->getSTR(ATTR_CURRENT);
            Attr_t CDEX = pSlayer->getDEX(ATTR_CURRENT);
            Attr_t CINT = pSlayer->getINT(ATTR_CURRENT);

            if ((CSTR + CDEX + CINT <= 40) && (pItem->getItemClass() == Item::ITEM_CLASS_POTION) &&
                (pItem->getItemType() == 0 || pItem->getItemType() == 5)) {
                finalPrice = getPercentValue((int)finalPrice, 70);
            }
        } else if (pCreature->isVampire()) {
            // A vampire selling a skull gets half the skull's price.
            if (pItem->getItemClass() == Item::ITEM_CLASS_SKULL) {
                finalPrice = finalPrice / 2.0;
            }
        } else if (pCreature->isOusters()) {
            // An ousters selling a skull gets 75% of the skull's price.
            if (pItem->getItemClass() == Item::ITEM_CLASS_SKULL) {
                finalPrice *= 0.75;
            }
        }
    }

    // For a paying user in a pay zone.
    if (g_pVariableManager->getVariable(PREMIUM_HALF_EVENT)) {
        if (pItem->getItemClass() == Item::ITEM_CLASS_POTION || pItem->getItemClass() == Item::ITEM_CLASS_SERUM ||
            pItem->getItemClass() == Item::ITEM_CLASS_LARVA || pItem->getItemClass() == Item::ITEM_CLASS_PUPA ||
            pItem->getItemClass() == Item::ITEM_CLASS_COMPOS_MEI) {
            if (pCreature->isPC()) {
                PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
                GamePlayer* pGamePlayer = dynamic_cast<GamePlayer*>(pPC->getPlayer());
                if (pGamePlayer->isPayPlaying()) {
                    // Half price.
                    finalPrice = finalPrice / 2;
                }
            }
        }
    }

    // Apply the Blood Bible bonus.
    if (pItem->getItemClass() == Item::ITEM_CLASS_POTION || pItem->getItemClass() == Item::ITEM_CLASS_SERUM) {
        if (pCreature->isPC()) {
            PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
            int ratio = pPC->getPotionPriceRatio();
            if (ratio != 0) {
                // The ratio value is negative.
                finalPrice += getPercentValue((int)finalPrice, ratio);
            }
        }
    }

    return max(1, (int)finalPrice);
}

//////////////////////////////////////////////////////////////////////////////
// getRepairPrice()
// Returns the cost of repairing an item.
// For a completely ruined item the repair cost is
// one tenth of the item's original price.
//////////////////////////////////////////////////////////////////////////////
Price_t PriceManager::getRepairPrice(Item* pItem, Creature* pCreature) const {
    // Get the item's original price.
    ItemInfo* pItemInfo = de::gameContext().itemInfos().getItemInfo(pItem->getItemClass(), pItem->getItemType());
    double originalPrice = pItemInfo->getPrice();
    double finalPrice = 0;

    if (pItem->getGrade() != -1) {
        double gradePercent = 80 + (5 * pItem->getGrade());
        //		originalPrice = getPercentValue( originalPrice, gradePercent );
        originalPrice *= (gradePercent / 100.0);
    }

    // A slayer portal cannot be repaired, but its charges can be topped up.
    if (pItem->getItemClass() == Item::ITEM_CLASS_SLAYER_PORTAL_ITEM) {
        SlayerPortalItem* pSlayerPortalItem = dynamic_cast<SlayerPortalItem*>(pItem);
        int MaxCharge = pSlayerPortalItem->getMaxCharge();
        int CurCharge = pSlayerPortalItem->getCharge();

        return (MaxCharge - CurCharge) * PORTAL_ITEM_CHARGE_PRICE;
    }

    if (pItem->getItemClass() == Item::ITEM_CLASS_OUSTERS_SUMMON_ITEM) {
        OustersSummonItem* pOustersSummonItem = dynamic_cast<OustersSummonItem*>(pItem);
        int MaxCharge = pOustersSummonItem->getMaxCharge();
        int CurCharge = pOustersSummonItem->getCharge();

        return (MaxCharge - CurCharge) * SUMMON_ITEM_CHARGE_PRICE;
    }

    // If the item has options, multiply the price by the option multiplier.
    const list<OptionType_t>& optionTypes = pItem->getOptionTypeList();
    if (!optionTypes.empty()) {
        finalPrice = 0;
        // price = (original price * sum of the options' PriceMultipliers / 100) * number of options
        double priceMultiplier = 0;
        list<OptionType_t>::const_iterator itr;
        for (itr = optionTypes.begin(); itr != optionTypes.end(); itr++) {
            OptionInfo* pOptionInfo = g_pOptionInfoManager->getOptionInfo(*itr);
            Assert(pOptionInfo != NULL);
            priceMultiplier = (double)(pOptionInfo->getPriceMultiplier());
            finalPrice += (originalPrice * priceMultiplier / 100);
        }

        originalPrice = finalPrice;
    }

    // A damaged item loses price in proportion to the damage.
    double maxDurability = (double)computeMaxDurability(pItem);
    double curDurability = (double)(pItem->getDurability());

    // Some items have no durability, so handle that case.
    if (maxDurability != 0) {
        // Return early if the item is at full durability.
        if (curDurability == maxDurability) {
            return 0;
        }

        // Current durability divided by maximum durability gives how damaged the item is.
        // Multiplying the original price by it lowers the value as durability drops.
        finalPrice = originalPrice * curDurability / maxDurability;
    } else {
        // An item without durability cannot be damaged, so
        // its durability-adjusted price equals the original price.
        finalPrice = originalPrice;
    }

    // The repair cost is one tenth of the lost value.
    finalPrice = (originalPrice - finalPrice) / 10.0;

    if (finalPrice < 1.0) {
        return 1;
    }

    return max(0, (int)finalPrice);
}

//////////////////////////////////////////////////////////////////////////////
// getSilverCoatingPrice()
// The price of silver-coating an item.
//////////////////////////////////////////////////////////////////////////////
Price_t PriceManager::getSilverCoatingPrice(Item* pItem, Creature* pCreature) const {
    if (pItem == NULL)
        return 0;

    switch (pItem->getItemClass()) {
    case Item::ITEM_CLASS_BLADE:
    case Item::ITEM_CLASS_SWORD:
    case Item::ITEM_CLASS_CROSS:
    case Item::ITEM_CLASS_MACE:
        break;
    default:
        return 0;
    }

    ItemInfo* pItemInfo = de::gameContext().itemInfos().getItemInfo(pItem->getItemClass(), pItem->getItemType());
    double maxSilver = pItemInfo->getMaxSilver();
    double finalPrice = 0;

    // Stopgap: the price is the amount of silver.
    finalPrice = maxSilver;

    return max(0, (int)finalPrice);
}

//////////////////////////////////////////////////////////////////////////////
// getStashPrice()
// Returns the price of a stash slot.
// It changes rarely, so the values are written into the code.
//////////////////////////////////////////////////////////////////////////////
Price_t PriceManager::getStashPrice(BYTE index, Creature* pCreature) const {
    Price_t price = 0;

    switch (index) {
    case 1:
        price = 100000;
        break;
    case 2:
        price = 1000000;
        break;
    case 3:
        price = 10000000;
        break;
    default:
        cerr << "PriceManager::getStashPrice() : Unknown Stash Index" << endl;
        Assert(false);
    }

    if (pCreature != NULL) {
        if (pCreature->isSlayer()) {
        } else if (pCreature->isVampire()) {
        } else if (pCreature->isOusters()) {
        }
    }

    return price;
}


//////////////////////////////////////////////////////////////////////////////
// Price function for events.
// Information for the star item used in the Christmas event.
// The same event is reused for Children's Day, so the code is enabled again.
//
// Since the star event may run again,
// renaming this to STAR_EVENT_CODE should be considered.
//////////////////////////////////////////////////////////////////////////////
int PriceManager::getStarPrice(Item* pItem, XMAS_STAR& star) const {
    Assert(pItem != NULL);

    ItemType_t IType = pItem->getItemType();
    OptionType_t OType = pItem->getFirstOptionType();

    Assert(OType != 0);

    OptionInfo* pOptionInfo = g_pOptionInfoManager->getOptionInfo(OType);
    Assert(pOptionInfo != NULL);
    OptionClass OClass = pOptionInfo->getClass();

    switch (OClass) {
    case OPTION_DAMAGE:
        star.color = STAR_COLOR_BLACK;
        break;
    case OPTION_STR:
        star.color = STAR_COLOR_RED;
        break;
    case OPTION_INT:
        star.color = STAR_COLOR_BLUE;
        break;
    case OPTION_DEX:
        star.color = STAR_COLOR_GREEN;
        break;
    case OPTION_ATTACK_SPEED:
        star.color = STAR_COLOR_CYAN;
        break;
    default:
        Assert(false);
        break;
    }

    star.amount = (IType - 1) * 20;

    return 0;
}

int PriceManager::getBallPrice(int price, XMAS_STAR& star) const {
    star.amount = price;
    star.color = STAR_COLOR_PINK;

    return 0;
}

// Mysterious item price.
// The price varies with itemClass and with pCreature's attributes.
Price_t PriceManager::getMysteriousPrice(Item::ItemClass itemClass, Creature* pCreature) const {
    int multiplier = 1;

    if (pCreature->isSlayer()) {
        Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);

        Attr_t CSTR = pSlayer->getSTR(ATTR_BASIC);
        Attr_t CDEX = pSlayer->getDEX(ATTR_BASIC);
        Attr_t CINT = pSlayer->getINT(ATTR_BASIC);
        Attr_t CSUM = CSTR + CDEX + CINT;

        // Between 0 and 20
        multiplier = CSUM / 15;
    } else if (pCreature->isVampire()) {
        Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);

        Level_t CLevel = pVampire->getLevel();

        // Between 0 and 20
        multiplier = CLevel / 5;
    } else if (pCreature->isOusters()) {
        Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);

        Level_t CLevel = pOusters->getLevel();

        // Between 0 and 20
        multiplier = CLevel / 5;
    }

    // Between 1 and 20
    multiplier = max(1, multiplier);

    // Get the average price.
    InfoClassManager* pInfoClass = de::gameContext().itemInfos().getInfoManager(itemClass);
    Assert(pInfoClass != NULL);

    // Average price * attribute ratio.
    int finalPrice = (int)pInfoClass->getAveragePrice() * multiplier;

    // Apply the Blood Bible bonus.
    if (pCreature->isPC()) {
        PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
        int ratio = pPC->getGamblePriceRatio();
        if (ratio != 0) {
            // The ratio value is negative.
            finalPrice += getPercentValue(finalPrice, ratio);
        }
    }

    return finalPrice;
}
