//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionSwapAdvancementItem.h
// Written By  : excel96
// Description :
// The creature asks the PC a question. The line is printed in the NPC dialogue window.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_SWAP_ADVANCEMENT_ITEM_H__
#define __ACTION_SWAP_ADVANCEMENT_ITEM_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionSwapAdvancementItem
//////////////////////////////////////////////////////////////////////////////

class ActionSwapAdvancementItem : public Action {
public:
    virtual ActionType_t getActionType() const {
        return ACTION_SWAP_ADVANCEMENT_ITEM;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;
};


//////////////////////////////////////////////////////////////////////////////
// class ActionSwapAdvancementItemFactory;
//////////////////////////////////////////////////////////////////////////////

class ActionSwapAdvancementItemFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_SWAP_ADVANCEMENT_ITEM;
    }
    virtual string getActionName() const {
        return "SwapAdvancementItem";
    }
    virtual Action* createAction() const {
        return new ActionSwapAdvancementItem();
    }
};
#endif
