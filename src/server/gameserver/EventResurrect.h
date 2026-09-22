//////////////////////////////////////////////////////////////////////////////
// Filename    : EventResurrect.h
// Written by  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EVENTRESURRECT_H__
#define __EVENTRESURRECT_H__

#include "Event.h"
#include "Zone.h"

//////////////////////////////////////////////////////////////////////////////
// class EventResurrect;
//////////////////////////////////////////////////////////////////////////////

class EventResurrect : public Event {
public:
    EventResurrect(GamePlayer* pGamePlayer);
    ~EventResurrect();

public:
    virtual EventClass getEventClass() const {
        return EVENT_CLASS_RESURRECT;
    }

    virtual void activate();

    virtual string toString() const;

public:
private:
    // Zone* m_pResurrectZone;

    // ZoneCoord_t  m_X;
    // ZoneCoord_t  m_Y;
};

#endif
