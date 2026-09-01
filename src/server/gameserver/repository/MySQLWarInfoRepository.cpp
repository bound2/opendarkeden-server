#include "DB.h"
#include "repository/WarInfoRepository.h"

namespace {

// MySQL implementation of the race-war seam. The legacy quirks are
// quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every statement is byte-for-byte the inline original: the castle
//    save's "GuildID=%d" unspaced list vs the sweeper writes'
//    "OwnerRace = %d" spaced ones; the SweeperOwnerInfo UPDATE's %ld
//    for an int OwnerRace (the 4-byte-through-8-byte conversion family
//    documented in MySQLCharacterRepository.cpp — register-passed here,
//    so GCC's sign-extension of the int keeps it benign) and %d for a
//    uint SweeperType; the master-lair SELECT that names 25 columns.
//  - CastleInfoManager::tinysave applies a caller-composed SET fragment
//    verbatim — the same quarantine as CharacterRepository::tinysave;
//    the fragment is raw SQL text built by the castle handlers.
//  - The MAX(Type) probe on SweeperBonusInfo returns false on the NULL
//    an empty table yields (see MySQLBalanceInfoRepository.cpp); both
//    SweeperBonusManager entry points re-run it before their rows read,
//    as the originals did.
//  - LevelWarHistory is keyless; its INSERT writes the "Old" sweeper
//    columns at war start and the UPDATE fills the "new" ones at war
//    end, keyed on (Level, LevelWarID) — a start time formatted as text
//    by the caller. A restart between the two leaves a half row.
//  - SweeperOwnerInfo's UPDATE keys on SweeperType alone (the table's
//    PK); the reads filter by ZoneID.
//  - Names and fragments are interpolated raw, as before.
class MySQLWarInfoRepository : public WarInfoRepository {
public:
    vector<ShrineRow> loadShrines() {
        vector<ShrineRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT ID, Name, ItemType, SlayerGuardZoneID, SlayerGuardX, SlayerGuardY, SlayerGuardMType, "
                "VampireGuardZoneID, VampireGuardX, VampireGuardY, VampireGuardMType, OustersGuardZoneID, "
                "OustersGuardX, "
                "OustersGuardY, OustersGuardMType, HolyZoneID, HolyX, HolyY, HolyMType, OwnerRace FROM ShrineInfo");

