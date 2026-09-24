#include "GQuestInfo.h"

#include "Assert.h"
#include "GQuestElement.h"
#include "GQuestLevelElement.h"
#include "GQuestStatus.h"
#include "KernelContext.h"
#include "Properties.h"

GQuestInfo::GQuestInfo(XMLTree* pInfo) {
    Assert(pInfo->GetName() == "Quest");
    Assert(pInfo->GetAttribute("id", m_QuestID));

    m_CheckTypes[HAPPEN] = SEQUENCE;
    m_CheckTypes[COMPLETE] = SEQUENCE;
    m_CheckTypes[FAIL] = OR;
    m_CheckTypes[REWARD] = SEQUENCE;

    makeVector(pInfo->GetChild("Happen"), HAPPEN);
    makeVector(pInfo->GetChild("Complete"), COMPLETE);
    makeVector(pInfo->GetChild("Fail"), FAIL);
    makeVector(pInfo->GetChild("Reward"), REWARD);
}

// void GQuestInfo::makeVector(XMLTree* pTree, vector<GQuestElement*>& eVector)
void GQuestInfo::makeVector(XMLTree* pTree, ElementType type) {
    //	if ( pTree == NULL ) return;
    Assert(pTree != NULL);
    string checkType;
    if (pTree->GetAttribute("type", checkType)) {
        cout << "Check type set: " << checkType << endl;

        if (checkType == "sequence")
            m_CheckTypes[type] = SEQUENCE;
        else if (checkType == "and")
            m_CheckTypes[type] = AND;
        else if (checkType == "or")
            m_CheckTypes[type] = OR;
        else
            Assert(false);
    }

    int num = pTree->GetChildCount();

    for (int i = 0; i < num; ++i) {
        XMLTree* pChild = pTree->GetChild(i);
        GQuestElement* pElement = GQuestElementFactory::Instance().makeElement(pChild);
        //		Assert( pElement != NULL );
        if (pElement != NULL) {
            pElement->setParent(this);
            pElement->setCondition(type);
            m_Elements[type].push_back(pElement);
        } else {
            cout << "Cannot make element : " << pChild->GetName() << endl;
            Assert(false);
        }
    }
}

GQuestElement* GQuestInfo::getElement(GQuestInfo::ElementType type, uint index) {
    vector<GQuestElement*>& eVector = m_Elements[type];
    if (eVector.size() <= index)
        return NULL;
    return eVector[index];
}

bool GQuestInfo::isInstanceSuccess(ElementType type, PlayerCreature* pPC) {
    vector<GQuestElement*>::iterator itr = m_Elements[type].begin();
    vector<GQuestElement*>::iterator endItr = m_Elements[type].end();

    for (; itr != endItr; ++itr) {
        GQuestElement* pElement = *itr;
        //		cout << "Instant Check : " << pElement->getElementName() << endl;
        GQuestElement::ResultType result = pElement->checkCondition(pPC);
        //		cout << "Result : " << result << endl;
        if (result != GQuestElement::OK)
            return false;
    }

    return true;
}

GQuestStatus* GQuestInfo::makeInitStatus(PlayerCreature* pOwner) {
    GQuestStatus* pStatus = new GQuestStatus(pOwner, this);
    pStatus->setStatus(QuestStatusInfo::CAN_ACCEPT);

    return pStatus;
}

void GQuestInfoManager::load() {
    m_pXMLInfo = new XMLTree;
    m_pXMLInfo->LoadFromFile((de::kernelContext().config().getProperty("HomePath") + "/data/SimpleGQuest.xml").c_str());

    int num = m_pXMLInfo->GetChildCount();

    for (int i = 0; i < num; ++i) {
        XMLTree* pChild = m_pXMLInfo->GetChild((size_t)i);
        if (pChild == NULL)
            break;

        GQuestInfo* pInfo = new GQuestInfo(pChild);
        m_Infos[pInfo->getQuestID()] = pInfo;
    }

    SAFE_DELETE(m_pXMLInfo);

    m_pXMLInfo = new XMLTree;
    m_pXMLInfo->LoadFromFile((de::kernelContext().config().getProperty("HomePath") + "/data/EventGQuest.xml").c_str());

    num = m_pXMLInfo->GetChildCount();

    for (int i = 0; i < num; ++i) {
        XMLTree* pChild = m_pXMLInfo->GetChild((size_t)i);
        if (pChild == NULL)
            break;

        GQuestInfo* pInfo = new GQuestInfo(pChild);
        m_Infos[pInfo->getQuestID()] = pInfo;
    }

    SAFE_DELETE(m_pXMLInfo);
}
