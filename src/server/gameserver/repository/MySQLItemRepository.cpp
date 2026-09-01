#include "DB.h"
#include "repository/ItemRepository.h"

namespace {

// MySQL implementation of the item bookkeeping seam. The legacy quirks
// are quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every statement is byte-for-byte the inline original, including
//    TimeLimitItems' lower-case "from"/"where"/"and", the trace logs'
//    "( %u,'%s',..." spacing and their SQL-side now(), and the two
//    spacings of the same increment: EventItemCount's and the reward
//    schedule's "Count = Count + 1" / "Count = Count - 1" against
//    UniqueItemInfo's "CurrentNumber=CurrentNumber+1" and
//    ResurrectItemCount's "Count=Count+1".
//  - Four varargs mismatches the originals had are kept: Item::destroy's
//    DELETE streams an ItemID_t (DWORD) through "%lu",
//    GlobalItemPositionLoader's SELECT the same type through "%d",
//    bWinPrize's two DWORDs go through "%d", and the trace log's
//    ItemType_t (WORD, promoted to int) through "%u". MySQLCharacterRepository.cpp
//    calls this family a latent bug, not a benign quirk: at stack-passed
//    vararg positions clang -O0 has been seen to read garbage. The
//    mismatched arguments here sit at positions 2-3, register-passed on
//    the SysV ABI, which is why they have never misbehaved; still latent.
//  - The two per-class item-object operations take the TABLE NAME as a
//    parameter and interpolate it raw through "%s". GlobalItemPositionLoader
//    and ConcreteItem::getObjectTableName() pick it from Item's
//    ItemObjectTableName[] table; Corpse::getObjectTableName() returns ""
//    (pre-existing, a destroy on a corpse would emit "DELETE FROM  WHERE").
//    Never user text either way.
//  - Names and dates are interpolated raw, as before.
class MySQLItemRepository : public ItemRepository {
public:
    void insertItemTraceLog(const ItemTraceRecord& record) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            pStmt->executeQuery(
                "INSERT INTO ItemTraceLog (ItemID, ItemClass, ItemType, OptionType, PreOwnerID, OwnerID, "
                "LogType, DetailType, Time) VALUES ( %u,'%s',%u,'%s','%s','%s','%s','%s',now() )",
                record.itemID, record.itemClass.c_str(), record.itemType, record.optionName.c_str(),
                record.preOwner.c_str(), record.owner.c_str(), record.logType.c_str(), record.detailType.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertMoneyTraceLog(const string& preOwner, const string& owner, const string& logType,
                             const string& detailType, int amount) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            pStmt->executeQuery(
                "INSERT INTO MoneyTraceLog (PreOwnerID, OwnerID, LogType, DetailType, Amount, Time) VALUES "
                "( '%s','%s','%s','%s', %d, now() )",
                preOwner.c_str(), owner.c_str(), logType.c_str(), detailType.c_str(), amount);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool takeEventQuestReward(DWORD rewardID, DWORD questLevel) {
        bool taken = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            pStmt->executeQuery(
                "UPDATE EventQuestRewardSchedule SET Count = Count - 1 WHERE Count > 0 AND RewardID = %d "
                "AND QuestLevel = %d AND Time < now() LIMIT 1",
                rewardID, questLevel);

            if (pStmt->getAffectedRowCount() > 0)
                taken = true;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return taken;
    }

    void incrementResurrectItemCount() {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            pStmt->executeQuery("UPDATE ResurrectItemCount SET Count=Count+1");

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void incrementCardCount(int cardKind) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            pStmt->executeQuery("UPDATE CardCount SET CARDCOUNT = CARDCOUNT + 1 WHERE CARDKIND = %d", cardKind);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void incrementLuckyBagCount(int bagKind) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            pStmt->executeQuery("UPDATE LuckyBagCount SET BAGCOUNT = BAGCOUNT + 1 WHERE BAGKIND = %d", bagKind);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void incrementGiftBoxCount(int boxKind) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            pStmt->executeQuery("UPDATE GiftBoxCount SET BOXCOUNT = BOXCOUNT + 1 WHERE BOXKIND = %d", boxKind);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void incrementEventItemCount(uint itemClass, uint itemType) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            pStmt->executeQuery("UPDATE EventItemCount SET Count = Count + 1 WHERE ItemClass=%u AND ItemType=%u",
                                itemClass, itemType);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<UniqueItemRow> loadUniqueItems() {
        vector<UniqueItemRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            Result* pResult = pStmt->executeQuery("SELECT ItemClass, ItemType FROM UniqueItemInfo");

            while (pResult->next()) {
                UniqueItemRow row;
                row.itemClass = pResult->getInt(1);
                row.itemType = pResult->getInt(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadUniqueItemNumbers(int itemClass, int itemType, int& limitNumber, int& currentNumber) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            Result* pResult = pStmt->executeQuery(
                "SELECT LimitNumber, CurrentNumber FROM UniqueItemInfo WHERE ItemClass=%d AND ItemType=%d", itemClass,
                itemType);

            if (pResult->next()) {
                limitNumber = pResult->getInt(1);
                currentNumber = pResult->getInt(2);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void incrementUniqueItemCount(int itemClass, int itemType) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            pStmt->executeQuery(
                "UPDATE UniqueItemInfo SET CurrentNumber=CurrentNumber+1 WHERE ItemClass=%d AND ItemType=%d", itemClass,
                itemType);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void decrementUniqueItemCount(int itemClass, int itemType) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            pStmt->executeQuery(
                "UPDATE UniqueItemInfo SET CurrentNumber=CurrentNumber-1 WHERE ItemClass=%d AND ItemType=%d", itemClass,
                itemType);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<TimeLimitItemRow> loadTimeLimitItems(const string& owner, uint status) {
        vector<TimeLimitItemRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT ItemClass, ItemID, LimitDateTime from TimeLimitItems where OwnerID='%s' and Status=%u",
                owner.c_str(), status);

            while (pResult->next()) {
                TimeLimitItemRow row;
                row.itemClass = pResult->getInt(1);
                row.itemID = pResult->getInt(2);
                row.limitDateTime = pResult->getString(3);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void insertTimeLimitItem(const string& owner, uint itemClass, uint itemID, const string& limitDateTime) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "INSERT INTO TimeLimitItems (OwnerID, ItemClass, ItemID, LimitDateTime) VALUES ('%s',%u,%u,'%s')",
                owner.c_str(), itemClass, itemID, limitDateTime.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool updateTimeLimitItemStatus(uint status, const string& owner, uint itemClass, uint itemID) {
        bool changed = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE TimeLimitItems SET Status=%u where OwnerID='%s' and ItemClass=%u and ItemID=%u",
                                status, owner.c_str(), itemClass, itemID);

            changed = pStmt->getAffectedRowCount() != 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return changed;
    }

    bool deleteItemRow(const string& tableName, ItemID_t itemID) {
        bool deleted = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            pStmt->executeQuery("DELETE FROM %s WHERE ItemID = %lu", tableName.c_str(), itemID);

            deleted = pStmt->getAffectedRowCount() != 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return deleted;
    }

    bool loadItemPosition(const string& tableName, ItemID_t itemID, ItemPositionRow& row) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT OwnerID, Storage, StorageID, X, Y, ObjectID FROM %s WHERE ItemID = %d",
                                    tableName.c_str(), itemID);

            if (pResult->next()) {
                int i = 0;
                row.ownerID = pResult->getString(++i);
                row.storage = pResult->getInt(++i);
                row.storageID = pResult->getInt(++i);
                row.x = pResult->getInt(++i);
                row.y = pResult->getInt(++i);
                row.objectID = pResult->getInt(++i);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }
};

} // namespace

ItemRepository& defaultItemRepository() {
    static MySQLItemRepository instance;
    return instance;
}