            while (pResult->next()) {
                int i = 0;
                ShrineRow row;
                row.id = pResult->getInt(++i);
                row.name = pResult->getString(++i);
                row.itemType = pResult->getInt(++i);
                row.slayerGuardZoneID = pResult->getInt(++i);
                row.slayerGuardX = pResult->getInt(++i);
                row.slayerGuardY = pResult->getInt(++i);
                row.slayerGuardMonsterType = pResult->getInt(++i);
                row.vampireGuardZoneID = pResult->getInt(++i);
                row.vampireGuardX = pResult->getInt(++i);
                row.vampireGuardY = pResult->getInt(++i);
                row.vampireGuardMonsterType = pResult->getInt(++i);
                row.oustersGuardZoneID = pResult->getInt(++i);
                row.oustersGuardX = pResult->getInt(++i);
                row.oustersGuardY = pResult->getInt(++i);
                row.oustersGuardMonsterType = pResult->getInt(++i);
                row.holyZoneID = pResult->getInt(++i);
                row.holyX = pResult->getInt(++i);
                row.holyY = pResult->getInt(++i);
                row.holyMonsterType = pResult->getInt(++i);
                row.ownerRace = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<ShrineOwnerRow> loadShrineOwners() {
        vector<ShrineOwnerRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT ID, OwnerRace FROM ShrineInfo");

            while (pResult->next()) {
                int i = 0;
                ShrineOwnerRow row;
                row.id = pResult->getInt(++i);
                row.ownerRace = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void saveShrineOwner(int ownerRace, int shrineID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE ShrineInfo SET OwnerRace=%d WHERE ID=%d", ownerRace, shrineID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<CastleRow> loadCastles(int serverID) {
        vector<CastleRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT ZoneID, ShrineID, GuildID, Name, Race, ItemTaxRatio, EntranceFee, TaxBalance, "
                "BonusOptionType, FirstResurrectZoneID, FirstResurrectX, FirstResurrectY, "
                "SecondResurrectZoneID, SecondResurrectX, SecondResurrectY, ThirdResurrectZoneID, "
                "ThirdResurrectX, ThirdResurrectY, ZoneIDList FROM CastleInfo WHERE ServerID = %d",
                serverID);

            while (pResult->next()) {
                uint i = 0;
                CastleRow row;
                row.zoneID = pResult->getInt(++i);
                row.shrineID = pResult->getInt(++i);
                row.guildID = pResult->getInt(++i);
                row.name = pResult->getString(++i);
                row.race = pResult->getInt(++i);
                row.itemTaxRatio = pResult->getInt(++i);
                row.entranceFee = pResult->getInt(++i);
                row.taxBalance = pResult->getInt(++i);
                row.bonusOptionType = pResult->getString(++i);
                row.firstResurrectZoneID = pResult->getInt(++i);
                row.firstResurrectX = pResult->getInt(++i);
                row.firstResurrectY = pResult->getInt(++i);
                row.secondResurrectZoneID = pResult->getInt(++i);
                row.secondResurrectX = pResult->getInt(++i);
                row.secondResurrectY = pResult->getInt(++i);
                row.thirdResurrectZoneID = pResult->getInt(++i);
                row.thirdResurrectX = pResult->getInt(++i);
                row.thirdResurrectY = pResult->getInt(++i);
                row.zoneIDList = pResult->getString(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void saveCastle(int serverID, int zoneID, const CastleStateRecord& record) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "UPDATE CastleInfo SET GuildID=%d, Name='%s', Race=%d, ItemTaxRatio=%d, EntranceFee=%d, "
                "TaxBalance=%d WHERE ServerID=%d AND ZoneID=%d",
                record.guildID, record.name.c_str(), record.race, record.itemTaxRatio, record.entranceFee,
                record.taxBalance, serverID, zoneID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool tinysaveCastle(const string& fieldFragment, ZoneID_t zoneID, int serverID) {
        bool isAffected = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE CastleInfo SET %s WHERE ZoneID=%d AND ServerID=%d", fieldFragment.c_str(),
                                zoneID, serverID);

            if (pStmt->getAffectedRowCount() > 0)
                isAffected = true;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return isAffected;
    }

    bool loadMaxSweeperBonusType(int& maxType) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT MAX(Type) FROM SweeperBonusInfo");

            if (pResult->next()) {
                const char* field = pResult->getField(1);
                if (field != NULL) {
                    maxType = atoi(field);
                    found = true;
                }
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    vector<SweeperBonusRow> loadSweeperBonuses() {
        vector<SweeperBonusRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT Type, Name, OptionList, OwnerRace, Level FROM SweeperBonusInfo");

            while (pResult->next()) {
                int i = 0;
                SweeperBonusRow row;
                row.type = pResult->getInt(++i);
                row.name = pResult->getString(++i);
                row.optionList = pResult->getString(++i);
                row.ownerRace = pResult->getInt(++i);
                row.level = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<SweeperBonusOwnerRow> loadSweeperBonusOwners(int level) {
        vector<SweeperBonusOwnerRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT Type, OwnerRace FROM SweeperBonusInfo WHERE Level = %d", level);

            while (pResult->next()) {
                int i = 0;
                SweeperBonusOwnerRow row;
                row.type = pResult->getInt(++i);
                row.ownerRace = pResult->getInt(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void saveSweeperBonusOwner(Race_t ownerRace, SweeperBonusType_t type) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE SweeperBonusInfo SET OwnerRace = %d WHERE Type = %d", ownerRace, type);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<SweeperSetRow> loadSweeperSets(ZoneID_t zoneID) {
        vector<SweeperSetRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT ItemType, "
                                                  "SlayerX, SlayerY, SlayerMType, "
                                                  "VampireX, VampireY, VampireMType, "
                                                  "OustersX, OustersY, OustersMType, "
                                                  "DefaultX, DefaultY, DefaultMType, "
                                                  "Name "
                                                  "FROM SweeperSetInfo WHERE ZoneID = %d",
                                                  zoneID);

            while (pResult->next()) {
                SweeperSetRow row;
                row.itemType = pResult->getInt(1);
                row.slayerX = pResult->getInt(2);
                row.slayerY = pResult->getInt(3);
                row.slayerMonsterType = pResult->getInt(4);
                row.vampireX = pResult->getInt(5);
                row.vampireY = pResult->getInt(6);
                row.vampireMonsterType = pResult->getInt(7);
                row.oustersX = pResult->getInt(8);
                row.oustersY = pResult->getInt(9);
                row.oustersMonsterType = pResult->getInt(10);
                row.defaultX = pResult->getInt(11);
                row.defaultY = pResult->getInt(12);
                row.defaultMonsterType = pResult->getInt(13);
                row.name = pResult->getString(14);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<SweeperOwnerRow> loadSweeperOwners(ZoneID_t zoneID) {
        vector<SweeperOwnerRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT SweeperType, OwnerRace, SweeperSafeType FROM SweeperOwnerInfo WHERE ZoneID = %d", zoneID);

            while (pResult->next()) {
                SweeperOwnerRow row;
                row.sweeperType = pResult->getInt(1);
                row.ownerRace = pResult->getInt(2);
                row.sweeperSafeType = pResult->getInt(3);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    vector<SweeperBonusOwnerRow> loadSweeperOwnerRaces(ZoneID_t zoneID) {
        vector<SweeperBonusOwnerRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT SweeperType, OwnerRace FROM SweeperOwnerInfo WHERE ZoneID = %d", zoneID);

            while (pResult->next()) {
                SweeperBonusOwnerRow row;
                row.type = pResult->getInt(1);
                row.ownerRace = pResult->getInt(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void saveSweeperOwner(int ownerRace, int safeType, uint itemType) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "UPDATE SweeperOwnerInfo SET OwnerRace = %ld, SweeperSafeType = %d WHERE SweeperType = %d", ownerRace,
                safeType, itemType);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertLevelWarHistory(int level, const string& levelWarID, const string& slayerOld, const string& vampireOld,
                               const string& oustersOld, const string& defaultOld) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO LevelWarHistory (Level, LevelWarID, SlayerOldSweeper, VampireOldSweeper, "
                                "OustersOldSweeper, DefaultOldSweeper) VALUES (%d, '%s', '%s', '%s', '%s', '%s')",
                                level, levelWarID.c_str(), slayerOld.c_str(), vampireOld.c_str(), oustersOld.c_str(),
                                defaultOld.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateLevelWarHistory(const string& slayerNew, const string& vampireNew, const string& oustersNew,
                               const string& defaultNew, int level, const string& levelWarID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "UPDATE LevelWarHistory SET SlayerSweeper = '%s', VampireSweeper = '%s', OustersSweeper = "
                "'%s', DefaultSweeper = '%s' WHERE Level = %d AND LevelWarID = '%s'",
                slayerNew.c_str(), vampireNew.c_str(), oustersNew.c_str(), defaultNew.c_str(), level,
                levelWarID.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<MasterLairRow> loadMasterLairs() {
        vector<MasterLairRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT ZoneID, MasterNotReadyMonsterType, MasterMonsterType, MasterRemainNotReady, MasterX, MasterY, "
                "MasterDir, MaxPassPlayer, SummonX, SummonY, FirstRegenDelay, RegenDelay, StartDelay, EndDelay, "
                "KickOutDelay, KickZoneID, KickZoneX, KickZoneY, LairAttackTick, LairAttackMinNumber, "
                "LairAttackMaxNumber, "
                "MasterSummonSay, MasterDeadSlayerSay, MasterDeadVampireSay, MasterNotDeadSay FROM MasterLairInfo");

            while (pResult->next()) {
                uint i = 0;
                MasterLairRow row;
                row.zoneID = pResult->getInt(++i);
                row.masterNotReadyMonsterType = pResult->getInt(++i);
                row.masterMonsterType = pResult->getInt(++i);
                row.masterRemainNotReady = pResult->getInt(++i);
                row.masterX = pResult->getInt(++i);
                row.masterY = pResult->getInt(++i);
                row.masterDir = pResult->getInt(++i);
                row.maxPassPlayer = pResult->getInt(++i);
                row.summonX = pResult->getInt(++i);
                row.summonY = pResult->getInt(++i);
                row.firstRegenDelay = pResult->getInt(++i);
                row.regenDelay = pResult->getInt(++i);
                row.startDelay = pResult->getInt(++i);
                row.endDelay = pResult->getInt(++i);
                row.kickOutDelay = pResult->getInt(++i);
                row.kickZoneID = pResult->getInt(++i);
                row.kickZoneX = pResult->getInt(++i);
                row.kickZoneY = pResult->getInt(++i);
                row.lairAttackTick = pResult->getInt(++i);
                row.lairAttackMinNumber = pResult->getInt(++i);
                row.lairAttackMaxNumber = pResult->getInt(++i);
                row.masterSummonSay = pResult->getString(++i);
                row.masterDeadSlayerSay = pResult->getString(++i);
                row.masterDeadVampireSay = pResult->getString(++i);
                row.masterNotDeadSay = pResult->getString(++i);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }
};

} // namespace

WarInfoRepository& defaultWarInfoRepository() {
    static MySQLWarInfoRepository instance;
    return instance;
}
