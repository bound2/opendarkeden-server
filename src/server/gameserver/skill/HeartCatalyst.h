//////////////////////////////////////////////////////////////////////////////
// Filename    : HeartCatalyst.h
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_HEART_CATALYST_HANDLER_H__
#define __SKILL_HEART_CATALYST_HANDLER_H__

#include "SkillHandler.h"

//////////////////////////////////////////////////////////////////////////////
// class HeartCatalyst;
//////////////////////////////////////////////////////////////////////////////

class HeartCatalyst : public SkillHandler {
public:
    HeartCatalyst() {}
    ~HeartCatalyst() {}

public:
    string getSkillHandlerName() const {
        return "HeartCatalyst";
    }
    SkillType_t getSkillType() const {
        return SKILL_HEART_CATALYST;
    }

    void execute(Slayer* pSlayer, SkillSlot* pSkillSlot, CEffectID_t CEffectID);

    void computeOutput(const SkillInput& input, SkillOutput& output);
};

// global variable declaration
extern HeartCatalyst g_HeartCatalyst;

#endif // __SKILL_HEART_CATALYST_HANDLER_H__
