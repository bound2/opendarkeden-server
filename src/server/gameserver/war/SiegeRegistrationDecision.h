//////////////////////////////////////////////////////////////////////////////
// Filename    : SiegeRegistrationDecision.h
// Description : what a guild master's siege registration does to the castle's
//               next scheduled war. Kept apart from the quest action so it can
//               be exercised without a creature, a zone or a scheduler.
//////////////////////////////////////////////////////////////////////////////

#ifndef __SIEGE_REGISTRATION_DECISION_H__
#define __SIEGE_REGISTRATION_DECISION_H__

// A siege's five attacker slots, the width of the schedule row's
// AttackGuildID..AttackGuildID5 and of SiegeWar's own challenger array.
const unsigned int MaxSiegeChallengerGuilds = 5;

// The castle's next scheduled war, as the registration sees it. A castle's
// scheduler holds only that castle's wars, so the earliest schedule is the
// war a registration either joins or is turned away by.
struct SiegeRegistrationState {
    // A war already waits on this castle.
    bool hasScheduledWar = false;
    // ... and it is a siege, the one castle war class several guilds share.
    // A guild war is the other, applied for by one guild alone, and has no
    // slot for a second challenger.
    bool isSiege = false;
    // How many guilds have already joined that siege.
    unsigned int challengerCount = 0;
};

enum SiegeRegistrationAction {
    // Nothing waits on the castle: open a siege for this guild.
    SIEGE_REGISTRATION_CREATE,
    // A siege with room: add this guild to it.
    SIEGE_REGISTRATION_JOIN,
    // No room for this guild, either because the siege is full or because
    // the war waiting on the castle is a guild war nobody else may enter.
    // Answered with NPC_RESPONSE_WAR_SCHEDULE_FULL, the refusal a full
    // siege already gives and the one the reinforcement registration gives
    // when the castle's next war is not a siege.
    SIEGE_REGISTRATION_FULL
};

inline SiegeRegistrationAction decideSiegeRegistration(const SiegeRegistrationState& state) {
    if (!state.hasScheduledWar)
        return SIEGE_REGISTRATION_CREATE;
    if (!state.isSiege)
        return SIEGE_REGISTRATION_FULL;
    if (state.challengerCount < MaxSiegeChallengerGuilds)
        return SIEGE_REGISTRATION_JOIN;
    return SIEGE_REGISTRATION_FULL;
}

#endif // __SIEGE_REGISTRATION_DECISION_H__
