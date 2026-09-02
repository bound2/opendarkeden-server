//////////////////////////////////////////////////////////////////////////////
// Filename    : ScriptManager.cpp
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "ScriptManager.h"

#include "Assert.h"
#include "Properties.h"
#include "SXml.h"
#include "repository/ContentInfoRepository.h"

//////////////////////////////////////////////////////////////////////////////
// load from database with NPC ID
//////////////////////////////////////////////////////////////////////////////
void ScriptManager::load(const string& ownerID)

{
    __BEGIN_TRY

    // Every script, whatever the owner: the query never used ownerID.
    vector<ScriptRow> rows = defaultContentInfoRepository().loadScripts();
    //		XMLTree* pTree = new XMLTree("Scripts");

    for (size_t r = 0; r < rows.size(); r++) {
        ScriptID_t scriptID = rows[r].scriptID;
        string owner = rows[r].ownerID;
        string subject = rows[r].subject;
        string content = rows[r].content;
        Script* pScript = new Script;
        XMLTree* pChild = new XMLTree("Script");

        pScript->setScriptID(scriptID);
        pChild->AddAttribute("ScriptID", scriptID);
        //			pChild->AddAttribute("Owner", owner);

        if (m_XMLS[owner] == NULL)
            m_XMLS[owner] = new XMLTree("Scripts");

        string seperator = "**";
        size_t start = 0;
        size_t end = 0;
        string msg;

        ////////////////////////////////////////////////////////////
        // subject를 파싱한다.
        ////////////////////////////////////////////////////////////
        start = 0;
        end = 0;

        while (end < subject.size()) {
            start = end;
            end = subject.find(seperator, start);

            if (end == string::npos)
                end = subject.size();
            end++;

            msg = trim(subject.substr(start, end - start - 1));

            pScript->addSubject(msg);

            XMLTree* pSubject = new XMLTree("Subject");
            pSubject->SetText(msg);
            pChild->AddChild(pSubject);

            // cout << "SUBJECT:" << msg << endl;
        }

        ////////////////////////////////////////////////////////////
        // content를 파싱한다.
        ////////////////////////////////////////////////////////////
        start = 0;
        end = 0;
        uint answerid = 0;

        while (end < content.size()) {
            start = end;
            end = content.find(seperator, start);

            if (end == string::npos)
                end = content.size();
            end++;

            msg = trim(content.substr(start, end - start - 1));

            pScript->addContent(msg);

            end++;

            XMLTree* pContent = new XMLTree("Content");
            pContent->SetText(msg);
            pContent->AddAttribute("AuintnswerID", ++answerid);
            pChild->AddChild(pContent);

            // cout << "CONTENT:" << msg << endl;
        }

        setScript(scriptID, pScript);
        m_XMLS[owner]->AddChild(pChild);
        Assert(m_ScriptXMLs[scriptID] == NULL);
        m_ScriptXMLs[scriptID] = pChild;
        //			pTree->AddChild(pChild);
    }

    //		pTree->SaveToFile( (g_pConfig->getProperty("HomePath") + "/data/Script.xml").c_str() );
    //		SAFE_DELETE( pTree );

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// get script
//////////////////////////////////////////////////////////////////////////////
Script* ScriptManager::getScript(ScriptID_t scriptID) const {
    __BEGIN_TRY

    unordered_map<ScriptID_t, Script*>::const_iterator itr = m_Scripts.find(scriptID);

    if (itr == m_Scripts.end()) {
        StringStream msg;
        msg << "No Such Script(ID:" << scriptID << ") exist...";
        cerr << msg.toString() << endl;
        throw NoSuchElementException(msg.toString());
    }

    return itr->second;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// set script
//////////////////////////////////////////////////////////////////////////////
void ScriptManager::setScript(ScriptID_t scriptID, Script* pScript)

{
    __BEGIN_TRY

    unordered_map<ScriptID_t, Script*>::iterator itr = m_Scripts.find(scriptID);

    if (itr != m_Scripts.end()) {
        StringStream msg;
        msg << scriptID << " script already exist.";
        throw DuplicatedException(msg.toString());
    }

    m_Scripts[scriptID] = pScript;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// get debug string
//////////////////////////////////////////////////////////////////////////////
string ScriptManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "ScriptManager(";

    unordered_map<ScriptID_t, Script*>::const_iterator i = m_Scripts.begin();
    for (; i != m_Scripts.end(); i++) {
        msg << "(ScriptID:" << (int)(i->first) << ",";
        msg << i->second->toString() << ")";
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}

// global variable definition
ScriptManager* g_pPublicScriptManager = NULL;
