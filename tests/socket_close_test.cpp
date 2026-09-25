// SocketImpl::close() runs twice on every player socket: Player's destructor
// closes the socket and then deletes it, and ~SocketImpl closes again. The
// second close must not release the descriptor number a second time, because
// by then another thread may already own that number.
#include <unistd.h>

#include <gtest/gtest.h>
#include <sys/socket.h>

#include "Socket.h"
#include "SocketImpl.h"

namespace {
bool isOpen(int fd) {
    int type = 0;
    socklen_t length = sizeof(type);
    return ::getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, &length) == 0;
}
} // namespace

TEST(SocketImpl, CloseInvalidatesTheDescriptorAndIsIdempotent) {
    SocketImpl socket("127.0.0.1", 9);
    socket.create();
    ASSERT_TRUE(socket.isValid());
    socket.close();
    EXPECT_FALSE(socket.isValid());
    EXPECT_EQ(INVALID_SOCKET, socket.getSOCKET());
    EXPECT_NO_THROW(socket.close());
}

TEST(SocketImpl, SecondCloseLeavesAReusedDescriptorOpen) {
    SocketImpl first("127.0.0.1", 9);
    first.create();
    const int released = first.getSOCKET();
    first.close();
    // POSIX hands out the lowest free descriptor, so this receives the number
    // just released.
    const int reused = ::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(reused, 0);
    EXPECT_EQ(released, reused);
    ASSERT_TRUE(isOpen(reused));
    first.close();
    EXPECT_TRUE(isOpen(reused));
    ::close(reused);
}
