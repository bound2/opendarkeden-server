//////////////////////////////////////////////////////////////////////////////
// Filename    : SharedPacketDispatch.cpp
// Description : the sharedserver composition root (docs/RESTRUCTURING.md
//               task 2.3): every GS (game -> shared) packet id is bound
//               to its handler here.
//////////////////////////////////////////////////////////////////////////////

#include "SharedPacketDispatch.h"

#include "GSAddGuild.h"
#include "GSAddGuildMember.h"
#include "GSExpelGuildMember.h"
#include "GSGuildMemberLogOn.h"
#include "GSModifyGuildIntro.h"
#include "GSModifyGuildMember.h"
#include "GSQuitGuild.h"
#include "GSRequestGuildInfo.h"
#include "PacketDispatcher.h"

void registerSharedServerPacketHandlers() {
    DE_REGISTER_PACKET_HANDLER(GSAddGuild);
    DE_REGISTER_PACKET_HANDLER(GSAddGuildMember);
    DE_REGISTER_PACKET_HANDLER(GSExpelGuildMember);
    DE_REGISTER_PACKET_HANDLER(GSGuildMemberLogOn);
    DE_REGISTER_PACKET_HANDLER(GSModifyGuildIntro);
    DE_REGISTER_PACKET_HANDLER(GSModifyGuildMember);
    DE_REGISTER_PACKET_HANDLER(GSQuitGuild);
    DE_REGISTER_PACKET_HANDLER(GSRequestGuildInfo);
}
