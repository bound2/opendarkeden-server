//////////////////////////////////////////////////////////////////////////////
// Filename    : BrambleHalo.h
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_Bramble_Halo_HANDLER_H__
#define __SKILL_Bramble_Halo_HANDLER_H__

#include "SkillHandler.h"

//////////////////////////////////////////////////////////////////////////////
// class BrambleHalo;
//////////////////////////////////////////////////////////////////////////////

class BrambleHalo : public SkillHandler {
public:
    BrambleHalo() {}
    ~BrambleHalo() {}

public:
    string getSkillHandlerName() const {
        return "BrambleHalo";
    }
    SkillType_t getSkillType() const {
        return SKILL_Bramble_Halo;
    }

    void execute(Slayer* pSlayer, SkillSlot* pSkillSlot, CEffectID_t CEffectID);

    void computeOutput(const SkillInput& input, SkillOutput& output);
};

// global variable declaration
extern BrambleHalo g_BrambleHalo;

#endif // __SKILL_Bramble_Halo_HANDLER_H__
