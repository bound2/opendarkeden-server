#include "DB.h"
#include "repository/QuestInfoRepository.h"

namespace {

// MySQL implementation of the quest-info seam. The legacy quirks are quarantined
// HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every literal is byte-for-byte the mission/*.cpp original, including
//    SimpleQuestInfoManager's "WHERE NPC = '%s'" (spaces around the equals sign)
//    against the event manager's and the reward manager's "WHERE NPC='%s'".
//  - The NPC and owner names are interpolated raw, as before.
//  - EventQuestLootingInfo's SELECT computes LootingType-1 in SQL; the row keeps
//    that value under `lootingType`, and the statement takes no arguments, so it
//    goes out through executeQueryString exactly as it did.
//  - Every numeric column is read through getInt, the getter the inline code
//    called, whatever the column's width; the callers do their own casting.
//  - EventQuestAdvance's save ran its UPDATE and, when no row went, an
//    INSERT IGNORE, both on one Statement. Here each is its own method, so each
//    makes its own Statement; the caller keeps the branch.
class MySQLQuestInfoRepository : public QuestInfoRepository {
public:
    vector<MonsterKillQuestRow> loadMonsterKillQuestsOfNPC(const string& npcName) {
        vector<MonsterKillQuestRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT QuestID, Race, MaxGrade, MinGrade, TimeLimitSec, RewardClass, "
                                    "TargetSType, IsChief, Goal FROM MonsterKillQuestInfo WHERE NPC = '%s'",
                                    npcName.c_str());

