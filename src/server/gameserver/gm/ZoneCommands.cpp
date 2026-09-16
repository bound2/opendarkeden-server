//////////////////////////////////////////////////////////////////////////////
// Filename    : ZoneCommands.cpp
// Description : GM commands that act on a zone or on what stands in it: warps, recalls, summons
//               and the zone flags.
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include <list>

#include "CreatureUtil.h"
#include "DatabaseError.h"
#include "GCSystemMessage.h"
#include "GamePlayer.h"
#include "Guild.h"
#include "GuildManager.h"
#include "ItemUtil.h"
#include "Monster.h"
#include "OptionInfo.h"
#include "PCFinder.h"
#include "PacketUtil.h"
#include "Properties.h"
#include "RelicUtil.h"
#include "SiegeManager.h"
#include "VSDateTime.h"
#include "Zone.h"
#include "ZoneGroupManager.h"
#include "ZoneInfoManager.h"
#include "ZoneUtil.h"
#include "gm/GMCommands.h"

namespace de::gm {

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void opzone(string msg, int i) {
    __BEGIN_TRY

    //////////////
    // Zone Info
    //////////////
    /*	ZoneInfo* pZoneInfo = new ZoneInfo();
        pZoneInfo->setZoneID( 10001 );
        pZoneInfo->setZoneGroupID( 6 );
        pZoneInfo->setZoneType( "NORMAL_FIELD" );
        pZoneInfo->setZoneLevel( 0 );
        pZoneInfo->setZoneAccessMode( "PUBLIC" );
        pZoneInfo->setZoneOwnerID( "" );
        pZoneInfo->setPayPlay( "" );

        pZoneInfo->setSMPFilename( "team_hdqrs.smp" );
        pZoneInfo->setSSIFilename( "team_hdqrs.ssi" );
        pZoneInfo->setFullName( "team" );
        pZoneInfo->setShortName( "team" );

        g_pZoneInfoManager->addZoneInfo( pZoneInfo );

        /////////
        // Zone
        /////////
        Zone* pZone = new Zone( 10001 );
        Assert( pZone != NULL );

        ZoneGroup* pZoneGroup = g_pZoneGroupManager->getZoneGroup(6);
        Assert( pZoneGroup != NULL );

        pZone->setZoneGroup( pZoneGroup );
        pZoneGroup->addZone( pZone );
        pZone->init();
    */
    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void opwarp(GamePlayer* pGamePlayer, string msg, int i) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

#ifdef __GAME_SERVER__

        if (pGamePlayer == NULL) return;

    Creature* pCreature = pGamePlayer->getCreature();

    size_t j = msg.find_first_of(' ', i + 1);
    size_t k = msg.find_first_of(' ', j + 1);
    size_t l = msg.find_first_of(' ', k + 1);

    string ZoneName = trim(msg.substr(j + 1, k - j - 1));
    ZoneCoord_t ZoneX = atoi(msg.substr(k + 1, l - k - 1).c_str());
    ZoneCoord_t ZoneY = atoi(msg.substr(l + 1, msg.size() - l - 1).c_str());

    int ZoneID = 0;

    // ZoneName�� ���� �̸��� ���� �ְ�, ���� ID�� ���� �ִ�.
    // ZoneInfo�� NULL�� �ƴ϶�� ���� �� ���ڿ��� ���� �̸��̶�� ���̰�,
    // Zoneinfo�� NULL�̶�� ���� �� ���ڿ��� ���� ID�̶�� ���̴�.
    // (���� ������� �Է� �Ǽ��� �ְ�����, �̴� �����Ѵ�.)
    ZoneInfo* pZoneInfo = g_pZoneInfoManager->getZoneInfoByName(ZoneName);
    if (pZoneInfo != NULL) {
        ZoneID = pZoneInfo->getZoneID();
    } else {
        ZoneID = atoi(ZoneName.c_str());
    }

    if (pCreature->isPC() && ZoneX < 256 && ZoneY < 256) {
        // �����Ƽ� ������ �׷� ���� �ִ����� üũ�Ѵ�.
        // ���ٸ� �����Ѵ�.
        try {
            Zone* pZone = getZoneByZoneID(ZoneID);
            // evade warning
            pZone = NULL;
        } catch (Error) {
            return;
        }

        // �׷� ���� �ִٴ� ���� Ȯ�εǾ��ٸ� �̵���Ų��.
        try {
            transportCreature(pCreature, ZoneID, ZoneX, ZoneY, false);
        } catch (Throwable& t) {
            cerr << t.toString() << endl;
        }
    }

#endif

    __END_DEBUG_EX __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void oprecall(GamePlayer* pGamePlayer, string msg, int i) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

        if (pGamePlayer == NULL) return;

    Creature* pCreature = pGamePlayer->getCreature();

    PlayerCreature* pPC = dynamic_cast<PlayerCreature*>(pCreature);
    if (pPC->isPLAYER()) {
        if (g_pGuildManager->isGuildMaster(pPC->getGuildID(), pPC)) {
            if (!SiegeManager::Instance().isSiegeZone(pPC->getZoneID())) {
                return;
            }
        } else {
            return;
        }
    }


    // Zone* pCallZone = pCreature->getZone();

    /*
    uint j = msg.find_first_of(' ' , i+1);

    string Name = msg.substr(j+1, msg.size()-j-1).c_str();
    */

    Creature* pTCreature = NULL;

    size_t j = msg.find_first_of(' ', i + 1);

    string Name;

    while (j < msg.size()) {
        i = msg.find_first_of(' ', j + 1);

        Name = msg.substr(j + 1, i - j - 1).c_str();

        // cout << "Name : (" <<  Name << ")" << endl;

        // NoSuch����. by sigi. 2002.5.2
        __ENTER_CRITICAL_SECTION((*g_pPCFinder))

        pTCreature = g_pPCFinder->getCreature_LOCKED(Name);

        if (pTCreature == NULL) {
            return;
        }

        // if (pTCreature != NULL)
        {
            // �������� PCFinder���� ã�ƿ� Creature�� const�̴�.
            Zone* pTargetZone = pTCreature->getZone();

            Assert(pTargetZone != NULL);

            Creature* pTargetCreature = NULL;
            // try
            //{
            //  NoSuch����. by sigi. 2002.5.2
            pTargetCreature = pTargetZone->getCreature(pTCreature->getObjectID());
            //}
            // catch (NoSuchElementException)
            //{
            //}

            if (pTargetCreature != NULL) {
                // ���� �׾��ִ� ���� ������ �� �� ����.
                if (pTargetCreature->isEffect(Effect::EFFECT_CLASS_COMA)) {
                    return;
                }

                // ��ȯ���� ���� ��ǥ.
                ZoneID_t ZoneNum = pCreature->getZoneID();
                Coord_t ZoneX = pCreature->getX();
                Coord_t ZoneY = pCreature->getY();

                if (SiegeManager::Instance().isSiegeZone(pPC->getZoneID())) {
                    Effect::EffectClass eClass = Effect::EFFECT_CLASS_MAX;
                    if (pCreature->isFlag(Effect::EFFECT_CLASS_SIEGE_DEFENDER)) {
                        ZoneX = 172;
                        ZoneY = 38;
                        eClass = Effect::EFFECT_CLASS_SIEGE_DEFENDER;
                    } else if (pCreature->isFlag(Effect::EFFECT_CLASS_SIEGE_REINFORCE)) {
                        ZoneX = 172;
                        ZoneY = 38;
                        eClass = Effect::EFFECT_CLASS_SIEGE_REINFORCE;
                    } else if (pCreature->isFlag(Effect::EFFECT_CLASS_SIEGE_ATTACKER_1)) {
                        ZoneX = 20;
                        ZoneY = 232;
                        eClass = Effect::EFFECT_CLASS_SIEGE_ATTACKER_1;
                    } else if (pCreature->isFlag(Effect::EFFECT_CLASS_SIEGE_ATTACKER_2)) {
                        ZoneX = 20;
                        ZoneY = 232;
                        eClass = Effect::EFFECT_CLASS_SIEGE_ATTACKER_2;
                    } else if (pCreature->isFlag(Effect::EFFECT_CLASS_SIEGE_ATTACKER_3)) {
                        ZoneX = 20;
                        ZoneY = 232;
                        eClass = Effect::EFFECT_CLASS_SIEGE_ATTACKER_3;
                    } else if (pCreature->isFlag(Effect::EFFECT_CLASS_SIEGE_ATTACKER_4)) {
                        ZoneX = 20;
                        ZoneY = 232;
                        eClass = Effect::EFFECT_CLASS_SIEGE_ATTACKER_4;
                    } else if (pCreature->isFlag(Effect::EFFECT_CLASS_SIEGE_ATTACKER_5)) {
                        ZoneX = 20;
                        ZoneY = 232;
                        eClass = Effect::EFFECT_CLASS_SIEGE_ATTACKER_5;
                    }

                    if (eClass != Effect::EFFECT_CLASS_MAX && !pTargetCreature->isFlag(eClass)) {
                        if (!pTargetCreature->isPC() ||
                            dynamic_cast<PlayerCreature*>(pTargetCreature)->getGuildID() != pPC->getGuildID()) {
                            return;
                        }

                        addSimpleCreatureEffect(pTargetCreature, eClass, 72000);
                    }
                }

                Assert((int)ZoneX < 256);
                Assert((int)ZoneY < 256);

                Assert(pTargetCreature->isPC());

                transportCreature(pTargetCreature, ZoneNum, ZoneX, ZoneY, false);
            }
        }

        __LEAVE_CRITICAL_SECTION((*g_pPCFinder))

        j = i;
    }

    __END_DEBUG_EX __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void opsummon(GamePlayer* pGamePlayer, string msg, int i) {
    __BEGIN_TRY __BEGIN_DEBUG_EX

        if (pGamePlayer == NULL) return;

    Creature* pCreature = pGamePlayer->getCreature();

    size_t j = msg.find_first_of(' ', i + 1);
    size_t k = msg.find_first_of(' ', j + 1); // j~k : name
    size_t l = msg.find_first_of(' ', k + 1); // k~l : num
    size_t m = msg.find_first_of(' ', l + 1); // l~m : mad

    size_t o = msg.find_first_of('{', j);
    size_t p = msg.find_first_of('}', j + 1);

#ifdef __UNDERWORLD__

    size_t e = msg.find_first_of('[', i + 1);
    size_t v = msg.find_first_of(']', i + 1); // e~v : event ����

    string EventFlag = msg.substr(e + 1, v - e - 1);

    if (strstr(EventFlag.c_str(), "EventAll") != NULL) {
        //	�̺�Ʈ�� �ڵ� (����Ÿ�Ե� �׳� �ϵ��ڵ� -_-;)
        Zone* pZone = pCreature->getZone();
        Coord_t ZoneX = pCreature->getX();
        Coord_t ZoneY = pCreature->getY();

        TPOINT pt;

        pt = findSuitablePosition(pZone, ZoneX, ZoneY, Creature::MOVE_MODE_WALKING);

        if (pt.x == -1 || (pZone->getZoneLevel(pt.x, pt.y) & SAFE_ZONE))
            return;

        Monster* pMonster = NULL;
        Monster* pMonster1 = NULL;
        Monster* pMonster2 = NULL;

        try {
            pMonster = new Monster(599);
            pMonster1 = new Monster(599);
            pMonster2 = new Monster(599);

            if (pMonster == NULL)
                return;

            pMonster->setName("����");

            pMonster->setTreasure(true);
            pMonster->setUnderworld(true);

            pMonster1->setTreasure(true);
            pMonster2->setTreasure(true);

            pMonster->setClanType(CLAN_VAMPIRE_MONSTER);
            pMonster1->setClanType(CLAN_VAMPIRE_MONSTER);
            pMonster2->setClanType(CLAN_VAMPIRE_MONSTER);

            pZone->addCreature(pMonster, pt.x, pt.y, Directions(rand() % 8));
            pZone->addCreature(pMonster1, pt.x, pt.y, Directions(rand() % 8));
            pZone->addCreature(pMonster2, pt.x, pt.y, Directions(rand() % 8));


        } catch (...) {
            SAFE_DELETE(pMonster);
            return;
        }

        return;
    } else if (strstr(EventFlag.c_str(), "Event") != NULL) {
        //	�̺�Ʈ�� �ڵ� (����Ÿ�Ե� �׳� �ϵ��ڵ� -_-;)
        Zone* pZone = pCreature->getZone();
        Coord_t ZoneX = pCreature->getX();
        Coord_t ZoneY = pCreature->getY();

        TPOINT pt;

        pt = findSuitablePosition(pZone, ZoneX, ZoneY, Creature::MOVE_MODE_WALKING);

        if (pt.x == -1 || (pZone->getZoneLevel(pt.x, pt.y) & SAFE_ZONE))
            return;

        Monster* pMonster = NULL;

        try {
            pMonster = new Monster(599);

            if (pMonster == NULL)
                return;

            pMonster->setName("BOSS����");

            pMonster->setTreasure(true);
            pMonster->setUnderworld(true);

            pMonster->setClanType(CLAN_VAMPIRE_MONSTER);

            pZone->addCreature(pMonster, pt.x, pt.y, Directions(rand() % 8));

        } catch (...) {
            SAFE_DELETE(pMonster);
            return;
        }

        return;
    }

#endif


    SpriteType_t SpriteType = 0;
    MonsterType_t MonsterType = 0;
    int MonsterNum = 0;

    string MonsterName = msg.substr(j + 1, k - j - 1);

    //	cout << MonsterName << endl;

    // SpriteType�� �ƴϰ� MonsterType�� ������ ���
    if (o != string::npos && p != string::npos) {
        MonsterType = atoi(msg.substr(o + 1, p - o - 1).c_str());
    } else if (strstr(MonsterName.c_str(), "ġ��") != NULL) {
        // ġ������ ��ȯ�ϱ� ��.��; by DEW
        MonsterType = g_pMonsterInfoManager->getChiefMonsterTypeByName(MonsterName);
    } else {
        SpriteType = g_pMonsterInfoManager->getSpriteTypeByName(MonsterName);

        if (SpriteType == 0) {
            SpriteType = atoi(msg.substr(j + 1, k - j - 1).c_str());
        }
    }

    MonsterNum = atoi(msg.substr(k + 1, msg.size() - k - 1).c_str());

    // 1~30 ����
    MonsterNum = max(1, MonsterNum);
    MonsterNum = min(30, MonsterNum);

    // ClanType
    // "mad" monsters -_-;
    string mad = trim(msg.substr(l + 1, m - l - 1));

    SUMMON_INFO summonInfo;
    if (mad == "mad") {
        summonInfo.canScanEnemy = true;

        string group = trim(msg.substr(m + 1, msg.size() - m - 1));

        if (group == "group") {
            summonInfo.clanType = SUMMON_INFO::CLAN_TYPE_RANDOM_GROUP;
            summonInfo.clanID = rand() % 90 + 2;
        } else {
            summonInfo.clanType = SUMMON_INFO::CLAN_TYPE_RANDOM_EACH;
        }
    } else {
        summonInfo.canScanEnemy = false;
        summonInfo.clanType = SUMMON_INFO::CLAN_TYPE_DEFAULT;
    }

    Zone* pZone = pCreature->getZone();
    Coord_t ZoneX = pCreature->getX();
    Coord_t ZoneY = pCreature->getY();

    try {
        // �Ϲ� ���̶�� ���͸� �����Ѵ�.
        if (!(pZone->getZoneLevel() & SAFE_ZONE)) {
            filelog("summon.txt", "[%s] ZoneID=%d, %s", pCreature->getName().c_str(), pCreature->getZone()->getZoneID(),
                    msg.c_str());

            // monsterInfo�� �ִ°��� ���°��� üũ�ϱ� ���ؼ�..
            // ������ NoSuchElementException�� ���.
            if (SpriteType != 0) {
                g_pMonsterInfoManager->getMonsterTypeBySprite(SpriteType);
                addMonstersToZone(pZone, ZoneX, ZoneY, SpriteType, MonsterType, MonsterNum, summonInfo);
            } else if (MonsterType != 0) {
                g_pMonsterInfoManager->getMonsterInfo(MonsterType);
                addMonstersToZone(pZone, ZoneX, ZoneY, SpriteType, MonsterType, MonsterNum, summonInfo);
            }
        }
    } catch (Throwable& t) {
        cout << t.toString() << endl;
        filelog("summon.txt", "[%s]%s",
                (pGamePlayer == NULL ? "Nobody" : pGamePlayer->getCreature()->getName().c_str()), t.toString().c_str());
    }


    __END_DEBUG_EX __END_CATCH
}

void opopenpaymap(GamePlayer* pGamePlayer, string msg, int i) {
    ZoneInfo* pZoneInfo = g_pZoneInfoManager->getZoneInfo(1013);
    pZoneInfo->setNoPortalZone(true);
    GCSystemMessage gcSystemMessage;
    gcSystemMessage.setMessage("�շѵ�ͼ�Ѿ���");
    pGamePlayer->sendPacket(&gcSystemMessage);
}

void opclosepaymap(GamePlayer* pGamePlayer, string msg, int i) {
    ZoneInfo* pZoneInfo = g_pZoneInfoManager->getZoneInfo(1013);
    pZoneInfo->setNoPortalZone(false);
    GCSystemMessage gcSystemMessage1;
    gcSystemMessage1.setMessage("�շѵ�ͼ�Ѿ��ر�");
    pGamePlayer->sendPacket(&gcSystemMessage1);
}

} // namespace de::gm
