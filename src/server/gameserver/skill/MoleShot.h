//////////////////////////////////////////////////////////////////////////////
// Filename    : MoleShot.h
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __SKILL_MOLE_SHOT_HANDLER_H__
#define __SKILL_MOLE_SHOT_HANDLER_H__

#include "SkillHandler.h"

//////////////////////////////////////////////////////////////////////////////
// class MoleShot;
//////////////////////////////////////////////////////////////////////////////

class MoleShot : public SkillHandler {
public:
    MoleShot();
    ~MoleShot() {}

public:
    string getSkillHandlerName() const {
        return "MoleShot";
    }
    SkillType_t getSkillType() const {
        return SKILL_MOLE_SHOT;
    }

    void execute(Slayer* pSlayer, ObjectID_t targetObjectID, SkillSlot* pSkillSlot, CEffectID_t CEffectID);
    void execute(Slayer* pSlayer, ZoneCoord_t X, ZoneCoord_t Y, SkillSlot* pSkillSlot, CEffectID_t CEffectID);

    void computeOutput(const SkillInput& input, SkillOutput& output);

protected:
    void SGexecute(Slayer* pSlayer, ZoneCoord_t X, ZoneCoord_t Y, SkillSlot* pSkillSlot, CEffectID_t CEffectID);
    void ARSMGexecute(Slayer* pSlayer, ZoneCoord_t X, ZoneCoord_t Y, SkillSlot* pSkillSlot, CEffectID_t CEffectID);

protected:
    POINT m_pSGMask[8];    // The SG mask does not depend on direction.
    POINT m_pARMask[8][3]; // The AR and SMG masks depend on direction.
};

// global variable declaration
extern MoleShot g_MoleShot;

#endif // __SKILL_MOLE_SHOT_H__
