//////////////////////////////////////////////////////////////////////////////
// Filename    : SharedPacketDispatch.h
// Description : registers every packet handler the sharedserver receives
//               (GS from gameservers) in the PacketDispatcher table. Call
//               once from main() before any player thread starts
//               (docs/RESTRUCTURING.md task 2.3).
//////////////////////////////////////////////////////////////////////////////

#ifndef __SHARED_PACKET_DISPATCH_H__
#define __SHARED_PACKET_DISPATCH_H__

void registerSharedServerPacketHandlers();

#endif
