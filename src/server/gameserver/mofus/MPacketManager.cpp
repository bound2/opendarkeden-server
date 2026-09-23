/////////////////////////////////////////////////////////////////////////////
// Filename	: MPacketManager.cpp
/////////////////////////////////////////////////////////////////////////////

// include files
#include "MPacketManager.h"

#include "Assert.h"
#include "MPacketHandler.h"
#include "MPacketID.h"
#include "Mofus.h"

// send packets
#include "PKTConnectAsk.h"
#include "PKTLogout.h"
#include "PKTReceiveOK.h"
#include "PKTResult.h"
#include "PKTSError.h"
#include "PKTUserInfo.h"

// receive packets
#include "PKTConnectAccept.h"
#include "PKTConnectAcceptHandler.h"
#include "PKTError.h"
#include "PKTErrorHandler.h"
#include "PKTPowerPoint.h"
#include "PKTPowerPointHandler.h"


// macro that keeps registering a packet ID short
#define REGISTER_SEND_PACKET_ID(PACKET, PACKET_ID) \
    MPacketID_t PACKET::getID() const {            \
        return PACKET_ID;                          \
    }

// macro that keeps registering a packet handler ID short
#define REGISTER_RECV_PACKET_ID(PACKET, PACKET_ID) \
    MPacketID_t PACKET::getID() const {            \
        return PACKET_ID;                          \
    }                                              \
    MPacketID_t PACKET##Handler::getID() const {   \
        return PACKET_ID;                          \
    }

// register the outgoing packet IDs
REGISTER_SEND_PACKET_ID(PKTConnectAsk, PTC_CONNECT_ASK)
REGISTER_SEND_PACKET_ID(PKTLogout, PTC_LOGOUT)
REGISTER_SEND_PACKET_ID(PKTUserInfo, PTC_USERINFO)
REGISTER_SEND_PACKET_ID(PKTReceiveOK, PTC_RECEIVE_OK)
REGISTER_SEND_PACKET_ID(PKTResult, PTC_RESULT)
REGISTER_SEND_PACKET_ID(PKTSError, PTC_ERROR)

// register the incoming packet IDs and the handler IDs
REGISTER_RECV_PACKET_ID(PKTConnectAccept, PTS_CONNECT_ACCEPT)
REGISTER_RECV_PACKET_ID(PKTPowerPoint, PTS_POWERPOINT)
REGISTER_RECV_PACKET_ID(PKTError, PTS_ERROR)


// internal implementation data
struct MPacketManager::IMPL {
    MPacket* pCreators[PTC_MAX];
    MPacketHandler* pHandlers[PTC_MAX];

    IMPL();
    ~IMPL();

    // Adds a packet creator.
    void addCreator(MPacket* pPacket);

    // Adds a packet handler.
    void addHandler(MPacketHandler* pHandler);
};


// constructor
MPacketManager::MPacketManager() : m_pImpl(new IMPL) {
    Assert(m_pImpl != NULL);
}

// destructor
MPacketManager::~MPacketManager() {
    SAFE_DELETE(m_pImpl);
}

// initialization
void MPacketManager::init() {
    // add the handlers and the creators
    m_pImpl->addCreator(new PKTConnectAccept);
    m_pImpl->addHandler(new PKTConnectAcceptHandler);
    m_pImpl->addCreator(new PKTPowerPoint);
    m_pImpl->addHandler(new PKTPowerPointHandler);
    m_pImpl->addCreator(new PKTError);
    m_pImpl->addHandler(new PKTErrorHandler);
}

// Adds a packet creator.
void MPacketManager::addCreator(MPacket* pPacket) {
    Assert(pPacket != NULL);
    m_pImpl->addCreator(pPacket);
}

// Adds a packet handler.
void MPacketManager::addHandler(MPacketHandler* pHandler) {
    Assert(pHandler != NULL);
    m_pImpl->addHandler(pHandler);
}

// Creates a new packet and returns it.
MPacket* MPacketManager::createPacket(MPacketID_t ID) const {
    if (ID < 0 || ID >= PTC_MAX) {
        filelog(MOFUS_ERROR_FILE, "MPacketManager::createPacket() out of ID");
        Assert(false);
    }

    return m_pImpl->pCreators[ID]->create();
}

// Runs the packet.
void MPacketManager::execute(MPlayer* pPlayer, MPacket* pPacket) {
    Assert(pPlayer != NULL);
    Assert(pPacket != NULL);

    MPacketID_t ID = pPacket->getID();

    if (ID < 0 || ID >= PTC_MAX) {
        filelog(MOFUS_ERROR_FILE, "MPacketManager::createPacket() out of ID");
        Assert(false);
    }

    if (m_pImpl->pHandlers[ID] == NULL) {
        filelog(MOFUS_ERROR_FILE, "MPacketManager::execute() Handler is NULL");
        Assert(false);
    }

    m_pImpl->pHandlers[ID]->execute(pPlayer, pPacket);
}

// Runs the packet.
bool MPacketManager::hasHandler(MPacketID_t ID) const {
    // First check the range
    if (ID < 0 || ID >= PTC_MAX) {
        return false;
    }

    // Check that a handler is there
    if (m_pImpl->pHandlers[ID] == NULL) {
        return false;
    }

    return true;
}

// Returns the packet's size.
MPacketSize_t MPacketManager::getPacketSize(MPacketID_t ID) const {
    if (ID < 0 || ID >= PTC_MAX) {
        filelog(MOFUS_ERROR_FILE, "MPacketManager::createPacket() out of ID");
        Assert(false);
    }

    return m_pImpl->pCreators[ID]->getSize();
}

// constructor
MPacketManager::IMPL::IMPL() {
    // Initialize each array.
    for (MPacketID_t i = 0; i < PTC_MAX; ++i) {
        pCreators[i] = NULL;
        pHandlers[i] = NULL;
    }
}

// destructor
MPacketManager::IMPL::~IMPL() {
    // Delete the packet creators and handlers.
    for (MPacketID_t i = 0; i < PTC_MAX; ++i) {
        SAFE_DELETE(pCreators[i]);
        SAFE_DELETE(pHandlers[i]);
    }
}

// Adds a packet creator.
void MPacketManager::IMPL::addCreator(MPacket* pPacket) {
    Assert(pPacket != NULL);

    // Duplicate check
    if (pCreators[pPacket->getID()] != NULL) {
        filelog(MOFUS_ERROR_FILE, "MPacketManager::IMPL::addCreator() dup creator");
        Assert(false);
    }

    // Add the creator.
    pCreators[pPacket->getID()] = pPacket;
}

// Adds a packet handler.
void MPacketManager::IMPL::addHandler(MPacketHandler* pHandler) {
    Assert(pHandler != NULL);

    // Duplicate check
    if (pHandlers[pHandler->getID()] != NULL) {
        filelog(MOFUS_ERROR_FILE, "MPacketManager::IMPL::addHandler() dup handler");
        Assert(false);
    }

    // Add the handler.
    pHandlers[pHandler->getID()] = pHandler;
}
