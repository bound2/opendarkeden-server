//////////////////////////////////////////////////////////////////////////////
// Filename    : SummonMonsters.h
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_SUMMON_MONSTERS_HANDLER_H__
#define __SKILL_SUMMON_MONSTERS_HANDLER_H__

#include "SkillHandler.h"

//////////////////////////////////////////////////////////////////////////////
// class SummonMonsters;
//////////////////////////////////////////////////////////////////////////////

class SummonMonsters : public SkillHandler {
public:
    SummonMonsters();
    ~SummonMonsters() {}

public:
    string getSkillHandlerName() const {
        return "SummonMonsters";
    }
    SkillType_t getSkillType() const {
        return SKILL_SUMMON_MONSTERS;
    }

    void execute(Monster* pMonster);

    void computeOutput(const SkillInput& input, SkillOutput& output);
};

// global variable declaration
extern SummonMonsters g_SummonMonsters;

#endif // __SKILL_HIDE_HANDLER_H__
