//////////////////////////////////////////////////////////////////////////////
// FileName 	: ZoneMove.cpp
// Description	: Zone movement: stepping a creature to a neighbouring tile and fast moves.
//////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <fstream>

#include "Assert.h"
#include "BloodBibleBonusManager.h"
#include "CastleInfoManager.h"
#include "CombatInfoManager.h"
#include "Creature.h"
#include "DarkLightInfo.h"
#include "DefaultOptionSetInfo.h"
#include "DynamicZone.h"
#include "EffectAddItem.h"
#include "EffectAddItemToCorpse.h"
#include "EffectCastingTrap.h"
#include "EffectContinualGroundAttack.h"
#include "EffectDarkness.h"
#include "EffectDecayCorpse.h"
#include "EffectDecayItem.h"
#include "EffectDeleteItem.h"
#include "EffectGnomesWhisper.h"
#include "EffectHasBloodBible.h"
#include "EffectHasCastleSymbol.h"
#include "EffectHasSlayerRelic.h"
#include "EffectHasVampireRelic.h"
#include "EffectLoaderManager.h"
#include "EffectManager.h"
#include "EffectObservingEye.h"
#include "EffectPKZoneRegen.h"
#include "EffectRelicTable.h"
#include "EffectSanctuary.h"
#include "EffectSchedule.h"
#include "EffectShrineGuard.h"
#include "EffectShrineHoly.h"
#include "EffectShrineShield.h"
#include "EffectSlayerRelic.h"
#include "EffectTransportItem.h"
#include "EffectTransportItemToCorpse.h"
#include "EffectVampirePortal.h"
#include "EffectVampireRelic.h"
#include "EventTransport.h"
#include "FlagSet.h"
#include "GCAddBat.h"
#include "GCAddBurrowingCreature.h"
#include "GCAddEffect.h"
#include "GCAddEffectToTile.h"
#include "GCAddInstalledMineToZone.h"
#include "GCAddMonster.h"
#include "GCAddMonsterCorpse.h"
#include "GCAddMonsterFromBurrowing.h"
#include "GCAddMonsterFromTransformation.h"
#include "GCAddNPC.h"
#include "GCAddNewItemToZone.h"
#include "GCAddOusters.h"
#include "GCAddOustersCorpse.h"
#include "GCAddSlayer.h"
#include "GCAddSlayerCorpse.h"
#include "GCAddVampire.h"
#include "GCAddVampireCorpse.h"
#include "GCAddVampireFromBurrowing.h"
#include "GCAddVampireFromTransformation.h"
#include "GCAddVampirePortal.h"
#include "GCAddWolf.h"
#include "GCDeleteEffectFromTile.h"
#include "GCDeleteObject.h"
#include "GCDropItemToZone.h"
#include "GCFastMove.h"
#include "GCHolyLandBonusInfo.h"
#include "GCKnockBack.h"
#include "GCMineExplosionOK1.h"
#include "GCMineExplosionOK2.h"
#include "GCModifyInformation.h"
#include "GCMove.h"
#include "GCMoveError.h"
#include "GCMoveOK.h"
#include "GCMyStoreInfo.h"
#include "GCNPCInfo.h"
#include "GCNoticeEvent.h"
#include "GCRegenZoneStatus.h"
#include "GCRemoveEffect.h"
#include "GCSetPosition.h"
#include "GCSweeperBonusInfo.h"
#include "GCSystemMessage.h"
#include "GCUnburrowFail.h"
#include "GCUnburrowOK.h"
#include "GCUnionOfferList.h"
#include "GCUntransformFail.h"
#include "GCUntransformOK.h"
#include "GDRLairManager.h"
#include "GGCommand.h"
#include "GQuestManager.h"
#include "GamePlayer.h"
#include "GameServerInfoManager.h"
#include "GuildManager.h"
#include "GuildUnion.h"
#include "HolyLandManager.h"
#include "Item.h"
#include "ItemFactoryManager.h"
#include "ItemInfo.h"
#include "LevelWarManager.h"
#include "LevelWarZoneInfoManager.h"
#include "LogClient.h"
#include "LoginServerManager.h"
#include "MasterLairInfoManager.h"
#include "MasterLairManager.h"
#include "Monster.h"
#include "MonsterCorpse.h"
#include "MonsterManager.h"
#include "NPC.h"
#include "NPCInfo.h"
#include "NPCManager.h"
#include "NicknameBook.h"
#include "Ousters.h"
#include "OustersCorpse.h"
#include "PCFinder.h"
#include "PCManager.h"
#include "PKZoneInfoManager.h"
#include "PacketUtil.h"
#include "ParkingCenter.h"
#include "Party.h"
#include "PaySystem.h"
#include "Player.h"
#include "Profile.h"
#include "Properties.h"
#include "QuestManager.h"
#include "RegenZoneManager.h"
#include "Relic.h"
#include "RelicUtil.h"
#include "ResurrectLocationManager.h"
#include "ShrineInfoManager.h"
#include "SiegeManager.h"
#include "SkillUtil.h"
#include "Slayer.h"
#include "SlayerCorpse.h"
#include "Store.h"
#include "StringPool.h"
#include "SweeperBonusManager.h"
#include "TimeManager.h"
#include "TradeManager.h"
#include "Vampire.h"
#include "VampireCorpse.h"
#include "VariableManager.h"
#include "VisionInfo.h"
#include "War.h"
#include "WarScheduler.h"
#include "WarSystem.h"
#include "WeatherManager.h"
#include "Zone.h"
#include "ZoneGroup.h"
#include "ZoneInfo.h"
#include "ZoneInfoManager.h"
#include "ZoneInternal.h"
#include "ZoneUtil.h"
#include "ctf/FlagManager.h"
#include "item/Motorcycle.h"
#include "item/VampirePortalItem.h"
#include "repository/ComebackEventRepository.h"
#include "repository/MessageRepository.h"
#include "repository/ZoneInfoRepository.h"


#ifdef __PROFILE_BROADCAST__
#define __BEGIN_PROFILE_ZONE(name) beginProfileEx(name);
#define __END_PROFILE_ZONE(name) endProfileEx(name);
#else
#define __BEGIN_PROFILE_ZONE(name) ((void)0);
#define __END_PROFILE_ZONE(name) ((void)0);
#endif


#ifndef __FULL_PROFILE__
#undef beginProfileEx
#define beginProfileEx(name) ((void)0)
#undef endProfileEx
#define endProfileEx(name) ((void)0)
#endif

int g_FastMoveSearchX[8][4] = {
    {0, 1, 1, 1},    // LEFT
    {0, 1, 0, 1},    // LEFTDOWN
    {0, 0, -1, 1},   // DOWN
    {0, -1, 0, -1},  // RIGHTDOWN
    {0, -1, -1, -1}, // RIGHT
    {0, -1, 0, -1},  // RIGHTUP
    {0, 0, -1, 1},   // UP
    {0, 1, 0, 1},    // LEFTUP
};

int g_FastMoveSearchY[8][4] = {
    {0, 0, -1, 1},   // LEFT
    {0, -1, -1, 0},  // LEFTDOWN
    {0, -1, -1, -1}, // DOWN
    {0, -1, -1, 0},  // RIGHTDOWN
    {0, 0, -1, 1},   // RIGHT
    {0, 1, 1, 0},    // RIGHTUP
    {0, 1, 1, 1},    // UP
    {0, 1, 1, 0},    // LEFTUP
};

