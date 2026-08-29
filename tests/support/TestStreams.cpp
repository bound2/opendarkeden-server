//////////////////////////////////////////////////////////////////////
//
// Filename    : TestStreams.cpp
//
//////////////////////////////////////////////////////////////////////

#include "TestStreams.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "ServerSocket.h"
#include "Socket.h"

#include <gtest/gtest.h>

namespace wiretest {

std::vector<unsigned char> writeBody(const Packet& packet, uchar code) {
    // A NULL socket is fine: write() only touches the buffer; nothing is
    // flushed. A fresh stream starts at head 0, so the bytes are contiguous.
    SocketEncryptOutputStream oStream(NULL);
    oStream.setEncryptCode(code);
    packet.write(oStream);
    const char* pBuffer = oStream.getBuffer();
    return std::vector<unsigned char>(pBuffer, pBuffer + oStream.length());
}

std::string toHex(const std::vector<unsigned char>& bytes) {
    std::string hex;
    hex.reserve(bytes.size() * 2);
    static const char* digits = "0123456789abcdef";
    for (size_t i = 0; i < bytes.size(); i++) {
        hex.push_back(digits[bytes[i] >> 4]);
        hex.push_back(digits[bytes[i] & 0x0F]);
    }
    return hex;
}

static std::string goldenPath(const std::string& name, uchar code) {
    std::ostringstream path;
    path << WIRETEST_GOLDEN_DIR << "/" << name << ".code" << (int)code << ".hex";
    return path.str();
}

void expectGolden(const std::string& name, uchar code, const std::vector<unsigned char>& bytes) {
    const std::string path = goldenPath(name, code);
    const std::string actual = toHex(bytes);

    if (getenv("UPDATE_GOLDENS") != NULL) {
        std::ofstream file(path.c_str(), std::ios::trunc);
        ASSERT_TRUE(file.good()) << "cannot write golden file " << path;
        file << actual << "\n";
        std::printf("recorded golden %s\n", path.c_str());
        return;
    }

    std::ifstream file(path.c_str());
    ASSERT_TRUE(file.good()) << "missing golden file " << path
                             << " — run the tests once with UPDATE_GOLDENS=1 to record it, "
                             << "review the diff, and commit it";
    std::string expected;
    std::getline(file, expected);
    EXPECT_EQ(expected, actual) << "wire bytes changed for " << name << " (encrypt code " << (int)code
                                << ") — this is a protocol-breaking change unless the client ships "
                                << "the identical change; if intended, re-record with UPDATE_GOLDENS=1";
}

Loopback::Loopback()
    : m_pServerSocket(NULL), m_pClientSocket(NULL), m_pAcceptedSocket(NULL), m_pOut(NULL), m_pIn(NULL) {
    // Bind an ephemeral-ish port; walk forward on conflicts so parallel test
    // runs don't trip over each other.
    uint port = 43210;
    for (uint attempt = 0; attempt < 64; attempt++, port++) {
        try {
            m_pServerSocket = new ServerSocket(port, 1);
            break;
        } catch (...) {
            m_pServerSocket = NULL;
        }
    }
    if (m_pServerSocket == NULL)
        throw std::runtime_error("Loopback: could not bind any test port");

    m_pClientSocket = new Socket("127.0.0.1", port);
    m_pClientSocket->connect();
    m_pAcceptedSocket = m_pServerSocket->accept();
    if (m_pAcceptedSocket == NULL)
        throw std::runtime_error("Loopback: accept() returned NULL");

    m_pOut = new SocketEncryptOutputStream(m_pClientSocket);
    m_pIn = new SocketEncryptInputStream(m_pAcceptedSocket);
}

Loopback::~Loopback() {
    delete m_pOut;
    delete m_pIn;
    if (m_pClientSocket != NULL) {
        try {
            m_pClientSocket->close();
        } catch (...) {
        }
    }
    if (m_pAcceptedSocket != NULL) {
        try {
            m_pAcceptedSocket->close();
        } catch (...) {
        }
    }
    if (m_pServerSocket != NULL) {
        try {
            m_pServerSocket->close();
        } catch (...) {
        }
    }
    delete m_pClientSocket;
    delete m_pAcceptedSocket;
    delete m_pServerSocket;
}

void Loopback::setCodes(uchar code) {
    m_pOut->setEncryptCode(code);
    m_pIn->setEncryptCode(code);
}

void Loopback::pump(uint nBytes) {
    m_pOut->flush();
    while (m_pIn->length() < nBytes)
        m_pIn->fill();
}

void roundTrip(const Packet& src, Packet& dst, uchar code) {
    Loopback loopback;
    loopback.setCodes(code);
    src.write(loopback.out());
    loopback.pump(src.getPacketSize());
    dst.read(loopback.in());
}

} // namespace wiretest
