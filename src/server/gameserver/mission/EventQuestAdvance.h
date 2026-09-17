#ifndef __EVENT_QUEST_ADVANCE_H__
#define __EVENT_QUEST_ADVANCE_H__

#include <vector>

#include "Assert.h"
#include "Exception.h"
#include "Types.h"

class PlayerCreature;

class EventQuestAdvance {
public:
    enum Status {
        EVENT_QUEST_INIT,     // not even started.
        EVENT_QUEST_STARTED,  // started but not finished.
        EVENT_QUEST_SUCCESS,  // finished but the reward was not taken.
        EVENT_QUEST_REWARDED, // a lottery ticket was taken as the reward.
        EVENT_QUEST_ADVANCED  // going on to the next quest was chosen as the reward.
    };

    EventQuestAdvance(const EventQuestAdvance& org) : m_QuestLevel(org.m_QuestLevel), m_Status(org.m_Status) {}

    explicit EventQuestAdvance(int qLevel, Status status = EVENT_QUEST_INIT) : m_QuestLevel(qLevel), m_Status(status) {}
    virtual ~EventQuestAdvance() {}

    int getLevel() const {
        return m_QuestLevel;
    }
    Status getStatus() const {
        return m_Status;
    }

    bool start() {
        return transition(EVENT_QUEST_INIT, EVENT_QUEST_STARTED);
    }
    bool success() {
        Assert(false);
        return transition(EVENT_QUEST_STARTED, EVENT_QUEST_SUCCESS);
    }
    bool rewarded() {
        return transition(EVENT_QUEST_STARTED, EVENT_QUEST_REWARDED);
    }
    bool advanced() {
        return transition(EVENT_QUEST_STARTED, EVENT_QUEST_ADVANCED);
    }
    void reset() {
        setStatus(EVENT_QUEST_INIT);
    }

    void save(const string& name);

protected:
    bool transition(Status I, Status O) {
        if (getStatus() != I)
            return false;
        setStatus(O);
        return true;
    }
    void setStatus(Status status) {
        m_Status = status;
    }

private:
    int m_QuestLevel;
    Status m_Status;
};

class EventQuestAdvanceManager {
public:
    EventQuestAdvanceManager(PlayerCreature* pPC);
    ~EventQuestAdvanceManager();
    void clear();

public:
    bool start(int questLevel);
    bool success(int questLevel);
    bool rewarded(int questLevel);
    bool advanced(int questLevel);

    EventQuestAdvance::Status getStatus(int questLevel);

    void save();
    void save(int questLevel);
    void load();

    bool canExecute(int questLevel);
    int getQuestLevel();
    static const int EVENT_QUEST_LEVEL_MAX = 5;

private:
    vector<EventQuestAdvance*> m_Advances;

    PlayerCreature* m_pOwner;
};

#endif // __EVENT_QUEST_ADVANCE_H__