//////////////////////////////////////////////////////////////////////////////
// 기본적으로 Zone 의 처리는 mutex 를 사용하지 않는다.
// 왜냐하면, ZoneGroupThread의 단일 처리를 받기 때문이다. 그런데, 새로 존에
// PC를 추가하는 것은 IPM에서 이루어지게 되므로, 이런 연유로 mutex 멤버를
// 추가해야 하며, 아래 메소드에 locking 을 걸어줘야 한다.
//////////////////////////////////////////////////////////////////////////////
void Zone::pushPC(Creature* pCreature)

{
    __BEGIN_TRY

    Assert(pCreature != NULL);

    __ENTER_CRITICAL_SECTION(m_Mutex)

    m_PCListQueue.push_back(pCreature);

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// P(cx,cy)에 있는 PC를 dir 방향으로 이동시켜 Q(nx,ny)로 옮긴다.
// 그리고나서, 주변의 PC들에게 이동 정보를 브로드캐스트한다.
//
// *CAUTION*
// PC가 아닌 크리처(NPC,Monster)의 이동은 moveCreature를 사용한다.
//////////////////////////////////////////////////////////////////////////////
void Zone::movePC(Creature* pCreature, ZoneCoord_t cx, ZoneCoord_t cy, Dir_t dir)

{
    __BEGIN_TRY

    Assert(pCreature->isPC());

    Player* pPlayer = pCreature->getPlayer();
    Assert(pPlayer != NULL);

    if (dir >= DIR_MAX || !isAbleToMove(pCreature)) {
        GCMoveError gcMoveError(pCreature->getX(), pCreature->getY());
        pPlayer->sendPacket(&gcMoveError);
        filelog("ZoneDebug.txt", "movePC - 1\n\r");
        return;
    }

    const int threshold = 6;
    ////////////////////////////////////////////////////////////
    // 일단 크리처가 점프를 하려는 건지를 체크한다.
    // 만약 약간 점프했을 경우, GCMoveError 패킷을 전송한다.
    //
    // OX, OY : 플레이어의 현재 좌표
    // CX, CY : 타겟 좌표
    //
    // max(0, OX - threshold) <= CX <= min(OX + threshold, ZONEWIDTH-1)
    // max(0, OY - threshold) <= CY <= min(OY + threshold, ZONEHEIGHT-1)
    // 를 만족해야 정상적인 이동이다.
    ////////////////////////////////////////////////////////////
    if (pCreature->getX() != cx || pCreature->getY() != cy) {
        if (cx >= max(0, pCreature->getX() - threshold) && cx <= min(m_Width - 1, pCreature->getX() + threshold) &&
            cy >= max(0, pCreature->getY() - threshold) && cy <= min(m_Height - 1, pCreature->getY() + threshold)) {
            // 허용가능한 오차 범위내에서의 점프는 그냥 무시해준다.

            filelog("ZoneDebug.txt", "movePC - 2\n\r");
            return;
        } else {
            // 허용가능한 오차 범위를 넘어설 경우 접속을 차단한다.

            GCMoveError gcMoveError(cx, cy);
            pPlayer->sendPacket(&gcMoveError);
            filelog("ZoneDebug.txt", "movePC - 3\n\r");
            return;
        }
    }

    // 다음 좌표를 계산한다.
    int nx = cx;
    int ny = cy;

    //////////////////////////////////////////////////////////////////////////////
    // *CAUTION*
    // 경계지점에서 경계의 외곽으로 움직이는 패킷이 날아와서는 안된다.
    // ex> (0,10)에서 LEFT 이동은 날아올 수 없다. (10,0)에서 UP 이동도 마찬가지이다.
    //////////////////////////////////////////////////////////////////////////////
    nx = nx + dirMoveMask[dir].x;
    ny = ny + dirMoveMask[dir].y;

    VSRect rect(0, 0, m_Width - 1, m_Height - 1);
    if (!rect.ptInRect(nx, ny))
        throw InvalidProtocolException("invalid coordination");

    ////////////////////////////////////////////////////////////
    // 이동할 곳이 block 되어있다면, GCMoveError 를 전송한다.
    // (PC와 같은 위치가 block 되어야 한다.)
    ////////////////////////////////////////////////////////////
    Tile& newTile = m_pTiles[nx][ny];
    Tile& oldTile = m_pTiles[cx][cy];

    // 성물을 가지고 있는 경우라면.. 안전지대에 들어갈 수 없다.
    if (pCreature->hasRelicItem() || pCreature->isFlag(Effect::EFFECT_CLASS_HAS_FLAG) ||
        pCreature->isFlag(Effect::EFFECT_CLASS_HAS_SWEEPER)) {
        ZoneLevel_t ZoneLevel = getZoneLevel(nx, ny);

        // 슬레이어이면 슬레이어 안전지대에 못 들어간다.
        // 뱀파이어이면  뱀파이어안전지대에 못 들어간다.
        // 공통 안전지대이면 못 들어간다.
        if (pCreature->isSlayer() && (ZoneLevel & SLAYER_SAFE_ZONE) ||
            pCreature->isVampire() && (ZoneLevel & VAMPIRE_SAFE_ZONE) ||
            pCreature->isOusters() && (ZoneLevel & OUSTERS_SAFE_ZONE) || (ZoneLevel & COMPLETE_SAFE_ZONE)) {
            GCMoveError gcMoveError(cx, cy);
            pPlayer->sendPacket(&gcMoveError);
            filelog("ZoneDebug.txt", "movePC - 4\n\r");
            return;
        }
    }

    if (newTile.hasCreature(pCreature->getMoveMode())) {
        Creature* pTargetCreature = newTile.getCreature(pCreature->getMoveMode());
        if (pTargetCreature != NULL && pTargetCreature->isMonster()) {
            Monster* pMonster = dynamic_cast<Monster*>(pTargetCreature);
            if (pMonster->getMonsterType() >= 738 && pMonster->getMonsterType() <= 740) {
                pMonster->addEnemy(pTargetCreature);
                pMonster->setHP(0);
            }
        }

        GCMoveError gcMoveError(cx, cy);
        pPlayer->sendPacket(&gcMoveError);
    }

    if (newTile.isBlocked(pCreature->getMoveMode())
        // BloodyWallBlocked나
        // Sanctuary 이펙트가 걸려있다면 못 간다.
        || newTile.hasEffect() && (newTile.getEffect(Effect::EFFECT_CLASS_BLOODY_WALL_BLOCKED) ||
                                   newTile.getEffect(Effect::EFFECT_CLASS_SANCTUARY)) ||
        oldTile.getEffect(Effect::EFFECT_CLASS_SANCTUARY) != NULL) {
        GCMoveError gcMoveError(cx, cy);
        pPlayer->sendPacket(&gcMoveError);
    } else {
        // 우선 크리처의 좌표를 변경한다.
        pCreature->setXYDir(nx, ny, dir);

        try {
            // 이전 타일에서 크리처를 삭제한다.
            m_pTiles[cx][cy].deleteCreature(pCreature->getObjectID());

            // 새 타일에 크리처를 추가한다.
            if (!newTile.addCreature(pCreature)) {
                // Portal을 activate 시킨 경우이다. by sigi. 2002.5.6
                return;
            }

            try {
                checkMine(this, pCreature, nx, ny);
                checkTrap(this, pCreature);
            } catch (Throwable& t) {
                filelog("CheckMineBug.txt", "%s : %s", "movePC", t.toString().c_str());
            }

            // 클라이언트에게 GCMoveOK 를 전송할때, (nx,ny)는 도착 좌표여야 하며,
            // dir 은 바라보는(이동할) 방향이어야 한다. 그것이 현재의 정책!
            GCMoveOK gcMoveOK(nx, ny, dir);
            pPlayer->sendPacket(&gcMoveOK);

            // 자동으로 GCMove/GCAddSlayer/GCAddVampire 패킷을 브로드캐스트한다.
            movePCBroadcast(pCreature, cx, cy, nx, ny);
        } catch (NoSuchElementException& nsee) {
            throw Error("The creature is not on the previous tile.");
        } catch (DuplicatedException& de) {
            throw Error("A creature is already on the new tile.");
        } catch (PortalException&) {
        } catch (Error& e) {
            filelog("assertTile.txt", "Zone::movePC : %s", e.toString().c_str());
            throw;
        }
    }
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// PC가 아닌 크리처(NPC,Monster)를 움직일 때 이 메소드를 사용한다.
//
// *CAUTION*
//
// 이때, (nx,ny,dir)은 크리처가 도달할 다음 좌표와 바라보는 방향을 나타낸다.
// 그리고, 이 좌표에 아무 것도 없다는 검증이 먼저 이루어져야 한다. (길찾기 루틴)
//////////////////////////////////////////////////////////////////////////////
void Zone::moveCreature(Creature* pCreature, ZoneCoord_t nx, ZoneCoord_t ny, Dir_t dir)

{
    __BEGIN_TRY

    if (m_pZoneGroup != NULL)
        m_pZoneGroup->assertOwned();

    ZoneCoord_t cx = pCreature->getX();
    ZoneCoord_t cy = pCreature->getY();

    // 이전 타일에서 크리처를 삭제하고, 다음 타일에 크리처를 추가한다.
    try {
        // 이전 타일에서 크리처를 삭제한다.
        m_pTiles[cx][cy].deleteCreature(pCreature->getObjectID());

        // 도착 타일에 크리처를 추가한다.
        m_pTiles[nx][ny].addCreature(pCreature);

        // 크리처의 좌표와 방향을 설정한다.
        pCreature->setXYDir(nx, ny, dir);

        try {
            checkMine(this, pCreature, nx, ny);
            checkTrap(this, pCreature);
        } catch (Throwable& t) {
            filelog("CheckMineBug.txt", "%s : %s", "moveCreature", t.toString().c_str());
        }

    } catch (NoSuchElementException& nsee) {
        throw Error("The creature is not on the previous tile.");
    } catch (DuplicatedException& de) {
        throw Error("A creature is already on the new tile.");
    } catch (Error& e) {
        filelog("assertTile.txt", "Zone::moveCreature : %s", e.toString().c_str());
        throw;
    }

    // 알아서 GCMove 랑 GCAddMonster/GCAddNPC 를 브로드캐스트한다.
    moveCreatureBroadcast(pCreature, cx, cy, nx, ny);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// P(x1,y1)에서 Q(x2,y2)로 빠른 이동한 크리처가 주변 영역에 존재하는 PC들에게
// 브로드캐스트하는 메쏘드이다.
// for Skill FlashSliding, ShadowWalk
//////////////////////////////////////////////////////////////////////////////
bool Zone::moveFastPC(Creature* pPC, ZoneCoord_t x1, ZoneCoord_t y1, ZoneCoord_t x2, ZoneCoord_t y2,
                      SkillType_t skillType) {
    __BEGIN_TRY
    __BEGIN_DEBUG

    // 이 메쏘드는 PC 를 대상으로 한다.
    Assert(pPC->isPC());

    // isAbleToMove 로 바꾼다. by bezz. 2002.12.28
    if (!isAbleToMove(pPC))
        return false;


    // 성물을 가지고 있는 경우라면.. 안전지대에 들어갈 수 없다.
    if (pPC->hasRelicItem()) {
        return false;
    }

    if (pPC->isFlag(Effect::EFFECT_CLASS_HAS_FLAG) || pPC->isFlag(Effect::EFFECT_CLASS_HAS_SWEEPER))
        return false;

    if (m_ZoneID == 1410 || m_ZoneID == 1411)
        return false;

    Tile& rTile = getTile(x1, y1);

    if (rTile.getEffect(Effect::EFFECT_CLASS_ON_BRIDGE) != NULL)
        return false;

    // 적당한 종착지점을 찾는다.
    // 전면부에서 적당한 지점..4개 검색
    Dir_t dir = calcDirection(x1, y1, x2, y2);

    // g_FastMoveSearchX, Y로 찾으면 된다. by sigi. 2002.5.8
    int* searchX = g_FastMoveSearchX[dir];
    int* searchY = g_FastMoveSearchY[dir];

    // 빈 타일인지 확인.
    int i = 0;
    for (i = 0; i < 4; i++) {
        int targetX = x2 + searchX[i], targetY = y2 + searchY[i];
        if (targetX >= 0 && targetX < m_Width && targetY >= 0 && targetY < m_Height &&
            !m_pTiles[targetX][targetY].isBlocked(pPC->getMoveMode()) && !m_pTiles[targetX][targetY].hasPortal() &&
            // Sanctuary 가 걸려있지 않아야 한다. by Sequoia 2003.3.25
            m_pTiles[targetX][targetY].getEffect(Effect::EFFECT_CLASS_SANCTUARY) == NULL &&
            m_pTiles[x1][y1].getEffect(Effect::EFFECT_CLASS_SANCTUARY) == NULL) {
            x2 = targetX;
            y2 = targetY;
            break;
        }
    }
    if (i == 4) {
        return false; // 빈타일을 못찾았다!
    }

    Player* pPlayer = pPC->getPlayer();
    Assert(pPlayer);

    GCFastMove gcFastMove;
    gcFastMove.setObjectID(pPC->getObjectID());
    gcFastMove.setXY(x1, y1, x2, y2);
    gcFastMove.setSkillType(skillType);

#ifdef __USE_ENCRYPTER__
    SocketEncryptOutputStream outputStream(NULL, szPacketHeader + gcFastMove.getPacketSize() + 2);
    outputStream.setEncryptCode(m_EncryptCode);
#else
    SocketOutputStream outputStream(NULL, szPacketHeader + gcFastMove.getPacketSize() + 2);
#endif
    gcFastMove.writeHeaderNBody(outputStream);

    pPlayer->sendStream(&outputStream);

    // 퀘스트..
    dynamic_cast<PlayerCreature*>(pPC)->getGQuestManager()->fastMove();

    //////////////////////////////////////////////////////////////
    // move의 종류....
    // 이에따라 GCDelete나 Add등을 보내줘야 할 수 도 있다.

    // PC의 좌표 변경.
    pPC->setXYDir(x2, y2, dir);
    // 이전 타일에서 크리처를 삭제한다.

    try {
        m_pTiles[x1][y1].deleteCreature(pPC->getObjectID());
    } catch (Error& e) {
        filelog("assertTile.txt", "moveFastPC : %s", e.toString().c_str());
        throw;
    }

    // 새 타일에 크리처를 추가한다.
    m_pTiles[x2][y2].addCreature(pPC);

    try {
        checkMine(this, pPC, x2, y2);
        checkTrap(this, pPC);
    } catch (Throwable& t) {
        filelog("CheckMineBug.txt", "%s : %s", "moveFastPC", t.toString().c_str());
    }

    if (pPC->isFlag(Effect::EFFECT_CLASS_GHOST)) {
    }


    //--------------------------------------------------------------------------------
    // GCAddSlayer/GCAddVampire 패킷을 만들어둔다.
    // 현재의 정책에 의하면, GCAdd 패킷은 현재의 좌표를 바탕으로 한다.
    //--------------------------------------------------------------------------------
    Packet* pGCAddXXX = NULL;

    if (pPC->getCreatureClass() == Creature::CREATURE_CLASS_SLAYER) {
        Slayer* pSlayer = dynamic_cast<Slayer*>(pPC);
        GCAddSlayer* pGCAddSlayer = new GCAddSlayer;
        makeGCAddSlayer(pGCAddSlayer, pSlayer);

        pGCAddXXX = pGCAddSlayer;
    } else if (pPC->getCreatureClass() == Creature::CREATURE_CLASS_VAMPIRE) {
        Vampire* pVampire = dynamic_cast<Vampire*>(pPC);

        // 음.. hide상태에서 움직일 수는 없지만..
        // 미래를 대비.
        if (pPC->isFlag(Effect::EFFECT_CLASS_HIDE)) {
            GCAddBurrowingCreature* pGCABC = new GCAddBurrowingCreature();
            pGCABC->setObjectID(pVampire->getObjectID());
            pGCABC->setName(pVampire->getName());
            pGCABC->setX(x2);
            pGCABC->setY(y2);
            pGCAddXXX = pGCABC;
        } else {
            GCAddVampire* pGCAddVampire = new GCAddVampire;
            makeGCAddVampire(pGCAddVampire, pVampire);

            pGCAddXXX = pGCAddVampire;
        }
    } else if (pPC->getCreatureClass() == Creature::CREATURE_CLASS_OUSTERS) {
        Ousters* pOusters = dynamic_cast<Ousters*>(pPC);
        GCAddOusters* pGCAddOusters = new GCAddOusters;
        makeGCAddOusters(pGCAddOusters, pOusters);

        pGCAddXXX = pGCAddOusters;
    }

    //--------------------------------------------------------------------------------
    // GCDeleteObject 패킷을 만들어둔다.
    //--------------------------------------------------------------------------------
    GCDeleteObject gcDeleteObject;
    gcDeleteObject.setObjectID(pPC->getObjectID());


    // 총 시야의 범위를 구한다.
    ZoneCoord_t minX, maxX, minY, maxY;
    if (x1 < x2) {
        minX = max(0, x1 - maxViewportWidth);
        maxX = min(m_Width - 1, x2 + maxViewportWidth);
    } else {
        minX = max(0, x2 - maxViewportWidth);
        maxX = min(m_Width - 1, x1 + maxViewportWidth);
    }
    if (y1 < y2) {
        minY = max(0, y1 - maxViewportUpperHeight);
        maxY = min(m_Height - 1, y2 + maxViewportLowerHeight);
    } else {
        minY = max(0, y2 - maxViewportUpperHeight);
        maxY = min(m_Height - 1, y1 + maxViewportLowerHeight);
    }


    // ObservingEye 이펙트를 가져온다.
    //		//Assert( pEffectObservingEye != NULL );
    //
    for (ZoneCoord_t ix = minX; ix <= maxX; ix++) {
        for (ZoneCoord_t iy = minY; iy <= maxY; iy++) {
            const forward_list<Object*>& objectList = m_pTiles[ix][iy].getObjectList();

            forward_list<Object*>::const_iterator itr = objectList.begin();

            // visionInfo 때문에..
            // if - do~while()로 구조 변경 by sigi. 2002.5.8
            if (itr != objectList.end()) {
                // 이전 좌표 P(x1,y1)에서 I(ix,iy)가 어떻게 보이는가?
                VisionState prevVisionState = VisionInfoManager::getVisionState(x1, y1, ix, iy);
                // 현재 좌표 Q(x2,y2)에서 I(ix,iy)가 어떻게 보이는가?
                VisionState curVisionState = VisionInfoManager::getVisionState(x2, y2, ix, iy);

                do {
                    Assert(*itr != NULL);

                    //--------------------------------------------------------------------------------
                    //
                    // 각 객체의 OBJECT CLASS에 따라서 적합한 GCAddXXX 패킷을 만들어서
                    // owner 에게 전송한다.
                    //
                    // *NOTES*
                    //
                    // 가장 출현 확률이 높은 객체 CLASS 가 case 앞부분에 나와야 한다.
                    //
                    //--------------------------------------------------------------------------------
                    switch ((*itr)->getObjectClass()) {
                    //--------------------------------------------------------------------------------
                    // 타일 위에 크리처가 있을 경우
                    //--------------------------------------------------------------------------------
                    case Object::OBJECT_CLASS_CREATURE: {
                        Creature* pCreature = dynamic_cast<Creature*>(*itr);
                        Assert(pCreature != NULL);

                        // 자기 자신의 정보는 받을 필요가 없다.
                        if (pCreature == pPC)
                            continue;

                        switch (pCreature->getCreatureClass()) {
                        case Creature::CREATURE_CLASS_MONSTER: {
                            Monster* pMonster = dynamic_cast<Monster*>(pCreature);

                            //--------------------------------------------------------------------------------
                            //
                            // 이전 좌표에서는 이 몬스터를 볼 수 없었으나, 도착 좌표에서 이 몬스터를 보게 될
                            // 경우 GCAddMonster 패킷을 전송한다.
                            //
                            //--------------------------------------------------------------------------------
                            if (prevVisionState == OUT_OF_SIGHT && curVisionState >= IN_SIGHT) {
                                Packet* pAddMonsterPacket = createMonsterAddPacket(pMonster, pPC);

                                if (pAddMonsterPacket != NULL) {
                                    pPlayer->sendPacket(pAddMonsterPacket);
                                    delete pAddMonsterPacket;
                                }
                            }

                            //--------------------------------------------------------------------------------
                            // PC를 몬스터의 잠재적인 적으로 지정해준다.
                            //--------------------------------------------------------------------------------
                            VisionState vs = pMonster->getVisionState(x2, y2);

                            // Aggressive 몬스터에게만 적으로 등록시켜준다.
                            if (vs >= IN_SIGHT && pMonster->getAlignment() == ALIGNMENT_AGGRESSIVE) {
                                if (isPotentialEnemy(pMonster, pPC)) {
                                    pMonster->addPotentialEnemy(pPC);
                                }
                            }

                        } break;

                        //--------------------------------------------------------------------------------
                        //
                        //--------------------------------------------------------------------------------
                        case Creature::CREATURE_CLASS_SLAYER: {
                            //--------------------------------------------------------------------------------
                            // 이전 좌표에서는 보이지 않다가, 이번 좌표에서 새로 보이게 된 크리처만
                            // GCAddXXX 를 받아온다. 계속 보일 경우에는 받아오지 않는다.
                            //--------------------------------------------------------------------------------
                            if (curVisionState >= IN_SIGHT && prevVisionState == OUT_OF_SIGHT) {
                                // 보는 이가 스나이핑 상태라면 디텍트 되어 있어야 한다.
                                //												if
                                //(!pCreature->isFlag(Effect::EFFECT_CLASS_SNIPING_MODE)
                                //													||
                                // pPC->isFlag(Effect::EFFECT_CLASS_DETECT_INVISIBILITY) )
                                //													|| ( pEffectRevealer != NULL &&
                                // pEffectRevealer->canSeeSniping( pCreature ) ) )
                                if (canSee(pPC, pCreature)) {
                                    Slayer* pSlayer = dynamic_cast<Slayer*>(pCreature);
                                    //													GCAddSlayer
                                    GCAddSlayer gcAddSlayer;
                                    makeGCAddSlayer(&gcAddSlayer, pSlayer);

                                    pPlayer->sendPacket(&gcAddSlayer);
                                }
                            }

                            Assert(pCreature->getPlayer() != NULL);

                            //--------------------------------------------------------------------------------
                            //
                            // Q(x2,y2)가 이 크리처의 시야 사각형의 경계에 위치하면서, P(x1,y1)은 사각형의 외부,
                            // 즉 보이지 않는 경우에만 GCAddXXX 패킷을 전송한다. 이렇게 하지 않으면, PC
                            // 크리처가 pCreature의 시야 경계에서 계속 움직이게 되면 계속 서버는 GCAddXXX 패킷을
                            // 보내야만 한다.
                            //
                            // 요약하면,
                            //
                            // OUT_OF_SIGHT -> ON_SIGHT/NEW_SIGHT : GCAddXXX
                            // IN_SIGHT/ON_SIGHT/NEW_SIGHT -> IN_SIGHT/ON_SIGHT/NEW_SIGHT : GCMove
                            //
                            //--------------------------------------------------------------------------------
                            VisionState prevVS = pCreature->getVisionState(x1, y1);
                            VisionState currVS = pCreature->getVisionState(x2, y2);

                            // 보이지 않는 영역에서, 경계 영역을 거치지 않고 바로
                            // 시야 내부 영역으로 들어온다는 것은 불가능하다.

                            // canSee 로 대체. by bezz 2003.05.29
                            if (canSee(pCreature, pPC)) {
                                if (prevVS == OUT_OF_SIGHT && currVS >= IN_SIGHT) {
                                    pCreature->getPlayer()->sendPacket(pGCAddXXX);
                                    pCreature->getPlayer()->sendStream(&outputStream);
                                } else if (prevVS >= IN_SIGHT && currVS >= IN_SIGHT) {
                                    pCreature->getPlayer()->sendStream(&outputStream);
                                } else if (prevVS >= IN_SIGHT && currVS == OUT_OF_SIGHT) {
                                    pCreature->getPlayer()->sendPacket(&gcDeleteObject);
                                }
                            }
                        } break;

                        case Creature::CREATURE_CLASS_VAMPIRE: {
                            //--------------------------------------------------------------------------------
                            // 이전 좌표에서는 보이지 않다가, 이번 좌표에서 새로 보이게 된 크리처만
                            // GCAddXXX 를 받아온다. 이전에도 NEW_SIGHT 이고, 지금도 NEW_SIGHT 이면,
                            // 새로 받아오지 않는다.
                            //--------------------------------------------------------------------------------
                            if (curVisionState >= IN_SIGHT && prevVisionState == OUT_OF_SIGHT) {
                                if (canSee(pPC, pCreature)) {
                                    if (pCreature->isFlag(Effect::EFFECT_CLASS_HIDE)) {
                                        {
                                            GCAddBurrowingCreature gcABC;
                                            gcABC.setObjectID(pCreature->getObjectID());
                                            gcABC.setName(pCreature->getName());
                                            gcABC.setX(ix);
                                            gcABC.setY(iy);
                                            pPlayer->sendPacket(&gcABC);
                                        }
                                    } else {
                                        //													if
                                        //(!pCreature->isFlag(Effect::EFFECT_CLASS_INVISIBILITY))
                                        //													{
                                        Vampire* pVampire = dynamic_cast<Vampire*>(pCreature);
                                        //															GCAddVampire
                                        GCAddVampire gcAddVampire;
                                        makeGCAddVampire(&gcAddVampire, pVampire);
                                        pPlayer->sendPacket(&gcAddVampire);
                                        // pCreature는 invisibility상태..
                                        // pPC->isFlag(Effect::EFFECT_CLASS_DETECT_INVISIBILITY)
                                    }
                                }
                            }

                            Assert(pCreature->getPlayer() != NULL);

                            //--------------------------------------------------------------------------------
                            // Q(x2,y2)가 이 크리처의 시야 사각형의 경계에 위치하면서, P(x1,y1)은 사각형의 외부,
                            // 즉 보이지 않는 경우에만 GCAddXXX 패킷을 전송한다. 이렇게 하지 않으면, PC
                            // 크리처가 pCreature의 시야 경계에서 계속 움직이게 되면 계속 서버는 GCAddXXX 패킷을
                            // 보내야만 한다.
                            //
                            // 요약하면,
                            //
                            // OUT_OF_SIGHT -> ON_SIGHT/NEW_SIGHT : GCAddXXX
                            // IN_SIGHT/ON_SIGHT/NEW_SIGHT -> IN_SIGHT/ON_SIGHT/NEW_SIGHT : GCMove
                            //
                            //--------------------------------------------------------------------------------
                            VisionState prevVS = pCreature->getVisionState(x1, y1);
                            VisionState currVS = pCreature->getVisionState(x2, y2);

                            // 상대는 뱀파이어이므로 나의 darkness상태는 관계없다.
                            // Hide도 관계없다.
                            // *NOTE
                            // 상대가 슬레이어라면 슬레이어가 스나이핑 상태인지를 체크 해야 한다.
                            if (canSee(pCreature, pPC)) {
                                if (prevVS == OUT_OF_SIGHT && currVS >= IN_SIGHT) {
                                    pCreature->getPlayer()->sendPacket(pGCAddXXX);
                                    pCreature->getPlayer()->sendStream(&outputStream);
                                } else if (prevVS >= IN_SIGHT && currVS >= IN_SIGHT) {
                                    pCreature->getPlayer()->sendStream(&outputStream);
                                } else if (prevVS >= IN_SIGHT && currVS == OUT_OF_SIGHT) {
                                    pCreature->getPlayer()->sendPacket(&gcDeleteObject);
                                }
                            }
                        } break;

                        case Creature::CREATURE_CLASS_OUSTERS: {
                            //--------------------------------------------------------------------------------
                            // 이전 좌표에서는 보이지 않다가, 이번 좌표에서 새로 보이게 된 크리처만
                            // GCAddXXX 를 받아온다. 이전에도 NEW_SIGHT 이고, 지금도 NEW_SIGHT 이면,
                            // 새로 받아오지 않는다.
                            //--------------------------------------------------------------------------------
                            if (curVisionState >= IN_SIGHT && prevVisionState == OUT_OF_SIGHT &&
                                canSee(pPC, pCreature)) {
                                Ousters* pOusters = dynamic_cast<Ousters*>(pCreature);
                                //												GCAddOusters
                                GCAddOusters gcAddOusters;
                                makeGCAddOusters(&gcAddOusters, pOusters);
                                pPlayer->sendPacket(&gcAddOusters);
                            }

                            Assert(pCreature->getPlayer() != NULL);

                            //--------------------------------------------------------------------------------
                            // Q(x2,y2)가 이 크리처의 시야 사각형의 경계에 위치하면서, P(x1,y1)은 사각형의 외부,
                            // 즉 보이지 않는 경우에만 GCAddXXX 패킷을 전송한다. 이렇게 하지 않으면, PC
                            // 크리처가 pCreature의 시야 경계에서 계속 움직이게 되면 계속 서버는 GCAddXXX 패킷을
                            // 보내야만 한다.
                            //
                            // 요약하면,
                            //
                            // OUT_OF_SIGHT -> ON_SIGHT/NEW_SIGHT : GCAddXXX
                            // IN_SIGHT/ON_SIGHT/NEW_SIGHT -> IN_SIGHT/ON_SIGHT/NEW_SIGHT : GCMove
                            //
                            //--------------------------------------------------------------------------------
                            VisionState prevVS = pCreature->getVisionState(x1, y1);
                            VisionState currVS = pCreature->getVisionState(x2, y2);

                            // 상대는 뱀파이어이므로 나의 darkness상태는 관계없다.
                            // Hide도 관계없다.
                            // *NOTE
                            // 상대가 슬레이어라면 슬레이어가 스나이핑 상태인지를 체크 해야 한다.
                            if (canSee(pCreature, pPC)) {
                                if (prevVS == OUT_OF_SIGHT && currVS >= IN_SIGHT) {
                                    pCreature->getPlayer()->sendPacket(pGCAddXXX);
                                    pCreature->getPlayer()->sendStream(&outputStream);
                                } else if (prevVS >= IN_SIGHT && currVS >= IN_SIGHT) {
                                    pCreature->getPlayer()->sendStream(&outputStream);
                                } else if (prevVS >= IN_SIGHT && currVS == OUT_OF_SIGHT) {
                                    pCreature->getPlayer()->sendPacket(&gcDeleteObject);
                                }
                            }
                        } break;

                        case Creature::CREATURE_CLASS_NPC: {
                            NPC* pNPC = dynamic_cast<NPC*>(pCreature);

                            //--------------------------------------------------------------------------------
                            //
                            // 이전 좌표에서는 이 몬스터를 볼 수 없었으나, 도착 좌표에서 이 몬스터를 보게 될
                            // 경우 GCAddMonster 패킷을 전송한다.
                            //
                            //--------------------------------------------------------------------------------
                            if (prevVisionState == OUT_OF_SIGHT && curVisionState >= IN_SIGHT) {
                                GCAddNPC gcAddNPC;
                                makeGCAddNPC(&gcAddNPC, pNPC);
                                pPlayer->sendPacket(&gcAddNPC);
                            }
                        } break;

                        default:
                            throw Error("invalid creature class");

                        } // switch (pCreature->getCreatureClass())

                    } // case Object::OBJECT_CLASS_CREATURE :

                    break;

                    //--------------------------------------------------------------------------------
                    // 타일 위에 아이템이 있을 경우
                    //--------------------------------------------------------------------------------
                    case Object::OBJECT_CLASS_ITEM: {
                        if (curVisionState >= IN_SIGHT && prevVisionState == OUT_OF_SIGHT) {
                            Item* pItem = dynamic_cast<Item*>(*itr);

                            if (pItem->getItemClass() == Item::ITEM_CLASS_CORPSE) {
                                switch (pItem->getItemType()) {
                                case SLAYER_CORPSE: {
                                    SlayerCorpse* pSlayerCorpse = dynamic_cast<SlayerCorpse*>(pItem);
                                    GCAddSlayerCorpse gcAddSlayerCorpse;
                                    makeGCAddSlayerCorpse(&gcAddSlayerCorpse, pSlayerCorpse);
                                    pPlayer->sendPacket(&gcAddSlayerCorpse);
                                } break;
                                case VAMPIRE_CORPSE: {
                                    VampireCorpse* pVampireCorpse = dynamic_cast<VampireCorpse*>(pItem);
                                    GCAddVampireCorpse gcAddVampireCorpse;
                                    makeGCAddVampireCorpse(&gcAddVampireCorpse, pVampireCorpse);
                                    pPlayer->sendPacket(&gcAddVampireCorpse);
                                } break;
                                case OUSTERS_CORPSE: {
                                    OustersCorpse* pOustersCorpse = dynamic_cast<OustersCorpse*>(pItem);
                                    GCAddOustersCorpse gcAddOustersCorpse;
                                    makeGCAddOustersCorpse(&gcAddOustersCorpse, pOustersCorpse);
                                    pPlayer->sendPacket(&gcAddOustersCorpse);
                                } break;
                                case NPC_CORPSE: {
                                    throw UnsupportedError();
                                } break;
                                case MONSTER_CORPSE: {
                                    MonsterCorpse* pMonsterCorpse = dynamic_cast<MonsterCorpse*>(pItem);
                                    GCAddMonsterCorpse gcAddMonsterCorpse;
                                    makeGCAddMonsterCorpse(&gcAddMonsterCorpse, pMonsterCorpse, ix, iy);
                                    pPlayer->sendPacket(&gcAddMonsterCorpse);

                                    sendRelicEffect(pMonsterCorpse, pPlayer);
                                } break;
                                } // switch
                            } else if (pItem->getItemClass() == Item::ITEM_CLASS_MINE &&
                                       pItem->isFlag(Effect::EFFECT_CLASS_INSTALL)) {
                                if (pPC->isFlag(Effect::EFFECT_CLASS_REVEALER)) {
                                    GCAddInstalledMineToZone gcAddMine;
                                    gcAddMine.setObjectID(pItem->getObjectID());
                                    gcAddMine.setX(ix);
                                    gcAddMine.setY(iy);
                                    gcAddMine.setItemClass(pItem->getItemClass());
                                    gcAddMine.setItemType(pItem->getItemType());
                                    gcAddMine.setOptionType(pItem->getOptionTypeList());
                                    gcAddMine.setDurability(pItem->getDurability());
                                    pPlayer->sendPacket(&gcAddMine);
                                }
                            } else {
                                GCAddNewItemToZone gcAddNewItemToZone;
                                makeGCAddNewItemToZone(&gcAddNewItemToZone, pItem, ix, iy);
                                pPlayer->sendPacket(&gcAddNewItemToZone);
                            }
                        }
                    } break;

                    //--------------------------------------------------------------------------------
                    // 타일 위에 이펙트가 있을 경우
                    //--------------------------------------------------------------------------------
                    case Object::OBJECT_CLASS_EFFECT: {
                        Effect* pEffect = dynamic_cast<Effect*>(*itr);

                        if (curVisionState >= IN_SIGHT && prevVisionState == OUT_OF_SIGHT) {
                            if (pEffect->getEffectClass() == Effect::EFFECT_CLASS_VAMPIRE_PORTAL) {
                                EffectVampirePortal* pEffectVampirePortal = dynamic_cast<EffectVampirePortal*>(pEffect);
                                ZONE_COORD zonecoord = pEffectVampirePortal->getZoneCoord();

                                GCAddVampirePortal gcAddVampirePortal;
                                gcAddVampirePortal.setObjectID(pEffect->getObjectID());
                                gcAddVampirePortal.setOwnerID(pEffectVampirePortal->getOwnerID());
                                gcAddVampirePortal.setX(ix);
                                gcAddVampirePortal.setY(iy);
                                gcAddVampirePortal.setTargetZoneID(zonecoord.id);
                                gcAddVampirePortal.setTargetX(zonecoord.x);
                                gcAddVampirePortal.setTargetY(zonecoord.y);
                                gcAddVampirePortal.setDuration(pEffectVampirePortal->getRemainDuration());
                                gcAddVampirePortal.setCreateFlag(0);

                                pPlayer->sendPacket(&gcAddVampirePortal);
                            }
                            // by sigi. 2002.6.10
                            else if (pEffect->getEffectClass() == Effect::EFFECT_CLASS_SANCTUARY) {
                                EffectSanctuary* pEffectSanctuary = dynamic_cast<EffectSanctuary*>(pEffect);

                                ZoneCoord_t centerX = pEffectSanctuary->getCenterX();
                                ZoneCoord_t centerY = pEffectSanctuary->getCenterY();

                                // sanctuary는 중심좌표인 경우만 packet을 보낸다.
                                if (centerX == ix && centerY == iy) {
                                    GCAddEffectToTile gcAddEffectToTile;

                                    gcAddEffectToTile.setObjectID(pEffect->getObjectID());
                                    gcAddEffectToTile.setXY(ix, iy);
                                    gcAddEffectToTile.setEffectID(pEffect->getSendEffectClass());
                                    gcAddEffectToTile.setDuration(pEffect->getRemainDuration());

                                    pPlayer->sendPacket(&gcAddEffectToTile);
                                }
                            }
                            // Broadcasting Effect 체크 추가 by Sequoia 2003.3.31
                            else if (pEffect->isBroadcastingEffect()) {
                                GCAddEffectToTile gcAddEffectToTile;

                                gcAddEffectToTile.setObjectID(pEffect->getObjectID());
                                gcAddEffectToTile.setXY(ix, iy);
                                gcAddEffectToTile.setEffectID(pEffect->getSendEffectClass());
                                gcAddEffectToTile.setDuration(pEffect->getRemainDuration());

                                pPlayer->sendPacket(&gcAddEffectToTile);
                            }
                        }
                    } break;

                    //--------------------------------------------------------------------------------
                    // 타일 위에 장애물이 있을 경우
                    //--------------------------------------------------------------------------------
                    case Object::OBJECT_CLASS_OBSTACLE: {
                        // darkness
                    } break;

                    //--------------------------------------------------------------------------------
                    // 타일 위에 포탈이 있을 경우
                    //--------------------------------------------------------------------------------
                    case Object::OBJECT_CLASS_PORTAL: {
                        // darkness
                    } break;

                    default:
                        throw Error("invalid object class");

                    } // switch ((*itr)->getObjectClass())
                } while (++itr != objectList.end()); // do ~ while
            } // if
        } // for
    } // for

    SAFE_DELETE(pGCAddXXX);

    return true;

    __END_DEBUG
    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// P(x1,y1)에서 Q(x2,y2)로 빠른 이동한 크리처가 주변 영역에 존재하는 PC들에게
// 브로드캐스트하는 메쏘드이다.
// for Skill FlashSliding, ShadowWalk
//////////////////////////////////////////////////////////////////////////////
bool Zone::moveFastMonster(Monster* pMonster, ZoneCoord_t x1, ZoneCoord_t y1, ZoneCoord_t x2, ZoneCoord_t y2,
                           SkillType_t skillType) {
    __BEGIN_TRY
    __BEGIN_DEBUG

    if (
        /*		pMonster->isFlag(Effect::EFFECT_CLASS_PARALYZE)
                || pMonster->isFlag(Effect::EFFECT_CLASS_HIDE)
                || pMonster->isFlag(Effect::EFFECT_CLASS_CAUSE_CRITICAL_WOUNDS)
                || pMonster->isFlag(Effect::EFFECT_CLASS_SLEEP)
                || pMonster->isFlag(Effect::EFFECT_CLASS_ARMAGEDDON) */
        !isAbleToMove(pMonster)) {
        // do nothing
        return false;
    }

    ZoneLevel_t ZoneLevel = getZoneLevel(x2, y2);

    // 안전 지대에 못 들어간다.
    if ((ZoneLevel & SLAYER_SAFE_ZONE) || (ZoneLevel & VAMPIRE_SAFE_ZONE) || (ZoneLevel & COMPLETE_SAFE_ZONE)) {
        return false;
    }

    // 적당한 종착지점을 찾는다.
    // 전면부에서 적당한 지점..4개 검색
    Dir_t dir = calcDirection(x1, y1, x2, y2);

    // g_FastMoveSearchX, Y로 찾으면 된다. by sigi. 2002.5.8
    int* searchX = g_FastMoveSearchX[dir];
    int* searchY = g_FastMoveSearchY[dir];

    // 빈 타일인지 확인.
    int i = 0;
    for (i = 0; i < 4; i++) {
        int targetX = x2 + searchX[i], targetY = y2 + searchY[i];
        if (targetX >= 0 && targetX < m_Width && targetY >= 0 && targetY < m_Height &&
            !m_pTiles[targetX][targetY].isBlocked(pMonster->getMoveMode()) && !m_pTiles[targetX][targetY].hasPortal()) {
            x2 = targetX;
            y2 = targetY;
            break;
        }
    }
    if (i == 4) {
        return false; // 빈타일을 못찾았다!
    }

    // 일단 패킷을 만들어두고 밑에서 보낸다.
    GCFastMove gcFastMove;
    gcFastMove.setObjectID(pMonster->getObjectID());
    gcFastMove.setXY(x1, y1, x2, y2);
    gcFastMove.setSkillType(skillType);

#ifdef __USE_ENCRYPTER__
    SocketEncryptOutputStream outputStream(NULL, szPacketHeader + gcFastMove.getPacketSize() + 2);
    outputStream.setEncryptCode(m_EncryptCode);
#else
    SocketOutputStream outputStream(NULL, szPacketHeader + gcFastMove.getPacketSize() + 2);
#endif
    gcFastMove.writeHeaderNBody(outputStream);

    // 몬스터한테는 보낼 필요가 없다.

    //////////////////////////////////////////////////////////////
    // move의 종류....
    // 이에따라 GCDelete나 Add등을 보내줘야 할 수 도 있다.

    // Monster 좌표 변경.
    pMonster->setXYDir(x2, y2, dir);
    // 이전 타일에서 크리처를 삭제한다.

    try {
        m_pTiles[x1][y1].deleteCreature(pMonster->getObjectID());
    } catch (Error& e) {
        filelog("assertTile.txt", "moveFastMonster : %s", e.toString().c_str());
        throw;
    }

    // 새 타일에 크리처를 추가한다.
    m_pTiles[x2][y2].addCreature(pMonster);

    try {
        checkMine(this, pMonster, x2, y2);
        checkTrap(this, pMonster);
    } catch (Throwable& t) {
        filelog("CheckMineBug.txt", "%s : %s", "moveFastMonster", t.toString().c_str());
    }


    //--------------------------------------------------------------------------------
    // GCAddSlayer/GCAddVampire 패킷을 만들어둔다.
    // 현재의 정책에 의하면, GCAdd 패킷은 현재의 좌표를 바탕으로 한다.
    //--------------------------------------------------------------------------------
    Packet* pAddMonsterPacket = createMonsterAddPacket(pMonster, NULL);

    if (pAddMonsterPacket != NULL) {
        //--------------------------------------------------------------------------------
        // GCDeleteObject 패킷을 만들어둔다.
        //--------------------------------------------------------------------------------
        GCDeleteObject gcDeleteObject;
        gcDeleteObject.setObjectID(pMonster->getObjectID());


        // 총 시야의 범위를 구한다.
        ZoneCoord_t minX, maxX, minY, maxY;
        if (x1 < x2) {
            minX = max(0, x1 - maxViewportWidth);
            maxX = min(m_Width - 1, x2 + maxViewportWidth);
        } else {
            minX = max(0, x2 - maxViewportWidth);
            maxX = min(m_Width - 1, x1 + maxViewportWidth);
        }
        if (y1 < y2) {
            minY = max(0, y1 - maxViewportUpperHeight);
            maxY = min(m_Height - 1, y2 + maxViewportLowerHeight);
        } else {
            minY = max(0, y2 - maxViewportUpperHeight);
            maxY = min(m_Height - 1, y1 + maxViewportLowerHeight);
        }


        for (ZoneCoord_t ix = minX; ix <= maxX; ix++) {
            for (ZoneCoord_t iy = minY; iy <= maxY; iy++) {
                const forward_list<Object*>& objectList = m_pTiles[ix][iy].getObjectList();

                forward_list<Object*>::const_iterator itr = objectList.begin();

                // visionInfo 때문에..
                // if - do~while()로 구조 변경 by sigi. 2002.5.8
                if (itr != objectList.end()) {
                    // 이전 좌표 P(x1,y1)에서 I(ix,iy)가 어떻게 보이는가?
                    // 현재 좌표 Q(x2,y2)에서 I(ix,iy)가 어떻게 보이는가?

                    do {
                        Assert(*itr != NULL);

                        //--------------------------------------------------------------------------------
                        //
                        // 각 객체의 OBJECT CLASS에 따라서 적합한 GCAddXXX 패킷을 만들어서
                        // owner 에게 전송한다.
                        //
                        // *NOTES*
                        //
                        // 가장 출현 확률이 높은 객체 CLASS 가 case 앞부분에 나와야 한다.
                        //
                        //--------------------------------------------------------------------------------
                        switch ((*itr)->getObjectClass()) {
                        //--------------------------------------------------------------------------------
                        // 타일 위에 크리처가 있을 경우
                        //--------------------------------------------------------------------------------
                        case Object::OBJECT_CLASS_CREATURE: {
                            Creature* pCreature = dynamic_cast<Creature*>(*itr);
                            Assert(pCreature != NULL);

                            // 자기 자신의 정보는 받을 필요가 없다.
                            if (pCreature == pMonster)
                                continue;

                            switch (pCreature->getCreatureClass()) {
                            case Creature::CREATURE_CLASS_MONSTER: {
                                Monster* pOtherMonster = dynamic_cast<Monster*>(pCreature);

                                //--------------------------------------------------------------------------------
                                // PC를 몬스터의 잠재적인 적으로 지정해준다.
                                //--------------------------------------------------------------------------------

                                // Aggressive 몬스터에게만 적으로 등록시켜준다.
                                {
                                    if (isPotentialEnemy(pOtherMonster, pMonster)) {
                                        pMonster->addPotentialEnemy(pOtherMonster);
                                        pOtherMonster->addPotentialEnemy(pMonster);
                                    }
                                }
                            } break;

                            //--------------------------------------------------------------------------------
                            //
                            //--------------------------------------------------------------------------------
                            case Creature::CREATURE_CLASS_SLAYER: {
                                Assert(pCreature->getPlayer() != NULL);

                                VisionState prevVS = pCreature->getVisionState(x1, y1);
                                VisionState currVS = pCreature->getVisionState(x2, y2);

                                // Creature 가 ObservingEye 이펙트를 가지고 있다면 가져온다.
                                //												EffectObservingEye* pEffectObservingEye
                                //													//Assert( pEffectObservingEye !=

                                // 상대에게 PC의 등장을 알리는 패킷.
                                //												if
                                //((!pMonster->isFlag(Effect::EFFECT_CLASS_HIDE) ||
                                // pCreature->isFlag(Effect::EFFECT_CLASS_DETECT_HIDDEN) ) //|| (
                                // pEffectRevealerCreature
                                //!= NULL && pEffectRevealerCreature->canSeeHide( pPC ) ) )
                                //													&&
                                //(!pMonster->isFlag(Effect::EFFECT_CLASS_INVISIBILITY) ||
                                // pCreature->isFlag(Effect::EFFECT_CLASS_DETECT_INVISIBILITY) || ( pEffectObservingEye
                                //!= NULL && pEffectObservingEye->canSeeInvisibility( pMonster ) ) )
                                //													&&
                                //(!pMonster->isFlag(Effect::EFFECT_CLASS_SNIPING_MODE) ||
                                // pCreature->isFlag(Effect::EFFECT_CLASS_DETECT_INVISIBILITY) )) //|| (
                                // pEffectRevealerCreature != NULL && pEffectRevealerCreature->canSeeSniping( pPC) ) ) )
                                if (canSee(pCreature, pMonster)) {
                                    if (prevVS == OUT_OF_SIGHT && currVS >= IN_SIGHT) {
                                        pCreature->getPlayer()->sendPacket(pAddMonsterPacket);
                                        pCreature->getPlayer()->sendStream(&outputStream);
                                    } else if (prevVS >= IN_SIGHT && currVS >= IN_SIGHT) {
                                        pCreature->getPlayer()->sendStream(&outputStream);
                                    } else if (prevVS >= IN_SIGHT && currVS == OUT_OF_SIGHT) {
                                        pCreature->getPlayer()->sendPacket(&gcDeleteObject);
                                    }
                                }
                            } break;

                            case Creature::CREATURE_CLASS_VAMPIRE: {
                                Assert(pCreature->getPlayer() != NULL);

                                //--------------------------------------------------------------------------------
                                // Q(x2,y2)가 이 크리처의 시야 사각형의 경계에 위치하면서, P(x1,y1)은 사각형의 외부,
                                // 즉 보이지 않는 경우에만 GCAddXXX 패킷을 전송한다. 이렇게 하지 않으면, PC
                                // 크리처가 pCreature의 시야 경계에서 계속 움직이게 되면 계속 서버는 GCAddXXX 패킷을
                                // 보내야만 한다.
                                //
                                // 요약하면,
                                //
                                // OUT_OF_SIGHT -> ON_SIGHT/NEW_SIGHT : GCAddXXX
                                // IN_SIGHT/ON_SIGHT/NEW_SIGHT -> IN_SIGHT/ON_SIGHT/NEW_SIGHT : GCMove
                                //
                                //--------------------------------------------------------------------------------
                                VisionState prevVS = pCreature->getVisionState(x1, y1);
                                VisionState currVS = pCreature->getVisionState(x2, y2);

                                // 상대는 뱀파이어이므로 나의 darkness상태는 관계없다.
                                // Hide도 관계없다.
                                // *NOTE
                                // 상대가 슬레이어라면 슬레이어가 스나이핑 상태인지를 체크 해야 한다.
                                if (prevVS == OUT_OF_SIGHT && currVS >= IN_SIGHT) {
                                    pCreature->getPlayer()->sendPacket(pAddMonsterPacket);
                                    pCreature->getPlayer()->sendStream(&outputStream);
                                } else if (prevVS >= IN_SIGHT && currVS >= IN_SIGHT) {
                                    pCreature->getPlayer()->sendStream(&outputStream);
                                } else if (prevVS >= IN_SIGHT && currVS == OUT_OF_SIGHT) {
                                    pCreature->getPlayer()->sendPacket(&gcDeleteObject);
                                }
                            } break;

                            case Creature::CREATURE_CLASS_OUSTERS: {
                                Assert(pCreature->getPlayer() != NULL);

                                VisionState prevVS = pCreature->getVisionState(x1, y1);
                                VisionState currVS = pCreature->getVisionState(x2, y2);

                                // 상대에게 PC의 등장을 알리는 패킷.
                                if (canSee(pCreature, pMonster)) {
                                    if (prevVS == OUT_OF_SIGHT && currVS >= IN_SIGHT) {
                                        pCreature->getPlayer()->sendPacket(pAddMonsterPacket);
                                        pCreature->getPlayer()->sendStream(&outputStream);
                                    } else if (prevVS >= IN_SIGHT && currVS >= IN_SIGHT) {
                                        pCreature->getPlayer()->sendStream(&outputStream);
                                    } else if (prevVS >= IN_SIGHT && currVS == OUT_OF_SIGHT) {
                                        pCreature->getPlayer()->sendPacket(&gcDeleteObject);
                                    }
                                }
                            } break;
                            case Creature::CREATURE_CLASS_NPC: {
                            } break;

                            default:
                                throw Error("invalid creature class");

                            } // switch (pCreature->getCreatureClass())

                        } // case Object::OBJECT_CLASS_CREATURE :

                        break;

                        //--------------------------------------------------------------------------------
                        // 타일 위에 아이템이 있을 경우
                        //--------------------------------------------------------------------------------
                        case Object::OBJECT_CLASS_ITEM: {
                        } break;

                        //--------------------------------------------------------------------------------
                        // 타일 위에 이펙트가 있을 경우
                        //--------------------------------------------------------------------------------
                        case Object::OBJECT_CLASS_EFFECT: {
                        } break;

                        //--------------------------------------------------------------------------------
                        // 타일 위에 장애물이 있을 경우
                        //--------------------------------------------------------------------------------
                        case Object::OBJECT_CLASS_OBSTACLE: {
                            // darkness
                        } break;

                        //--------------------------------------------------------------------------------
                        // 타일 위에 포탈이 있을 경우
                        //--------------------------------------------------------------------------------
                        case Object::OBJECT_CLASS_PORTAL: {
                            // darkness
                        } break;

                        default:
                            throw Error("invalid object class");

                        } // switch ((*itr)->getObjectClass())
                    } while (++itr != objectList.end()); // do ~ while
                } // if
            } // for
        } // for


        delete pAddMonsterPacket;
    }

    return true;

    __END_DEBUG
    __END_CATCH
}
