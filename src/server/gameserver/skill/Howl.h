//////////////////////////////////////////////////////////////////////////////
// Filename    : Howl.h
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_HOWL_HANDLER_H__
#define __SKILL_HOWL_HANDLER_H__

#include "SkillHandler.h"

//////////////////////////////////////////////////////////////////////////////
// class Howl;
//////////////////////////////////////////////////////////////////////////////

class Howl : public SkillHandler {
public:
    Howl() {}
    ~Howl() {}

public:
    string getSkillHandlerName() const {
        return "Howl";
    }
    SkillType_t getSkillType() const {
        return SKILL_HOWL;
    }

    void execute(Vampire* pVampire, VampireSkillSlot* pVampireSkillSlot, CEffectID_t CEffectID);

    void computeOutput(const SkillInput& input, SkillOutput& output);
};

// global variable declaration
extern Howl g_Howl;

#endif // __SKILL_HOWL_HANDLER_H__
