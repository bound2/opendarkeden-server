#include "FlagWar.h"

#include "FlagManager.h"
#include "GCNoticeEvent.h"
#include "GlobalItemPositionLoader.h"
#include "ItemFactoryManager.h"
#include "MonsterSummonInfo.h"
#include "Zone.h"
#include "ZoneGroupManager.h"
#include "ZoneInfoManager.h"
#include "ZoneUtil.h"

list<FlagWar::FlagGenZone> FlagWar::m_FlagGenInfo;

void FlagWar::execute() {
    __BEGIN_TRY

    switch (getState()) {
    case STATE_WAIT:
        filelog("FlagWar.log", "FlagWar Ready..");
        executeReady();
        setState(STATE_READY);
        break;
    case STATE_READY:
        filelog("FlagWar.log", "FlagWar Start..");
        executeStart();
        setState(STATE_START);
        break;
    case STATE_START:
        filelog("FlagWar.log", "FlagWar Finising..");
        executeFinish();
        setState(STATE_FINISH);
        break;
    case STATE_FINISH:
        filelog("FlagWar.log", "FlagWar End..");
        executeEnd();
        setState(STATE_WAIT);
        break;
    default:
        filelog("FlagWar.log", "이상한 FlagWar 상태..");
        break;
    }

    __END_CATCH
}

void FlagWar::executeReady() {
    __BEGIN_TRY


    GCNoticeEvent gcNE;
    gcNE.setCode(NOTICE_EVENT_FLAG_WAR_READY);

    VSDateTime current = VSDateTime::currentDateTime();
    gcNE.setParameter(((DWORD)((DWORD)(current.date().year() - 2000)) * 1000000) +
                      ((DWORD)((DWORD)current.date().month()) * 10000) + ((DWORD)((DWORD)current.date().day()) * 100) +
                      ((DWORD)((DWORD)current.time().hour())));

    m_Context.zoneGroups().broadcast(&gcNE);

    // Start in 5 minutes
    m_FlagManager.addSchedule(new Schedule(this, VSDateTime::currentDateTime().addSecs(300)));

    __END_CATCH
}

void FlagWar::addFlagsRandom(ZoneID_t zoneID, uint no) {
    Zone* pZone = getZoneByZoneID(zoneID);
    Assert(pZone != NULL);
    VSRect rect(0, 0, pZone->getWidth() - 1, pZone->getHeight() - 1);
    BPOINT pt;

    pZone->lock();

    for (uint i = 0; i < no; ++i) {
        pt.x = rand() % pZone->getWidth();
        pt.y = rand() % pZone->getHeight();

        while (!rect.ptInRect(pt.x, pt.y) || pZone->getTile(pt.x, pt.y).hasItem() ||
               pZone->getTile(pt.x, pt.y).isBlocked(Creature::MOVE_MODE_WALKING) ||
               ((pZone->getZoneLevel(pt.x, pt.y)) & SAFE_ZONE == 0)) {
            pt.x = rand() % pZone->getWidth();
            pt.y = rand() % pZone->getHeight();
        }

        Item* pItem = m_Context.itemFactories().createItem(Item::ITEM_CLASS_EVENT_ITEM, 27, list<OptionType_t>());
        Assert(pItem != NULL);

        pZone->registerObject(pItem);
        TPOINT ptInZone = pZone->addItem(pItem, pt.x, pt.y, true, 36000);
        pItem->create("", STORAGE_ZONE, pZone->getZoneID(), ptInZone.x, ptInZone.y);

        filelog("FlagWar.log", "%d : (%d,%d) 에 깃발이 만들어졌습니다.", pZone->getZoneID(), ptInZone.x, ptInZone.y);

        m_Flags.push_back(pItem->getItemID());
    }

    pZone->unlock();

    m_FlagManager.getAllowMap()[zoneID] = no;
}

void FlagWar::executeStart() {
    __BEGIN_TRY

    GCNoticeEvent gcNE;
    gcNE.setCode(NOTICE_EVENT_FLAG_WAR_START);

    VSDateTime current = VSDateTime::currentDateTime();
    gcNE.setParameter(((DWORD)((DWORD)(current.date().year() - 2000)) * 1000000) +
                      ((DWORD)((DWORD)current.date().month()) * 10000) + ((DWORD)((DWORD)current.date().day()) * 100) +
                      ((DWORD)((DWORD)current.time().hour())));

    m_Context.zoneGroups().broadcast(&gcNE);

    m_Flags.clear();
    m_FlagManager.getAllowMap().clear();

    addFlags();

    // Pick a zone at random and create 100 flags.
    // Let it run for 2 hours
    m_FlagManager.addSchedule(new Schedule(this, VSDateTime::currentDateTime().addSecs(getWarTime())));
    m_FlagManager.startFlagWar();

    __END_CATCH
}

