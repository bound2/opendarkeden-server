#ifndef __FINITE_STATE_MACHINE_H__
#define __FINITE_STATE_MACHINE_H__

#include <unordered_map>

#include "Assert.h"
#include "Timeval.h"
#include "Types.h"
#include "VSDateTime.h"

/**
 * Base abstract classes
 **/
class State {
public:
    State() {}
    virtual ~State() {}

    virtual DWORD getStateType() const = 0;

    // Return the next state. StateType 0 means the state does not change.
    virtual DWORD heartbeat(Timeval currentTime) = 0;

    virtual void start() {}
    virtual void end() {}

    virtual State* clone() = 0;
    virtual string toString() const = 0;

private:
};

class StateFactory {
public:
    virtual ~StateFactory() {}
    virtual State* makeState(DWORD sType) = 0;
    virtual void wasteState(State* pState) {}
};

class FiniteStateMachine {
public:
    FiniteStateMachine() : m_pCurrentState(NULL), m_pStateFactory(NULL), m_bReset(false), m_ResetState(0) {}
    virtual ~FiniteStateMachine() {}

    virtual void init() = 0;
    virtual void heartbeat(Timeval currentTime);

    DWORD getCurrentState() const {
        return m_pCurrentState->getStateType();
    }
    State* getCurrentState_Object() const {
        return m_pCurrentState;
    }

    void reset() {
        m_bReset = true;
    }

protected:
    State* m_pCurrentState;
    StateFactory* m_pStateFactory;

    bool m_bReset;
    DWORD m_ResetState;
};

/**
 * Base classes that are commonly useful
 **/
class TimerState : public State {
public:
    // The lifetime is in seconds.
    TimerState(DWORD nState, Turn_t life) : m_TimeOutState(nState), m_LifeSpan(life) {}
    void start();
    DWORD heartbeat(Timeval currentTime);
    void expire();

private:
    DWORD m_TimeOutState;
    Turn_t m_LifeSpan;
    Timeval m_Deadline;
};

class SetTimeState : public State {
public:
    SetTimeState(DWORD nState, const VSDateTime& date) : m_TimeOutState(nState), m_SetTime(date) {}

    void start();
    DWORD heartbeat(Timeval currentTime);
    void expire();

protected:
    void setTimer(const VSDateTime& date) {
        m_SetTime = date;
    }
    const VSDateTime& getTimer() const {
        return m_SetTime;
    }

private:
    DWORD m_TimeOutState;
    VSDateTime m_SetTime;
    Timeval m_Deadline;
};

// Simply holds every state and hands out the object it holds.
// Careful: an object that carries state cannot be shared between FSMs.
class FlyweightStateFactory : public StateFactory {
public:
    void registerState(State* pState) {
        Assert(m_StateMap[pState->getStateType()] == NULL);
        m_StateMap[pState->getStateType()] = pState;
    }
    State* makeState(DWORD sType) {
        return m_StateMap[sType];
    }

private:
    unordered_map<DWORD, State*> m_StateMap;
};

#endif
