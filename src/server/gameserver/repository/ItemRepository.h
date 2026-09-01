#ifndef __ITEM_REPOSITORY_H__
#define __ITEM_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// Persistence seam for the item bookkeeping tables (task 3.2): the trace
// logs (ItemTraceLog, MoneyTraceLog), the unique-item quotas
// (UniqueItemInfo), the time-limited items (TimeLimitItems), the event
// counters (CardCount, LuckyBagCount, GiftBoxCount, EventItemCount,
// ResurrectItemCount), the event-quest reward schedule
// (EventQuestRewardSchedule), and the two per-class item-object table
// operations that take the table NAME as data — Item::destroy's DELETE
// and GlobalItemPositionLoader's position read. Reads are typed to the
// driver getter the inline code called (getInt → int, getString →
// std::string); write parameters are typed to the expression each caller
// streamed, so the varargs bytes reaching the format strings are
// unchanged (an ItemID_t still goes through "%lu" in the delete and
// "%d" in the position read, as before).
//
// Not enclosed: the item-object tables' own INSERT/UPDATE/SELECT paths in
// gameserver/item/ (their own round), and MoonCardUtil.cpp's copy of the
// CardCount UPDATE, which no build target compiles.

// remainTraceLog's INSERT: the item's id and type as their own types, the
// rest the strings the caller resolved from its lookup tables.
struct ItemTraceRecord {
    ItemID_t itemID;
    std::string itemClass;
    ItemType_t itemType;
    std::string optionName;
    std::string preOwner;
    std::string owner;
    std::string logType;
    std::string detailType;
};

struct UniqueItemRow {
    int itemClass;
    int itemType;
};

struct TimeLimitItemRow {
    int itemClass;
    int itemID;
    std::string limitDateTime;
};

// One item-object row's position columns (GlobalItemPositionLoader).
struct ItemPositionRow {
    std::string ownerID;
    int storage;
    int storageID;
    int x;
    int y;
    int objectID;
};

class ItemRepository {
public:
    virtual ~ItemRepository() {}

    // --- trace logs ---------------------------------------------------------
    virtual void insertItemTraceLog(const ItemTraceRecord& record) = 0;
    virtual void insertMoneyTraceLog(const std::string& preOwner, const std::string& owner, const std::string& logType,
                                     const std::string& detailType, int amount) = 0;

    // --- event-quest rewards and counters ---------------------------------------
    // bWinPrize: decrements one still-open schedule row (Count > 0, Time
    // past); true when a row changed. The DWORDs stream through "%d".
    virtual bool takeEventQuestReward(DWORD rewardID, DWORD questLevel) = 0;
    virtual void incrementResurrectItemCount() = 0;
    virtual void incrementCardCount(int cardKind) = 0;
    virtual void incrementLuckyBagCount(int bagKind) = 0;
    virtual void incrementGiftBoxCount(int boxKind) = 0;
    virtual void incrementEventItemCount(uint itemClass, uint itemType) = 0;

    // --- unique items ---------------------------------------------------------
    virtual std::vector<UniqueItemRow> loadUniqueItems() = 0;
    // False when the (class, type) has no row.
    virtual bool loadUniqueItemNumbers(int itemClass, int itemType, int& limitNumber, int& currentNumber) = 0;
    virtual void incrementUniqueItemCount(int itemClass, int itemType) = 0;
    virtual void decrementUniqueItemCount(int itemClass, int itemType) = 0;

    // --- time-limited items -----------------------------------------------------
    virtual std::vector<TimeLimitItemRow> loadTimeLimitItems(const std::string& owner, uint status) = 0;
    virtual void insertTimeLimitItem(const std::string& owner, uint itemClass, uint itemID,
                                     const std::string& limitDateTime) = 0;
    // True when a row changed.
    virtual bool updateTimeLimitItemStatus(uint status, const std::string& owner, uint itemClass, uint itemID) = 0;

    // --- per-class item-object tables (the table name is data) ------------------
    // Item::destroy — true when a row was deleted.
    virtual bool deleteItemRow(const std::string& tableName, ItemID_t itemID) = 0;
    // GlobalItemPositionLoader::load — false when the item has no row.
    virtual bool loadItemPosition(const std::string& tableName, ItemID_t itemID, ItemPositionRow& row) = 0;
};

// The process-wide MySQL-backed instance, wired in MySQLItemRepository.cpp.
// An accessor function rather than a g_p* extern: ratchet R1 counts those.
ItemRepository& defaultItemRepository();

#endif