void FlagWar::executeFinish() {
    __BEGIN_TRY

    GCNoticeEvent gcNE;
    gcNE.setCode(NOTICE_EVENT_FLAG_WAR_FINISH);

    Race_t winnerRace = m_FlagManager.getWinnerRace();
    gcNE.setParameter(((DWORD)((DWORD)winnerRace << 16)) | (DWORD)m_FlagManager.getFlagCount(winnerRace));

    m_Context.zoneGroups().broadcast(&gcNE);

    // The items burst in 3 minutes.
    m_FlagManager.addSchedule(new Schedule(this, VSDateTime::currentDateTime().addSecs(180)));
    m_FlagManager.endFlagWar();

    __END_CATCH
}

void FlagWar::executeEnd() {
    __BEGIN_TRY

    // Chase down every flag that was created and erase it.
    vector<ItemID_t>::iterator itr = m_Flags.begin();
    vector<ItemID_t>::iterator endItr = m_Flags.end();

    for (; itr != endItr; ++itr) {
        GlobalItemPosition* pItemPosition =
            GlobalItemPositionLoader::getInstance()->load(Item::ITEM_CLASS_EVENT_ITEM, *itr);
        if (pItemPosition == NULL)
            continue;

        // popItem takes the item out of its position, so it may be deleted.
        // This is called from the thread FlagManager runs on, so the lock has to be taken inside.
        Item* pItem = pItemPosition->popItem(true);
        if (pItem != NULL) {
            pItem->destroy();
            SAFE_DELETE(pItem);
        } else {
            filelog("FlagWar.log", "깃발 아이템 추적 실패... ㅜ.ㅠ");
        }
    }

    m_FlagManager.resetFlagCounts();
    m_Flags.clear();

    // Until next time
    m_FlagManager.addSchedule(new Schedule(this, getNextFlagWarTime()));


    __END_CATCH
}

VSDateTime FlagWar::getNextFlagWarTime() {
    static const int NextFlagWarDay[8] = {0, 0, 1, 0, 1, 0, 0, 0};

    VSDateTime dt = VSDateTime::currentDateTime();

    VSDateTime nextWarDateTime;
    nextWarDateTime = dt.addDays(NextFlagWarDay[dt.date().dayOfWeek()]);

    if (nextWarDateTime.date().dayOfWeek() < 6)
        nextWarDateTime.setTime(VSTime(18, 55, 0));
    else
        nextWarDateTime.setTime(VSTime(13, 55, 0));

    if (nextWarDateTime < VSDateTime::currentDateTime()) {
        bool anotherDay = true;
        if (nextWarDateTime.date().dayOfWeek() == 6) {
            nextWarDateTime.setTime(VSTime(18, 55, 0));
            if (nextWarDateTime >= VSDateTime::currentDateTime())
                anotherDay = false;
        }

        if (anotherDay) {
            nextWarDateTime = nextWarDateTime.addDays(1);
            nextWarDateTime = nextWarDateTime.addDays(NextFlagWarDay[nextWarDateTime.date().dayOfWeek()]);

            if (nextWarDateTime.date().dayOfWeek() < 6)
                nextWarDateTime.setTime(VSTime(18, 55, 0));
            else
                nextWarDateTime.setTime(VSTime(13, 55, 0));
        }
    }

    filelog("FlagWar.log", "%s에 깃발 뺏기 이벤트 시작", nextWarDateTime.toString().c_str());

    return nextWarDateTime;
}

void FlagWar::addFlags() {
    addFlagsRandom(21, 6);
    addFlagsRandom(22, 6);
    addFlagsRandom(23, 6);
    addFlagsRandom(24, 7);

    addFlagsRandom(11, 7);
    addFlagsRandom(12, 6);
    addFlagsRandom(13, 6);
    addFlagsRandom(14, 6);

    addFlagsRandom(31, 6);
    addFlagsRandom(32, 6);
    addFlagsRandom(33, 7);
    addFlagsRandom(34, 6);

    addFlagsRandom(61, 11);
    addFlagsRandom(62, 11);
    addFlagsRandom(63, 12);
    addFlagsRandom(64, 11);
}

int FlagWar::getWarTime() const {
    return 7200;
}
