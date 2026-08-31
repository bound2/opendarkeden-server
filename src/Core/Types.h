//////////////////////////////////////////////////////////////////////////////
// Filename    : Types.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __TYPES_H__
#define __TYPES_H__

#include "types/CreatureTypes.h"
#include "types/GuildTypes.h"
#include "types/ItemTypes.h"
#include "types/ObjectTypes.h"
#include "types/PlayerTypes.h"
#include "types/QuestTypes.h"
#include "types/ServerType.h"
#include "types/ShopTypes.h"
#include "types/SystemTypes.h"
#include "types/WarTypes.h"
#include "types/ZoneTypes.h"

// clang-format off
// Utility.h uses the sz*/BYTE/WORD types defined above, and its own
// include of Types.h is an empty no-op here (the guard is already set),
// so it must stay BELOW the types/ block — do not let clang-format sort
// it up.
#include "Utility.h"
// clang-format on

#ifndef __XMAS_EVENT_CODE__
#define __XMAS_EVENT_CODE__
#endif

// #define __UNDERWORLD__

// #ifdef __NETMARBLE_SERVER__
//	#define __OLD_GUILD_WAR__
// #endif

#endif
