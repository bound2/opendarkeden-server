//////////////////////////////////////////////////////////////////////////////
// Filename    : GuildJoinDecision.cpp
// Description : the eligibility rules the guild NPC applies before a
//               character may found a guild or join one.
//////////////////////////////////////////////////////////////////////////////

#include "GuildJoinDecision.h"

#include <cstdlib>

#include "GCNPCResponse.h"

namespace {

typedef Outcome<void, GuildJoinRejection> VoidResult;

// The response codes of one race, in the order guildJoinResponseCode needs
// them. A zero entry means the conversation sends nothing for that reason.
struct RaceResponses {
    uint16_t startingQuitTimeout;
    uint16_t startingAlreadyJoin;
    uint16_t startingLevel;
    uint16_t startingMoney;
    uint16_t startingFame;
    uint16_t registName;
    uint16_t registDeny;
};

const RaceResponses kSlayerResponses = {NPC_RESPONSE_TEAM_STARTING_FAIL_QUIT_TIMEOUT,
                                        NPC_RESPONSE_TEAM_STARTING_FAIL_ALREADY_JOIN,
                                        NPC_RESPONSE_TEAM_STARTING_FAIL_LEVEL,
                                        NPC_RESPONSE_TEAM_STARTING_FAIL_MONEY,
                                        NPC_RESPONSE_TEAM_STARTING_FAIL_FAME,
                                        NPC_RESPONSE_TEAM_REGIST_FAIL_NAME,
                                        NPC_RESPONSE_TEAM_REGIST_FAIL_DENY};

const RaceResponses kVampireResponses = {NPC_RESPONSE_CLAN_STARTING_FAIL_QUIT_TIMEOUT,
                                         NPC_RESPONSE_CLAN_STARTING_FAIL_ALREADY_JOIN,
                                         NPC_RESPONSE_CLAN_STARTING_FAIL_LEVEL,
                                         NPC_RESPONSE_CLAN_STARTING_FAIL_MONEY,
                                         NPC_RESPONSE_CLAN_STARTING_FAIL_FAME,
                                         NPC_RESPONSE_CLAN_REGIST_FAIL_NAME,
                                         NPC_RESPONSE_CLAN_REGIST_FAIL_DENY};

const RaceResponses kOustersResponses = {NPC_RESPONSE_GUILD_STARTING_FAIL_QUIT_TIMEOUT,
                                         NPC_RESPONSE_GUILD_STARTING_FAIL_ALREADY_JOIN,
                                         NPC_RESPONSE_GUILD_STARTING_FAIL_LEVEL,
                                         NPC_RESPONSE_GUILD_STARTING_FAIL_MONEY,
                                         NPC_RESPONSE_GUILD_STARTING_FAIL_FAME,
                                         NPC_RESPONSE_GUILD_REGIST_FAIL_NAME,
                                         NPC_RESPONSE_GUILD_REGIST_FAIL_DENY};

const RaceResponses& responsesOf(GuildJoinRace race) {
    switch (race) {
    case GUILD_JOIN_RACE_VAMPIRE:
        return kVampireResponses;
    case GUILD_JOIN_RACE_OUSTERS:
        return kOustersResponses;
    case GUILD_JOIN_RACE_SLAYER:
    default:
        return kSlayerResponses;
    }
}

// The membership probe both join conversations make, expressed once: the
// row exists, and either its stamp still carries a penalty or it names a
// live membership.
VoidResult membershipGate(GuildJoinContext context, bool rowExists, const std::string& expireDate, time_t now,
                          int penaltyTermDays) {
    if (!rowExists)
        return VoidResult::Ok();

    if (expireDate.size() != kGuildExpireDateLength)
        return VoidResult::Rejected(GuildJoinRejection(context, GUILD_JOIN_REJECT_ALREADY_MEMBER));

    if (guildQuitPenaltyPending(expireDate, now, penaltyTermDays))
        return VoidResult::Rejected(GuildJoinRejection(context, GUILD_JOIN_REJECT_PENALTY_PENDING));

    return VoidResult::Ok();
}

} // namespace

bool guildJoinResponseCode(const GuildJoinRejection& rejection, GuildJoinRace race, uint16_t& code) {
    const RaceResponses& responses = responsesOf(race);

    switch (rejection.context) {
    case GUILD_JOIN_CONTEXT_STARTING:
        switch (rejection.reason) {
        // Both of these close the dialogue without a word of explanation.
        case GUILD_JOIN_REJECT_GUILD_MISSING:
        case GUILD_JOIN_REJECT_WAIT_LIST_FULL:
            code = NPC_RESPONSE_QUIT_DIALOGUE;
            return true;
        case GUILD_JOIN_REJECT_PENALTY_PENDING:
            code = responses.startingQuitTimeout;
            return true;
        case GUILD_JOIN_REJECT_ALREADY_MEMBER:
            code = responses.startingAlreadyJoin;
            return true;
        case GUILD_JOIN_REJECT_LEVEL_TOO_LOW:
            code = responses.startingLevel;
            return true;
        case GUILD_JOIN_REJECT_NOT_ENOUGH_GOLD:
            code = responses.startingMoney;
            return true;
        case GUILD_JOIN_REJECT_NOT_ENOUGH_FAME:
            code = responses.startingFame;
            return true;
        default:
            return false;
        }

    case GUILD_JOIN_CONTEXT_REGIST:
        switch (rejection.reason) {
        case GUILD_JOIN_REJECT_NAME_INVALID:
        case GUILD_JOIN_REJECT_NAME_IN_USE:
            code = responses.registName;
            return true;
        case GUILD_JOIN_REJECT_PENALTY_PENDING:
            code = responses.registDeny;
            return true;
        // The thresholds a founder misses are answered with silence: the
        // NPC simply does not create the guild.
        default:
            return false;
        }

    // Everything the confirmed join refuses is silent.
    case GUILD_JOIN_CONTEXT_CONFIRM:
    default:
        return false;
    }
}

