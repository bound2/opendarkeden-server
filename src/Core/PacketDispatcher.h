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

    // Runs the registered handler for the packet's id; receiving an id
    // with no registered handler is a protocol error and throws
    // InvalidProtocolException. The table is written only during startup,
    // so this is safe to call from every zone thread without locking.
    static void dispatch(Packet* pPacket, Player* pPlayer);

private:
    static HandlerFn s_Handlers[];
};

// Registration helpers for the composition roots: bind packet class Cls
// to Cls##Handler::execute, preserving the exact call the packet's own
// execute() used to make before task 2.3. The _NOPLAYER form is for
// handlers that take only the packet (the inter-server directions).
#define DE_REGISTER_PACKET_HANDLER(Cls)                                       \
    {                                                                         \
        struct Thunk {                                                        \
            static void call(Packet* pPacket, Player* pPlayer) {              \
                Cls##Handler::execute(static_cast<Cls*>(pPacket), pPlayer);   \
            }                                                                 \
        };                                                                    \
        PacketDispatcher::registerHandler(Cls().getPacketID(), &Thunk::call); \
    }

#define DE_REGISTER_PACKET_HANDLER_NOPLAYER(Cls)                              \
    {                                                                         \
        struct Thunk {                                                        \
            static void call(Packet* pPacket, Player*) {                      \
                Cls##Handler::execute(static_cast<Cls*>(pPacket));            \
            }                                                                 \
        };                                                                    \
        PacketDispatcher::registerHandler(Cls().getPacketID(), &Thunk::call); \
    }

#endif
