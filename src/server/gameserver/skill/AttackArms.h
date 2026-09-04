//////////////////////////////////////////////////////////////////////////////
// Filename    : AttackArms.h
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_ATTACK_ARMS_HANDLER_H__
#define __SKILL_ATTACK_ARMS_HANDLER_H__

#include "SkillHandler.h"

//////////////////////////////////////////////////////////////////////////////
// class AttackArms;
//////////////////////////////////////////////////////////////////////////////

class AttackArms : public SkillHandler {
public:
    AttackArms() {}
    ~AttackArms() {}

public:
    string getSkillHandlerName() const {
        return "AttackArms";
    }
    SkillType_t getSkillType() const {
        return SKILL_ATTACK_ARMS;
    }

    void execute(Slayer* pPlayer, ObjectID_t ObjectID);
    void execute(Monster* pPlayer, Creature* pEnemy);

    void computeOutput(const SkillInput& input, SkillOutput& output) {}
};

// global variable declaration
extern AttackArms g_AttackArms;

#endif // __SKILL_ATTACK_ARMS_HANDLER_H__
