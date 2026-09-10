//////////////////////////////////////////////////////////////////////////////
// Filename    : GCNPCAskDynamic.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_NPC_ASK_DYNAMIC_H__
#define __GC_NPC_ASK_DYNAMIC_H__

#include "Packet.h"
#include "PacketFactory.h"

//////////////////////////////////////////////////////////////////////////////
// class GCNPCAskDynamic;
// NPC 의 대사를 주변의 PC 들에게 전송한다.
//////////////////////////////////////////////////////////////////////////////

class GCNPCAskDynamic : public Packet {
public:
    GCNPCAskDynamic();
    virtual ~GCNPCAskDynamic();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_NPC_ASK_DYNAMIC;
    }
    // The choice count travels in one byte, a script holds at most
    // SCRIPT_MAX_CONTENTS choices, and the factory max budgets this many.
    static constexpr uint kMaxCount = 15;

    PacketSize_t getPacketSize() const {
        PacketSize_t size = 0;

        size += szObjectID;                // npc object id
        size += szScriptID;                // script id size
        size += szWORD + m_Subject.size(); // subject length & actual string
        size += szBYTE;                    // contents count

        list<string>::const_iterator itr = m_Contents.begin();
        for (; itr != m_Contents.end(); itr++)
            size += (szWORD + (*itr).size()); // contents length & actual string

        return size;
    }
    string getPacketName() const {
        return "GCNPCAskDynamic";
    }
    string toString() const;

public:
    ObjectID_t getObjectID(void) const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t creatureID) {
        m_ObjectID = creatureID;
    }

    ScriptID_t getScriptID(void) const {
        return m_ScriptID;
    }
    void setScriptID(ScriptID_t id) {
        m_ScriptID = id;
    }

    string getSubject(void) const {
        return m_Subject;
    }
    void setSubject(string subject) {
        m_Subject = subject;
    }

    BYTE getContentsCount(void) const {
        return (BYTE)m_Contents.size();
    }

    void addContent(string content);
    string popContent(void);

private:
    ObjectID_t m_ObjectID = 0; // NPC's object id
    ScriptID_t m_ScriptID = 0; // script id
    string m_Subject;          // subject
    list<string> m_Contents;   // actual content
};

//////////////////////////////////////////////////////////////////////////////
// class GCNPCAskDynamicFactory;
//////////////////////////////////////////////////////////////////////////////

class GCNPCAskDynamicFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_NPC_ASK_DYNAMIC;
    static constexpr std::string_view kName = "GCNPCAskDynamic";
    static constexpr PacketSize_t kMaxSize{[] {
        PacketSize_t size = 0;

        size += szObjectID;                                   // npc object id
        size += szScriptID;                                   // script id size
        size += szWORD + 1024;                                // subject length & actual string
        size += szBYTE;                                       // contents count
        size += (szWORD + 1024) * GCNPCAskDynamic::kMaxCount; // contents length & actual strings

        return size;
    }()};

    Packet* createPacket() override {
        return new GCNPCAskDynamic();
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
