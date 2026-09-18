//////////////////////////////////////////////////////////////////////////////
// Filename    : SummonMonsters.cpp
// Written by  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "SummonMonsters.h"

#include "GCSay.h"
#include "GCSkillToTileOK5.h"
#include "GameContext.h"
#include "MasterLairInfoManager.h"
#include "MasterLairManager.h"
#include "MonsterSummonInfo.h"
#include "ZoneUtil.h"


//////////////////////////////////////////////////////////////////////////////
// 생성자
// 마스크를 초기화한다.
//////////////////////////////////////////////////////////////////////////////
SummonMonsters::SummonMonsters() {
    __BEGIN_TRY
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// 뱀파이어 셀프 핸들러
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// 몬스터 셀프 핸들러
//////////////////////////////////////////////////////////////////////////////
void SummonMonsters::execute(Monster* pMonster)

{
    __BEGIN_TRY


    Assert(pMonster != NULL);
    Zone* pZone = pMonster->getZone();
    Assert(pZone != NULL);


    try {
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
            //--------------------------------------------------------
            // 주위에 knockback되는맞는 애들을 체크해준다.
            //--------------------------------------------------------

            SUMMON_INFO2 summonInfo;

            bool hasInfo = pMonster->getMonsterSummonInfo(summonInfo);

            if (!hasInfo || summonInfo.pMonsters == NULL) {
                //  소환할 몹이 없는 경우다. -_-;
                executeSkillFailNormal(pMonster, getSkillType(), NULL);

                // 마스터 레어에서 마스터가 몹을 소환할려고 한 경우
                if (pZone->isMasterLair() && pMonster->isMaster()) {
                    MasterLairManager* pMasterLairManager = pZone->getMasterLairManager();
                    Assert(pMasterLairManager != NULL);

                    // 더 이상 소환할게 없다면..
                    // 마스터가 직접 나서서 싸워야겠지..
                    pMasterLairManager->setMasterReady();
                }
            }

            if (pMonster->isMaster() && pZone->isMasterLair()) {
                MasterLairManager* pMasterLairManager = pZone->getMasterLairManager();
                Assert(pMasterLairManager != NULL);
                // minion combat에서는 지정된 좌표에 소환한다.

                MasterLairInfo* pInfo = de::gameContext().masterLairInfos().getMasterLairInfo(pZone->getZoneID());
                Assert(pInfo != NULL);

                if (!pMasterLairManager->isMasterReady()) {
                    x = pInfo->getSummonX();
                    y = pInfo->getSummonY();

                    GCSay gcSay;
                    gcSay.setObjectID(pMonster->getObjectID());
                    gcSay.setColor(MASTER_SAY_COLOR);
                    gcSay.setMessage(pInfo->getRandomMasterSummonSay());
                    if (!gcSay.getMessage().empty())
                        pZone->broadcastPacket(pMonster->getX(), pMonster->getY(), &gcSay);
                }

                // 마스터 레어에서는 소환된 몬스터들이 아템 안 준다.
                // by sigi. 2002.11.21
                summonInfo.hasItem = false;
            }

            summonInfo.scanEnemy = true;
            summonInfo.clanType = SUMMON_INFO::CLAN_TYPE_GROUP;
            summonInfo.clanID = pMonster->getClanType(); // 주인의 clan을 따른다.
            summonInfo.X = x;
            summonInfo.Y = y;
            summonInfo.regenType = REGENTYPE_PORTAL;

            // 몬스터를 존에 추가한다.
            addMonstersToZone(pZone, summonInfo);

            GCSkillToTileOK5 _GCSkillToTileOK5;

            _GCSkillToTileOK5.setObjectID(pMonster->getObjectID());
            _GCSkillToTileOK5.setSkillType(getSkillType());
            _GCSkillToTileOK5.setX(x);
            _GCSkillToTileOK5.setY(y);
            _GCSkillToTileOK5.setDuration(0);

            pZone->broadcastPacket(x, y, &_GCSkillToTileOK5);
        } else {
            executeSkillFailNormal(pMonster, getSkillType(), NULL);
        }
    } catch (Throwable& t) {
        executeSkillFailException(pMonster, getSkillType());
    }

    __END_CATCH
}

SummonMonsters g_SummonMonsters;
