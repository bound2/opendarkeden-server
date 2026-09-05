//////////////////////////////////////////////////////////////////////////////
// Filename    : GCNPCAskVariable.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __GC_NPC_ASK_VARIABLE_H__
#define __GC_NPC_ASK_VARIABLE_H__

#include <map>

#include "Packet.h"
#include "PacketFactory.h"
#include "ScriptParameter.h"

//////////////////////////////////////////////////////////////////////////////
// class GCNPCAskVariable;
// Sends an NPC question with script variables to nearby PCs.
//////////////////////////////////////////////////////////////////////////////

typedef map<string, ScriptParameter*> HashMapScriptParameter;
typedef HashMapScriptParameter::iterator HashMapScriptParameterItor;
typedef HashMapScriptParameter::const_iterator HashMapScriptParameterConstItor;

class GCNPCAskVariable : public Packet {
public:
    GCNPCAskVariable();
    virtual ~GCNPCAskVariable();

public:
    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;
    PacketID_t getPacketID() const {
        return PACKET_GC_NPC_ASK_VARIABLE;
    }
    PacketSize_t getPacketSize() const;
    string getPacketName() const {
        return "GCNPCAskVariable";
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

    void addScriptParameter(ScriptParameter* pParam);
    void clearScriptParameters();
    HashMapScriptParameter& getScriptParameters() {
        return m_ScriptParameters;
    }
    string getValue(const string& name) const;

private:
    ObjectID_t m_ObjectID;                     // NPC's object id
    ScriptID_t m_ScriptID;                     // script id
    HashMapScriptParameter m_ScriptParameters; // script parameters
};

//////////////////////////////////////////////////////////////////////////////
// class GCNPCAskVariableFactory;
//////////////////////////////////////////////////////////////////////////////

class GCNPCAskVariableFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_GC_NPC_ASK_VARIABLE;
    static constexpr std::string_view kName = "GCNPCAskVariable";
    static constexpr PacketSize_t kMaxSize{szObjectID + szScriptID + szBYTE + ScriptParameter::getMaxSize() * 255};

    Packet* createPacket() override {
        return new GCNPCAskVariable();
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
