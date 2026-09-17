//////////////////////////////////////////////////////////////////////////////
// Filename    : OpenCasket.cpp
// Written by  :
// Description :
//////////////////////////////////////////////////////////////////////////////
#include "OpenCasket.h"

#include "GCAddEffect.h"
#include "GCSkillToObjectOK1.h"
#include "GCSkillToObjectOK2.h"
#include "GCSkillToObjectOK3.h"
#include "GCSkillToObjectOK4.h"
#include "GCSkillToObjectOK5.h"
#include "GCSkillToSelfOK1.h"
#include "GCSkillToSelfOK2.h"
#include "HitRoll.h"

//////////////////////////////////////////////////////////////////////////////
// 슬레이어 셀프 핸들러
//////////////////////////////////////////////////////////////////////////////
void OpenCasket::execute(Vampire* pVampire, VampireSkillSlot* pVampireSkillSlot, CEffectID_t CEffectID)

{
    __BEGIN_TRY


    Assert(pVampire != NULL);

    try {
        Player* pPlayer = pVampire->getPlayer();
        Zone* pZone = pVampire->getZone();

        Assert(pPlayer != NULL);
        Assert(pZone != NULL);

        GCSkillToSelfOK1 _GCSkillToSelfOK1;
        GCSkillToSelfOK2 _GCSkillToSelfOK2;

        SkillType_t SkillType = getSkillType();

        // 관 속에 들어있는 경우만 사용 가능하다.
        bool bEffected = pVampire->isFlag(Effect::EFFECT_CLASS_CASKET);

        if (bEffected) {
            // 이펙트 클래스를 만들어 붙인다.
            EffectManager* pEffectManager = pVampire->getEffectManager();
            Assert(pEffectManager != NULL);

            Effect* pEffect = pEffectManager->findEffect(Effect::EFFECT_CLASS_CASKET);

            if (pEffect != NULL) {
                pEffect->setDeadline(0);
            }

            // 패킷을 만들어 보낸다.
            _GCSkillToSelfOK1.setSkillType(SkillType);
            _GCSkillToSelfOK1.setCEffectID(CEffectID);
            _GCSkillToSelfOK1.setDuration(0);

            _GCSkillToSelfOK2.setObjectID(pVampire->getObjectID());
            _GCSkillToSelfOK2.setSkillType(SkillType);
            _GCSkillToSelfOK2.setDuration(0);

            pPlayer->sendPacket(&_GCSkillToSelfOK1);

            pZone->broadcastPacket(pVampire->getX(), pVampire->getY(), &_GCSkillToSelfOK2, pVampire);

            // set Next Run Time
        } else {
            executeSkillFailNormal(pVampire, getSkillType(), NULL);
        }
    } catch (Throwable& t) {
        executeSkillFailException(pVampire, getSkillType());
    }


    __END_CATCH
}

OpenCasket g_OpenCasket;
