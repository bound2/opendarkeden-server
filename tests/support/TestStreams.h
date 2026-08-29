//////////////////////////////////////////////////////////////////////
//
// Filename    : TestStreams.h
// Description : Wire-contract test support. Serializes packets through
//               the real stream classes and moves bytes through a real
//               loopback TCP connection, so the tests exercise exactly
//               the path the servers use — no test doubles in the wire
//               layer.
//
//////////////////////////////////////////////////////////////////////

#ifndef __TEST_STREAMS_H__
#define __TEST_STREAMS_H__

#include <string>
#include <vector>

#include "Packet.h"
#include "SocketEncryptInputStream.h"
#include "SocketEncryptOutputStream.h"

class ServerSocket;
class Socket;

namespace wiretest {

// Serialize a packet body exactly as a session with encrypt code `code`
// would put it on the wire (0 = the unencrypted branch). Returns the raw
// buffer contents; nothing is flushed to any socket.
std::vector<unsigned char> writeBody(const Packet& packet, uchar code);

std::string toHex(const std::vector<unsigned char>& bytes);

// Golden pin: compares `bytes` against tests/golden/<name>.code<code>.hex.
// Set UPDATE_GOLDENS=1 in the environment to (re)record the file instead
// of comparing. A missing golden fails with instructions.
void expectGolden(const std::string& name, uchar code, const std::vector<unsigned char>& bytes);

// A real loopback TCP connection built from the project's own socket
// classes: the output stream sits on the connecting side, the input
// stream on the accepted side — the same pairing a live client/server
// session has.
class Loopback {
public:
    Loopback();
    ~Loopback();

    SocketEncryptOutputStream& out() {
        return *m_pOut;
    }
    SocketEncryptInputStream& in() {
        return *m_pIn;
    }

    // Set the same encrypt code on both ends, like the session handshake does.
    void setCodes(uchar code);

    // Flush the output stream, then fill the input stream until at least
    // `nBytes` are buffered.
    void pump(uint nBytes);

private:
    ServerSocket* m_pServerSocket;
    Socket* m_pClientSocket;
    Socket* m_pAcceptedSocket;
    SocketEncryptOutputStream* m_pOut;
    SocketEncryptInputStream* m_pIn;
};

// Write `src` through a fresh loopback connection with encrypt code `code`
// and read it back into `dst`.
void roundTrip(const Packet& src, Packet& dst, uchar code);

} // namespace wiretest

#endif
