//////////////////////////////////////////////////////////////////////////////
// Filename    : login_handler_failure_test.cpp
// Description : What a login handler's throw does to its session and to the
//               manager's loop. A ProtocolException (a logout's
//               DisconnectException) and a ConnectException are the handlers'
//               ways to end a session, and the manager retires the player for
//               each; anything else a handler throws ends the session the same
//               way instead of leaving processCommands for the main loop.
//
//               The packet arrives over a real loopback connection, through a
//               factory, validator and handler registered in the child only.
//////////////////////////////////////////////////////////////////////////////

#include <poll.h>
#include <unistd.h>

#include <cstdlib>
#include <exception>
#include <memory>
#include <stdexcept>

#include <gtest/gtest.h>
#include <sys/socket.h>

#include "Exception.h"
#include "KernelContext.h"
#include "LCVersionCheckOK.h"
#include "LoginPlayerManager.h"
#include "PacketDispatcher.h"
#include "PacketFactoryManager.h"
#include "PacketValidator.h"
#include "Socket.h"
#include "SocketOutputStream.h"
#include "support/LoopbackListener.h"

namespace {

std::exception_ptr g_failure;

void throwFailure(Packet*, Player*) {
    std::rethrow_exception(g_failure);
}

bool readable(int descriptor) {
    pollfd ready{descriptor, POLLIN, 0};
    return ::poll(&ready, 1, 2000) == 1 && (ready.revents & POLLIN) != 0;
}

// Delivers one packet whose handler throws failure, and exits 0 when the
// player is retired and the manager's loop runs on.
void checkHandlerFailureRetiresThePlayer(std::exception_ptr failure) {
    g_failure = failure;
    PacketFactoryManager factories;
    factories.addFactory(new LCVersionCheckOKFactory());
    PacketValidator validator;
    auto allowed = std::make_unique<PacketIDSet>(LPS_BEGIN_SESSION);
    allowed->addPacketID(Packet::PACKET_LC_VERSION_CHECK_OK);
    validator.addPacketIDSet(LPS_BEGIN_SESSION, allowed.release());
    de::kernelContext().setPacketFactoryManager(&factories);
    de::kernelContext().setPacketValidator(&validator);
    PacketDispatcher::registerHandler(Packet::PACKET_LC_VERSION_CHECK_OK, throwFailure);

    LoopbackListener listener;
    LoginPlayerManager manager;
    auto incoming = std::make_unique<Socket>("127.0.0.1", listener.port());
    incoming->connect();
    const int peer = listener.accept();
    const int descriptor = incoming->getSOCKET();
    manager.acceptNewConnection(incoming.release());

    LCVersionCheckOK packet;
    SocketOutputStream wire(nullptr, 64);
    wire.writePacket(&packet);
    if (::send(peer, wire.getBuffer(), wire.length(), 0) != wire.length() || !readable(descriptor))
        std::_Exit(1);
    manager.pollSockets();
    manager.processInputs();
    try {
        manager.processCommands();
    } catch (...) {
        std::_Exit(2);
    }
    if (manager.size() != 0)
        std::_Exit(3);
    try {
        manager.processCommands();
    } catch (...) {
        std::_Exit(4);
    }
    ::close(peer);
    std::_Exit(0);
}

TEST(LoginHandlerFailure, AnyThrowFromAHandlerRetiresThePlayerAndTheLoopRunsOn) {
    const std::exception_ptr failures[] = {
        std::make_exception_ptr(RuntimeException("handler failed")),
        std::make_exception_ptr(std::runtime_error("handler failed")),
        std::make_exception_ptr(DisconnectException("logout")),
        std::make_exception_ptr(ConnectException("send failed")),
    };
    for (const std::exception_ptr& failure : failures)
        EXPECT_EXIT(checkHandlerFailureRetiresThePlayer(failure), ::testing::ExitedWithCode(0), "");
}

} // namespace
