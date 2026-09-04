//////////////////////////////////////////////////////////////////////////////
// Filename    : Flare.h
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_FLARE_HANDLER_H__
#define __SKILL_FLARE_HANDLER_H__

#include "GCChangeDarkLight.h"
#include "SkillHandler.h"

//////////////////////////////////////////////////////////////////////////////
// class Flare;
//////////////////////////////////////////////////////////////////////////////

class Flare : public SkillHandler {
public:
    Flare() {}
    ~Flare() {}

public:
    string getSkillHandlerName() const {
        return "Flare";
    }
    SkillType_t getSkillType() const {
        return SKILL_FLARE;
    }

    void execute(Slayer* pSlayer, ObjectID_t ObjectID, SkillSlot* pSkillSlot, CEffectID_t CEffectID);

    void computeOutput(const SkillInput& input, SkillOutput& output);
};

// global variable declaration
extern Flare g_Flare;

#endif // __SKILL_FLARE_HANDLER_H__
