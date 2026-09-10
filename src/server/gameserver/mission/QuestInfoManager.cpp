#include "QuestInfoManager.h"

#include "EventQuestAdvance.h"
#include "GatherItemQuestInfo.h"
#include "Item.h"
#include "ItemRewardInfo.h"
#include "MonsterKillQuestInfo.h"
#include "NPC.h"
#include "PlayerCreature.h"
#include "QuestInfo.h"
#include "QuestManager.h"
#include "VariableManager.h"

QuestInfoManager::~QuestInfoManager()

{
    clear();
}

void QuestInfoManager::clear() {
    __BEGIN_TRY

    QuestInfoHashMap::iterator itr = m_QuestInfos.begin();
    QuestInfoHashMap::iterator endItr = m_QuestInfos.end();

    for (; itr != endItr; ++itr) {
        if (itr->second != NULL)
            SAFE_DELETE(itr->second);
    }

    m_QuestInfos.clear();

    __END_CATCH
}

bool QuestInfoManager::isEventQuest(QuestID_t qID) const {
    QuestInfoHashMap::const_iterator itr = m_QuestInfos.find(qID);
    if (itr != m_QuestInfos.end()) {
        return itr->second->isEventQuest();
    }
    return false;
}

void QuestInfoManager::addQuestInfo(QuestInfo* pQI)

{
    __BEGIN_TRY

    QuestInfoHashMap::iterator itr = m_QuestInfos.find(pQI->getQuestID());

    if (itr != m_QuestInfos.end())
        throw Error("QuestInfoManager::addQuestInfo : QuestID 가 중복됩니다.");

    m_QuestInfos[pQI->getQuestID()] = pQI;

    __END_CATCH
}

QuestMessage QuestInfoManager::canExecuteQuest(QuestID_t qID, PlayerCreature* pPC) const {
    __BEGIN_TRY

    if (!g_pVariableManager->canApplyQuest())
        return START_FAIL_CANNOT_APPLY_QUEST;
    if (pPC->getQuestManager()->hasQuest(qID))
        return START_FAIL_DUPLICATED_QUEST_ID;
    if (!pPC->getQuestManager()->canStartMoreQuest())
        return START_FAIL_QUEST_NUM_EXCEEDED;

    QuestInfoHashMap::const_iterator itr = m_QuestInfos.find(qID);
    if (itr == m_QuestInfos.end()) {
        filelog("EventBug.txt",
                "QuestInfoManager::canExcuteQuest : 클라이언트가 보내준 qID가 이 NPC한테 없다 ㅜ.ㅠ %s:%d",
                m_pOwnerNPC->getName().c_str(), qID);
        return FAIL_BUG;
    }
    if (!itr->second->canExecute(pPC))
        return START_FAIL_PC;

    return START_SUCCESS;

    __END_CATCH
}

QuestMessage QuestInfoManager::startQuest(QuestID_t qID, PlayerCreature* pPC) {
    __BEGIN_TRY

    QuestMessage code = canExecuteQuest(qID, pPC);
    if (code != START_SUCCESS)
        return code;

    QuestStatus* pQuestStatus = makeQuestStatus(qID, pPC);

    if (pQuestStatus->isEventQuest()) {
        if (!pPC->getQuestManager()->getEventQuestAdvanceManager()->start(pQuestStatus->getQuestLevel())) {
            filelog("EventBug.txt",
                    "QuestInfoManager::startQuest : 퀘스트 시작해야 되는데 퀘스트 어드밴스 매니저가 INIT가 아니다. "
                    "%s:%d (Level %d)",
                    pPC->getName().c_str(), qID, pQuestStatus->getQuestLevel());
        }
    }

    pPC->getQuestManager()->addQuest(pQuestStatus);

    return code;

    __END_CATCH
}

QuestStatus* QuestInfoManager::makeQuestStatus(QuestID_t qID, PlayerCreature* pPC) const {
    __BEGIN_TRY

    QuestInfoHashMap::const_iterator itr = m_QuestInfos.find(qID);

    if (itr == m_QuestInfos.end())
        return NULL;

    QuestStatus* pQuestStatus = itr->second->makeQuestStatus(pPC);

    pQuestStatus->setEventQuest(itr->second->isEventQuest());
    pQuestStatus->setQuestLevel(itr->second->getQuestLevel());
    pQuestStatus->setTimeLimit(itr->second->getTimeLimit() != 0);

    pQuestStatus->setRewardClass(itr->second->getRewardClass());

    return pQuestStatus;

    __END_CATCH
}

/*bool QuestInfoManager::canGiveReward( QuestID_t qID, PlayerCreature* pPC ) const
{
    __BEGIN_TRY

    QuestInfoHashMap::const_iterator itr = m_QuestInfos.find( qID );

    if ( itr == m_QuestInfos.end() ) return false;

    return itr->second->canGiveReward( pPC );

    __END_CATCH
}

bool QuestInfoManager::giveReward( QuestID_t qID, PlayerCreature* pPC ) const
{
    __BEGIN_TRY

    QuestInfoHashMap::const_iterator itr = m_QuestInfos.find( qID );

    if ( itr == m_QuestInfos.end() ) return false;

    return itr->second->giveReward( pPC );

    __END_CATCH
}*/
