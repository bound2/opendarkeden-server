#include "DB.h"
#include "repository/CharacterPurgeRepository.h"

namespace {

// MySQL implementation of the character-purge seam. The legacy quirks are
// quarantined HERE, per docs/RESTRUCTURING.md 3.2:
//  - kPurgeStatements is deletePC's list in deletePC's order, every
//    literal byte for byte (generated from the original text, not
//    retyped): the Active updates keep "Name = '%s'" with spaces, the
//    four bookkeeping deletes before the objects keep "OwnerID = '%s'"
//    with spaces, the 81 object deletes and GQuestSave keep the same
//    spaced form their concatenation produced, the sixteen effect deletes
//    keep their lower-case "where OwnerID='%s'", and the last three keep
//    "WHERE OwnerID='%s'".
//  - One Statement for the whole list, as before; no transaction, as
//    before — a failure at statement N leaves 1..N-1 applied.
//  - The name is interpolated raw into every statement, as before.
const char* const kPurgeStatements[] = {
    "UPDATE Slayer SET Active='INACTIVE' WHERE Name = '%s'",
    "UPDATE Vampire SET Active='INACTIVE' WHERE Name = '%s'",
    "UPDATE Ousters SET Active='INACTIVE' WHERE Name = '%s'",
    "DELETE FROM SkillSave WHERE OwnerID = '%s'",
    "DELETE FROM VampireSkillSave WHERE OwnerID = '%s'",
    "DELETE FROM OustersSkillSave WHERE OwnerID = '%s'",
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
    "DELETE FROM SMSItemObject WHERE OwnerID = '%s'",
    "DELETE FROM CoreZapObject WHERE OwnerID = '%s'",
    "DELETE FROM GQuestItemObject WHERE OwnerID = '%s'",
    "DELETE FROM GQuestSave WHERE OwnerID = '%s'",
    "DELETE FROM TrapItemObject WHERE OwnerID = '%s'",
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
};

class MySQLCharacterPurgeRepository : public CharacterPurgeRepository {
public:
    void purgeCharacter(const string& name) {
        Statement* pStmt = NULL;

        BEGIN_DB {
            pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();

            for (size_t i = 0; i < sizeof(kPurgeStatements) / sizeof(kPurgeStatements[0]); i++) {
                pStmt->executeQuery(kPurgeStatements[i], name.c_str());
            }

            SAFE_DELETE(pStmt);
        }
        END_DB(pStmt)
    }
};

} // namespace

CharacterPurgeRepository& defaultCharacterPurgeRepository() {
    static MySQLCharacterPurgeRepository instance;
    return instance;
}
