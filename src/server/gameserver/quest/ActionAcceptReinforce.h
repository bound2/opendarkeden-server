//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionAcceptReinforce.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_ACCEPT_REINFORCE_H__
#define __ACTION_ACCEPT_REINFORCE_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionAcceptReinforce
//////////////////////////////////////////////////////////////////////////////

class ActionAcceptReinforce : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_ACCEPT_REINFORCE;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2);
    virtual string toString() const;

public:
    ZoneID_t getZoneID() const {
        return m_ZoneID;
    }
    void setZoneID(ZoneID_t ZoneID) {
        m_ZoneID = ZoneID;
    }

private:
    ZoneID_t m_ZoneID; // ZoneID of the castle the war was declared on
};


//////////////////////////////////////////////////////////////////////////////
// class ActionAcceptReinforceFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionAcceptReinforceFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_ACCEPT_REINFORCE;
    }
    virtual string getActionName() const {
        return "AcceptReinforce";
    }
    virtual Action* createAction() const {
        return new ActionAcceptReinforce();
    }
};
#endif
