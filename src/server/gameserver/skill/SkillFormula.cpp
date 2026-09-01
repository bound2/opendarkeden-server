//////////////////////////////////////////////////////////////////////////////
// Filename    : SkillFormula.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "Assert.h"
#include "Monster.h"
#include "SkillHandler.h"
#include "Vampire.h"

// °Ë°è¿­
#include "CrossCounter.h"
#include "DancingSword.h"
#include "DoubleImpact.h"
#include "Expansion.h"
#include "FlashSliding.h"
#include "HitConvert.h"
#include "LightningHand.h"
#include "MentalSword.h"
#include "MiracleShield.h"
#include "RainbowSlasher.h"
#include "SharpShield.h"
#include "SnakeCombo.h"
#include "SwordRay.h"
#include "SwordWave.h"
#include "ThunderBolt.h"
#include "ThunderFlash.h"
#include "ThunderSpark.h"
#include "ThunderStorm.h"
#include "TripleSlasher.h"
#include "WideLightning.h"
#include "WindDivider.h"

// µµ°è¿­
#include "AirShield.h"
#include "Berserker.h"
#include "ChargingPower.h"
#include "Earthquake.h"
#include "GhostBlade.h"
#include "HurricaneCombo.h"
#include "MoonlightSever.h"
#include "MultiAmputate.h"
#include "PotentialExplosion.h"
#include "PowerOfLand.h"
#include "ShadowDancing.h"
#include "ShadowWalk.h"
#include "SingleBlow.h"
#include "SpiralSlay.h"
#include "TornadoSever.h"
#include "TripleBreak.h"
#include "Typhoon.h"
#include "WildSmash.h"
#include "WildTyphoon.h"

// ±ºÀÎ °è¿­
#include "DoubleShot.h"
#include "HeadShot.h"
#include "MindControl.h"
#include "MultiShot.h"
#include "Piercing.h"
#include "QuickFire.h"
#include "Sniping.h"
#include "TripleShot.h"
// #include "DetectMine.h"
#include "BulletOfLight.h"
#include "Concealment.h"
#include "CreateBomb.h"
#include "CreateMine.h"
#include "DisarmMine.h"
#include "GunShotGuidance.h"
#include "InstallMine.h"
#include "ObservingEye.h"
#include "Revealer.h"
#include "UltimateBlow.h"

// ÀÎÃ¦ °è¿­
#include "AuraBall.h"
#include "AuraRing.h"
#include "Bless.h"
#include "ContinualLight.h"
#include "CreateHolyWater.h"
#include "DetectHidden.h"
#include "DetectInvisibility.h"
#include "Flare.h"
#include "Light.h"
#include "Purify.h"
#include "Striking.h"
// #include "Identify.h"
#include "AuraShield.h"
// #include "Enchant.h"
#include "Hymn.h"
#include "LightBall.h"
#include "Lightness.h"
#include "Rebuke.h"
#include "Reflection.h"
#include "Requital.h"
#include "Sanctuary.h"
#include "SpiritGuard.h"
#include "VigorDrop.h"
#include "Visible.h"

// Èú¸µ °è¿­
#include "Activation.h"
#include "CauseCriticalWounds.h"
#include "CauseLightWounds.h"
#include "CauseSeriousWounds.h"
#include "CureAll.h"
#include "CureCriticalWounds.h"
#include "CureLightWounds.h"
#include "CurePoison.h"
#include "CureSeriousWounds.h"
#include "EnergyDrop.h"
#include "HolyBlast.h"
#include "ProtectionFromAcid.h"
#include "ProtectionFromCurse.h"
#include "ProtectionFromPoison.h"
#include "RegenerationSkill.h"
#include "RemoveCurse.h"
#include "Resurrect.h"
#include "Sacrifice.h"
// #include "MassCure.h"
// #include "MassHeal.h"
#include "DenialMagic.h"
#include "HolyArrow.h"
#include "Illendue.h"
#include "Regeneration.h"
#include "TurnUndead.h"


// ¹ìÇÁ °è¿­
#include "AcidBall.h"
#include "AcidBolt.h"
#include "AcidSwamp.h"
#include "AcidTouch.h"
#include "DarkBluePoison.h"
#include "Doom.h"
#include "GreenPoison.h"
#include "GreenStalker.h"
#include "Paralyze.h"
#include "PoisonousHands.h"
#include "Seduction.h"
#include "YellowPoison.h"
// #include "Blind.h"
#include "BloodyBall.h"
#include "BloodyBreaker.h"
#include "BloodyKnife.h"
#include "BloodyMasterWave.h"
#include "BloodyNail.h"
#include "BloodyWarp.h"
#include "BloodyWave.h"
#include "Death.h"
#include "Mephisto.h"
// #include "BloodyWall.h"
#include "BloodySnake.h"
#include "BloodySpear.h"
#include "BloodyWall.h"
#include "Darkness.h"
#include "Hide.h"
#include "Invisibility.h"
#include "RapidGliding.h"
#include "TransformToBat.h"
#include "TransformToWolf.h"