            while (pResult->next()) {
                uint i = 0;
                MonsterKillQuestRow row;
                readQuestHead(pResult, i, row.head);
                row.targetSType = pResult->getInt(++i);
                row.isChief = pResult->getInt(++i);
                row.goal = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ItemRewardRow> loadItemRewardsOfNPC(const string& npcName) {
        return loadRewards("SELECT RewardClass, RewardID, IClass, IType, OptionType, TimeLimitSec FROM ItemRewardInfo "
                           "WHERE NPC='%s'",
                           npcName);
    }

    vector<ItemRewardRow> loadSlayerWeaponRewardsOfNPC(const string& npcName) {
        return loadRewards("SELECT RewardClass, RewardID, IClass, IType, OptionType, TimeLimitSec FROM "
                           "SlayerWeaponRewardInfo WHERE NPC='%s'",
                           npcName);
    }

    vector<EventMonsterKillQuestRow> loadEventMonsterKillQuestsOfNPC(const string& npcName) {
        vector<EventMonsterKillQuestRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT QuestID, Race, MaxGrade, MinGrade, TimeLimitSec, RewardClass, TargetSType, "
                                    "IsChief, Goal, EventQuest, QuestLevel FROM MonsterKillQuestInfo WHERE NPC='%s'",
                                    npcName.c_str());

            while (pResult->next()) {
                uint i = 0;
                EventMonsterKillQuestRow row;
                readQuestHead(pResult, i, row.quest.head);
                row.quest.targetSType = pResult->getInt(++i);
                row.quest.isChief = pResult->getInt(++i);
                row.quest.goal = pResult->getInt(++i);
                row.eventQuest = pResult->getInt(++i);
                row.questLevel = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<EventGatherItemQuestRow> loadEventGatherItemQuestsOfNPC(const string& npcName) {
        vector<EventGatherItemQuestRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT QuestID, Race, MaxGrade, MinGrade, TimeLimitSec, RewardClass, TargetIClass, "
                "TargetIType, Goal, EventQuest, QuestLevel FROM GatherItemQuestInfo WHERE NPC='%s'",
                npcName.c_str());

            while (pResult->next()) {
                uint i = 0;
                EventGatherItemQuestRow row;
                readQuestHead(pResult, i, row.quest.head);
                row.quest.targetIClass = pResult->getInt(++i);
                row.quest.targetIType = pResult->getInt(++i);
                row.quest.goal = pResult->getInt(++i);
                row.eventQuest = pResult->getInt(++i);
                row.questLevel = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<EventMeetNPCQuestRow> loadEventMeetNPCQuestsOfNPC(const string& npcName) {
        vector<EventMeetNPCQuestRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT QuestID, Race, MaxGrade, MinGrade, TimeLimitSec, RewardClass, TargetNPCID, "
                                    "SecondNPCID, EventQuest, QuestLevel FROM MeetNPCQuestInfo WHERE NPC='%s'",
                                    npcName.c_str());

            while (pResult->next()) {
                uint i = 0;
                EventMeetNPCQuestRow row;
                readQuestHead(pResult, i, row.quest.head);
                row.quest.targetNPCID = pResult->getInt(++i);
                row.quest.secondNPCID = pResult->getInt(++i);
                row.eventQuest = pResult->getInt(++i);
                row.questLevel = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<EventMiniGameQuestRow> loadEventMiniGameQuestsOfNPC(const string& npcName) {
        vector<EventMiniGameQuestRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT QuestID, Race, MaxGrade, MinGrade, TimeLimitSec, RewardClass, "
                                    "GameType, EventQuest, QuestLevel FROM MiniGameQuestInfo WHERE NPC='%s'",
                                    npcName.c_str());

            while (pResult->next()) {
                uint i = 0;
                EventMiniGameQuestRow row;
                readQuestHead(pResult, i, row.quest.head);
                row.quest.gameType = pResult->getInt(++i);
                row.eventQuest = pResult->getInt(++i);
                row.questLevel = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool updateEventQuestAdvance(uint status, const string& ownerName, uint questLevel) {
        bool updated = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE EventQuestAdvance SET Status=%u WHERE OwnerID='%s' AND QuestLevel=%u", status,
                                ownerName.c_str(), questLevel);
            updated = pStmt->getAffectedRowCount() != 0;
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return updated;
    }

    void insertEventQuestAdvance(uint questLevel, const string& ownerName, uint status) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "INSERT IGNORE INTO EventQuestAdvance (QuestLevel, OwnerID, Status) VALUES (%u, '%s', %u)", questLevel,
                ownerName.c_str(), status);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<EventQuestAdvanceRow> loadEventQuestAdvances(const string& ownerName) {
        vector<EventQuestAdvanceRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT QuestLevel, Status FROM EventQuestAdvance WHERE OwnerID='%s'",
                                                  ownerName.c_str());

            while (pResult->next()) {
                EventQuestAdvanceRow row;
                row.questLevel = pResult->getInt(1);
                row.status = pResult->getInt(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<EventQuestLootingRow> loadEventQuestLootingInfos() {
        vector<EventQuestLootingRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQueryString(
                "SELECT QuestLevel, LootingType-1, LootingZone, LootingMType, LootingIClass, LootingITypeMin, "
                "LootingITypeMax, Race, MinGrade, MaxGrade FROM EventQuestLootingInfo");

            while (pResult->next()) {
                uint i = 0;
                EventQuestLootingRow row;
                row.questLevel = pResult->getInt(++i);
                row.lootingType = pResult->getInt(++i);
                row.lootingZone = pResult->getInt(++i);
                row.lootingMType = pResult->getInt(++i);
                row.lootingIClass = pResult->getInt(++i);
                row.lootingITypeMin = pResult->getInt(++i);
                row.lootingITypeMax = pResult->getInt(++i);
                row.race = pResult->getInt(++i);
                row.minGrade = pResult->getInt(++i);
                row.maxGrade = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

private:
    // The six columns every quest-info SELECT starts with, in SELECT order.
    static void readQuestHead(Result* pResult, uint& i, QuestHeadRow& head) {
        head.questID = pResult->getInt(++i);
        head.race = pResult->getInt(++i);
        head.maxGrade = pResult->getInt(++i);
        head.minGrade = pResult->getInt(++i);
        head.timeLimitSec = pResult->getInt(++i);
        head.rewardClass = pResult->getInt(++i);
    }

    // The two reward tables' SELECTs differ only in the table name; both take the
    // NPC name and read the same six columns.
    vector<ItemRewardRow> loadRewards(const char* statement, const string& npcName) {
        vector<ItemRewardRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(statement, npcName.c_str());

            while (pResult->next()) {
                uint i = 0;
                ItemRewardRow row;
                row.rewardClass = pResult->getInt(++i);
                row.rewardID = pResult->getInt(++i);
                row.itemClass = pResult->getInt(++i);
                row.itemType = pResult->getInt(++i);
                row.optionType = pResult->getString(++i);
                row.timeLimitSec = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }
};

MySQLQuestInfoRepository g_QuestInfoRepository;

} // namespace

QuestInfoRepository& defaultQuestInfoRepository() {
    return g_QuestInfoRepository;
}
