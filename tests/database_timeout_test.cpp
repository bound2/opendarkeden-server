#include <poll.h>
#include <unistd.h>

#include <chrono>
#include <mutex>
#include <thread>

#include <condition_variable>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "Connection.h"

class DatabaseTimeoutTest : public testing::Test {
    void TearDown() override {
        // Also runs after a fatal assertion in either test.
        unsetenv("DARKEDEN_DB_CONNECT_TIMEOUT_SECONDS");
        unsetenv("DARKEDEN_DB_IO_TIMEOUT_SECONDS");
    }
};

// Accept TCP but never send a MySQL greeting. No external database, network
// route or credentials are involved: this reproduces blocked initialization.
TEST_F(DatabaseTimeoutTest, SilentPeerCannotBlockConnectionIndefinitely) {
    ASSERT_EQ(setenv("DARKEDEN_DB_CONNECT_TIMEOUT_SECONDS", "1", 1), 0);
    ASSERT_EQ(setenv("DARKEDEN_DB_IO_TIMEOUT_SECONDS", "1", 1), 0);
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(listener, 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ASSERT_EQ(::bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)), 0);
    ASSERT_EQ(listen(listener, 1), 0);
    socklen_t size = sizeof(address);
    ASSERT_EQ(getsockname(listener, reinterpret_cast<sockaddr*>(&address), &size), 0);
    std::jthread peer([listener](std::stop_token token) {
        pollfd ready{listener, POLLIN, 0};
        while (!token.stop_requested()) {
            if (poll(&ready, 1, 10) <= 0)
                continue;
            int client = accept(listener, nullptr, nullptr);
            std::mutex mutex;
            std::condition_variable_any wake;
            std::unique_lock lock(mutex);
            wake.wait(lock, token, [] { return false; });
            close(client);
            break;
        }
    });
    Connection connection;
    const auto start = std::chrono::steady_clock::now();
    EXPECT_THROW(connection.connect("127.0.0.1", "unused", "unused", "unused", ntohs(address.sin_port)),
                 SQLConnectException);
    EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::seconds(5));
    // mysql_real_connect resets options after a failed handshake. Verify the
    // observed bounded operation, rather than querying that cleared handle.
    peer.request_stop();
    peer.join();
    close(listener);
}

TEST_F(DatabaseTimeoutTest, InvalidTimeoutFailsBeforeConnecting) {
    Connection connection;
    ASSERT_EQ(setenv("DARKEDEN_DB_IO_TIMEOUT_SECONDS", "invalid", 1), 0);
    EXPECT_THROW(connection.connect("127.0.0.1", "unused", "unused", "unused", 1), SQLConnectException);
}
