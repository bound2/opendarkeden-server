#include "DB.h"
#include "repository/WarInfoRepository.h"

namespace {

// MySQL implementation of the race-war seam. The legacy quirks are
// quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - Every statement is byte-for-byte the inline original: the castle
//    save's "GuildID=%d" unspaced list vs the sweeper writes'
//    "OwnerRace = %d" spaced ones; the SweeperOwnerInfo UPDATE's %ld
//    for an int OwnerRace (the 4-byte-through-8-byte conversion family
//    documented in MySQLCharacterRepository.cpp; the ABI leaves the high
//    half of a register-passed int undefined, so "benign" is codegen,
//    not contract — the value is 0..3 and the literal is the original's,
//    so behaviour is unchanged either way) and %d for a uint
//    SweeperType; the master-lair SELECT that names 25 columns.
//  - CastleInfoManager::tinysave applies a caller-composed SET fragment
//    verbatim — the same quarantine as CharacterRepository::tinysave;
//    the fragment is raw SQL text built by the castle handlers.
//  - The MAX(Type) probe on SweeperBonusInfo returns false on the NULL
//    an empty table yields (see MySQLBalanceInfoRepository.cpp); both
//    SweeperBonusManager entry points re-run it before their rows read,
//    as the originals did.
//  - LevelWarHistory has no primary or unique key (two non-unique
//    indexes only); its INSERT writes the "Old" sweeper
//    columns at war start and the UPDATE fills the "new" ones at war
//    end, keyed on (Level, LevelWarID) — a start time formatted as text
//    by the caller. A restart between the two leaves a half row.
//  - SweeperOwnerInfo's UPDATE keys on SweeperType alone (the table's
//    PK); the reads filter by ZoneID.
//  - The war histories divide the same way LevelWarHistory does: a row
//    written at war start and filled in at war end. GuildWarHistory's
//    start is an INSERT IGNORE keyed on its WarID and its end updates
//    WHERE WarID; RaceWarHistory's start is a PLAIN INSERT and its end
//    updates WHERE RaceWarID — a start time the caller formatted as
//    text — so a repeated start leaves a second row there where the
//    guild war drops it, and the update then rewrites both. Preserved.
//  - The RaceWarPCLimit totals arrive through getInt on a SUM() column;
//    the caller assigned them to uints, and still does.
//  - The reinforcement statements pass the config's int ServerID and a
//    WORD GuildID through "%u", and the DWORD WarID through "%u" as
//    well: the originals' conversions, kept.
//  - SiegeWar's six statements asked for the connection by the name
//    "Darkeden" where every other site writes "DARKEDEN".
//    DatabaseManager::getConnection(const string&) never looks at the
//    name — it keys the lookup on Thread::self() and falls back to the
//    default connection — so the two spellings selected the same
//    socket; the seam writes "DARKEDEN" like its neighbours.
//  - WarSchedule's INSERT IGNORE and REPLACE were written with a
//    backslash-continued source line, which splices the next line's four
//    leading TABS into the literal right before "VALUES". They are kept:
//    the seam writes them as an explicit "\t\t\t\t" so clang-format
//    cannot reflow them away, and the bytes MySQL receives are the same.
//  - War::initWarIDRegistry called next() on both probes without
//    checking it and read column 1 through getDWORD. Kept: a COUNT(*)
//    always answers with one row, and the MAX probe runs only after the
//    count came back non-zero, so neither read can meet an empty result.
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

    vector<WarScheduleRow> loadWarSchedules(int serverID, int zoneID) {
        vector<WarScheduleRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
#ifndef __OLD_GUILD_WAR__
                "SELECT WarID, WarType, AttackerCount, AttackGuildID, AttackGuildID2, AttackGuildID3, AttackGuildID4, "
                "AttackGuildID5, "
                "WarFee, StartTime FROM WarScheduleInfo "
#else
                "SELECT WarID, WarType, AttackGuildID, WarFee, StartTime FROM WarScheduleInfo "
#endif
                "WHERE ServerID = %u AND ZoneID = %u AND ( Status = 'WAIT' OR Status = 'START' ) "
                "ORDER BY StartTime",
                serverID, zoneID);

