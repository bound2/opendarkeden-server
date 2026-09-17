//////////////////////////////////////////////////////////////////////////////
// Filename    : Teleport.cpp
// Written by  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "Teleport.h"

#include "GCModifyInformation.h"
#include "GCSkillToTileOK1.h"
#include "GCSkillToTileOK5.h"
#include "GCStatusCurrentHP.h"

//////////////////////////////////////////////////////////////////////////////
// 뱀파이어 타일 핸들러
//////////////////////////////////////////////////////////////////////////////
void Teleport::execute(Ousters* pOusters, ZoneCoord_t X, ZoneCoord_t Y, OustersSkillSlot* pOustersSkillSlot,
                       CEffectID_t CEffectID)

{
    __BEGIN_TRY

    Assert(pOusters != NULL);
    Assert(pOustersSkillSlot != NULL);

    try {
        Player* pPlayer = pOusters->getPlayer();
        Zone* pZone = pOusters->getZone();
        Assert(pPlayer != NULL);
        Assert(pZone != NULL);

        SkillType_t SkillType = pOustersSkillSlot->getSkillType();

        // NoSuch제거. by sigi. 2002.5.2
        // NPC는 공격할 수가 없다.

        GCSkillToTileOK1 _GCSkillToTileOK1;
        GCSkillToTileOK5 _GCSkillToTileOK5;

        VSRect rect(0, 0, pZone->getWidth() - 1, pZone->getHeight() - 1);
        if (!rect.ptInRect(X, Y)) {
            executeSkillFailException(pOusters, getSkillType());
        }

        SkillInput input(pOusters, pOustersSkillSlot);
        SkillOutput output;
        computeOutput(input, output);

        SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(SkillType);

        int RequiredMP = (int)pSkillInfo->getConsumeMP() + pOustersSkillSlot->getExpLevel() / 10;
        bool bManaCheck = hasEnoughMana(pOusters, RequiredMP);
        bool bTimeCheck = verifyRunTime(pOustersSkillSlot);
        bool bRangeCheck = verifyDistance(pOusters, X, Y, output.Range);
        bool bEffected = pOusters->hasRelicItem() || pOusters->isFlag(Effect::EFFECT_CLASS_HAS_FLAG) ||
                         pOusters->isFlag(Effect::EFFECT_CLASS_HAS_SWEEPER);

        if (bManaCheck && bTimeCheck && bRangeCheck && !bEffected) {
            // 빠르게 PC를 움직여준다.
            if (pZone->moveFastPC(pOusters, pOusters->getX(), pOusters->getY(), X, Y, getSkillType())) {
                decreaseMana(pOusters, RequiredMP, _GCSkillToTileOK1);

                _GCSkillToTileOK1.setSkillType(SkillType);
                _GCSkillToTileOK1.setCEffectID(0);
                _GCSkillToTileOK1.setX(X);
                _GCSkillToTileOK1.setY(Y);
                _GCSkillToTileOK1.setRange(0);
                _GCSkillToTileOK1.setDuration(0);

                _GCSkillToTileOK5.setObjectID(pOusters->getObjectID());
                _GCSkillToTileOK5.setSkillType(SkillType);
                _GCSkillToTileOK5.setX(X);
                _GCSkillToTileOK5.setY(Y);
                _GCSkillToTileOK5.setRange(0);
                _GCSkillToTileOK5.setDuration(0);

                // 자신에게 바뀐 MP를 알려준다.
                pPlayer->sendPacket(&_GCSkillToTileOK1);
                pZone->broadcastPacket(pOusters->getX(), pOusters->getY(), &_GCSkillToTileOK5, pOusters);

                pOustersSkillSlot->setRunTime(output.Delay);
            } else {
                executeSkillFailException(pOusters, getSkillType());
            }
        } else {
            executeSkillFailException(pOusters, getSkillType());
        }
    } catch (Throwable& t) {
        executeSkillFailException(pOusters, getSkillType());
    }


    __END_CATCH
}
//////////////////////////////////////////////////////////////////////////////
// 뱀파이어 오브젝트 핸들러
//////////////////////////////////////////////////////////////////////////////
void Teleport::execute(Ousters* pOusters, ObjectID_t TargetObjectID, OustersSkillSlot* pOustersSkillSlot,
                       CEffectID_t CEffectID)

{
    __BEGIN_TRY


    Assert(pOusters != NULL);
    Assert(pOustersSkillSlot != NULL);

    try {
        Player* pPlayer = pOusters->getPlayer();
        Zone* pZone = pOusters->getZone();
        Assert(pPlayer != NULL);
        Assert(pZone != NULL);

        Creature* pTargetCreature = pZone->getCreature(TargetObjectID);

        // NoSuch제거. by sigi. 2002.5.2
        // NPC는 공격할 수가 없다.
        if (pTargetCreature == NULL) {
            executeSkillFailException(pOusters, getSkillType());
            return;
        }


        execute(pOusters, pTargetCreature->getX(), pTargetCreature->getY(), pOustersSkillSlot, CEffectID);
    } catch (Throwable& t) {
        executeSkillFailException(pOusters, getSkillType());
    }


    __END_CATCH
}

Teleport g_Teleport;
