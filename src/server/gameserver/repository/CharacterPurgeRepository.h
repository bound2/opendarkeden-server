#ifndef __CHARACTER_PURGE_REPOSITORY_H__
#define __CHARACTER_PURGE_REPOSITORY_H__

#include <string>

// The character-deletion purge: the 109 statements that retire a
// character's rows across the whole schema when PCManager deletes it.
// They run in a fixed order, on ONE Statement, with no transaction — a
// failure part-way leaves the earlier deletes done and the later ones
// not.
//
// The list, in order: the three race tables' Active='INACTIVE' updates
// (all three, whatever the character's race — the row is left, not
// deleted); the three SkillSave tables and RankBonusData; the 81
// <Class>Object tables with GQuestSave among them (between
// GQuestItemObject and TrapItemObject); CoupleInfo by either partner
// column; fourteen Effect* tables and EnemyErase — seven of those Effect
// tables (AcidTouch, DetectHidden, Paralysis, Poison, PoisonousHands,
// ProtectionFromParalysis, ProtectionFromPoison) are written by nothing
// else in the tree, so the purge deletes from tables nothing fills;
// FlagSet, TimeLimitItems and EventQuestAdvance.
//
// The character NAME is interpolated raw into every statement (Name /
// OwnerID / the CoupleInfo partner columns). The loginserver has its own
// per-character purge (LoginCharacterPurgeRepository), which covers six
// more object tables and MofusPowerPoint.
class CharacterPurgeRepository {
public:
    virtual ~CharacterPurgeRepository() {}

    // The whole list, in order, on one Statement.
    virtual void purgeCharacter(const std::string& name) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLCharacterPurgeRepository.cpp.
CharacterPurgeRepository& defaultCharacterPurgeRepository();

#endif
