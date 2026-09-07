////////////////////////////////////////////////////////////////////////////////
// Filename    : TriggerManager.cpp
// Written By  :
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "TriggerManager.h"

#include <stdio.h>

#include "Assert.h"
#include "Properties.h"
#include "ScriptManager.h"
#include "TriggerParser.h"
#include "repository/ContentInfoRepository.h"
#include "repository/ZoneInfoRepository.h"

class isSameTriggerID {
public:
    isSameTriggerID(TriggerID_t triggerID) : m_TriggerID(triggerID) {}
    bool operator()(Trigger* pTrigger) {
        return pTrigger->getTriggerID() == m_TriggerID;
    }

private:
    TriggerID_t m_TriggerID;
};

////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////
TriggerManager::TriggerManager()

    {__BEGIN_TRY __END_CATCH}


////////////////////////////////////////////////////////////////////////////////
// destructor
////////////////////////////////////////////////////////////////////////////////
TriggerManager::~TriggerManager()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


////////////////////////////////////////////////////////////////////////////////
// NPC 이름을 파라미터로 주면, NPC와 관련된 트리거들을 DB에서 로딩한다.
////////////////////////////////////////////////////////////////////////////////
void TriggerManager::load(const string& name)

{
    __BEGIN_TRY

    //	TriggerParser parser;

    vector<NPCTriggerRow> rows = defaultContentInfoRepository().loadNPCTriggers(name);

    for (vector<NPCTriggerRow>::const_iterator it = rows.begin(); it != rows.end(); ++it) {
        Trigger* pTrigger = new Trigger();

        pTrigger->setTriggerID(it->triggerID);

        // cout << "Trigger[" << pTrigger->getTriggerID() << "] loading > ";
        // cout << "CONDITIONS:\n" << trim(it->conditions) << endl;
        // cout << "ACTIONS:\n" << trim(it->actions) << endl;

        pTrigger->setTriggerType(trim(it->triggerType));
        pTrigger->setConditions(trim(it->conditions));
        pTrigger->setActions(trim(it->actions));

        addTrigger(pTrigger);

        //			parser.parseTrigger(trim(it->triggerType), trim(it->conditions),
        // trim(it->actions));

        // cout << "Trigger[" << pTrigger->getTriggerID() << "] loaded" <<  endl;
    }

    //	XMLTree* pXML = parser.getResult();
    //	if ( pXML != NULL )
    //	{
    //		pXML->SaveToFile((g_pConfig->getProperty("HomePath") + "/data/" + name + ".xml").c_str());
    //		SAFE_DELETE( pXML );
    //	}

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////////////
// 존 좌표를 파라미터로 주면, 그 좌표와 관련된 트리거들을 DB에서 로딩한다.
////////////////////////////////////////////////////////////////////////////////
void TriggerManager::load(ZoneID_t zoneid, int left, int top, int right, int bottom) {
    __BEGIN_TRY

    vector<ZoneTriggerRow> rows = defaultZoneInfoRepository().loadZoneTriggers((int)zoneid, left, top, right, bottom);

    for (vector<ZoneTriggerRow>::const_iterator it = rows.begin(); it != rows.end(); ++it) {
        Trigger* pTrigger = new Trigger();

        pTrigger->setTriggerID(it->triggerID);

        // printf("ZoneTrigger[%d] loading > \n", (int)pTrigger->getTriggerID());

        pTrigger->setTriggerType(trim(it->triggerType));
        pTrigger->setConditions(trim(it->conditions));
        pTrigger->setActions(trim(it->actions));
        pTrigger->setCounterActions(trim(it->counterActions));

        // printf("ZoneTrigger[%d] loaded > \n", (int)pTrigger->getTriggerID());

        addTrigger(pTrigger);
    }

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// refresh condition set
////////////////////////////////////////////////////////////////////////////////
void TriggerManager::refresh() {
    __BEGIN_TRY

    // 소속된 모든 트리거들의 ConditionSet 을 m_ConditionSet 에 OR 연산한다.
    for (list<Trigger*>::const_iterator itr = m_Triggers.begin(); itr != m_Triggers.end(); itr++) {
        m_ConditionSet |= (*itr)->getConditionSet();
    }

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// add trigger
////////////////////////////////////////////////////////////////////////////////
void TriggerManager::addTrigger(Trigger* pTrigger) {
    __BEGIN_TRY

    Assert(pTrigger != NULL);

    // auto itr = find(m_Triggers.begin() , m_Triggers.end(), pTrigger);
    list<Trigger*>::iterator itr = m_Triggers.begin();
    for (; itr != m_Triggers.end(); itr++) {
        if ((*itr) == pTrigger) {
            break;
        }
    }

    if (itr != m_Triggers.end())
        throw DuplicatedException("duplicated trigger");

    m_Triggers.push_back(pTrigger);

    m_ConditionSet |= pTrigger->getConditionSet();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// delete trigger
////////////////////////////////////////////////////////////////////////////////
void TriggerManager::deleteTrigger(TriggerID_t triggerID) {
    __BEGIN_TRY

    // list<Trigger*>::iterator itr = find_if(m_Triggers.begin() , m_Triggers.end() , isSameTriggerID(triggerID));
    list<Trigger*>::iterator itr;
    for (itr = m_Triggers.begin(); itr != m_Triggers.end(); itr++) {
        if ((*itr)->getTriggerID() == triggerID) {
            break;
        }
    }

    if (itr != m_Triggers.end())
        throw NoSuchElementException();

    // delete trigger object
    delete *itr;

    // delete node
    m_Triggers.erase(itr);

    // condition set 을 새로 구성한다.
    refresh();

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// get trigger
////////////////////////////////////////////////////////////////////////////////
Trigger* TriggerManager::getTrigger(TriggerID_t triggerID) {
    __BEGIN_TRY

    // list<Trigger*>::iterator itr = find_if(m_Triggers.begin() , m_Triggers.end() , isSameTriggerID(triggerID));
    list<Trigger*>::iterator itr;
    for (itr = m_Triggers.begin(); itr != m_Triggers.end(); itr++) {
        if ((*itr)->getTriggerID() == triggerID) {
            break;
        }
    }

    if (itr != m_Triggers.end())
        throw NoSuchElementException();

    return *itr;

    __END_CATCH
}


////////////////////////////////////////////////////////////////////////////////
// get debug string
////////////////////////////////////////////////////////////////////////////////
string TriggerManager::toString() const {
    __BEGIN_TRY

    StringStream msg;
    return msg.toString();

    __END_CATCH
}