// #include "SummonWolf.h"
#include "OpenCasket.h"
#include "SummonCasket.h"
// #include "RaisingDead.h"
// #include "SummonServant.h"

#include "AcidStorm.h"
#include "AcidStrike.h"
#include "Armageddon.h"
#include "BloodyMarker.h"
#include "BloodyStorm.h"
#include "BloodyStrike.h"
#include "BloodyTunnel.h"
#include "Extreme.h"
#include "HandsOfWisdom.h"
#include "PoisonStorm.h"
#include "PoisonStrike.h"
#include "Transfusion.h"

// ±âÅ¸
#include "CriticalGround.h"
#include "DuplicateSelf.h"
#include "GroundAttack.h"
#include "Hallucination.h"
#include "MeteorStrike.h"
#include "Peace.h"
#include "Restore.h"
#include "SoulChain.h"
#include "SummonMonsters.h"

// ¼ºÁö½ºÅ³
#include "IllusionOfAvenge.h"
#include "MagicElusion.h"
#include "PoisonMesh.h"
#include "WillOfLife.h"

// ¾Æ¿ì½ºÅÍÁî ½ºÅ³
#include "AcidEruption.h"
#include "BackStab.h"
#include "BeatHead.h"
#include "BlazeBolt.h"
#include "BlazeWalk.h"
#include "BlitzSliding.h"
#include "BloodyZenith.h"
#include "Blunting.h"
#include "BombingStar.h"
#include "BurningSolCharging.h"
#include "BurningSolLaunch.h"
#include "Cannonade.h"
#include "ChainThrowingAxe.h"
#include "ChargingAttack.h"
#include "ChoppingFirewood.h"
#include "ClaymoreExplosion.h"
#include "CreateHolyPotion.h"
#include "CrossGuard.h"
#include "DeleoEfficio.h"
#include "DestructionSpear.h"
#include "DistanceBlitz.h"
#include "DivineGuidance.h"
#include "DivineSpirits.h"
#include "DuckingWallop.h"
#include "EarthsTeeth.h"
#include "EmissionWater.h"
#include "Eternity.h"
#include "Evade.h"
#include "ExplosionWater.h"
#include "FatalSnick.h"
#include "FirePiercing.h"
#include "Flourish.h"
#include "FrozenArmor.h"
#include "GammaChop.h"
#include "Glacier1.h"
#include "Glacier2.h"
#include "GnomesWhisper.h"
#include "GoreGlandFire.h"
#include "GrayDarkness.h"
#include "GreatHeal.h"
#include "GroundBless.h"
#include "HandsOfFire.h"
#include "HandsOfNizie.h"
#include "HeartCatalyst.h"
#include "HellFire.h"
#include "HolyArmor.h"
#include "Howl.h"
#include "IceAuger.h"
#include "IceField.h"
#include "IceHail.h"
#include "IceLance.h"
#include "IceWave.h"
#include "InfinityThunderbolt.h"
#include "InstallTrap.h"
#include "InstallTurret.h"
#include "IntimateGrail.h"
#include "JabbingVein.h"
#include "KasasArrow.h"
#include "LandMineExplosion.h"
#include "LarSlash.h"
#include "Liberty.h"
#include "MagnumSpear.h"
#include "MercyGround.h"
#include "MeteorStorm.h"
#include "MoleShot.h"
#include "MultiThrowingAxe.h"
#include "NooseOfWraith.h"
#include "NymphRecovery.h"
#include "PlasmaRocketLauncher.h"
#include "PlayingWithFire.h"
#include "PleasureExplosion.h"
#include "Prominence.h"
#include "ProtectionFromBlood.h"
#include "ReactiveArmor.h"
#include "Rediance.h"
#include "RefusalEther.h"
#include "ReputoFactum.h"
#include "RingOfFlare.h"
#include "SetAfire.h"
#include "SharpChakram.h"
#include "SharpRound.h"
#include "ShiftBreak.h"
#include "SoulRebirth.h"
#include "SpitStream.h"
#include "StoneAuger.h"
#include "StoneSkin.h"
#include "SummonFireElemental.h"
#include "SummonGoreGland.h"
#include "SummonGroundElemental.h"
#include "SummonWaterElemental.h"
#include "SweepVice.h"
#include "SwordOfThor.h"
#include "TalonOfCrow.h"
#include "Teleport.h"
#include "Tendril.h"
#include "ThrowingAxe.h"
#include "TransformToWerwolf.h"
#include "Trident.h"
#include "TurretFire.h"
#include "ViolentPhantom.h"
#include "WaterBarrier.h"
#include "Whitsuntide.h"
#include "WideIceField.h"
#include "WideIceHail.h"