bool guildQuitPenaltyPending(const std::string& expireDate, time_t now, int penaltyTermDays) {
    if (expireDate.size() != kGuildExpireDateLength)
        return false;

    // The three fields are read back into the same members that wrote them,
    // so the month stays zero based and the year keeps counting from 1900.
    // The time of day is midnight and daylight saving is taken as not in
    // effect.
    tm stamp = {};
    stamp.tm_year = atoi(expireDate.substr(0, 3).c_str());
    stamp.tm_mon = atoi(expireDate.substr(3, 2).c_str());
    stamp.tm_mday = atoi(expireDate.substr(5, 2).c_str());
    stamp.tm_hour = 0;
    stamp.tm_min = 0;
    stamp.tm_sec = 0;

    return difftime(now, mktime(&stamp)) < penaltyTermDays * 24 * 3600;
}

Outcome<void, GuildJoinRejection> decideGuildRequirements(GuildJoinContext context, const GuildJoinStats& stats,
                                                          const GuildJoinRequirements& requirements) {
    if (stats.level < requirements.level)
        return VoidResult::Rejected(GuildJoinRejection(context, GUILD_JOIN_REJECT_LEVEL_TOO_LOW));

    if (stats.gold < requirements.gold)
        return VoidResult::Rejected(GuildJoinRejection(context, GUILD_JOIN_REJECT_NOT_ENOUGH_GOLD));

    if (requirements.checkFame && stats.fame < requirements.fame)
        return VoidResult::Rejected(GuildJoinRejection(context, GUILD_JOIN_REJECT_NOT_ENOUGH_FAME));

    return VoidResult::Ok();
}

Outcome<void, GuildJoinRejection> decideGuildJoinAttempt(GuildRepository& repository, const GuildJoinAttempt& attempt) {
    if (!attempt.guildExists)
        return VoidResult::Rejected(GuildJoinRejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_GUILD_MISSING));

    std::string expireDate;
    bool rowExists = repository.loadMemberExpireDate(attempt.name, expireDate);

    VoidResult membership =
        membershipGate(GUILD_JOIN_CONTEXT_STARTING, rowExists, expireDate, attempt.now, attempt.penaltyTermDays);
    if (membership.isRejected())
        return membership;

    switch (attempt.rank) {
    case GUILD_JOIN_RANK_STARTING:
        return decideGuildRequirements(GUILD_JOIN_CONTEXT_STARTING, attempt.stats, attempt.requirements);

    case GUILD_JOIN_RANK_WAITING:
        if (attempt.waitMemberCount >= attempt.waitMemberLimit)
            return VoidResult::Rejected(
                GuildJoinRejection(GUILD_JOIN_CONTEXT_STARTING, GUILD_JOIN_REJECT_WAIT_LIST_FULL));
        return VoidResult::Ok();

    // Any other rank opens no dialogue and is refused nothing.
    case GUILD_JOIN_RANK_OTHER:
    default:
        return VoidResult::Ok();
    }
}

Outcome<GuildRegistrationClearance, GuildJoinRejection>
decideGuildRegistration(GuildRepository& repository, const GuildRegistrationRequest& request) {
    typedef Outcome<GuildRegistrationClearance, GuildJoinRejection> Result;

    // A quote or a backslash in the name is refused as if the name were
    // taken.
    if (request.guildName.find_first_of("'\\") < request.guildName.size())
        return Result::Rejected(GuildJoinRejection(GUILD_JOIN_CONTEXT_REGIST, GUILD_JOIN_REJECT_NAME_INVALID));

    if (repository.guildNameInUse(request.guildName))
        return Result::Rejected(GuildJoinRejection(GUILD_JOIN_CONTEXT_REGIST, GUILD_JOIN_REJECT_NAME_IN_USE));

    int rank = 0;
    std::string expireDate;

    GuildRegistrationClearance clearance;
    if (!repository.loadMemberRankExpireDate(request.name, rank, expireDate))
        return Result::Ok(clearance);

    // Only a voluntary quit carries the founding penalty; an expelled
    // character may found a guild at once. A row of any other shape is
    // stale and is cleared by the caller.
    if (expireDate.size() == kGuildExpireDateLength && rank == kGuildMemberRankLeave &&
        guildQuitPenaltyPending(expireDate, request.now, request.penaltyTermDays))
        return Result::Rejected(GuildJoinRejection(GUILD_JOIN_CONTEXT_REGIST, GUILD_JOIN_REJECT_PENALTY_PENDING));

    clearance.clearStaleMemberRow = true;
    return Result::Ok(clearance);
}

Outcome<void, GuildJoinRejection> decideGuildJoinConfirm(GuildRepository& repository, const std::string& name,
                                                         time_t now, int penaltyTermDays) {
    int guildID = 0;
    int rank = 0;
    std::string expireDate;

    bool rowExists = repository.loadMemberGuildRankExpireDate(name, guildID, rank, expireDate);

    return membershipGate(GUILD_JOIN_CONTEXT_CONFIRM, rowExists, expireDate, now, penaltyTermDays);
}
