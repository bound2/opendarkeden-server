//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionDestroyGuild.h
// Written By  :
// Description :
// Makes the client open the guild creation window through an NPC.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_DESTROY_GUILD_H__
#define __ACTION_DESTROY_GUILD_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionDestroyGuild;
//////////////////////////////////////////////////////////////////////////////

class ActionDestroyGuild : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_DESTROY_GUILD;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;
};

//////////////////////////////////////////////////////////////////////////////
// class ActionDestroyGuildFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionDestroyGuildFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_DESTROY_GUILD;
    }
    virtual string getActionName() const {
        return "DestroyGuild";
    }
    virtual Action* createAction() const {
        return new ActionDestroyGuild();
    }
};

#endif