// ¾Æ¿ì½ºÅÍÁî 140·¹º§ ½ºÅ³
#include "ARAttack.h"
#include "Aberration.h"
#include "BikeCrash.h"
#include "Destinies.h"
#include "DragonTornado.h"
#include "FierceFlame.h"
#include "FuryOfGnome.h"
#include "GrenadeAttack.h"
#include "Halo.h"
#include "HarpoonBomb.h"
#include "IceHorizon.h"
#include "PassingHeal.h"
#include "RottenApple.h"
#include "SMGAttack.h"
#include "SelfDestruction.h"
#include "ShadowOfStorm.h"
#include "SharpHail.h"
#include "SummonMiga.h"
#include "SummonMigaAttack.h"
#include "WildWolf.h"
// add by coffee 2007-2-17
#include "BloodCurse.h"        //ÑªÖ®?Öä
#include "BloodyScarify.h"     //ÑªÖ®ÀÓÓ¡
#include "BombCrashWalk.h"     //¾ÞÅÚºäÕ¨
#include "HeavenGround.h"      //ÌìÉñ½µÁÙ
#include "IllusionInversion.h" //¿Ö²À»Ã¾õ(ÈËÀà·¨Ê¦)
#include "SatelliteBomb.h"     //ÎÀÐÇºä»÷ (ÈËÀàÇ¹ÐÂ¼¼ÄÜ)
#include "ShineSword.h"        //ÉÁÒ«Ö®½£
// Ä§Áé
#include "BigRockfall.h"      //395 ADD BY RALLSER
#include "BrambleHalo.h"      //390 ADD BY RALLSER
#include "CutStorm.h"         //387 ADD BY RALLSER
#include "DeadlyClaw.h"       //391ÖÂÉËÁÑ×¦ add by rallser
#include "DummyDrake.h"       // µÂÀ×¿Ë¿þÀÜ(»ð·¨)
#include "FireMeteor.h"       //394 ADD BY RALLSER
#include "HeterChakram.h"     // ÏÄ²¼Àû»ùÒò(ÃôÕ½)
#include "HydroConvergence.h" // ¸´ºÏË®ÁÆ(Ë®·¨)
#include "PenetrateWheel.h"   //393 ADD BY RALLSER
#include "RapidFreeze.h"      //396 ADD BY RALLSER
#include "SacredStamp.h"      //389 ADD BY RALLSER
#include "SkyFire.h"          //386 ÌìÀ×»ð»¨ ADD BY RALLSER
#include "SummonClay.h"       // ÍÁ·¨
#include "VoodooRing.h"       //392 ADD BY RALLSER
#include "XRLMissile.h"       //388 ADD BY RALLSER


// add by coffee 2007-3-1

#include "domain/SkillOutputFormulas.h"

//////////////////////////////////////////////////////////////////////////////
// Thin adapters (docs/RESTRUCTURING.md task 3.3): every computeOutput body
// moved verbatim to de-core (src/domain/SkillOutputFormulas.cpp); each
// member function here converts SkillInput, delegates, and copies the
// result back. All 393 compiled call sites pass a freshly zero-initialized
// SkillOutput (and no formula body reads an output field before writing
// it), so copying all six fields back is identical to the original
// partial assignments. The single known reuse of an output object is in
// the never-built legacy gameserver/test/ dir; keep the fresh-output
// pattern when adding callers.
//////////////////////////////////////////////////////////////////////////////

