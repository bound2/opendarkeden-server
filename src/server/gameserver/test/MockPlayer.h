#ifndef __MOCK_PLAYER__
#define __MOCK_PLAYER__

#include "Player.h"
class Packet;

class MockPlayer : public Player {
public:
    MockPlayer() {}
    ~MockPlayer() noexcept {}

    void sendPacket(Packet* pPacket) {}
};

#endif
