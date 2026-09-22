//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionRegenShop.h
// Written By  :
// Description :
// Action that prepares the items a shop NPC will sell, run when the NPC is
// first loaded. See the ShopTemplate class and its manager.
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_REGEN_SHOP_H__
#define __ACTION_REGEN_SHOP_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Timeval.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionRegenShop
//////////////////////////////////////////////////////////////////////////////

class ActionRegenShop : public Action {
public:
    ActionRegenShop();
    virtual ~ActionRegenShop();

public:
    virtual ActionType_t getActionType() const {
        return ACTION_REGEN_SHOP;
    }
    virtual void read(PropertyBuffer& propertyBuffer);
    virtual void execute(Creature* pCreature1, Creature* pCreature2 = NULL);
    virtual string toString() const;

public:
    void addListElement(ShopTemplateID_t id) {
        m_List.push_back(id);
    }
    void clearList() {
        m_List.clear();
    }

private:
    list<ShopTemplateID_t> m_List; // Shop template ID list
    Timeval m_Period;              // Shop regeneration period
    Timeval m_NextRegen;           // Next shop regeneration time
};


////////////////////////////////////////////////////////////////////////////////
// class ActionRegenShopFactory;
////////////////////////////////////////////////////////////////////////////////

class ActionRegenShopFactory : public ActionFactory {
public:
    virtual ActionType_t getActionType() const {
        return Action::ACTION_REGEN_SHOP;
    }
    virtual string getActionName() const {
        return "RegenShop";
    }
    virtual Action* createAction() const {
        return new ActionRegenShop();
    }
};
#endif
