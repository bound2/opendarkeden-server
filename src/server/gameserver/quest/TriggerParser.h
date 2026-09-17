#ifndef __TRIGGER_PARSER_H__
#define __TRIGGER_PARSER_H__

#include <map>

#include "SXml.h"
#include "Types.h"

namespace de {
class GameContext;
}

class XMLTree;

class TriggerParser {
public:
    explicit TriggerParser(de::GameContext& context)
        : m_Context(context), m_pTargetTree(NULL), m_TargetScriptID(0), m_TargetContentID(0) {}
    void parseTrigger(const string& type, const string& condition, const string& action);

    bool parseElement(XMLTree* pTree, const string& key, const string& element);
    bool findText(XMLTree* pTree);

    XMLTree* getResult() const {
        return m_pTargetTree;
    }

private:
    de::GameContext& m_Context;
    map<ScriptID_t, XMLTree*> m_ScriptMap;
    XMLTree* m_pTargetTree;
    uint m_TargetScriptID, m_TargetContentID;
};

#endif
