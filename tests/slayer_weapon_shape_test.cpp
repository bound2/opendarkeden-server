// slayer_weapon_shape_test.cpp — pins the weapon a slayer is seen holding.
//
// The five weapon bits of a slayer's outlook index the client's weapon
// template table, whose numbering is the retail client's: each family has
// the base model first and the high-tier models after it. A server that
// numbers the families 0..8 sends a cross as the client's SR2 and a rifle
// as its blade, so gunners and enchanters see each other with the wrong
// weapon and attack animation. These expectations are the client's table
// (Client/Packet/Types/PacketItemDef.h) and the type thresholds at which
// its Item.inf changes a weapon's addon frames.

#include <gtest/gtest.h>

#include "types/SlayerWeaponShape.h"

TEST(SlayerWeaponShape, EnumMatchesTheClientTable) {
    EXPECT_EQ(0, WEAPON_NONE);
    EXPECT_EQ(1, WEAPON_SWORD);
    EXPECT_EQ(2, WEAPON_SWORD1);
    EXPECT_EQ(3, WEAPON_BLADE);
    EXPECT_EQ(4, WEAPON_BLADE1);
    EXPECT_EQ(5, WEAPON_SR);
    EXPECT_EQ(6, WEAPON_SR1);
    EXPECT_EQ(7, WEAPON_SR2);
    EXPECT_EQ(8, WEAPON_SR3);
    EXPECT_EQ(9, WEAPON_AR);
    EXPECT_EQ(10, WEAPON_AR1);
    EXPECT_EQ(11, WEAPON_AR2);
    EXPECT_EQ(12, WEAPON_AR3);
    EXPECT_EQ(13, WEAPON_SG);
    EXPECT_EQ(14, WEAPON_SMG);
    EXPECT_EQ(15, WEAPON_CROSS);
    EXPECT_EQ(16, WEAPON_CROSS1);
    EXPECT_EQ(17, WEAPON_MACE);
    EXPECT_EQ(18, WEAPON_MACE1);
    EXPECT_EQ(19, WEAPON_MAX);
    EXPECT_EQ(WEAPON_MAX, static_cast<int>(sizeof(WeaponType2String) / sizeof(WeaponType2String[0])));
    EXPECT_EQ("WEAPON_MACE1", WeaponType2String[WEAPON_MACE1]);
}

TEST(SlayerWeaponShape, BaseModelsKeepTheirFamily) {
    for (ItemType_t type = 0; type < 14; ++type) {
        EXPECT_EQ(WEAPON_SWORD, slayerWeaponShape(WEAPON_SWORD, type)) << type;
        EXPECT_EQ(WEAPON_BLADE, slayerWeaponShape(WEAPON_BLADE, type)) << type;
        EXPECT_EQ(WEAPON_SR, slayerWeaponShape(WEAPON_SR, type)) << type;
        EXPECT_EQ(WEAPON_AR, slayerWeaponShape(WEAPON_AR, type)) << type;
        EXPECT_EQ(WEAPON_SG, slayerWeaponShape(WEAPON_SG, type)) << type;
        EXPECT_EQ(WEAPON_SMG, slayerWeaponShape(WEAPON_SMG, type)) << type;
        EXPECT_EQ(WEAPON_CROSS, slayerWeaponShape(WEAPON_CROSS, type)) << type;
        EXPECT_EQ(WEAPON_MACE, slayerWeaponShape(WEAPON_MACE, type)) << type;
    }
    EXPECT_EQ(WEAPON_NONE, slayerWeaponShape(WEAPON_NONE, 17));
}

TEST(SlayerWeaponShape, HighTierModelsTakeTheirOwnFrames) {
    // Swords and blades change frames at type 16 (Fear Bringer(180), Angle Cutter(180)).
    EXPECT_EQ(WEAPON_SWORD, slayerWeaponShape(WEAPON_SWORD, 15));
    EXPECT_EQ(WEAPON_SWORD1, slayerWeaponShape(WEAPON_SWORD, 16));
    EXPECT_EQ(WEAPON_SWORD1, slayerWeaponShape(WEAPON_SWORD, 21));
    EXPECT_EQ(WEAPON_BLADE, slayerWeaponShape(WEAPON_BLADE, 15));
    EXPECT_EQ(WEAPON_BLADE1, slayerWeaponShape(WEAPON_BLADE, 16));
    // Rifles have three: types 14 and 15 borrow other frames, 16 and up their own.
    EXPECT_EQ(WEAPON_SR1, slayerWeaponShape(WEAPON_SR, 14));
    EXPECT_EQ(WEAPON_SR2, slayerWeaponShape(WEAPON_SR, 15));
    EXPECT_EQ(WEAPON_SR3, slayerWeaponShape(WEAPON_SR, 16));
    EXPECT_EQ(WEAPON_SR3, slayerWeaponShape(WEAPON_SR, 21));
    EXPECT_EQ(WEAPON_AR1, slayerWeaponShape(WEAPON_AR, 14));
    EXPECT_EQ(WEAPON_AR2, slayerWeaponShape(WEAPON_AR, 15));
    EXPECT_EQ(WEAPON_AR3, slayerWeaponShape(WEAPON_AR, 16));
    // Shotguns and submachine guns are one frame set for the whole class.
    EXPECT_EQ(WEAPON_SG, slayerWeaponShape(WEAPON_SG, 13));
    EXPECT_EQ(WEAPON_SMG, slayerWeaponShape(WEAPON_SMG, 20));
    // Crosses and maces change at type 14 (Crucis(180), Crook Asser(180)).
    EXPECT_EQ(WEAPON_CROSS, slayerWeaponShape(WEAPON_CROSS, 13));
    EXPECT_EQ(WEAPON_CROSS1, slayerWeaponShape(WEAPON_CROSS, 14));
    EXPECT_EQ(WEAPON_CROSS1, slayerWeaponShape(WEAPON_CROSS, 19));
    EXPECT_EQ(WEAPON_MACE, slayerWeaponShape(WEAPON_MACE, 13));
    EXPECT_EQ(WEAPON_MACE1, slayerWeaponShape(WEAPON_MACE, 14));
}

TEST(SlayerWeaponShape, CharacterListFitsFourBits) {
    for (int shape = WEAPON_NONE; shape < WEAPON_MAX; ++shape) {
        EXPECT_LT(slayerWeaponListShape(static_cast<WeaponType>(shape)), 16) << shape;
    }
    EXPECT_EQ(WEAPON_SR3, slayerWeaponListShape(WEAPON_SR3));
    EXPECT_EQ(WEAPON_CROSS, slayerWeaponListShape(WEAPON_CROSS));
    EXPECT_EQ(WEAPON_CROSS, slayerWeaponListShape(WEAPON_CROSS1));
    EXPECT_EQ(WEAPON_NONE, slayerWeaponListShape(WEAPON_MACE));
    EXPECT_EQ(WEAPON_NONE, slayerWeaponListShape(WEAPON_MACE1));
}
