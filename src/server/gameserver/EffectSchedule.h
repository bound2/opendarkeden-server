//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectSchedule.h
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_SCHEDULE_H__
#define __EFFECT_SCHEDULE_H__

#include <list>

#include "Exception.h"
#include "Mutex.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

enum EffectScheduleWorkCode {
    WORKCODE_ADD_VAMPIRE_PORTAL = 0,
    WORKCODE_DELETE,

    WORKCODE_MAX
};


//////////////////////////////////////////////////////////////////////////////
// forward declaration
//////////////////////////////////////////////////////////////////////////////
class Effect;


//////////////////////////////////////////////////////////////////////////////
// class EffectScheduleWork
//////////////////////////////////////////////////////////////////////////////

class EffectScheduleWork {
public:
    EffectScheduleWork();
    ~EffectScheduleWork();

public:
    int getCode(void) const {
        return m_Code;
    }
    void setCode(int code) {
        m_Code = code;
    }

    void* getData(void) const {
        return m_pData;
    }
    void setData(void* pData) {
        m_pData = pData;
    }

private:
    int m_Code;    // Work code for the effect
    void* m_pData; // Data the effect work code needs
};


//////////////////////////////////////////////////////////////////////////////
// class EffectSchedule
//////////////////////////////////////////////////////////////////////////////

class EffectSchedule {
public:
    EffectSchedule();
    ~EffectSchedule();

public:
    // Add the effect the work applies to.
    Effect* getEffect(void) const {
        return m_pEffect;
    }
    void setEffect(Effect* pEffect) {
        m_pEffect = pEffect;
    }

    // Add a work item.
    void addWork(int WorkCode, void* pData);

    // Return a pointer to the work item at the front of the list.
    EffectScheduleWork* getFrontWork(void);

private:
    Effect* m_pEffect;
    list<EffectScheduleWork*> m_WorkList;
};


//////////////////////////////////////////////////////////////////////////////
// class EffectScheduleManager
//////////////////////////////////////////////////////////////////////////////

class EffectScheduleManager {
public:
    EffectScheduleManager();
    ~EffectScheduleManager();

public:
    // Add an effect schedule.
    void addEffectSchedule(EffectSchedule* pEffectSchedule);

    // Run the effect schedules.
    void heartbeat(void);

protected:
    list<EffectSchedule*> m_EffectScheduleList;
    mutable Mutex m_Mutex;
};


#endif
