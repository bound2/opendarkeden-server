//////////////////////////////////////////////////////////////////////////////
// Filename    : GCGQuestStatusModify.h
// Written By  : elca@ewestsoft.com
// Description :
// Class definition for the packet sent when a skill succeeds
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_GQUEST_STATUS_MODIFY_H__
#define __GC_GQUEST_STATUS_MODIFY_H__

#include "Exception.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "QuestStatusInfo.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class GCGQuestStatusModify;
// Class the game server uses to tell the client that its own skill succeeded
//////////////////////////////////////////////////////////////////////////////

class GCGQuestStatusModify : public Packet {
public:
    enum ModifyType {
        NO_MODIFY, // The status did not change
        CURRENT,   // The quest started
        SUCCESS,   // The quest succeeded
        FAIL,      // The quest failed
    };

    GCGQuestStatusModify();
    ~GCGQuestStatusModify();

public:
    void read(SocketInputStream& iStream) {
        // The record replaces the one the packet holds.
        clearInfo();

        iStream.read(m_Type);
        m_pInfo = new QuestStatusInfo(0);
        m_bOwnsInfo = true;
        m_pInfo->read(iStream);
    }
    void write(SocketOutputStream& oStream) const {
        if (m_pInfo == NULL)
            throw InvalidProtocolException("quest status record missing");

        oStream.write(m_Type);
        m_pInfo->write(oStream);
    }
    PacketID_t getPacketID() const {
        return PACKET_GC_GQUEST_STATUS_MODIFY;
    }
    PacketSize_t getPacketSize() const {
        if (m_pInfo == NULL)
            throw InvalidProtocolException("quest status record missing");
        return szBYTE + m_pInfo->getSize();
    }
    string getPacketName() const {
        return "GCGQuestStatusModify";
    }
    string toString() const;

public:
    BYTE getType() const {
        return m_Type;
    }
    void setType(BYTE type) {
        m_Type = type;
    }

    QuestStatusInfo* getInfo() const {
        return m_pInfo;
    }

    // A sender keeps the record it hands over; only the one read()
    // allocates belongs to the packet.
    void setInfo(QuestStatusInfo* pInfo) {
        clearInfo();
        m_pInfo = pInfo;
    }

private:
    void clearInfo() {
        if (m_bOwnsInfo)
            delete m_pInfo;

        m_pInfo = NULL;
        m_bOwnsInfo = false;
    }

    BYTE m_Type = 0;
    QuestStatusInfo* m_pInfo = NULL;
    bool m_bOwnsInfo = false;
};


//////////////////////////////////////////////////////////////////////////////
// class GCGQuestStatusModifyFactory;
//////////////////////////////////////////////////////////////////////////////

class GCGQuestStatusModifyFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_GQUEST_STATUS_MODIFY;
    static constexpr std::string_view kName = "GCGQuestStatusModify";
    static constexpr PacketSize_t kMaxSize{szBYTE + QuestStatusInfo::getMaxSize()};

    GCGQuestStatusModifyFactory() {}
    virtual ~GCGQuestStatusModifyFactory() {}

public:
    Packet* createPacket() override {
        return new GCGQuestStatusModify();
    }
    string getPacketName() const override {
        return string(kName);
    }
    PacketID_t getPacketID() const override {
        return kPacketID;
    }
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};

#endif
