//////////////////////////////////////////////////////////////////////////////
// Filename    : BurningSolCharging.cpp
// Written by  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "BurningSolCharging.h"

#include "EffectBurningSolCharging.h"
#include "GCAddEffect.h"
#include "GCSkillToTileOK1.h"
#include "GCSkillToTileOK5.h"

//////////////////////////////////////////////////////////////////////////////
// 슬레이어 셀프 핸들러
//////////////////////////////////////////////////////////////////////////////
void BurningSolCharging::execute(Slayer* pSlayer, ZoneCoord_t X, ZoneCoord_t Y, SkillSlot* pSkillSlot,
                                 CEffectID_t CEffectID)

{
    __BEGIN_TRY


    Assert(pSlayer != NULL);
    Assert(pSkillSlot != NULL);

    try {
        Player* pPlayer = pSlayer->getPlayer();
        Zone* pZone = pSlayer->getZone();

        Assert(pPlayer != NULL);
        Assert(pZone != NULL);

        // 무장하고 있는 무기가 널이거나, 도가 아니라면 사용할 수 없다.
        Item* pItem = pSlayer->getWearItem(Slayer::WEAR_RIGHTHAND);
        if (pItem == NULL || pItem->getItemClass() != Item::ITEM_CLASS_BLADE) {
            executeSkillFailException(pSlayer, getSkillType());
            return;
        }


        GCSkillToTileOK1 _GCSkillToTileOK1;
        GCSkillToTileOK5 _GCSkillToTileOK5;

        SkillType_t SkillType = pSkillSlot->getSkillType();
        SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(SkillType);

        int RequiredMP = (int)pSkillInfo->getConsumeMP();
        bool bManaCheck = hasEnoughMana(pSlayer, RequiredMP);
        bool bTimeCheck = verifyRunTime(pSkillSlot);
        bool bRangeCheck = checkZoneLevelToUseSkill(pSlayer);
        bool bHitRoll = true; // HitRoll::isSuccessMagic(pSlayer, pSkillInfo, pSkillSlot);
        bool bEffected = pSlayer->isFlag(Effect::EFFECT_CLASS_BURNING_SOL_CHARGE_1);

        if (bManaCheck && bTimeCheck && bRangeCheck && bHitRoll && !bEffected) {
            decreaseMana(pSlayer, RequiredMP, _GCSkillToTileOK1);

            // 지속 시간을 계산한다.
            SkillInput input(pSlayer, pSkillSlot);
            SkillOutput output;
            computeOutput(input, output);

            // 이팩트 클래스를 만들어 붙인다.
            EffectBurningSolCharging* pEffect = new EffectBurningSolCharging(pSlayer);
            pEffect->setDeadline(300);
            pEffect->setNextTime(10);
            pEffect->setLevel(0);
            pSlayer->addEffect(pEffect);
            pSlayer->setFlag(Effect::EFFECT_CLASS_BURNING_SOL_CHARGE_1);

            // 경험치를 올린다.

            _GCSkillToTileOK1.setSkillType(SkillType);
            _GCSkillToTileOK1.setCEffectID(CEffectID);
            _GCSkillToTileOK1.setX(X);
            _GCSkillToTileOK1.setY(Y);

            _GCSkillToTileOK5.setObjectID(pSlayer->getObjectID());
            _GCSkillToTileOK5.setSkillType(SkillType);
            _GCSkillToTileOK5.setX(X);
            _GCSkillToTileOK5.setY(Y);

            pPlayer->sendPacket(&_GCSkillToTileOK1);
            pZone->broadcastPacket(pSlayer->getX(), pSlayer->getY(), &_GCSkillToTileOK5, pSlayer);

            GCAddEffect gcAddEffect;
            gcAddEffect.setObjectID(pSlayer->getObjectID());
            gcAddEffect.setEffectID(pEffect->getSendEffectClass());
            gcAddEffect.setDuration(300);
            pZone->broadcastPacket(pSlayer->getX(), pSlayer->getY(), &gcAddEffect);

            pSkillSlot->setRunTime(output.Delay);
        } else {
            executeSkillFailNormal(pSlayer, getSkillType(), NULL);
        }
    } catch (Throwable& t) {
        executeSkillFailException(pSlayer, getSkillType());
    }


    __END_CATCH
}

BurningSolCharging g_BurningSolCharging;
