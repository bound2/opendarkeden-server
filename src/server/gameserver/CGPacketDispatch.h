//////////////////////////////////////////////////////////////////////////////
// Filename    : CGPacketDispatch.h
// Description : registers every CG (client->game) packet handler in the
//               PacketDispatcher table. Call once from main() before any
//               player thread starts (docs/RESTRUCTURING.md task 2.3).
//////////////////////////////////////////////////////////////////////////////

#ifndef __CG_PACKET_DISPATCH_H__
#define __CG_PACKET_DISPATCH_H__

void registerCGPacketHandlers();

#endif
