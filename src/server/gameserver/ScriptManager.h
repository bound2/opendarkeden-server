//////////////////////////////////////////////////////////////////////////////
// Filename    : ScriptManager.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SCRIPT_MANAGER_H__
#define __SCRIPT_MANAGER_H__

#include <unordered_map>

#include "Exception.h"
#include "Script.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ScriptManager
//
// Script manager for the public script shared by every NPC, or for the private
// script db used by one specific NPC.
//////////////////////////////////////////////////////////////////////////////

class XMLTree;

class ScriptManager {
public:
    void init() {
        load("PUBLIC");
    }
    void load(const string& name);

public:
    Script* getScript(ScriptID_t scriptID) const;
    void setScript(ScriptID_t scriptID, Script* pScript);

    string toString() const;

    // Do not use this.
    XMLTree* getXML(const string& owner) {
        return m_XMLS[owner];
    }
    XMLTree* getScriptXML(ScriptID_t scriptID) {
        return m_ScriptXMLs[scriptID];
    }

private:
    unordered_map<ScriptID_t, Script*> m_Scripts; // hash map of script
    unordered_map<string, XMLTree*> m_XMLS;
    unordered_map<ScriptID_t, XMLTree*> m_ScriptXMLs;
};

#endif