            while (pResult->next()) {
                WarScheduleRow row;
                int i = 0;

                row.warID = pResult->getInt(++i);
                row.warType = pResult->getString(++i);
#ifndef __OLD_GUILD_WAR__
                row.attackerCount = pResult->getInt(++i);

                for (int j = 0; j < 5; ++j) {
                    row.attackGuildID[j] = pResult->getInt(++i);
                }
#else
                row.attackGuildID = pResult->getInt(++i);
#endif
                row.warFee = pResult->getInt(++i);
                row.startTime = pResult->getString(++i);

                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    bool loadAcceptedReinforceGuild(WarID_t warID, int& guildID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT ReinforceGuildID FROM ReinforceRegisterInfo WHERE WarID=%u AND Status='ACCEPT'", warID);

            if (pResult->next()) {
                guildID = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void cancelGuildWarSchedules(int serverID, int zoneID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE WarScheduleInfo SET Status='CANCEL' WHERE ServerID = %d AND ZoneID = %d "
                                "\t\t\t\tAND WarType='GUILD' AND (Status='WAIT' OR Status='START')",
                                serverID, zoneID);

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

    int countWarSchedules() {
        int count = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT COUNT(*) from WarScheduleInfo");
            pResult->next();
            count = pResult->getDWORD(1);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return count;
    }

    DWORD loadMaxWarID() {
        DWORD maxWarID = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT MAX(WarID) FROM WarScheduleInfo");
            pResult->next();
            maxWarID = pResult->getDWORD(1);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return maxWarID;
    }

    bool insertWarSchedule(int warID, int serverID, int zoneID, const string& warType, int attackGuildID, int warFee,
                           const string& startTime, const string& status) {
        bool changed = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT IGNORE INTO WarScheduleInfo ( WarID, ServerID, ZoneID, WarType, "
                                "AttackGuildID, WarFee, StartTime, Status ) "
                                "\t\t\t\tVALUES ( %u, %u, %u, '%s', %u, %u, '%s', '%s' )",
                                warID, serverID, zoneID, warType.c_str(), attackGuildID, warFee, startTime.c_str(),
                                status.c_str());

            changed = pStmt->getAffectedRowCount() > 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return changed;
    }

    bool replaceWarSchedule(int warID, int serverID, int zoneID, const string& warType, int attackerCount,
                            int attackGuildID, int attackGuildID2, int attackGuildID3, int attackGuildID4,
                            int attackGuildID5, int warFee, const string& startTime, const string& status) {
        bool changed = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("REPLACE INTO WarScheduleInfo ( WarID, ServerID, ZoneID, WarType, AttackerCount, "
                                "AttackGuildID, AttackGuildID2, AttackGuildID3, AttackGuildID4, AttackGuildID5, "
                                "WarFee, StartTime, Status ) "
                                "\t\t\t\tVALUES ( %u, %u, %u, '%s', %u, %u, %u, %u, %u, %u, %u, '%s', '%s' )",
                                warID, serverID, zoneID, warType.c_str(), attackerCount, attackGuildID, attackGuildID2,
                                attackGuildID3, attackGuildID4, attackGuildID5, warFee, startTime.c_str(),
                                status.c_str());

            changed = pStmt->getAffectedRowCount() > 0;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return changed;
    }

    void tinysaveWarSchedule(const string& fieldFragment, WarID_t warID, int serverID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE WarScheduleInfo SET %s WHERE WarID = %d AND ServerID = %d",
                                fieldFragment.c_str(), warID, serverID);
        }
        END_DB(pStmt)

        SAFE_DELETE(pStmt);
    }

    void insertGuildWarHistory(int warID, const string& guildWarID, int serverID, const string& castleName,
                               int defenseGuildID, const string& defenseGuildName, int attackGuildID,
                               const string& attackGuildName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "INSERT IGNORE INTO GuildWarHistory (WarID, GuildWarID, ServerID, CastleName, DefenseGuildID, "
                "DefenseGuildName, AttackGuildID, AttackGuildName) VALUES (%d, '%s', %d, '%s', %d, '%s', %d, '%s')",
                warID, guildWarID.c_str(), serverID, castleName.c_str(), defenseGuildID, defenseGuildName.c_str(),
                attackGuildID, attackGuildName.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateGuildWarWinner(int winnerGuildID, const string& winnerGuildName, int warID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "UPDATE GuildWarHistory SET WinnerGuildID = %d , WinnerGuildName = '%s' WHERE WarID = %d",
                winnerGuildID, winnerGuildName.c_str(), warID);
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<RaceCurrentNumRow> loadRaceWarCurrentNums() {
        vector<RaceCurrentNumRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT Race, SUM(CurrentNum) FROM RaceWarPCLimit GROUP BY Race");

            while (pResult->next()) {
                RaceCurrentNumRow row;
                row.race = pResult->getInt(1);
                row.currentNum = pResult->getInt(2);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void insertRaceWarHistory(const string& raceWarID, uint slayerNum, uint vampireNum, uint oustersNum,
                              const string& slayerOld, const string& vampireOld, const string& oustersOld) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "INSERT INTO RaceWarHistory (RaceWarID, SlayerNum, VampireNum, OustersNum, SlayerOldBloodBible, "
                "VampireOldBloodBible, OustersOldBloodBible) VALUES ('%s', %d, %d, %d, '%s', '%s', '%s')",
                raceWarID.c_str(), slayerNum, vampireNum, oustersNum, slayerOld.c_str(), vampireOld.c_str(),
                oustersOld.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void updateRaceWarBloodBibles(const string& slayerNew, const string& vampireNew, const string& oustersNew,
                                  const string& raceWarID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE RaceWarHistory SET SlayerBloodBible = '%s', VampireBloodBible = '%s', "
                                "OustersBloodBible = '%s' WHERE RaceWarID = '%s'",
                                slayerNew.c_str(), vampireNew.c_str(), oustersNew.c_str(), raceWarID.c_str());
            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<RaceWarLimitRow> loadRaceWarLimits(const string& tableName, int race) {
        vector<RaceWarLimitRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT ID, MinLevel, MaxLevel, LimitNum, CurrentNum FROM %s WHERE Race=%d", tableName.c_str(), race);

            while (pResult->next()) {
                RaceWarLimitRow row;
                row.id = pResult->getInt(1);
                row.minLevel = pResult->getInt(2);
                row.maxLevel = pResult->getInt(3);
                row.limitNum = pResult->getInt(4);
                row.currentNum = pResult->getInt(5);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void clearRaceWarCurrentNums(const string& tableName) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE %s SET CurrentNum=0", tableName.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void saveRaceWarCurrentNum(const string& tableName, int currentNum, int id) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE %s SET CurrentNum=%d WHERE ID=%d", tableName.c_str(), currentNum, id);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    vector<RaceWarPCListRow> loadRaceWarPCList() {
        vector<RaceWarPCListRow> rows;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQueryString("SELECT Name, Race FROM RaceWarPCList");

            while (pResult->next()) {
                RaceWarPCListRow row;
                row.name = pResult->getString(1);
                // Column 1, not 2 — the inline loop's own indexing. See
                // WarInfoRepository.h.
                row.race = pResult->getInt(1);
                rows.push_back(row);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return rows;
    }

    void deleteRaceWarPCList() {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQueryString("DELETE FROM RaceWarPCList");

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void insertRaceWarPCListEntry(const string& name, int race) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT IGNORE INTO RaceWarPCList (Name, Race) VALUES ('%s', %d)", name.c_str(), race);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    int countRaceWarPCListEntries(const string& name) {
        int count = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery("SELECT count(*) FROM RaceWarPCList WHERE Name='%s'", name.c_str());

            if (pResult->next()) {
                count = pResult->getInt(1);
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return count;
    }

    void deleteRaceWarPCListEntry(const string& name) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM RaceWarPCList WHERE Name='%s'", name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    int countWaitingReinforceRegistrations(WarID_t warID, int serverID) {
        int count = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT COUNT(*) FROM ReinforceRegisterInfo WHERE WarID=%u AND ServerID=%u AND Status='WAIT'", warID,
                serverID);

            if (pResult->next())
                count = pResult->getInt(1);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return count;
    }

    int countDeniedReinforceRegistrations(WarID_t warID, int serverID, GuildID_t guildID) {
        int count = 0;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT COUNT(*) FROM ReinforceRegisterInfo WHERE WarID=%u AND ServerID=%u AND "
                                    "ReinforceGuildID=%u AND Status='DENY'",
                                    warID, serverID, guildID);

            if (pResult->next())
                count = pResult->getInt(1);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return count;
    }

    bool loadWaitingReinforceGuild(WarID_t warID, int serverID, GuildID_t& guildID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            Result* pResult = pStmt->executeQuery(
                "SELECT ReinforceGuildID FROM ReinforceRegisterInfo WHERE WarID=%u AND ServerID=%u AND Status='WAIT'",
                warID, serverID);

            if (pResult->next()) {
                guildID = pResult->getInt(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    void insertReinforceRegistration(WarID_t warID, int serverID, GuildID_t guildID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO ReinforceRegisterInfo (WarID, ServerID, ReinforceGuildID, Status) VALUES "
                                "(%u, %u, %u, 'WAIT')",
                                warID, serverID, guildID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    bool acceptReinforceRegistration(WarID_t warID, int serverID, GuildID_t guildID) {
        bool ret = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("UPDATE ReinforceRegisterInfo SET Status='ACCEPT' WHERE WarID=%u AND ServerID=%u AND "
                                "ReinforceGuildID=%u",
                                warID, serverID, guildID);

            if (pStmt->getAffectedRowCount() > 0)
                ret = true;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return ret;
    }

    bool denyReinforceRegistration(WarID_t warID, int serverID, GuildID_t guildID) {
        bool ret = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery(
                "UPDATE ReinforceRegisterInfo SET Status='DENY' WHERE WarID=%u AND ServerID=%u AND ReinforceGuildID=%u",
                warID, serverID, guildID);

            if (pStmt->getAffectedRowCount() > 0)
                ret = true;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return ret;
    }

    void deleteReinforceRegistrations(WarID_t warID, int serverID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("DELETE FROM ReinforceRegisterInfo WHERE WarID=%u AND ServerID=%u", warID, serverID);

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

WarInfoRepository& defaultWarInfoRepository() {
    static MySQLWarInfoRepository instance;
    return instance;
}
