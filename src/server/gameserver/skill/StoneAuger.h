//////////////////////////////////////////////////////////////////////////////
// Filename    : StoneAuger.h
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_STONE_AUGER_HANDLER_H__
#define __SKILL_STONE_AUGER_HANDLER_H__

#include "SkillHandler.h"

//////////////////////////////////////////////////////////////////////////////
// class StoneAuger;
//////////////////////////////////////////////////////////////////////////////

class StoneAuger : public SkillHandler {
public:
    StoneAuger() {}
    ~StoneAuger() {}

public:
    string getSkillHandlerName() const {
        return "StoneAuger";
    }
    SkillType_t getSkillType() const {
        return SKILL_STONE_AUGER;
    }

    void execute(Ousters* pOusters, ZoneCoord_t X, ZoneCoord_t Y, OustersSkillSlot* pOustersSkillSlot,
                 CEffectID_t CEffectID);
    void execute(Ousters* pOusters, ObjectID_t ObjectID, OustersSkillSlot* pOustersSkillSlot, CEffectID_t CEffectID);

    void computeOutput(const SkillInput& input, SkillOutput& output);
};

// global variable declaration
extern StoneAuger g_StoneAuger;

#endif // __SKILL_STONE_AUGER_HANDLER_H__
