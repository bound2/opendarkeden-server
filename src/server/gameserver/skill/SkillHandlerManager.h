//////////////////////////////////////////////////////////////////////////////
// Filename    : SkillHandlerManager.h
// Wrrtten by  : elca@ewestsoft.com
// Description : Manages the skill handlers.
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_HANDLER_MANAGER_H__
#define __SKILL_HANDLER_MANAGER_H__

#include "Skill.h"
#include "SkillHandler.h"

//////////////////////////////////////////////////////////////////////////////
// class SkillHandlerManager
//////////////////////////////////////////////////////////////////////////////

class SkillHandlerManager {
public:
    SkillHandlerManager();
    ~SkillHandlerManager();

public:
    // Called from ObjectManager::init().
    void init();

    // Registers a skill handler object.
    void addSkillHandler(SkillHandler* pSkillHandler);

    // Gets a skill handler.
    SkillHandler* getSkillHandler(SkillType_t SkillType);

    // toString
    string toString() const;

private:
    SkillHandler** m_SkillHandlers; // Skill handler array
    ushort m_Size;                  // Size of the skill handler array
};

extern SkillHandlerManager* g_pSkillHandlerManager;

#endif