namespace {

decore::skillformula::GunClass toGunClass(Item::ItemClass IClass) {
    switch (IClass) {
    case Item::ITEM_CLASS_SG:
        return decore::skillformula::GunClass::SG;
    case Item::ITEM_CLASS_AR:
        return decore::skillformula::GunClass::AR;
    case Item::ITEM_CLASS_SMG:
        return decore::skillformula::GunClass::SMG;
    case Item::ITEM_CLASS_SR:
        return decore::skillformula::GunClass::SR;
    default:
        return decore::skillformula::GunClass::Other;
    }
}

// This name-to-name mapping is the one piece of the extraction no test can
// see (formula_tests deliberately links only de-core): a transposed pair
// here compiles, links, and passes every suite while corrupting the game
// balance. It was hand-verified against both structs in the 3.3
// adversarial review — re-verify by eye on any edit. Note it also reads
// every field unconditionally (master read IClass in only 4 bodies), which
// is safe because all four SkillInput constructors assign every field and
// no compiled caller default-constructs a SkillInput.
decore::skillformula::SkillInput toFormulaInput(const SkillInput& input) {
    decore::skillformula::SkillInput in;
    in.SkillLevel = input.SkillLevel;
    in.DomainLevel = input.DomainLevel;
    in.DomainGrade = -1; // only the grade adapters fetch the real grade
    in.STR = input.STR;
    in.DEX = input.DEX;
    in.INTE = input.INTE;
    in.TargetType = input.TargetType;
    in.Range = input.Range;
    in.Gun = toGunClass(input.IClass);
    in.PartySize = input.PartySize;
    return in;
}

void fromFormulaOutput(const decore::skillformula::SkillOutput& o, SkillOutput& output) {
    output.Damage = o.Damage;
    output.Duration = o.Duration;
    output.Tick = o.Tick;
    output.ToHit = o.ToHit;
    output.Range = o.Range;
    output.Delay = o.Delay;
}

} // namespace

#define DE_SKILL_FORMULA(ClassName)                                               \
    void ClassName::computeOutput(const SkillInput& input, SkillOutput& output) { \
        decore::skillformula::SkillOutput o;                                      \
        decore::skillformula::ClassName(toFormulaInput(input), o);                \
        fromFormulaOutput(o, output);                                             \
    }

// The three grade-switch formulas fetched the domain grade from
// g_pSkillInfoManager mid-body; the adapter fetches it up front, keeping
// the manager's out-of-range throw on exactly the invocations that could
// throw before (the partially-written output an in-body throw left behind
// was never observable: every caller's output is freshly zeroed and
// abandoned on the exception path).
#define DE_SKILL_FORMULA_GRADE(ClassName)                                                    \
    void ClassName::computeOutput(const SkillInput& input, SkillOutput& output) {            \
        decore::skillformula::SkillInput in = toFormulaInput(input);                         \
        in.DomainGrade = (int)g_pSkillInfoManager->getGradeByDomainLevel(input.DomainLevel); \
        decore::skillformula::SkillOutput o;                                                 \
        decore::skillformula::ClassName(in, o);                                              \
        fromFormulaOutput(o, output);                                                        \
    }

// HeadShot Asserts on a non-gun ItemClass; the Assert moves ahead of the
// delegation (see SkillOutputFormulas.h).
#define DE_SKILL_FORMULA_GUN_ASSERT(ClassName)                                               \
    void ClassName::computeOutput(const SkillInput& input, SkillOutput& output) {            \
        Assert(input.IClass == Item::ITEM_CLASS_SG || input.IClass == Item::ITEM_CLASS_AR || \
               input.IClass == Item::ITEM_CLASS_SMG || input.IClass == Item::ITEM_CLASS_SR); \
        decore::skillformula::SkillOutput o;                                                 \
        decore::skillformula::ClassName(toFormulaInput(input), o);                           \
        fromFormulaOutput(o, output);                                                        \
    }

