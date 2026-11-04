//////////////////////////////////////////////////////////////////////////////
// Filename    : game_zone_database_failure_test.cpp
// Description : A player whose command processing fails on a statement
//               leaves the zone group and the tick goes on:
//               ZonePlayerManager::processCommands stops the DatabaseError,
//               logs it and moves the player out, and the incoming manager's
//               cleanup disconnects it as any other leaving player. The
//               contexts are process-wide, so the case runs in its own
//               process, in a scratch directory for the log it writes.
//////////////////////////////////////////////////////////////////////////////

#include <unistd.h>

#include <cstdlib>
#include <memory>

#include <gtest/gtest.h>

#include "DatabaseError.h"
#include "GameContext.h"
#include "GamePlayer.h"
#include "IncomingPlayerManager.h"
#include "KernelContext.h"
#include "Properties.h"
#include "Socket.h"
#include "ZonePlayerManager.h"
#include "support/LoopbackListener.h"

namespace {

struct Cleanup {
    unsigned destroyed = 0;
    unsigned disconnected = 0;
};

// A player whose every command processing fails on a statement.
class FailingPlayer : public GamePlayer {
public:
    FailingPlayer(Socket* socket, Cleanup& cleanup) : GamePlayer(socket), m_Cleanup(cleanup) {}
    ~FailingPlayer() noexcept override {
        ++m_Cleanup.destroyed;
    }
    void processCommand(bool) override {
        ++commands;
        throw DatabaseError("UPDATE Slayer failed");
    }
    void disconnect(bool) override {
        ++m_Cleanup.disconnected;
        setPlayerStatus(GPS_END_SESSION);
    }
    unsigned commands = 0;

private:
    Cleanup& m_Cleanup;
};

} // namespace

TEST(GameZoneDatabaseFailure, AFailedStatementMovesThePlayerOutAndTheTickGoesOn) {
    ASSERT_EXIT(
        {
            char directory[] = "/tmp/darkeden-zonefail-XXXXXX";
            if (!::mkdtemp(directory) || ::chdir(directory) != 0)
                std::_Exit(80);
            Properties config;
            de::kernelContext().setConfig(&config);
            de::GameContext context;
            IncomingPlayerManager incoming(std::make_unique<ServerSocket>(0), context);
            de::gameContext().setIncomingPlayerManager(&incoming);
            ZonePlayerManager zone;
            LoopbackListener listener;
            auto socket = std::make_unique<Socket>("127.0.0.1", listener.port());
            socket->connect();
            const int peer = listener.accept();
            Cleanup cleanup;
            FailingPlayer* player = new FailingPlayer(socket.release(), cleanup);
            player->setPlayerStatus(GPS_NORMAL);
            zone.addPlayer(player);

            // The failure is stopped in the tick and the player is out of the
            // table: the next tick does not reach it.
            zone.processCommands();
            if (player->commands != 1)
                std::_Exit(1);
            zone.processCommands();
            if (player->commands != 1)
                std::_Exit(2);

            // The outgoing player reaches the incoming manager, whose cleanup
            // disconnects and frees it.
            zone.heartbeat();
            incoming.clearPlayers();
            if (cleanup.disconnected != 1 || cleanup.destroyed != 1)
                std::_Exit(3);
            ::close(peer);
            std::_Exit(0);
        },
        ::testing::ExitedWithCode(0), "");
}
