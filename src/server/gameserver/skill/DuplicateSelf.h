//////////////////////////////////////////////////////////////////////////////
// Filename    : DuplicateSelf.h
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_DUPLICATE_SELF_HANDLER_H__
#define __SKILL_DUPLICATE_SELF_HANDLER_H__

#include "SkillHandler.h"

//////////////////////////////////////////////////////////////////////////////
// class DuplicateSelf;
//////////////////////////////////////////////////////////////////////////////

class DuplicateSelf : public SkillHandler {
public:
    DuplicateSelf();
    ~DuplicateSelf() {}

public:
    string getSkillHandlerName() const {
        return "DuplicateSelf";
    }
    SkillType_t getSkillType() const {
        return SKILL_DUPLICATE_SELF;
    }

    void execute(Monster* pMonster);

    void computeOutput(const SkillInput& input, SkillOutput& output);

private:
    // [original monster type] = duplicate monster type
    unordered_map<MonsterType_t, MonsterType_t> m_DuplicateMonsterTypes;
};

// global variable declaration
extern DuplicateSelf g_DuplicateSelf;

#endif // __SKILL_HIDE_HANDLER_H__
