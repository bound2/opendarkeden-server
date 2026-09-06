#ifndef __CHARACTER_PURGE_REPOSITORY_H__
#define __CHARACTER_PURGE_REPOSITORY_H__

#include <string>

// Persistence seam for the character-deletion purge (task 3.2, the
// CreatureUtil round, 2026-09-06): deletePC's 109 statements, which
// retire a character's rows across the whole schema when PCManager
// deletes it. They run in the order the function wrote them, on ONE
// Statement, with no transaction — a failure part-way leaves the
// earlier deletes done and the later ones not, exactly as before.
//
// The list, in order: the three race tables' Active='INACTIVE' updates
// (all three, whatever the character's race — the row is left, not
// deleted; a commented-out DELETE beside each says the older flow
// deleted it); the three SkillSave tables and RankBonusData; the 81
// <Class>Object tables with GQuestSave among them (the 88th statement,
// between GQuestItemObject and TrapItemObject); CoupleInfo by either
// partner column; fourteen Effect* tables and EnemyErase (lower-case
// "where", written as "where OwnerID='%s'" with no spaces) — seven of
// those Effect tables (AcidTouch, DetectHidden, Paralysis, Poison,
// PoisonousHands, ProtectionFromParalysis, ProtectionFromPoison) are
// written by nothing else in the tree, so the purge deletes from tables
// nothing fills; FlagSet, TimeLimitItems and EventQuestAdvance. Every
// literal is byte for byte the original; the 82 that were built by
// string concatenation ("... OwnerID = '" + ownerID + "'") are the same
// bytes as a "'%s'" format with the name, which is how this seam writes
// them — the one transport difference is that those 82 now pass through
// executeQuery's 2048-byte format buffer (the Name columns are
// varchar(10); the 20-byte maxNameLength in PlayerTypes.h governs the
// account name, and nothing caps a character name but the column).
//
// Most tables on the list are another seam's (the item objects
// ItemObjectRepository's, the skill saves SkillSaveRepository's, and so
// on), and eleven of those headers say their purge DELETE lives here.
// EventQuestAdvance is QuestInfoRepository's, which does not mention the
// purge; the seven Effect tables above are no seam's. The purge is one
// operation in the game's eyes, so it is one method in one seam rather
// than a hundred calls across those.
//
// Not enclosed: the loginserver's per-character purges in
// CLDeletePCHandler.cpp and ItemDestroyer.cpp, which repeat much of this
// list for that binary — their own seam. No other gameserver code runs
// the purge; single statements on the same tables do run elsewhere in
// their own seams (RankBonusRepository's per-owner DELETE is the same
// bytes as the seventh statement here; FlagSet, EffectSave and the war
// items' loaders delete by owner too).
//
// Pre-existing and preserved: the three Active updates and every DELETE
// key on the character NAME (Name / OwnerID / the CoupleInfo partner
// columns) interpolated raw, as before.
class CharacterPurgeRepository {
public:
    virtual ~CharacterPurgeRepository() {}

    // deletePC — the whole list, in order, on one Statement.
    virtual void purgeCharacter(const std::string& name) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLCharacterPurgeRepository.cpp. An accessor function rather than a
// g_p* extern: ratchet R1 counts those.
CharacterPurgeRepository& defaultCharacterPurgeRepository();

#endif
