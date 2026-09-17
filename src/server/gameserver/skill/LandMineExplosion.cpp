//////////////////////////////////////////////////////////////////////////////
// Filename    : LandMineExplosion.cpp
// Written by  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "LandMineExplosion.h"
// #include "GCSkillToSelfOK1.h"
#include "GCAddEffectToTile.h"
#include "SimpleTileMeleeSkill.h"


//////////////////////////////////////////////////////////////////////////////
// 생성자
// 마스크를 초기화한다.
//////////////////////////////////////////////////////////////////////////////
LandMineExplosion::LandMineExplosion() {
    __BEGIN_TRY

    int index = 0;

    for (int i = -3; i <= 3; ++i)
        for (int j = -3; j <= 3; ++j) {
            if (i == 0 && j == 0)
                continue;
            m_pLandMineExplosionMask[index++].set(i, j);
        }

    // 주위 8타일

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// 몬스터 셀프 핸들러
//////////////////////////////////////////////////////////////////////////////
void LandMineExplosion::execute(Monster* pMonster)

{
    __BEGIN_TRY


    Assert(pMonster != NULL);

    try {
        Zone* pZone = pMonster->getZone();
        Assert(pZone != NULL);


        ZoneCoord_t x = pMonster->getX();
        ZoneCoord_t y = pMonster->getY();

        bool bRangeCheck = checkZoneLevelToUseSkill(pMonster);

        if (bRangeCheck) {
            //--------------------------------------------------------
            // 주위에 knockback되는맞는 애들을 체크해준다.
            //--------------------------------------------------------
            SkillInput input(pMonster);
            SkillOutput output;
            computeOutput(input, output);

            SIMPLE_SKILL_INPUT param;
            param.SkillType = getSkillType();
            param.SkillDamage = output.Damage;
            param.Delay = output.Delay;
            param.ItemClass = Item::ITEM_CLASS_MAX;
            param.STRMultiplier = 0;
            param.DEXMultiplier = 0;
            param.INTMultiplier = 0;
            param.bMagicHitRoll = true;
            param.bMagicDamage = true;
            param.bAdd = false;

            SIMPLE_SKILL_OUTPUT result;

            for (int i = 0; i < 48; i++) {
                param.addMask(m_pLandMineExplosionMask[i].x, m_pLandMineExplosionMask[i].y, 100);
            }

            // 강제로 맞는 애들을 knockback 시킨다.
            g_SimpleTileMeleeSkill.execute(pMonster, x, y, param, result, 0, false);

            GCAddEffectToTile gcAE;
            gcAE.setXY(x, y);
            gcAE.setEffectID(Effect::EFFECT_CLASS_LAND_MINE_EXPLOSION);
            gcAE.setObjectID(0);
            gcAE.setDuration(0);
            pZone->broadcastPacket(x, y, &gcAE);
        } else {
            executeSkillFailNormal(pMonster, getSkillType(), NULL);
        }
    } catch (Throwable& t) {
        executeSkillFailException(pMonster, getSkillType());
    }


    __END_CATCH
}

LandMineExplosion g_LandMineExplosion;
