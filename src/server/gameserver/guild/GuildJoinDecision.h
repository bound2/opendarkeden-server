//////////////////////////////////////////////////////////////////////////////
// Filename    : GuildJoinDecision.h
// Description : the eligibility rules the guild NPC applies before a
//               character may found a guild or join one, kept apart from the
//               handlers so they can be exercised with neither a creature nor
//               a database. Everything here needs a GuildRepository and plain
//               values only.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GUILD_JOIN_DECISION_H__
#define __GUILD_JOIN_DECISION_H__

#include <cstdint>
#include <ctime>
#include <string>

#include "Outcome.h"
#include "repository/GuildRepository.h"

//////////////////////////////////////////////////////////////////////////////
// Vocabulary
//////////////////////////////////////////////////////////////////////////////

// The three playable races. The guild NPC answers the same refusal with a
// different response code for each: TEAM_ for a Slayer, CLAN_ for a Vampire,
// GUILD_ for an Ousters.
enum GuildJoinRace { GUILD_JOIN_RACE_SLAYER, GUILD_JOIN_RACE_VAMPIRE, GUILD_JOIN_RACE_OUSTERS };

// Which conversation the refusal happened in. The response code depends on
// it, and two of the three conversations answer some refusals with silence.
enum GuildJoinContext {
    // CGTryJoinGuild: the character asked the NPC to open a join dialogue.
    GUILD_JOIN_CONTEXT_STARTING,
    // CGRegistGuild: the character asked to found a guild.
    GUILD_JOIN_CONTEXT_REGIST,
    // CGJoinGuild: the character confirmed a join the dialogue already
    // offered. A refusal here means the client sent something the dialogue
    // should have prevented, so nothing is sent back.
    GUILD_JOIN_CONTEXT_CONFIRM
};

// The rank the client asked to join at. Anything else opens no dialogue.
enum GuildJoinRank {
    // GuildMember::GUILDMEMBER_RANK_SUBMASTER - a starting member, who pays
    // the join fee and has to meet the race's thresholds.
    GUILD_JOIN_RANK_STARTING,
    // GuildMember::GUILDMEMBER_RANK_WAIT - an ordinary applicant, who only
    // needs room on the guild's waiting list.
    GUILD_JOIN_RANK_WAITING,
    GUILD_JOIN_RANK_OTHER
};

// Why the NPC refused.
enum GuildJoinReason {
    // The id names no loaded guild.
    GUILD_JOIN_REJECT_GUILD_MISSING,
    // The character left or was expelled from a guild too recently.
    GUILD_JOIN_REJECT_PENALTY_PENDING,
    // The character still belongs to a guild.
    GUILD_JOIN_REJECT_ALREADY_MEMBER,
    GUILD_JOIN_REJECT_LEVEL_TOO_LOW,
    GUILD_JOIN_REJECT_NOT_ENOUGH_GOLD,
    GUILD_JOIN_REJECT_NOT_ENOUGH_FAME,
    // The guild name carries a quote or a backslash.
    GUILD_JOIN_REJECT_NAME_INVALID,
    GUILD_JOIN_REJECT_NAME_IN_USE,
    GUILD_JOIN_REJECT_WAIT_LIST_FULL
};

// A refusal, with the conversation it belongs to: the same reason maps to a
// different response, or to none at all, depending on the conversation.
struct GuildJoinRejection {
    GuildJoinRejection(GuildJoinContext rejectContext, GuildJoinReason rejectReason)
        : context(rejectContext), reason(rejectReason) {}

    GuildJoinContext context;
    GuildJoinReason reason;
};

// The GCNPCResponse code a refusal carries to one race, written to code.
// Answers false, leaving code untouched, where the server sends nothing.
bool guildJoinResponseCode(const GuildJoinRejection& rejection, GuildJoinRace race, uint16_t& code);

//////////////////////////////////////////////////////////////////////////////
// The quit penalty
//////////////////////////////////////////////////////////////////////////////

// The seven-character "%03d%02d%02d" stamp GuildMember::leave and
// GuildMember::expire write: tm_year, tm_mon and tm_mday as the C library
// keeps them, so the year counts from 1900 and the month is zero based, and
// the stamp is read back into the same three fields. A GuildMember row whose
// ExpireDate is any other length - the empty string a SQL NULL reads back
// as, in practice - is a live membership rather than a penalty.
const std::string::size_type kGuildExpireDateLength = 7;

