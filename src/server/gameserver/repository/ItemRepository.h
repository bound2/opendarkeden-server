#ifndef __ITEM_REPOSITORY_H__
#define __ITEM_REPOSITORY_H__

#include <string>
#include <vector>

#include "Types.h"

// The item bookkeeping tables: the trace logs (ItemTraceLog, MoneyTraceLog,
// OpCreate), the unique-item quotas (UniqueItemInfo), the time-limited
// items (TimeLimitItems), the event counters (CardCount, LuckyBagCount,
// GiftBoxCount, EventItemCount, EventItemCount2, ResurrectItemCount,
// EventStarObject), the event-quest reward schedule and record
// (EventQuestRewardSchedule, EventQuestRewardRecord), and the per-class
// item-object table operations that take the table NAME as data
// (Item::destroy's DELETE, GlobalItemPositionLoader's position read, the
// ItemIDRegistry counts). Reads are typed to the driver getter used for
// each column (getInt → int, getString → std::string).

// The item's id and type as their own types, the rest the strings the
// caller resolved from its lookup tables.
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
    // The GM item-creation log: OpCreate (OpName, DateTime, ItemDesc). The
    // datetime is the text the caller formatted, the description the item's
    // toString(); all three are interpolated unescaped.
    virtual void insertOpCreateLog(const std::string& opName, const std::string& dateTime,
                                   const std::string& itemDesc) = 0;
    virtual void insertMoneyTraceLog(const std::string& preOwner, const std::string& owner, const std::string& logType,
                                     const std::string& detailType, int amount) = 0;

    // --- event-quest rewards and counters ---------------------------------------
    // Decrements one still-open schedule row (Count > 0, Time past); true
    // when a row changed.
    virtual bool takeEventQuestReward(DWORD rewardID, DWORD questLevel) = 0;
    // The record of a scratch win. The PlayerID column gets the CHARACTER
    // name and RealPlayerID the account id.
    virtual void insertEventQuestRewardRecord(const std::string& name, DWORD rewardID,
                                              const std::string& accountID) = 0;
    // ifnull(sum(Num),0) over EventStarObject rows with ItemType 0. An
    // aggregate always answers one row, so this always returns true; the
    // caller throws ProtocolException on false anyway.
    virtual bool loadBlackStarCount(int& count) = 0;
    virtual void incrementResurrectItemCount() = 0;
    virtual void incrementCardCount(int cardKind) = 0;
    virtual void incrementLuckyBagCount(int bagKind) = 0;
    virtual void incrementGiftBoxCount(int boxKind) = 0;
    virtual void incrementEventItemCount(uint itemClass, uint itemType) = 0;
    // The per-race, per-index counter of the 2005 common event
    // (EventItemCount2).
    virtual void incrementEventItemCount2(Race_t race, int itemIndex) = 0;

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
    // ItemIDRegistry: the row count, then — only for a non-empty table — the
    // highest ItemID. Both through getDWORD, so a bigint ItemID above 32 bits
    // is truncated.
    virtual DWORD countItemRows(const std::string& tableName) = 0;
    virtual DWORD loadMaxItemID(const std::string& tableName) = 0;
};

// The process-wide MySQL-backed instance, wired in MySQLItemRepository.cpp.
ItemRepository& defaultItemRepository();

#endif
