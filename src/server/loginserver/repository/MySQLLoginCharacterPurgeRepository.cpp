#include "DB.h"
#include "repository/LoginCharacterPurgeRepository.h"

namespace {

// MySQL implementation of LoginCharacterPurgeRepository.
//  - loadActiveSlayerOwner, retireSlayer and purgeCharacterRows create
//    their Statement on g_pDatabaseManager->getConnection(worldID), the
//    int overload keyed by WorldID (see the header); recordDeletion and
//    destroyItems on the thread's DARKEDEN connection.
//  - purgeCharacterRows runs the Vampire and Ousters statements and then
//    kPurgeStatements in array order, all on one Statement with no
//    transaction: a failure at statement N leaves the earlier ones
//    applied. destroyItems does the same with kItemStatements.
//  - Under __CHINA_SERVER__, __THAILAND_SERVER__ or __NETMARBLE_SERVER__
//    the three race rows are DELETEd instead of set INACTIVE and the three
//    skill-save tables join the list; under __THAILAND_SERVER__ the five
//    tables from SMSItemObject to TrapItemObject leave it.
//  - The name, the account id and the slot text are interpolated raw; the
//    slot indexes Slot2String unchecked.
const char* const kPurgeStatements[] = {
#if defined(__CHINA_SERVER__) || defined(__THAILAND_SERVER__) || defined(__NETMARBLE_SERVER__)
    "DELETE FROM SkillSave WHERE OwnerID = '%s'",
    "DELETE FROM VampireSkillSave WHERE OwnerID = '%s'",
    "DELETE FROM OustersSkillSave WHERE OwnerID = '%s'",
#endif
    "DELETE FROM RankBonusData WHERE OwnerID = '%s'",
    "DELETE FROM ARObject WHERE OwnerID = '%s'",
    "DELETE FROM BeltObject WHERE OwnerID = '%s'",
    "DELETE FROM BladeObject WHERE OwnerID = '%s'",
    "DELETE FROM BloodBibleObject WHERE OwnerID = '%s'",
    "DELETE FROM BombMaterialObject WHERE OwnerID = '%s'",
    "DELETE FROM BombObject WHERE OwnerID = '%s'",
    "DELETE FROM BraceletObject WHERE OwnerID = '%s'",
    "DELETE FROM CastleSymbolObject WHERE OwnerID = '%s'",
    "DELETE FROM CoatObject WHERE OwnerID = '%s'",
    "DELETE FROM CrossObject WHERE OwnerID = '%s'",
    "DELETE FROM ETCObject WHERE OwnerID = '%s'",
    "DELETE FROM EventETCObject WHERE OwnerID = '%s'",
    "DELETE FROM EventGiftBoxObject WHERE OwnerID = '%s'",
    "DELETE FROM EventStarObject WHERE OwnerID = '%s'",
    "DELETE FROM EventTreeObject WHERE OwnerID = '%s'",
    "DELETE FROM GloveObject WHERE OwnerID = '%s'",
    "DELETE FROM HelmObject WHERE OwnerID = '%s'",
    "DELETE FROM HolyWaterObject WHERE OwnerID = '%s'",
    "DELETE FROM KeyObject WHERE OwnerID = '%s'",
    "DELETE FROM LearningItemObject WHERE OwnerID = '%s'",
    "DELETE FROM MaceObject WHERE OwnerID = '%s'",
    "DELETE FROM MagazineObject WHERE OwnerID = '%s'",
    "DELETE FROM MineObject WHERE OwnerID = '%s'",
    "DELETE FROM MoneyObject WHERE OwnerID = '%s'",
    "DELETE FROM MotorcycleObject WHERE OwnerID = '%s'",
    "DELETE FROM NecklaceObject WHERE OwnerID = '%s'",
    "DELETE FROM PotionObject WHERE OwnerID = '%s'",
    "DELETE FROM QuestItemObject WHERE OwnerID = '%s'",
    "DELETE FROM RelicObject WHERE OwnerID = '%s'",
    "DELETE FROM SGObject WHERE OwnerID = '%s'",
    "DELETE FROM SMGObject WHERE OwnerID = '%s'",
    "DELETE FROM SRObject WHERE OwnerID = '%s'",
    "DELETE FROM SerumObject WHERE OwnerID = '%s'",
    "DELETE FROM ShieldObject WHERE OwnerID = '%s'",
    "DELETE FROM ShoesObject WHERE OwnerID = '%s'",
    "DELETE FROM SkullObject WHERE OwnerID = '%s'",
    "DELETE FROM SlayerPortalItemObject WHERE OwnerID = '%s'",
    "DELETE FROM SwordObject WHERE OwnerID = '%s'",
    "DELETE FROM TrouserObject WHERE OwnerID = '%s'",
    "DELETE FROM RingObject WHERE OwnerID = '%s'",
    "DELETE FROM CoupleRingObject WHERE OwnerID = '%s'",
    "DELETE FROM VampireAmuletObject WHERE OwnerID = '%s'",
    "DELETE FROM VampireBraceletObject WHERE OwnerID = '%s'",
    "DELETE FROM VampireCoatObject WHERE OwnerID = '%s'",
    "DELETE FROM VampireETCObject WHERE OwnerID = '%s'",
    "DELETE FROM VampireEarringObject WHERE OwnerID = '%s'",
    "DELETE FROM VampireNecklaceObject WHERE OwnerID = '%s'",
    "DELETE FROM VampirePortalItemObject WHERE OwnerID = '%s'",
    "DELETE FROM VampireRingObject WHERE OwnerID = '%s'",
    "DELETE FROM VampireWeaponObject WHERE OwnerID = '%s'",
    "DELETE FROM VampireCoupleRingObject WHERE OwnerID = '%s'",
    "DELETE FROM WaterObject WHERE OwnerID = '%s'",
    "DELETE FROM EventItemObject WHERE OwnerID = '%s'",
    "DELETE FROM DyePotionObject WHERE OwnerID = '%s'",
    "DELETE FROM ResurrectItemObject WHERE OwnerID = '%s'",
    "DELETE FROM MixingItemObject WHERE OwnerID = '%s'",
    "DELETE FROM OustersArmsbandObject WHERE OwnerID = '%s'",
    "DELETE FROM OustersBootsObject WHERE OwnerID = '%s'",
    "DELETE FROM OustersChakramObject WHERE OwnerID = '%s'",
    "DELETE FROM OustersCircletObject WHERE OwnerID = '%s'",
    "DELETE FROM OustersCoatObject WHERE OwnerID = '%s'",
    "DELETE FROM OustersPendentObject WHERE OwnerID = '%s'",
    "DELETE FROM OustersRingObject WHERE OwnerID = '%s'",
    "DELETE FROM OustersStoneObject WHERE OwnerID = '%s'",
    "DELETE FROM OustersWristletObject WHERE OwnerID = '%s'",
    "DELETE FROM LarvaObject WHERE OwnerID = '%s'",
    "DELETE FROM PupaObject WHERE OwnerID = '%s'",
    "DELETE FROM ComposMeiObject WHERE OwnerID = '%s'",
    "DELETE FROM OustersSummonItemObject WHERE OwnerID = '%s'",
    "DELETE FROM EffectItemObject WHERE OwnerID = '%s'",
    "DELETE FROM CodeSheetObject WHERE OwnerID = '%s'",
    "DELETE FROM MoonCardObject WHERE OwnerID = '%s'",
    "DELETE FROM SweeperObject WHERE OwnerID = '%s'",
    "DELETE FROM PetItemObject WHERE OwnerID = '%s'",
    "DELETE FROM PetFoodObject WHERE OwnerID = '%s'",
    "DELETE FROM PetEnchantItemObject WHERE OwnerID = '%s'",
    "DELETE FROM LuckyBagObject WHERE OwnerID = '%s'",
#ifndef __THAILAND_SERVER__
    "DELETE FROM SMSItemObject WHERE OwnerID = '%s'",
    "DELETE FROM CoreZapObject WHERE OwnerID = '%s'",
    "DELETE FROM GQuestItemObject WHERE OwnerID = '%s'",
    "DELETE FROM GQuestSave WHERE OwnerID = '%s'",
    "DELETE FROM TrapItemObject WHERE OwnerID = '%s'",
#endif
    "DELETE FROM CarryingReceiverObject WHERE OwnerID = '%s'",
    "DELETE FROM ShoulderArmorObject WHERE OwnerID = '%s'",
    "DELETE FROM DermisObject WHERE OwnerID = '%s'",
    "DELETE FROM PersonaObject WHERE OwnerID = '%s'",
    "DELETE FROM FasciaObject WHERE OwnerID = '%s'",
    "DELETE FROM MittenObject WHERE OwnerID = '%s'",
    "DELETE FROM CoupleInfo WHERE FemalePartnerName='%s'",
    "DELETE FROM CoupleInfo WHERE MalePartnerName='%s'",
    "DELETE FROM EffectAcidTouch where OwnerID='%s'",
    "DELETE FROM EffectAftermath where OwnerID='%s'",
    "DELETE FROM EffectBloodDrain where OwnerID='%s'",
    "DELETE FROM EffectDetectHidden where OwnerID='%s'",
    "DELETE FROM EffectFlare where OwnerID='%s'",
    "DELETE FROM EffectLight where OwnerID='%s'",
    "DELETE FROM EffectParalysis where OwnerID='%s'",
    "DELETE FROM EffectPoison where OwnerID='%s'",
    "DELETE FROM EffectPoisonousHands where OwnerID='%s'",
    "DELETE FROM EffectProtectionFromParalysis where OwnerID='%s'",
    "DELETE FROM EffectProtectionFromPoison where OwnerID='%s'",
    "DELETE FROM EffectRestore where OwnerID='%s'",
    "DELETE FROM EffectYellowPoisonToCreature where OwnerID='%s'",
    "DELETE FROM EffectMute where OwnerID='%s'",
    "DELETE FROM EnemyErase where OwnerID='%s'",
    "DELETE FROM FlagSet WHERE OwnerID='%s'",
    "DELETE FROM TimeLimitItems WHERE OwnerID='%s'",
    "DELETE FROM EventQuestAdvance WHERE OwnerID='%s'",
    "DELETE FROM MofusPowerPoint WHERE OwnerID='%s'",
};

// ItemDestroyer's list. MaceObject appears twice.
const char* const kItemStatements[] = {
    "DELETE FROM MotorcycleObject WHERE OwnerID = '%s'",
    "DELETE FROM PotionObject WHERE OwnerID = '%s'",
    "DELETE FROM WaterObject WHERE OwnerID = '%s'",
    "DELETE FROM HolyWaterObject WHERE OwnerID = '%s'",
    "DELETE FROM MagazineObject WHERE OwnerID = '%s'",
    "DELETE FROM BombMaterialObject WHERE OwnerID = '%s'",
    "DELETE FROM ETCObject WHERE OwnerID = '%s'",
    "DELETE FROM KeyObject WHERE OwnerID = '%s'",
    "DELETE FROM RingObject WHERE OwnerID = '%s'",
    "DELETE FROM BraceletObject WHERE OwnerID = '%s'",
    "DELETE FROM NecklaceObject WHERE OwnerID = '%s'",
    "DELETE FROM CoatObject WHERE OwnerID = '%s'",
    "DELETE FROM TrouserObject WHERE OwnerID = '%s'",
    "DELETE FROM ShoesObject WHERE OwnerID = '%s'",
    "DELETE FROM SwordObject WHERE OwnerID = '%s'",
    "DELETE FROM BladeObject WHERE OwnerID = '%s'",
    "DELETE FROM ShieldObject WHERE OwnerID = '%s'",
    "DELETE FROM CrossObject WHERE OwnerID = '%s'",
    "DELETE FROM MaceObject WHERE OwnerID = '%s'",
    "DELETE FROM GloveObject WHERE OwnerID = '%s'",
    "DELETE FROM HelmObject WHERE OwnerID = '%s'",
    "DELETE FROM SGObject WHERE OwnerID = '%s'",
    "DELETE FROM SMGObject WHERE OwnerID = '%s'",
    "DELETE FROM ARObject WHERE OwnerID = '%s'",
    "DELETE FROM SRObject WHERE OwnerID = '%s'",
    "DELETE FROM BombObject WHERE OwnerID = '%s'",
    "DELETE FROM MineObject WHERE OwnerID = '%s'",
    "DELETE FROM BeltObject WHERE OwnerID = '%s'",
    "DELETE FROM LearningItemObject WHERE OwnerID = '%s'",
    "DELETE FROM VampireRingObject WHERE OwnerID = '%s'",
    "DELETE FROM VampireBraceletObject WHERE OwnerID = '%s'",
    "DELETE FROM VampireNecklaceObject WHERE OwnerID = '%s'",
    "DELETE FROM VampireCoatObject WHERE OwnerID = '%s'",
    "DELETE FROM SkullObject WHERE OwnerID = '%s'",
    "DELETE FROM MaceObject WHERE OwnerID = '%s'",
    "DELETE FROM SerumObject WHERE OwnerID = '%s'",
    "DELETE FROM VampireETCObject WHERE OwnerID = '%s'",
    "DELETE FROM SlayerPortalItemObject WHERE OwnerID = '%s'",
    "DELETE FROM VampirePortalItemObject WHERE OwnerID = '%s'",
    "DELETE FROM EventGiftBoxObject WHERE OwnerID = '%s'",
    "DELETE FROM EventStarObject WHERE OwnerID = '%s'",
};

class MySQLLoginCharacterPurgeRepository : public LoginCharacterPurgeRepository {
public:
    bool loadActiveSlayerOwner(WorldID_t worldID, const string& name, string& playerID) {
        bool found = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
            Result* pResult =
                pStmt->executeQuery("SELECT PlayerID FROM Slayer WHERE Name = '%s' AND Active='ACTIVE'", name.c_str());

            if (pResult->getRowCount() == 1 && pResult->next()) {
                playerID = pResult->getString(1);
                found = true;
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return found;
    }

    bool retireSlayer(WorldID_t worldID, const string& name, Slot slot) {
        bool affected = false;
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();
#if defined(__CHINA_SERVER__) || defined(__THAILAND_SERVER__) || defined(__NETMARBLE_SERVER__)
            pStmt->executeQuery("DELETE FROM Slayer WHERE Name = '%s' AND Slot = '%s'", name.c_str(),
                                Slot2String[slot].c_str());
#else
            pStmt->executeQuery("UPDATE Slayer SET Active='INACTIVE' WHERE Name = '%s' AND Slot = '%s'", name.c_str(),
                                Slot2String[slot].c_str());
#endif

            affected = pStmt->getAffectedRowCount() == 1;

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)

        return affected;
    }