DE_SKILL_FORMULA(DoubleImpact)
DE_SKILL_FORMULA(TripleSlasher)
DE_SKILL_FORMULA(RainbowSlasher)
DE_SKILL_FORMULA(ThunderSpark)
DE_SKILL_FORMULA(DancingSword)
DE_SKILL_FORMULA(CrossCounter)
DE_SKILL_FORMULA(FlashSliding)
DE_SKILL_FORMULA(LightningHand)
DE_SKILL_FORMULA(SwordWave)
DE_SKILL_FORMULA(SnakeCombo)
DE_SKILL_FORMULA(WindDivider)
DE_SKILL_FORMULA(ThunderBolt)
DE_SKILL_FORMULA(Expansion)
DE_SKILL_FORMULA(MiracleShield)
DE_SKILL_FORMULA(ThunderFlash)
DE_SKILL_FORMULA(ThunderStorm)
DE_SKILL_FORMULA(MentalSword)
DE_SKILL_FORMULA(SingleBlow)
DE_SKILL_FORMULA(SpiralSlay)
DE_SKILL_FORMULA(TripleBreak)
DE_SKILL_FORMULA(WildSmash)
DE_SKILL_FORMULA(GhostBlade)
DE_SKILL_FORMULA(PotentialExplosion)
DE_SKILL_FORMULA(ShadowWalk)
DE_SKILL_FORMULA(ChargingPower)
DE_SKILL_FORMULA(HurricaneCombo)
DE_SKILL_FORMULA(TornadoSever)
DE_SKILL_FORMULA(Earthquake)
DE_SKILL_FORMULA(Berserker)
DE_SKILL_FORMULA(MoonlightSever)
DE_SKILL_FORMULA(ShadowDancing)
DE_SKILL_FORMULA(Typhoon)
DE_SKILL_FORMULA(QuickFire)
DE_SKILL_FORMULA(DoubleShot)
DE_SKILL_FORMULA(TripleShot)
DE_SKILL_FORMULA(MultiShot)
DE_SKILL_FORMULA_GUN_ASSERT(HeadShot)
DE_SKILL_FORMULA(Piercing)
DE_SKILL_FORMULA(Sniping)
DE_SKILL_FORMULA(MindControl)
DE_SKILL_FORMULA(Revealer)
DE_SKILL_FORMULA(CreateBomb)
DE_SKILL_FORMULA(CreateMine)
DE_SKILL_FORMULA(InstallMine)
DE_SKILL_FORMULA(DisarmMine)
DE_SKILL_FORMULA(ObservingEye)
DE_SKILL_FORMULA(CreateHolyWater)
DE_SKILL_FORMULA(Light)
DE_SKILL_FORMULA(DetectHidden)
DE_SKILL_FORMULA(AuraBall)
DE_SKILL_FORMULA(Bless)
DE_SKILL_FORMULA_GRADE(ContinualLight)
DE_SKILL_FORMULA(Flare)
DE_SKILL_FORMULA_GRADE(Purify)
DE_SKILL_FORMULA(AuraRing)
DE_SKILL_FORMULA(Striking)
DE_SKILL_FORMULA_GRADE(DetectInvisibility)
DE_SKILL_FORMULA(AuraShield)
DE_SKILL_FORMULA(Visible)
DE_SKILL_FORMULA(CureLightWounds)
DE_SKILL_FORMULA(CureAll)
DE_SKILL_FORMULA(CurePoison)
DE_SKILL_FORMULA(ProtectionFromPoison)
DE_SKILL_FORMULA(CauseLightWounds)
DE_SKILL_FORMULA(CureSeriousWounds)
DE_SKILL_FORMULA(RemoveCurse)
DE_SKILL_FORMULA(ProtectionFromCurse)
DE_SKILL_FORMULA(Resurrect)
DE_SKILL_FORMULA(CauseSeriousWounds)
DE_SKILL_FORMULA(CureCriticalWounds)
DE_SKILL_FORMULA(ProtectionFromAcid)
DE_SKILL_FORMULA(Sacrifice)
DE_SKILL_FORMULA(CauseCriticalWounds)
DE_SKILL_FORMULA(RegenerationSkill)
DE_SKILL_FORMULA(EnergyDrop)
DE_SKILL_FORMULA(VigorDrop)
DE_SKILL_FORMULA(Activation)
DE_SKILL_FORMULA(HolyBlast)
DE_SKILL_FORMULA(Sanctuary)
DE_SKILL_FORMULA(Reflection)
DE_SKILL_FORMULA(Hymn)
DE_SKILL_FORMULA(PoisonousHands)
DE_SKILL_FORMULA(AcidTouch)
DE_SKILL_FORMULA(GreenPoison)
DE_SKILL_FORMULA(Darkness)
DE_SKILL_FORMULA(YellowPoison)
DE_SKILL_FORMULA(TransformToBat)
DE_SKILL_FORMULA(SummonCasket)
DE_SKILL_FORMULA(OpenCasket)
DE_SKILL_FORMULA(AcidBolt)
DE_SKILL_FORMULA(GreenStalker)
DE_SKILL_FORMULA(BloodyTunnel)
DE_SKILL_FORMULA(Paralyze)
DE_SKILL_FORMULA(BloodyMarker)
DE_SKILL_FORMULA(DarkBluePoison)
DE_SKILL_FORMULA(TransformToWolf)
DE_SKILL_FORMULA(Doom)
DE_SKILL_FORMULA(AcidBall)
DE_SKILL_FORMULA(Invisibility)
DE_SKILL_FORMULA(AcidSwamp)
DE_SKILL_FORMULA(Seduction)
DE_SKILL_FORMULA(BloodyNail)
DE_SKILL_FORMULA(BloodyKnife)
DE_SKILL_FORMULA(BloodyBall)
DE_SKILL_FORMULA(BloodyWave)
DE_SKILL_FORMULA(BloodyMasterWave)
DE_SKILL_FORMULA(BloodyWarp)
DE_SKILL_FORMULA(BloodyWall)
DE_SKILL_FORMULA(BloodySnake)
DE_SKILL_FORMULA(BloodySpear)
DE_SKILL_FORMULA(PoisonStrike)
DE_SKILL_FORMULA(AcidStrike)
DE_SKILL_FORMULA(BloodyStrike)
DE_SKILL_FORMULA(PoisonStorm)
DE_SKILL_FORMULA(AcidStorm)
DE_SKILL_FORMULA(BloodyStorm)
DE_SKILL_FORMULA(Extreme)
void CriticalGround::computeOutput(const SkillInput& input, SkillOutput& output) {
    // acid bolt¶û ¶È°°´ç - -;
    // output.Damage = min(40, 20 + (input.INTE-20)/6);
    // output.Delay  = 10; // 1ÃÊ

    // 20
    // 145 --> 145~290 --> 22~46
    // 500 --> 500~1000 --> 125~250 (±âÈ¹140~280)

    int divider = 1;
    if (input.STR < 200) {
        divider = 6;
    } else {
        divider = 4;
    }

    output.Damage = max(20, (input.STR + rand() % input.STR) / divider);
    output.Delay = 10; // 0.6ÃÊ

    // °ø°Ý°è ±â¼ú¿¡´Â ÆÄÆ¼ º¸³Ê½º°¡ Á¸ÀçÇÏÁö ¾Ê´Â´Ù.
}
DE_SKILL_FORMULA(Peace)
DE_SKILL_FORMULA(Death)
DE_SKILL_FORMULA(Mephisto)
DE_SKILL_FORMULA(Transfusion)
DE_SKILL_FORMULA(SummonMonsters)
DE_SKILL_FORMULA(GroundAttack)
void MeteorStrike::computeOutput(const SkillInput& input, SkillOutput& output) {
    output.Damage =
        (int)(input.SkillLevel * 0.8 + (rand() % (int)(input.SkillLevel * 0.4 + 1))) + (input.STR + input.DEX) / 6;
    output.Duration = 10; // ¸î ÃÊÈÄ Æø¹ß
    output.Delay = 10;
}
DE_SKILL_FORMULA(Hallucination)
void DuplicateSelf::computeOutput(const SkillInput& input, SkillOutput& output) {
    output.Damage = min((3 + rand() % 5), input.INTE / 100); // ºÐ½Å °³¼ö
    output.Duration = min(80, 30 + (input.INTE - 20) / 3) * 10;
    output.Delay = max(3, 5 - (input.INTE - 20) / 10) * 10;
}
DE_SKILL_FORMULA(SoulChain)
DE_SKILL_FORMULA(SharpShield)
DE_SKILL_FORMULA(WideLightning)
DE_SKILL_FORMULA(GunShotGuidance)
DE_SKILL_FORMULA(AirShield)
DE_SKILL_FORMULA(BulletOfLight)
DE_SKILL_FORMULA(HandsOfWisdom)
DE_SKILL_FORMULA(LightBall)
DE_SKILL_FORMULA(HolyArrow)
DE_SKILL_FORMULA(Rebuke)
DE_SKILL_FORMULA(SpiritGuard)
DE_SKILL_FORMULA(Regeneration)
DE_SKILL_FORMULA(PowerOfLand)
DE_SKILL_FORMULA(TurnUndead)
DE_SKILL_FORMULA(Armageddon)
DE_SKILL_FORMULA(BloodyBreaker)
DE_SKILL_FORMULA(RapidGliding)
DE_SKILL_FORMULA(MagicElusion)
DE_SKILL_FORMULA(PoisonMesh)
DE_SKILL_FORMULA(IllusionOfAvenge)
DE_SKILL_FORMULA(WillOfLife)
DE_SKILL_FORMULA(DenialMagic)
DE_SKILL_FORMULA(Requital)
DE_SKILL_FORMULA(Concealment)
DE_SKILL_FORMULA(SwordRay)
DE_SKILL_FORMULA(MultiAmputate)
DE_SKILL_FORMULA(HitConvert)
DE_SKILL_FORMULA(WildTyphoon)
DE_SKILL_FORMULA(UltimateBlow)
DE_SKILL_FORMULA(Illendue)
DE_SKILL_FORMULA(Lightness)
DE_SKILL_FORMULA(Flourish)
DE_SKILL_FORMULA(Evade)
DE_SKILL_FORMULA(SharpRound)
DE_SKILL_FORMULA(BackStab)
DE_SKILL_FORMULA(Blunting)
DE_SKILL_FORMULA(GammaChop)
DE_SKILL_FORMULA(CrossGuard)
DE_SKILL_FORMULA(KasasArrow)
DE_SKILL_FORMULA(HandsOfFire)
DE_SKILL_FORMULA(Prominence)
DE_SKILL_FORMULA(RingOfFlare)
DE_SKILL_FORMULA(BlazeBolt)
DE_SKILL_FORMULA(IceField)
DE_SKILL_FORMULA(WaterBarrier)
DE_SKILL_FORMULA(NymphRecovery)
DE_SKILL_FORMULA(Liberty)
DE_SKILL_FORMULA(Tendril)
DE_SKILL_FORMULA(StoneAuger)
DE_SKILL_FORMULA(EarthsTeeth)
DE_SKILL_FORMULA(HandsOfNizie)
DE_SKILL_FORMULA(GnomesWhisper)
DE_SKILL_FORMULA(RefusalEther)
DE_SKILL_FORMULA(EmissionWater)
DE_SKILL_FORMULA(BeatHead)
DE_SKILL_FORMULA(DivineSpirits)
DE_SKILL_FORMULA(BlitzSliding)
DE_SKILL_FORMULA(JabbingVein)
DE_SKILL_FORMULA(GreatHeal)
DE_SKILL_FORMULA(DivineGuidance)
DE_SKILL_FORMULA(BlazeWalk)
DE_SKILL_FORMULA(BloodyZenith)
DE_SKILL_FORMULA(Rediance)
DE_SKILL_FORMULA(LarSlash)
DE_SKILL_FORMULA(Trident)
DE_SKILL_FORMULA(HeartCatalyst)
DE_SKILL_FORMULA(ProtectionFromBlood)
DE_SKILL_FORMULA(MoleShot)
DE_SKILL_FORMULA(Eternity)
DE_SKILL_FORMULA(InstallTrap)
DE_SKILL_FORMULA(HolyArmor)
DE_SKILL_FORMULA(MercyGround)
DE_SKILL_FORMULA(CreateHolyPotion)
DE_SKILL_FORMULA(TransformToWerwolf)
DE_SKILL_FORMULA(GrayDarkness)
DE_SKILL_FORMULA(StoneSkin)
DE_SKILL_FORMULA(TalonOfCrow)
DE_SKILL_FORMULA(Howl)
DE_SKILL_FORMULA(AcidEruption)
DE_SKILL_FORMULA(Teleport)
DE_SKILL_FORMULA(FirePiercing)
DE_SKILL_FORMULA(SoulRebirth)
DE_SKILL_FORMULA(IceLance)
DE_SKILL_FORMULA(ExplosionWater)
DE_SKILL_FORMULA(FrozenArmor)
DE_SKILL_FORMULA(ReactiveArmor)
DE_SKILL_FORMULA(MagnumSpear)
DE_SKILL_FORMULA(HellFire)
DE_SKILL_FORMULA(GroundBless)
DE_SKILL_FORMULA(SharpChakram)
DE_SKILL_FORMULA(DestructionSpear)
DE_SKILL_FORMULA(ShiftBreak)
DE_SKILL_FORMULA(FatalSnick)
DE_SKILL_FORMULA(ChargingAttack)
DE_SKILL_FORMULA(DuckingWallop)
DE_SKILL_FORMULA(DistanceBlitz)
DE_SKILL_FORMULA(SummonGroundElemental)
DE_SKILL_FORMULA(SummonFireElemental)
DE_SKILL_FORMULA(SummonWaterElemental)
DE_SKILL_FORMULA(MeteorStorm)
DE_SKILL_FORMULA(WideIceField)
DE_SKILL_FORMULA(Glacier1)
DE_SKILL_FORMULA(Glacier2)
DE_SKILL_FORMULA(IceAuger)
DE_SKILL_FORMULA(IceHail)
DE_SKILL_FORMULA(WideIceHail)
DE_SKILL_FORMULA(IceWave)
DE_SKILL_FORMULA(LandMineExplosion)
DE_SKILL_FORMULA(ClaymoreExplosion)
DE_SKILL_FORMULA(PleasureExplosion)
DE_SKILL_FORMULA(DeleoEfficio)
DE_SKILL_FORMULA(ReputoFactum)
DE_SKILL_FORMULA(SwordOfThor)
DE_SKILL_FORMULA(BurningSolCharging)
DE_SKILL_FORMULA(BurningSolLaunch)
DE_SKILL_FORMULA(SweepVice)
DE_SKILL_FORMULA(Whitsuntide)
DE_SKILL_FORMULA(ViolentPhantom)
DE_SKILL_FORMULA(InstallTurret)
DE_SKILL_FORMULA(TurretFire)
DE_SKILL_FORMULA(SummonGoreGland)
DE_SKILL_FORMULA(GoreGlandFire)
void ThrowingAxe::computeOutput(const SkillInput& input, SkillOutput& output) {
    output.Damage = Random(870, 1000);
    output.Duration = 20; // ¸î ÃÊÈÄ Æø¹ß
    output.Delay = 10;
}
void ChoppingFirewood::computeOutput(const SkillInput& input, SkillOutput& output) {
    output.Damage = Random(870, 1000);
    output.Duration = 10; // ¸î ÃÊÈÄ Æø¹ß
    output.Delay = 10;
}
void ChainThrowingAxe::computeOutput(const SkillInput& input, SkillOutput& output) {
    output.Damage = Random(500, 650);
    output.Duration = 20; // ¸î ÃÊÈÄ Æø¹ß
    output.Delay = 10;
}
void MultiThrowingAxe::computeOutput(const SkillInput& input, SkillOutput& output) {
    output.Damage = Random(870, 1000);
    output.Duration = 20; // ¸î ÃÊÈÄ Æø¹ß
    output.Delay = 10;
}
DE_SKILL_FORMULA(PlayingWithFire)
DE_SKILL_FORMULA(InfinityThunderbolt)
DE_SKILL_FORMULA(SpitStream)
DE_SKILL_FORMULA(PlasmaRocketLauncher)
DE_SKILL_FORMULA(BombingStar)
DE_SKILL_FORMULA(IntimateGrail)
DE_SKILL_FORMULA(NooseOfWraith)
DE_SKILL_FORMULA(SetAfire)
DE_SKILL_FORMULA(SharpHail)
DE_SKILL_FORMULA(IceHorizon)
DE_SKILL_FORMULA(FuryOfGnome)
DE_SKILL_FORMULA(SummonMiga)
DE_SKILL_FORMULA(SummonMigaAttack)
void Cannonade::computeOutput(const SkillInput& input, SkillOutput& output) {
    output.Damage = 52 + (rand() % 30);
    output.Duration = 20;
}
void SelfDestruction::computeOutput(const SkillInput& input, SkillOutput& output) {
    output.Delay = 0;
    output.Damage = 110 + (rand() % 40);
}
DE_SKILL_FORMULA(ARAttack)
DE_SKILL_FORMULA(SMGAttack)
DE_SKILL_FORMULA(GrenadeAttack)
DE_SKILL_FORMULA(Halo)
DE_SKILL_FORMULA(Destinies)
DE_SKILL_FORMULA(FierceFlame)
DE_SKILL_FORMULA(ShadowOfStorm)
DE_SKILL_FORMULA(WildWolf)
DE_SKILL_FORMULA(Aberration)
DE_SKILL_FORMULA(DragonTornado)
DE_SKILL_FORMULA(BikeCrash)
DE_SKILL_FORMULA(HarpoonBomb)
DE_SKILL_FORMULA(PassingHeal)
DE_SKILL_FORMULA(RottenApple)
DE_SKILL_FORMULA(BloodyScarify)
void BloodCurse::computeOutput(const SkillInput& input, SkillOutput& output) {
    output.Damage =
        (int)(input.SkillLevel * 0.8 + (rand() % (int)(input.SkillLevel * 0.4 + 1))) + (input.INTE + input.DEX) / 4;
    output.Duration = 28; // ÑÓÊ±ÏÔÊ¾Ð§¹û
    output.Delay = max(5, (10 - (input.DEX / 200))) * 10;
    ;
}
DE_SKILL_FORMULA(ShineSword)
DE_SKILL_FORMULA(BombCrashWalk)
DE_SKILL_FORMULA(SatelliteBomb)
DE_SKILL_FORMULA(IllusionInversion)
DE_SKILL_FORMULA(HeavenGround)
DE_SKILL_FORMULA(DummyDrake)
DE_SKILL_FORMULA(HydroConvergence)
DE_SKILL_FORMULA(SummonClay)
DE_SKILL_FORMULA(HeterChakram)
DE_SKILL_FORMULA(SkyFire)
DE_SKILL_FORMULA(CutStorm)
DE_SKILL_FORMULA(XRLMissile)
DE_SKILL_FORMULA(SacredStamp)
DE_SKILL_FORMULA(BrambleHalo)
DE_SKILL_FORMULA(DeadlyClaw)
void VoodooRing::computeOutput(const SkillInput& input, SkillOutput& output) {
    output.Damage =
        (int)(input.SkillLevel * 0.8 + (rand() % (int)(input.SkillLevel * 0.4 + 1))) + (input.INTE + input.DEX) / 4;
    output.Duration = 28; // ÑÓÊ±ÏÔÊ¾Ð§¹û
    output.Delay = max(5, (10 - (input.DEX / 200))) * 10;
    ;
}
DE_SKILL_FORMULA(PenetrateWheel)
DE_SKILL_FORMULA(FireMeteor)
DE_SKILL_FORMULA(BigRockfall)
DE_SKILL_FORMULA(RapidFreeze)
