//////////////////////////////////////////////////////////////////////////////
// Filename    : GuildStepRunner.h
// Description : performs the steps a guild decision answered, so the expel
//               and the quit paths write and send exactly the same things in
//               exactly the same order.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GUILD_STEP_RUNNER_H__
#define __GUILD_STEP_RUNNER_H__

#include <vector>

#include "GuildDecision.h"

class Guild;

// Runs the steps in order: the character-side writes go to the
// SharedGuildRepository, the roster and guild mutations to pGuild, and the
// packets go out to every game server.
//
// departureLabel is the word GuildExit.log and GuildBroken.log name the
// departure with - "Expel" or "Quit"; a step list with no log step needs
// none.
//
// A DeleteGuild step frees pGuild, so the caller may not read it afterwards.
void runGuildSteps(const std::vector<SharedGuildStep>& steps, Guild* pGuild, const char* departureLabel = "");

#endif // __GUILD_STEP_RUNNER_H__
