//////////////////////////////////////////////////////////////////////////////
// Filename    : DuplicateSelf.cpp
// Written by  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "DuplicateSelf.h"

#include "GCFakeMove.h"
#include "GCSay.h"
#include "GCSkillToTileOK5.h"
#include "MasterLairInfoManager.h"
#include "MasterLairManager.h"
#include "MonsterManager.h"
#include "MonsterSummonInfo.h"
#include "ZoneUtil.h"


//////////////////////////////////////////////////////////////////////////////
// 생성자
// 마스크를 초기화한다.
//////////////////////////////////////////////////////////////////////////////
DuplicateSelf::DuplicateSelf() {
    __BEGIN_TRY

    // 머.. 답답하믄 테이블로 빼든지.. -_-;
    m_DuplicateMonsterTypes[432] = 435; // 바토리 분신
    m_DuplicateMonsterTypes[434] = 436; // 테페즈 분신

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// 뱀파이어 셀프 핸들러
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// 몬스터 셀프 핸들러
//////////////////////////////////////////////////////////////////////////////
void DuplicateSelf::execute(Monster* pMonster)

{
    __BEGIN_TRY


    MonsterType_t MType = pMonster->getMonsterType();

    unordered_map<MonsterType_t, MonsterType_t>::const_iterator itr = m_DuplicateMonsterTypes.find(MType);

    // 분신할 MonsterType이 없으면 분신 모하지..
    if (itr == m_DuplicateMonsterTypes.end()) {
        return;
    }

    MonsterType_t DuplicateMType = itr->second;

    Assert(pMonster != NULL);

    try {
        Zone* pZone = pMonster->getZone();
        Assert(pZone != NULL);

        if (pMonster->isFlag(Effect::EFFECT_CLASS_HIDE)) {
            return;
        }
        if (pMonster->isFlag(Effect::EFFECT_CLASS_INVISIBILITY)) {
            addVisibleCreature(pZone, pMonster, true);
        }


        ZoneCoord_t x = pMonster->getX();
        ZoneCoord_t y = pMonster->getY();

        bool bRangeCheck = checkZoneLevelToUseSkill(pMonster);

        if (bRangeCheck) // && bMoveModeCheck)
        {
            GCSkillToTileOK5 _GCSkillToTileOK5;

            _GCSkillToTileOK5.setObjectID(pMonster->getObjectID());
            _GCSkillToTileOK5.setSkillType(getSkillType());
            _GCSkillToTileOK5.setX(x);
            _GCSkillToTileOK5.setY(y);
            _GCSkillToTileOK5.setDuration(0);

            pZone->broadcastPacket(x, y, &_GCSkillToTileOK5);

            //--------------------------------------------------------
            // 주위에 knockback되는맞는 애들을 체크해준다.
            //--------------------------------------------------------
            // 몬스터를 존에 추가한다.
            SUMMON_INFO summonInfo;
            summonInfo.scanEnemy = true;
            summonInfo.hasItem = false;
            summonInfo.initHPPercent = pMonster->getHP(ATTR_CURRENT) * 100 / pMonster->getHP(ATTR_MAX);

            int numFake = min((1 + rand() % 3), pMonster->getINT() / 100);

            MonsterManager* pMonsterManager = pZone->getMonsterManager();
            Assert(pMonsterManager != NULL);

            list<Monster*> summonedMonsters;

            for (int i = 0; i < numFake; i++) {
                int X = max(0, min((int)pZone->getWidth() - 1, (x - 5 + rand() % 11)));
                int Y = max(0, min((int)pZone->getHeight() - 1, (y - 5 + rand() % 11)));

                try {
                    pMonsterManager->addMonsters(X, Y, DuplicateMType, 1, summonInfo, &summonedMonsters);
                } catch (Throwable& t) {
                    cerr << t.toString() << endl;
                }
            }

            // 잔상을 보여준다.
            list<Monster*>::const_iterator iMonster = summonedMonsters.begin();

            for (; iMonster != summonedMonsters.end(); iMonster++) {
                Monster* pFakeMonster = *iMonster;

                GCFakeMove gcFakeMove;
                gcFakeMove.setObjectID(pMonster->getObjectID());
                gcFakeMove.setXY(pFakeMonster->getX(), pFakeMonster->getY());

                pZone->broadcastPacket(x, y, &gcFakeMove);
            }

            // 괜히 몬스터도 어딘가로 이동해본다.
            // 50번 시도..
            for (int i = 0; i < 50; i++) {
                int X = max(0, min((int)pZone->getWidth() - 1, (x - 8 + rand() % 11)));
                int Y = max(0, min((int)pZone->getHeight() - 1, (y - 8 + rand() % 11)));

                if (pZone->moveFastMonster(pMonster, x, y, X, Y, getSkillType())) {
                    break;
                }
            }
        } else {
            executeSkillFailNormal(pMonster, getSkillType(), NULL);
        }
    } catch (Throwable& t) {
        executeSkillFailException(pMonster, getSkillType());
    }


    __END_CATCH
}

DuplicateSelf g_DuplicateSelf;
