//--------------------------------------------------------------------------------
// QuestManager.h
//--------------------------------------------------------------------------------

#ifndef __QUEST_MANAGER_H__
#define __QUEST_MANAGER_H__

#include <list>

#include "Exception.h"
#include "Types.h"
class Quest;
class QuestEvent;

//--------------------------------------------------------------------------------
// QuestManager
//--------------------------------------------------------------------------------
class QuestManager {
public:
    typedef list<Quest*> QUEST_LIST;

public:
    QuestManager();
    ~QuestManager() noexcept(false);

    //	void		load(const string& name) ;
    void release();

    int isEmpty() const {
        return m_Quests.empty();
    }

    bool addQuest(Quest* pQuest);

    Quest* checkEvent(QuestEvent* pQuestEvent);

    Quest* removeCompleteQuest();

    void heartbeat();


private:
    QUEST_LIST m_Quests;
};

#endif
