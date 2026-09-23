#ifndef __LOG_DEF__
#define __LOG_DEF__

#include "GameContext.h"
#include "VariableManager.h"

#define FILELOG_INCOMING_CONNECTION                                         \
    if (de::gameContext().variables().getVariable(LOG_INCOMING_CONNECTION)) \
    filelog

#endif
