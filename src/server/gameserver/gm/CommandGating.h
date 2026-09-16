//////////////////////////////////////////////////////////////////////////////
// Filename    : CommandGating.h
// Description : The level a creature speaks a GM command with.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GM_COMMAND_GATING_H__
#define __GM_COMMAND_GATING_H__

#include "Creature.h"
#include "gm/CommandRouter.h"

namespace de::gm {

// GOD, DM and every other competence that is not PLAYER map onto their own
// level. A competence outside the four the enum names is treated as the
// weakest of the three operator levels, which is what the "not a PLAYER"
// gates always granted it.
inline Permission permissionOf(const Creature* pCreature) {
    if (pCreature->isGOD())
        return Permission::God;
    if (pCreature->isDM())
        return Permission::DM;
    if (!pCreature->isPLAYER())
        return Permission::Helper;
    return Permission::Everyone;
}

} // namespace de::gm

#endif // __GM_COMMAND_GATING_H__
