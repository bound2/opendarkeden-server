//////////////////////////////////////////////////////////////////////////////
// Filename    : ObjectManager.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __OBJECT_MANAGER_H__
#define __OBJECT_MANAGER_H__

#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class ObjectManager;
// 하위 게임 객체들의 매니저를 관리하는 상위 관리 클래스이다.
//////////////////////////////////////////////////////////////////////////////

class ActionFactoryManager;
class ConditionFactoryManager;
class DynamicZoneFactoryManager;
class MonsterNameManager;
class ScriptManager;
class ShopTemplateManager;
class WeatherInfoManager;

class ObjectManager {
public:
    ObjectManager();
    ~ObjectManager();

public:
    void init();
    void load();
    void save();

private:
    // Managers this class creates and deletes, reached through
    // de::GameContext, which it registers them on, rather than through a
    // global.
    ActionFactoryManager* m_pActionFactoryManager = nullptr;
    ConditionFactoryManager* m_pConditionFactoryManager = nullptr;
    DynamicZoneFactoryManager* m_pDynamicZoneFactoryManager = nullptr;
    MonsterNameManager* m_pMonsterNameManager = nullptr;
    ScriptManager* m_pPublicScriptManager = nullptr;
    ShopTemplateManager* m_pShopTemplateManager = nullptr;
    WeatherInfoManager* m_pWeatherInfoManager = nullptr;
};

// global variable declaration
extern ObjectManager* g_pObjectManager;

#endif