// GuildMember::GUILDMEMBER_RANK_LEAVE - the rank left behind by a character
// that quit a guild of its own accord, as opposed to being expelled. Only
// that rank blocks founding a new guild. The handlers static_assert this
// against the enumerator.
const int kGuildMemberRankLeave = 5;

// Is the character still inside the penalty a guild quit imposes?
//
// The stamp names a local calendar day, taken at midnight, and the penalty
// runs for penaltyTermDays whole days from it. False for a stamp of any
// other length.
bool guildQuitPenaltyPending(const std::string& expireDate, time_t now, int penaltyTermDays);

//////////////////////////////////////////////////////////////////////////////
// Thresholds
//////////////////////////////////////////////////////////////////////////////

// What the character brings to the NPC. level is the character's level, or
// for a Slayer the level of its highest skill domain.
struct GuildJoinStats {
    int64_t level = 0;
    int64_t gold = 0;
    int64_t fame = 0;
};

// What one race asks of a founder or of a starting member. checkFame is
// false where the race's dialogue asks for no fame at all.
struct GuildJoinRequirements {
    int64_t level = 0;
    int64_t gold = 0;
    int64_t fame = 0;
    bool checkFame = false;
};

// Does the character clear the thresholds? Checked in the order the NPC
// answers them: level, then gold, then fame.
[[nodiscard]] Outcome<void, GuildJoinRejection> decideGuildRequirements(GuildJoinContext context,
                                                                        const GuildJoinStats& stats,
                                                                        const GuildJoinRequirements& requirements);

//////////////////////////////////////////////////////////////////////////////
// Decisions
//////////////////////////////////////////////////////////////////////////////

// What CGTryJoinGuild asks the NPC.
struct GuildJoinAttempt {
    // The character's name, the key of its GuildMember row.
    std::string name;
    GuildJoinRace race = GUILD_JOIN_RACE_SLAYER;
    // Whether a guild of the requested id is loaded.
    bool guildExists = false;
    GuildJoinRank rank = GUILD_JOIN_RANK_OTHER;
    // Read for a starting member only.
    GuildJoinStats stats;
    GuildJoinRequirements requirements;
    // The guild's waiting list and the cap on it, read for an ordinary
    // applicant only.
    int waitMemberCount = 0;
    int waitMemberLimit = 0;
    time_t now = 0;
    int penaltyTermDays = 0;
};

// May this character open a join dialogue for this guild?
//
// Refused, in this order, when no such guild is loaded, when the character
// is inside a quit penalty, when it still belongs to a guild, when a
// starting member misses one of its race's thresholds, and when an ordinary
// applicant finds the waiting list full. A rank that is neither is accepted
// and opens no dialogue.
//
// The repository is passed in because the membership probe is a database
// read; nothing here writes, so the decision needs no database in a test.
[[nodiscard]] Outcome<void, GuildJoinRejection> decideGuildJoinAttempt(GuildRepository& repository,
                                                                       const GuildJoinAttempt& attempt);

// What CGRegistGuild asks the NPC.
struct GuildRegistrationRequest {
    std::string name;
    std::string guildName;
    time_t now = 0;
    int penaltyTermDays = 0;
};

// What an accepted founder still owes before the guild is created.
struct GuildRegistrationClearance {
    // The character has a GuildMember row of a guild it no longer belongs
    // to; the caller drops it before creating the new guild.
    bool clearStaleMemberRow = false;
};

// May this character found a guild under this name?
//
// Refused, in this order, when the name carries a quote or a backslash,
// when an active or pending guild already holds it, and when the character
// left a guild of its own accord too recently. The race's thresholds are
// not part of this: they are answered with silence, so the caller checks
// them with decideGuildRequirements once the stale row is cleared.
[[nodiscard]] Outcome<GuildRegistrationClearance, GuildJoinRejection>
decideGuildRegistration(GuildRepository& repository, const GuildRegistrationRequest& request);

// May this character go through with a join the dialogue already offered?
//
// Refused while a quit penalty is pending and while the character still
// belongs to a guild. Every refusal is silent; the reason is reported so a
// caller can log it.
[[nodiscard]] Outcome<void, GuildJoinRejection>
decideGuildJoinConfirm(GuildRepository& repository, const std::string& name, time_t now, int penaltyTermDays);

#endif // __GUILD_JOIN_DECISION_H__
