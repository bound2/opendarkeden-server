//////////////////////////////////////////////////////////////////////////////
// Filename    : ActionFactoryManager.h
// Written By  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __ACTION_FACTORY_MANAGER_H__
#define __ACTION_FACTORY_MANAGER_H__

#include "Action.h"
#include "ActionFactory.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ActionFactoryManager
//////////////////////////////////////////////////////////////////////////////

namespace de {
class GameContext;
}

class ActionFactoryManager {
public:
    explicit ActionFactoryManager(de::GameContext& context);
    ~ActionFactoryManager();

public:
    void init();
    void addFactory(ActionFactory* pFactory);
    Action* createAction(ActionType_t conditionType) const;
    string getActionName(ActionType_t conditionType) const;
    ActionType_t getActionType(const string& actionname) const;
    string toString() const;

private:
    de::GameContext& m_Context;
    ActionFactory** m_Factories;
    ushort m_Size;
};

#endif
