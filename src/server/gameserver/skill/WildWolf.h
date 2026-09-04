//////////////////////////////////////////////////////////////////////////////
// Filename    : WildWolf.h
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_WILD_WOLF_HANDLER_H__
#define __SKILL_WILD_WOLF_HANDLER_H__

#include "SkillHandler.h"

//////////////////////////////////////////////////////////////////////////////
// class WildWolf;
//////////////////////////////////////////////////////////////////////////////

class WildWolf : public SkillHandler {
public:
    WildWolf() {}
    ~WildWolf() {}

public:
    string getSkillHandlerName() const {
        return "WildWolf";
    }

    SkillType_t getSkillType() const {
        return SKILL_WILD_WOLF;
    }

    void execute(Vampire* pVampire, ObjectID_t targetObject, VampireSkillSlot* pVampireSkillSlot,
                 CEffectID_t CEffectID);

    void eatCorpse(Vampire* pVampire, Item* pCorpse, VampireSkillSlot* pVampireSkillSlot);
    void eatComaCreature(Vampire* pVampire, Creature* pComaCreature);

    void computeOutput(const SkillInput& input, SkillOutput& output);
};

// global variable declaration
extern WildWolf g_WildWolf;

#endif // __SKILL_WILD_WOLF_HANDLER_H__
