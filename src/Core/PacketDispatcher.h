//////////////////////////////////////////////////////////////////////////////
// Filename    : PacketDispatcher.h
// Description : packet-id -> handler dispatch table, filled in at each
//               app's composition root (docs/RESTRUCTURING.md task 2.3).
//               Replaces the per-packet virtual execute(): the kernel
//               keeps the wire classes, the app owns which handler runs.
//////////////////////////////////////////////////////////////////////////////

#ifndef __PACKET_DISPATCHER_H__
#define __PACKET_DISPATCHER_H__

#include "Packet.h"
#include "Types.h"

class Player;

class PacketDispatcher {
public:
    typedef void (*HandlerFn)(Packet* pPacket, Player* pPlayer);

    // Call at the composition root only, before any player thread runs.
    // Asserts on an out-of-range id and on double registration.
    static void registerHandler(PacketID_t packetID, HandlerFn fn);

    // Runs the registered handler for the packet's id. Returns false when
    // no handler is registered, so receive loops can fall back to the
    // legacy Packet::execute() until every direction is migrated. The
    // table is written only during startup, so this is safe to call from
    // every zone thread without locking.
    static bool dispatch(Packet* pPacket, Player* pPlayer);

private:
    static HandlerFn s_Handlers[];
};

#endif
