//////////////////////////////////////////////////////////////////////////////
// Filename    : PacketDispatcher.cpp
//////////////////////////////////////////////////////////////////////////////

#include "PacketDispatcher.h"

#include "Assert.h"

PacketDispatcher::HandlerFn PacketDispatcher::s_Handlers[Packet::PACKET_MAX] = {0};

void PacketDispatcher::registerHandler(PacketID_t packetID, HandlerFn fn) {
    Assert(packetID < Packet::PACKET_MAX);
    Assert(fn != 0);
    Assert(s_Handlers[packetID] == 0);
    s_Handlers[packetID] = fn;
}

void PacketDispatcher::dispatch(Packet* pPacket, Player* pPlayer) {
    PacketID_t packetID = pPacket->getPacketID();
    if (packetID >= Packet::PACKET_MAX)
        throw InvalidProtocolException("packet id out of range");

    HandlerFn fn = s_Handlers[packetID];
    if (fn == 0)
        throw InvalidProtocolException("packet has no registered handler");

    fn(pPacket, pPlayer);
}
