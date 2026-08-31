//////////////////////////////////////////////////////////////////////////////
// Filename    : LoginPacketDispatch.h
// Description : registers every packet handler the loginserver receives
//               (CL from clients, GL datagrams from gameservers) in the
//               PacketDispatcher table. Call once from main() before any
//               player thread starts (docs/RESTRUCTURING.md task 2.3).
//////////////////////////////////////////////////////////////////////////////

#ifndef __LOGIN_PACKET_DISPATCH_H__
#define __LOGIN_PACKET_DISPATCH_H__

void registerLoginServerPacketHandlers();

#endif
