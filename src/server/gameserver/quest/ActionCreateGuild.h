////////////////////////////////////////////////////////////////////////////////
// Filename    : ActionCreateGuild.h
// Written By  : excel96
// Description :
// Makes the client open the guild creation window through an NPC.
////////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_CREATE_GUILD_H__
#define __ACTION_CREATE_GUILD_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionCreateGuild;
//////////////////////////////////////////////////////////////////////////////

class ActionCreateGuild : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_CREATE_GUILD;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;
};

//////////////////////////////////////////////////////////////////////////////
// class ActionCreateGuildFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionCreateGuildFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_CREATE_GUILD;
    }
    virtual string getActionName() const {
        return "CreateGuild";
    }
    virtual Action* createAction() const {
        return new ActionCreateGuild();
    }
};

#endif