    void recordDeletion(const string& playerID, WorldID_t worldID, const string& name) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
            pStmt->executeQuery("INSERT INTO DeleteChar (PlayerID, WorldID, Name, delDate) VALUES ('%s',%u,'%s',now())",
                                playerID.c_str(), worldID, name.c_str());

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void purgeCharacterRows(WorldID_t worldID, const string& name, Slot slot) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection(worldID)->createStatement();

#if defined(__CHINA_SERVER__) || defined(__THAILAND_SERVER__) || defined(__NETMARBLE_SERVER__)
            pStmt->executeQuery("DELETE FROM Vampire WHERE Name = '%s' AND Slot = '%s'", name.c_str(),
                                Slot2String[slot].c_str());
#else
            pStmt->executeQuery("UPDATE Vampire SET Active='INACTIVE' WHERE Name = '%s' AND Slot = '%s'", name.c_str(),
                                Slot2String[slot].c_str());
#endif

#if defined(__CHINA_SERVER__) || defined(__THAILAND_SERVER__) || defined(__NETMARBLE_SERVER__)
            pStmt->executeQuery("DELETE FROM Ousters WHERE Name = '%s' AND Slot = '%s'", name.c_str(),
                                Slot2String[slot].c_str());
#else
            pStmt->executeQuery("UPDATE Ousters SET Active='INACTIVE' WHERE Name = '%s' AND Slot = '%s'", name.c_str(),
                                Slot2String[slot].c_str());
#endif

            for (size_t i = 0; i < sizeof(kPurgeStatements) / sizeof(kPurgeStatements[0]); i++) {
                pStmt->executeQuery(kPurgeStatements[i], name.c_str());
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }

    void destroyItems(const string& ownerID) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            for (size_t i = 0; i < sizeof(kItemStatements) / sizeof(kItemStatements[0]); i++) {
                pStmt->executeQuery(kItemStatements[i], ownerID.c_str());
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

LoginCharacterPurgeRepository& defaultLoginCharacterPurgeRepository() {
    static MySQLLoginCharacterPurgeRepository instance;
    return instance;
}
