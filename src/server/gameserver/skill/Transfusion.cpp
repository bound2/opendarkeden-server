//////////////////////////////////////////////////////////////////////////////
// Filename    : Transfusion.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "Transfusion.h"

#include "EffectComa.h"
#include "EffectKillAftermath.h"
#include "GCAddEffectToTile.h"
#include "GCRemoveEffect.h"
#include "GCSkillToTileOK1.h"
#include "GCSkillToTileOK2.h"
#include "GCSkillToTileOK3.h"
#include "GCSkillToTileOK4.h"
#include "GCSkillToTileOK5.h"
#include "GCSkillToTileOK6.h"
#include "GCStatusCurrentHP.h"
#include "Properties.h"
#include "RankBonus.h"

//////////////////////////////////////////////////////////////////////////////
// 뱀파이어 오브젝트 핸들러
//////////////////////////////////////////////////////////////////////////////
void Transfusion::execute(Vampire* pVampire, ObjectID_t TargetObjectID, VampireSkillSlot* pVampireSkillSlot,
                          CEffectID_t CEffectID)

{
    __BEGIN_TRY


    Assert(pVampire != NULL);
    Assert(pVampireSkillSlot != NULL);


    try {
        Zone* pZone = pVampire->getZone();
        Assert(pZone != NULL);

        Creature* pTargetCreature = pZone->getCreature(TargetObjectID);

        // NPC는 공격할 수가 없다.
        // NoSuch제거. by sigi. 2002.5.2
        if (pTargetCreature == NULL ||
            pTargetCreature->isNPC()
            // HIDE 인 놈은 되살려내면 이상하게 된다. 일단 막아놓음.
            // 2003. 1. 17. DEW
            || pTargetCreature->isFlag(Effect::EFFECT_CLASS_HIDE) ||
            (g_pConfig->hasKey("Hardcore") && g_pConfig->getPropertyInt("Hardcore") != 0 &&
             pTargetCreature->isDead())) {
            executeSkillFailException(pVampire, getSkillType());
            return;
        }


        ZoneCoord_t X = pTargetCreature->getX();
        ZoneCoord_t Y = pTargetCreature->getY();

        GCSkillToTileOK1 _GCSkillToTileOK1;
        GCSkillToTileOK2 _GCSkillToTileOK2;
        GCSkillToTileOK3 _GCSkillToTileOK3;
        GCSkillToTileOK4 _GCSkillToTileOK4;
        GCSkillToTileOK5 _GCSkillToTileOK5;
        GCSkillToTileOK6 _GCSkillToTileOK6;

        SkillType_t SkillType = pVampireSkillSlot->getSkillType();
        SkillInfo* pSkillInfo = g_pSkillInfoManager->getSkillInfo(SkillType);

        // Knowledge of Innate 가 있다면 hit bonus 10
        int HitBonus = 0;
        if (pVampire->hasRankBonus(RankBonus::RANK_BONUS_KNOWLEDGE_OF_INNATE)) {
            RankBonus* pRankBonus = pVampire->getRankBonus(RankBonus::RANK_BONUS_KNOWLEDGE_OF_INNATE);
            Assert(pRankBonus != NULL);

            HitBonus = pRankBonus->getPoint();
        }

        // 15%를 사용
        int CurrentHP = pVampire->getHP(ATTR_CURRENT);
        int RequiredMP = CurrentHP * 12 / 100; // decreaseConsumeMP(pVampire, pSkillInfo);
        int RecoverHP = CurrentHP * 12 / 100;
        bool bTimeCheck = verifyRunTime(pVampireSkillSlot);
        bool bRangeCheck = verifyDistance(pVampire, X, Y, pSkillInfo->getRange());
        bool bHitRoll = HitRoll::isSuccessMagic(pVampire, pSkillInfo, pVampireSkillSlot, HitBonus);
        bool bHPCheck = false;

        Range_t Range = 1;

        // 상대방의 HP가 Full이 아니고
        // 자신의 HP가 30이상
        if (pVampire->getHP(ATTR_CURRENT) >= 30) {
            if (pTargetCreature->isVampire()) {
                Vampire* pTargetVampire = dynamic_cast<Vampire*>(pTargetCreature);

                // 현재HP+SilverDamage < MaxHP 여야 한다.
                if (pTargetVampire->getHP(ATTR_CURRENT) + pTargetVampire->getSilverDamage() <
                    pTargetVampire->getHP(ATTR_MAX))
                    bHPCheck = true;
            }
        }

        if (bTimeCheck && bRangeCheck && bHitRoll && bHPCheck) {
            decreaseMana(pVampire, RequiredMP, _GCSkillToTileOK1);


            // 데미지와 지속 시간을 계산한다.
            SkillInput input(pVampire);
            SkillOutput output;
            computeOutput(input, output);

            // TargetCreature의 HP를 채운다
            if (pTargetCreature->isVampire()) {
                Vampire* pTargetVampire = dynamic_cast<Vampire*>(pTargetCreature);

                HP_t maxHP = pTargetVampire->getHP(ATTR_MAX);
                HP_t currentHP = pTargetVampire->getHP(ATTR_CURRENT);
                HP_t newHP = min((int)maxHP, currentHP + RecoverHP);

                pTargetVampire->setHP(newHP);

                // HP가 30%가 되면(33% -_-;) 살아나게 된다.
                if (pTargetCreature->isFlag(Effect::EFFECT_CLASS_COMA)) {
                    if (newHP * 3 >= maxHP) {
                        EffectComa* pEffectComa = (EffectComa*)(pTargetCreature->findEffect(Effect::EFFECT_CLASS_COMA));
                        Assert(pEffectComa != NULL);

                        if (pEffectComa->canResurrect()) {
                            // 타겟의 이펙트 매니저에서 코마 이펙트를 삭제한다.
                            pTargetCreature->deleteEffect(Effect::EFFECT_CLASS_COMA);
                            pTargetCreature->removeFlag(Effect::EFFECT_CLASS_COMA);

                            // 코마 이펙트가 날아갔다고 알려준다.
                            GCRemoveEffect gcRemoveEffect;
                            gcRemoveEffect.setObjectID(pTargetCreature->getObjectID());
                            gcRemoveEffect.addEffectList((EffectID_t)Effect::EFFECT_CLASS_COMA);
                            pZone->broadcastPacket(pTargetCreature->getX(), pTargetCreature->getY(), &gcRemoveEffect);

                            // 이펙트 정보를 다시 보내준다. by sigi. 2002.11.14
                            pTargetCreature->getEffectManager()->sendEffectInfo(
                                pTargetCreature, pZone, pTargetCreature->getX(), pTargetCreature->getY());

                            // EffectKillAftermath 를 붙인다.
                            if (pTargetCreature->isFlag(Effect::EFFECT_CLASS_KILL_AFTERMATH)) {
                                Effect* pEffect = pTargetCreature->findEffect(Effect::EFFECT_CLASS_KILL_AFTERMATH);
                                EffectKillAftermath* pEffectKillAftermath = dynamic_cast<EffectKillAftermath*>(pEffect);
                                pEffectKillAftermath->setDeadline(5 * 600);
                            } else {
                                EffectKillAftermath* pEffectKillAftermath = new EffectKillAftermath(pTargetCreature);
                                pEffectKillAftermath->setDeadline(5 * 600);
                                pTargetCreature->addEffect(pEffectKillAftermath);
                                pTargetCreature->setFlag(Effect::EFFECT_CLASS_KILL_AFTERMATH);
                                pEffectKillAftermath->create(pTargetCreature->getName());
                            }
                        }
                    }
                }

                // 주위에 체력이 채워졌다는 사실을 알린다.
                GCStatusCurrentHP gcStatusCurrentHP;
                gcStatusCurrentHP.setObjectID(pTargetVampire->getObjectID());
                gcStatusCurrentHP.setCurrentHP(pTargetVampire->getHP(ATTR_CURRENT));
                pZone->broadcastPacket(X, Y, &gcStatusCurrentHP);

                // 자신의 에너지가 줄어든것도 보여주자
                gcStatusCurrentHP.setObjectID(pVampire->getObjectID());
                gcStatusCurrentHP.setCurrentHP(pVampire->getHP(ATTR_CURRENT));
                pZone->broadcastPacket(pVampire->getX(), pVampire->getY(), &gcStatusCurrentHP);
            }


            ZoneCoord_t myX = pVampire->getX();
            ZoneCoord_t myY = pVampire->getY();

            _GCSkillToTileOK1.setSkillType(SkillType);
            _GCSkillToTileOK1.setCEffectID(CEffectID);
            _GCSkillToTileOK1.setX(X);
            _GCSkillToTileOK1.setY(Y);
            _GCSkillToTileOK1.setDuration(output.Duration);
            _GCSkillToTileOK1.setRange(Range);

            _GCSkillToTileOK2.setSkillType(SkillType);
            _GCSkillToTileOK2.setObjectID(pVampire->getObjectID());
            _GCSkillToTileOK2.setX(X);
            _GCSkillToTileOK2.setY(Y);
            _GCSkillToTileOK2.setDuration(output.Duration);
            _GCSkillToTileOK2.setRange(Range);

            _GCSkillToTileOK3.setObjectID(pVampire->getObjectID());
            _GCSkillToTileOK3.setSkillType(SkillType);
            _GCSkillToTileOK3.setX(X);
            _GCSkillToTileOK3.setY(Y);

            _GCSkillToTileOK4.setSkillType(SkillType);
            _GCSkillToTileOK4.setX(X);
            _GCSkillToTileOK4.setY(Y);
            _GCSkillToTileOK4.setDuration(output.Duration);
            _GCSkillToTileOK4.setRange(Range);

            _GCSkillToTileOK5.setObjectID(pVampire->getObjectID());
            _GCSkillToTileOK5.setSkillType(SkillType);
            _GCSkillToTileOK5.setX(X);
            _GCSkillToTileOK5.setY(Y);
            _GCSkillToTileOK5.setDuration(output.Duration);
            _GCSkillToTileOK5.setRange(Range);

            Player* pPlayer = pVampire->getPlayer();
            Assert(pPlayer != NULL);
            pPlayer->sendPacket(&_GCSkillToTileOK1);

            list<Creature*> cList;
            cList.push_back(pVampire);

            cList = pZone->broadcastSkillPacket(myX, myY, X, Y, &_GCSkillToTileOK5, cList);

            pZone->broadcastPacket(myX, myY, &_GCSkillToTileOK3, cList);
            pZone->broadcastPacket(X, Y, &_GCSkillToTileOK4, cList);

            pVampireSkillSlot->setRunTime(output.Delay);
        } else {
            executeSkillFailNormal(pVampire, getSkillType(), NULL);
        }

    } catch (Throwable& t) {
        executeSkillFailException(pVampire, getSkillType());
    }


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// 뱀파이어 타일 핸들러
//////////////////////////////////////////////////////////////////////////////
void Transfusion::execute(Vampire* pVampire, ZoneCoord_t X, ZoneCoord_t Y, VampireSkillSlot* pVampireSkillSlot,
                          CEffectID_t CEffectID)

{
    __BEGIN_TRY

    try {
        Zone* pZone = pVampire->getZone();
        Assert(pZone != NULL);

        VSRect rect(0, 0, pZone->getWidth() - 1, pZone->getHeight() - 1);

        if (rect.ptInRect(X, Y)) {
            Tile& rTile = pZone->getTile(X, Y);

            if (rTile.hasWalkingCreature()) {
                Creature* pCreature = rTile.getCreature(Creature::MOVE_MODE_WALKING);

                execute(pVampire, pCreature->getObjectID(), pVampireSkillSlot, CEffectID);
            } else {
                executeSkillFailException(pVampire, getSkillType());
            }
        } else {
            executeSkillFailException(pVampire, getSkillType());
        }
    } catch (Throwable& t) {
        executeSkillFailException(pVampire, getSkillType());
    }

    __END_CATCH
}


Transfusion g_Transfusion;
