//////////////////////////////////////////////////////////////////////////////
// Filename    : Peace.cpp
// Written by  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "Peace.h"

#include "EffectPeace.h"
#include "GCSkillToSelfOK1.h"
#include "GCSkillToSelfOK2.h"
#include "GCSkillToTileOK1.h"
#include "GCSkillToTileOK2.h"
#include "GCSkillToTileOK3.h"
#include "GCSkillToTileOK4.h"
#include "GCSkillToTileOK5.h"


//////////////////////////////////////////////////////////////////////////////
// 슬레이어 오브젝트 핸들러
//////////////////////////////////////////////////////////////////////////////
void Peace::execute(Slayer* pSlayer, ObjectID_t TargetObjectID, SkillSlot* pSkillSlot, CEffectID_t CEffectID)

{
    __BEGIN_TRY


    SkillInput input(pSlayer, pSkillSlot);
    SkillOutput output;
    input.TargetType = SkillInput::TARGET_OTHER;
    computeOutput(input, output);

    SIMPLE_SKILL_INPUT param;
    param.SkillType = getSkillType();
    param.SkillDamage = output.Damage;
    param.Delay = output.Delay;
    param.STRMultiplier = 1;
    param.DEXMultiplier = 1;
    param.INTMultiplier = 8;

    SIMPLE_SKILL_OUTPUT result;

    execute(pSlayer, pSlayer->getX(), pSlayer->getY(), pSkillSlot, param, result, CEffectID);


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// 슬레이어 셀프 핸들러
//////////////////////////////////////////////////////////////////////////////
void Peace::execute(Slayer* pSlayer, SkillSlot* pSkillSlot, CEffectID_t CEffectID)

{
    __BEGIN_TRY


    SkillInput input(pSlayer, pSkillSlot);
    SkillOutput output;
    input.TargetType = SkillInput::TARGET_SELF;
    computeOutput(input, output);

    SIMPLE_SKILL_INPUT param;
    param.SkillType = getSkillType();
    param.SkillDamage = output.Damage;
    param.Delay = output.Delay;
    param.STRMultiplier = 1;
    param.DEXMultiplier = 1;
    param.INTMultiplier = 8;

    SIMPLE_SKILL_OUTPUT result;

    execute(pSlayer, pSlayer->getX(), pSlayer->getY(), pSkillSlot, param, result, CEffectID);


    __END_CATCH
}

void Peace::execute(Slayer* pSlayer, ZoneCoord_t X, ZoneCoord_t Y, SkillSlot* pSkillSlot, CEffectID_t CEffectID)

{
    __BEGIN_TRY

    SkillInput input(pSlayer, pSkillSlot);
    SkillOutput output;
    input.TargetType = SkillInput::TARGET_SELF;
    computeOutput(input, output);

    SIMPLE_SKILL_INPUT param;
    param.SkillType = getSkillType();
    param.SkillDamage = output.Damage;
    param.Delay = output.Delay;
    param.STRMultiplier = 1;
    param.DEXMultiplier = 1;
    param.INTMultiplier = 8;

    SIMPLE_SKILL_OUTPUT result;

    execute(pSlayer, X, Y, pSkillSlot, param, result, CEffectID);

    __END_CATCH
}


void Peace::execute(Slayer* pSlayer, ZoneCoord_t X, ZoneCoord_t Y, SkillSlot* pSkillSlot,
                    const SIMPLE_SKILL_INPUT& param, SIMPLE_SKILL_OUTPUT& result, CEffectID_t CEffectID)

{
    __BEGIN_TRY

    Assert(pSlayer != NULL);
    Assert(pSkillSlot != NULL);

    try {
        Player* pPlayer = pSlayer->getPlayer();
        Zone* pZone = pSlayer->getZone();

        GCSkillToTileOK1 _GCSkillToTileOK1;
        GCSkillToTileOK2 _GCSkillToTileOK2;
        GCSkillToTileOK3 _GCSkillToTileOK3;
        GCSkillToTileOK4 _GCSkillToTileOK4;
        GCSkillToTileOK5 _GCSkillToTileOK5;

        SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(param.SkillType);
        SkillDomainType_t DomainType = pSkillInfo->getDomainType();

        int RequiredMP = (int)pSkillInfo->getConsumeMP();
        bool bManaCheck = hasEnoughMana(pSlayer, RequiredMP);
        bool bTimeCheck = verifyRunTime(pSkillSlot);
        bool bRangeCheck = checkZoneLevelToUseSkill(pSlayer);

        ZoneCoord_t myX = pSlayer->getX();
        ZoneCoord_t myY = pSlayer->getY();

        if (bManaCheck && bTimeCheck && bRangeCheck) {
            int Splash = 3 + pSkillSlot->getExpLevel() / 33;

            list<Creature*> cList;
            list<Creature*> creatureList;
            getSplashVictims(pZone, X, Y, Creature::CREATURE_CLASS_MAX, creatureList, Splash);

            bool bSuccess = false;
            Duration_t duration = 100 + pSlayer->getINT() / 4 + pSkillSlot->getExpLevel() / 2;

            list<Creature*>::iterator itr = creatureList.begin();
            for (; itr != creatureList.end(); itr++) {
                Creature* pTargetCreature = (*itr);
                Assert(pTargetCreature != NULL);

                // 몬스터인 경우만 체크한다.
                // 뱀파이어 마스터는 나중에 제외시켜야한다.
                if (pTargetCreature->isMonster()) {
                    Monster* pMonster = dynamic_cast<Monster*>(pTargetCreature);
                    Assert(pMonster != NULL);

                    // 살아있고, 이미 Peace에 걸린 경우가 아닌 경우에 마법 건다.
                    if (pMonster->isAlive() && !pMonster->isFlag(Effect::EFFECT_CLASS_PEACE)) {
                        bool bHitRoll = HitRoll::isSuccessMagic(pSlayer, pSkillInfo, pSkillSlot);

                        if (bHitRoll) {
                            // pMonster가 pSlayer를 먼저 공격하지 않는 Effect
                            EffectPeace* pEffectPeace = new EffectPeace(pMonster, pSlayer->getObjectID());
                            pEffectPeace->setDeadline(duration); // 150+pSlayer->getINT()); // (15+INT/10)초*10
                            pMonster->addEffect(pEffectPeace);
                            pMonster->setFlag(Effect::EFFECT_CLASS_PEACE);

                            // 이미 때리고 있는 경우라면 제거..
                            pMonster->deleteEnemy(pSlayer->getObjectID());


                            bSuccess = true;

                            // 타겟 list를 패킷에 추가한다.

                            _GCSkillToTileOK1.addCListElement(pMonster->getObjectID());
                            _GCSkillToTileOK2.addCListElement(pMonster->getObjectID());
                            _GCSkillToTileOK4.addCListElement(pMonster->getObjectID());
                            _GCSkillToTileOK5.addCListElement(pMonster->getObjectID());
                        } else {
                        }
                    } else {
                    }
                }
            }

            if (bSuccess) {
                // 성공해야 마나를 떨어뜨린다.
                decreaseMana(pSlayer, RequiredMP, _GCSkillToTileOK1);

                // 경험치를 올려준다. (일단 detect hidden에서 긁었다. -_-;)
                SkillGrade Grade = g_pSkillInfoManager->getGradeByDomainLevel(pSlayer->getSkillDomainLevel(DomainType));
                Exp_t ExpUp = 10 * (Grade + 1) * 2;

                shareAttrExp(pSlayer, ExpUp, param.STRMultiplier, param.DEXMultiplier, param.INTMultiplier,
                             _GCSkillToTileOK1);
                increaseDomainExp(pSlayer, DomainType, pSkillInfo->getPoint(), _GCSkillToTileOK1);
                increaseSkillExp(pSlayer, DomainType, pSkillSlot, pSkillInfo, _GCSkillToTileOK1);

                pSkillSlot->setRunTime(param.Delay);
                result.bSuccess = true;
            }

            Dir_t dir = calcDirection(myX, myY, X, Y);

            _GCSkillToTileOK1.setSkillType(param.SkillType);
            _GCSkillToTileOK1.setCEffectID(CEffectID);
            _GCSkillToTileOK1.setX(X);
            _GCSkillToTileOK1.setY(Y);
            _GCSkillToTileOK1.setRange(dir);
            _GCSkillToTileOK1.setDuration(duration);

            _GCSkillToTileOK2.setObjectID(pSlayer->getObjectID());
            _GCSkillToTileOK2.setSkillType(param.SkillType);
            _GCSkillToTileOK2.setX(X);
            _GCSkillToTileOK2.setY(Y);
            _GCSkillToTileOK2.setRange(dir);
            _GCSkillToTileOK2.setDuration(duration);

            _GCSkillToTileOK3.setObjectID(pSlayer->getObjectID());
            _GCSkillToTileOK3.setSkillType(param.SkillType);
            _GCSkillToTileOK3.setX(X);
            _GCSkillToTileOK3.setY(Y);

            _GCSkillToTileOK4.setSkillType(param.SkillType);
            _GCSkillToTileOK4.setX(X);
            _GCSkillToTileOK4.setY(Y);
            _GCSkillToTileOK4.setDuration(duration);
            _GCSkillToTileOK4.setRange(dir);

            _GCSkillToTileOK5.setObjectID(pSlayer->getObjectID());
            _GCSkillToTileOK5.setSkillType(param.SkillType);
            _GCSkillToTileOK5.setX(X);
            _GCSkillToTileOK5.setY(Y);
            _GCSkillToTileOK5.setRange(dir);
            _GCSkillToTileOK5.setDuration(duration);

            pPlayer->sendPacket(&_GCSkillToTileOK1);


            // 이 기술에 의해 영향을 받는 놈들에게 패킷을 보내줘야 한다.
            // 몬스터라서 없다 - -;

            cList = pZone->broadcastSkillPacket(myX, myY, X, Y, &_GCSkillToTileOK5, cList);

            pZone->broadcastPacket(myX, myY, &_GCSkillToTileOK3, cList);
            pZone->broadcastPacket(X, Y, &_GCSkillToTileOK4, cList);
        } else {
            executeSkillFailNormal(pSlayer, param.SkillType, NULL);
        }
    } catch (Throwable& t) {
        executeSkillFailException(pSlayer, param.SkillType);
    }

    __END_CATCH
}

Peace g_Peace;
