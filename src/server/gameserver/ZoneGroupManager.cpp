//--------------------------------------------------------------------------------
//
// Filename    : ZoneGroupManager.cpp
// Written By  : Reiot
// Description :
//
//--------------------------------------------------------------------------------

// include files
#include "ZoneGroupManager.h"

#include <stdio.h>

#include <list>

#include <unordered_map>

#include "GamePlayer.h"
#include "IncomingPlayerManager.h"
#include "LogClient.h"
#include "LoginServerManager.h"
#include "PCManager.h"
#include "Portal.h"
#include "Tile.h"
#include "ZoneGroup.h"
#include "ZoneInfoManager.h"
#include "ZonePlayerManager.h"
#include "repository/ZoneInfoRepository.h"

//--------------------------------------------------------------------------------
// constructor
//--------------------------------------------------------------------------------
ZoneGroupManager::ZoneGroupManager()

    : m_ZoneGroups(10){__BEGIN_TRY __END_CATCH}


      //--------------------------------------------------------------------------------
      // destructor
      //--------------------------------------------------------------------------------
      ZoneGroupManager::~ZoneGroupManager()

{
    __BEGIN_TRY

    unordered_map<ZoneGroupID_t, ZoneGroup*>::iterator itr = m_ZoneGroups.begin();
    for (; itr != m_ZoneGroups.end(); itr++) {
        ZoneGroup* pZoneGroup = itr->second;
        SAFE_DELETE(pZoneGroup);
    }

    // 해쉬맵안에 있는 모든 pair 들을 삭제한다.
    m_ZoneGroups.clear();

    __END_CATCH_NO_RETHROW
}


//--------------------------------------------------------------------------------
// initialize zone manager
//--------------------------------------------------------------------------------
void ZoneGroupManager::init()

{
    __BEGIN_TRY

    load();

    __END_CATCH
}


//--------------------------------------------------------------------------------
//
// load data from database
//
// 데이타베이스에 연결해서 ZoneGroup 을 로드해온다.
//
//--------------------------------------------------------------------------------
void ZoneGroupManager::load()

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    list<ZoneGroupID_t> ZoneGroupIDList;

    // Read the zone group ids first.
    vector<int> zoneGroupIDs = defaultZoneInfoRepository().loadZoneGroupIDs(true);
    for (size_t g = 0; g < zoneGroupIDs.size(); g++) {
        ZoneGroupID_t ID = zoneGroupIDs[g];
        ZoneGroupIDList.push_back(ID);
    }


    list<ZoneGroupID_t>::iterator itr = ZoneGroupIDList.begin();
    for (; itr != ZoneGroupIDList.end(); itr++) {
        ZoneGroupID_t ID = (*itr);

        // Create the zone group for this id and add it to the manager.
        ZoneGroup* pZoneGroup = new ZoneGroup(ID);
        ZonePlayerManager* pZonePlayerManager = new ZonePlayerManager();
        pZonePlayerManager->setZGID(ID);
        pZoneGroup->setZonePlayerManager(pZonePlayerManager);
        addZoneGroup(pZoneGroup);

        // Read and initialize the zones that belong to this group.
        vector<int> zoneIDs = defaultZoneInfoRepository().loadZoneIDsOfGroup((int)ID, true);

        for (size_t z = 0; z < zoneIDs.size(); z++) {
            ZoneID_t zoneID = zoneIDs[z];

            // Create and initialize the zone object, then add it to the group.
            Zone* pZone = new Zone(zoneID);
            Assert(pZone != NULL);

            pZone->setZoneGroup(pZoneGroup);

            pZoneGroup->addZone(pZone);

            //--------------------------------------------------------------------------------
            // Mind the order: init() loads the NPCs, and the AtFirst-SetPosition
            // condition/action reaches the ZoneGroupManager while it runs, so the
            // zone must be added to the manager BEFORE it is initialized.
            //--------------------------------------------------------------------------------

            printf("\n@@@@@@@@@@@@@@@ [%d]th ZONE INITIALIZATION START @@@@@@@@@@@@@@@\n", zoneID);

            pZone->init();

            printf("\n@@@@@@@@@@@@@@@ [%d]th ZONE INITIALIZATION SUCCESS @@@@@@@@@@@@@@@\n", zoneID);
        }
    }

    ZoneGroupIDList.clear();

    __END_DEBUG
    __END_CATCH
}


//--------------------------------------------------------------------------------
// save data to database
//--------------------------------------------------------------------------------
void ZoneGroupManager::save()

{
    __BEGIN_TRY

    throw UnsupportedError();

    __END_CATCH
}


//--------------------------------------------------------------------------------
// add zone to zone manager
//--------------------------------------------------------------------------------
void ZoneGroupManager::addZoneGroup(ZoneGroup* pZoneGroup)

{
    __BEGIN_TRY

    unordered_map<ZoneGroupID_t, ZoneGroup*>::iterator itr = m_ZoneGroups.find(pZoneGroup->getZoneGroupID());

    if (itr != m_ZoneGroups.end())
        // 똑같은 아이디가 이미 존재한다는 소리다. - -;
        throw Error("duplicated zone id");

    // itr 이 가리키는
    m_ZoneGroups[pZoneGroup->getZoneGroupID()] = pZoneGroup;

    __END_CATCH
}

//--------------------------------------------------------------------------------
// get zone from zone manager
//--------------------------------------------------------------------------------
ZoneGroup* ZoneGroupManager::getZoneGroupByGroupID(ZoneGroupID_t ZoneGroupID) const {
    __BEGIN_TRY

    ZoneGroup* pZoneGroup = NULL;

    unordered_map<ZoneGroupID_t, ZoneGroup*>::const_iterator itr = m_ZoneGroups.find(ZoneGroupID);

    if (itr != m_ZoneGroups.end()) {
        pZoneGroup = itr->second;

    } else {
        // 그런 존 아이디를 찾을 수 없었을 때
        StringStream msg;
        msg << "ZoneGroupID : " << ZoneGroupID;
        throw NoSuchElementException(msg.toString());
    }

    return pZoneGroup;

    __END_CATCH
}


//--------------------------------------------------------------------------------
// Delete zone from zone manager
//--------------------------------------------------------------------------------
void ZoneGroupManager::deleteZoneGroup(ZoneGroupID_t zoneID) {
    __BEGIN_TRY

    unordered_map<ZoneGroupID_t, ZoneGroup*>::iterator itr = m_ZoneGroups.find(zoneID);

    if (itr != m_ZoneGroups.end()) {
        // 존을 삭제한다.
        SAFE_DELETE(itr->second);

        // pair를 삭제한다.
        m_ZoneGroups.erase(itr);
    } else {
        // 그런 존 아이디를 찾을 수 없었을 때
        StringStream msg;
        msg << "ZoneGroupID : " << zoneID;
        throw NoSuchElementException(msg.toString());
    }

    __END_CATCH
}


//--------------------------------------------------------------------------------
// get zone from zone manager
//--------------------------------------------------------------------------------
ZoneGroup* ZoneGroupManager::getZoneGroup(ZoneGroupID_t zoneID) const {
    __BEGIN_TRY

    ZoneGroup* pZoneGroup = NULL;

    unordered_map<ZoneGroupID_t, ZoneGroup*>::const_iterator itr = m_ZoneGroups.find(zoneID);

    if (itr != m_ZoneGroups.end()) {
        pZoneGroup = itr->second;

    } else {
        // 그런 존 아이디를 찾을 수 없었을 때
        StringStream msg;
        msg << "ZoneGroupID : " << zoneID;
        throw NoSuchElementException(msg.toString());
    }

    return pZoneGroup;

    __END_CATCH
}

void ZoneGroupManager::broadcast(Packet* pPacket)

{
    ZoneGroup* pZoneGroup = NULL;

    unordered_map<ZoneGroupID_t, ZoneGroup*>::const_iterator itr = m_ZoneGroups.begin();

    for (; itr != m_ZoneGroups.end(); itr++) {
        pZoneGroup = itr->second;

        pZoneGroup->getZonePlayerManager()->broadcastPacket(pPacket);
    }
}

void ZoneGroupManager::pushBroadcastPacket(Packet* pPacket, BroadcastFilter* pFilter)

{
    ZoneGroup* pZoneGroup = NULL;

    unordered_map<ZoneGroupID_t, ZoneGroup*>::const_iterator itr = m_ZoneGroups.begin();

    for (; itr != m_ZoneGroups.end(); itr++) {
        pZoneGroup = itr->second;

        pZoneGroup->getZonePlayerManager()->pushBroadcastPacket(pPacket, pFilter);
    }
}

void ZoneGroupManager::outputLoadValue()

{
    //------------------------------------------------------------------
    // ZoneGroup load
    //------------------------------------------------------------------
    ofstream file("loadBalance.txt", ios::app);

    VSDateTime current = VSDateTime::currentDateTime();
    file << current.toString() << endl;

    unordered_map<ZoneGroupID_t, ZoneGroup*>::const_iterator itr;

    for (itr = m_ZoneGroups.begin(); itr != m_ZoneGroups.end(); itr++) {
        ZoneGroup* pZoneGroup = itr->second;
        file << "[" << (int)pZoneGroup->getZoneGroupID() << "] ";

        const std::shared_ptr<const ZoneGroup::ZoneMap> zones = pZoneGroup->getZones();
        unordered_map<ZoneID_t, Zone*>::const_iterator iZone;

        // 각 Zone의 loadValue를 구한다.
        int totalLoad = 0;
        for (iZone = zones->begin(); iZone != zones->end(); iZone++) {
            Zone* pZone = iZone->second;

            int load = pZone->getLoadValue();
            int playerLoad = pZone->getPCCount();

            file << (int)pZone->getZoneID() << "(" << load << ", " << playerLoad << ") ";

            totalLoad += load;
        }

        file << " = " << totalLoad << endl;
    }

    file << endl;
    file.close();
}

//---------------------------------------------------------------------------
// make Balanced LoadInfo
//---------------------------------------------------------------------------
//
// bForce : balacing할 필요가 없다고 판단되는 경우에도
//          강제로 ZoneGroup을 balancing할 경우에 사용된다.
//
// Zone마다의 10초간의 loop 처리 회수를 load값으로 한다.
// 계산에 편의를 위해서 실제 load는 다음과 같의 정의한다.
//
//     load = (loadLimit - load)*loadMultiplier;
//
//---------------------------------------------------------------------------
bool ZoneGroupManager::makeBalancedLoadInfo(LOAD_INFOS& loadInfos, bool bForce)

{
    const int maxGroup = m_ZoneGroups.size(); // zoneGroup 수
    const int loadLimit = 500; // load 값 제한 - sleep에 의해서 제한돼서 루프 처리회수 500이 최고다.
    const int stableLoad = 120; // 안정적인 load - 이 정도면 balancing이 필요없다고 생각되는 수준
    // 일정 값 이상이어야지 balancing이 의미있다.
    const int minLoadGap =
        20; // load balancing을 하기 위한 load 차이 - 최고~최저의 차이가 일정 값 이상이어야지 balancing이 의미있다.
    const int averageLoadPercent = 90; // 한 group의 load % 제한. 100으로 해도 되겠지만 90정도가 괜찮은거 같다.

    int i;

    GROUPS groups;

    unordered_map<ZoneGroupID_t, ZoneGroup*>::const_iterator itr;

    // 전체 load
    int totalLoad = 0;


    //------------------------------------------------------------------
    // ZoneGroup마다 loadValue 조사
    //------------------------------------------------------------------
    int maxLoadValue = 0;
    int minLoadValue = loadLimit;
    for (itr = m_ZoneGroups.begin(); itr != m_ZoneGroups.end(); itr++) {
        ZoneGroup* pZoneGroup = itr->second;

        const std::shared_ptr<const ZoneGroup::ZoneMap> zones = pZoneGroup->getZones();
        unordered_map<ZoneID_t, Zone*>::const_iterator iZone;

        // 각 Zone의 loadValue를 구한다.
        for (iZone = zones->begin(); iZone != zones->end(); iZone++) {
            Zone* pZone = iZone->second;

            int load = pZone->getLoadValue();
            load = min(load, loadLimit);

            // 10~500
            maxLoadValue = max(maxLoadValue, load);
            minLoadValue = min(minLoadValue, load);

            // 숫자 적은게 느린 거다.
            // 계산의 편의를 위해서 숫자를 뒤집?는다. --> 큰 숫자 부하가 큰 걸로 바꾼다.
            // player숫자를 부하가중치로 사용한다.
            // playerLoad = 1 ~ 20정도?
            int playerLoad = pZone->getPCCount() / 10;
            playerLoad = max(1, playerLoad);
            load = (loadLimit - load) * playerLoad; // 부하 가중치

            LoadInfo* pInfo = new LoadInfo;
            pInfo->id = pZone->getZoneID();
            pInfo->oldGroupID = itr->first;
            pInfo->groupID = -1;
            pInfo->load = load;

            // 부하와 zoneID로 이루어진 key
            DWORD key = (load << 8) | pInfo->id;

            loadInfos[key] = pInfo;

            totalLoad += load;
        }
    }

    //------------------------------------------------------------------
    //
    // balancing이 필요한지 확인
    //
    //------------------------------------------------------------------
    if (!bForce) {
        int loadBoundary = stableLoad;

        // 부하 한계 수치보다 작거나
        // min~max 부하 수치 차이가 일정수치 이하이면
        // load balancing할 필요가 없다.
        if (minLoadValue >= loadBoundary || maxLoadValue - minLoadValue <= minLoadGap) {
            // load를 다시 조사해야 한다.
            for (itr = m_ZoneGroups.begin(); itr != m_ZoneGroups.end(); itr++) {
                ZoneGroup* pZoneGroup = itr->second;

                // loadValue를 초기화 시켜준다.
                const std::shared_ptr<const ZoneGroup::ZoneMap> zones = pZoneGroup->getZones();
                unordered_map<ZoneID_t, Zone*>::const_iterator iZone;

                // 각 Zone의 loadValue를 구한다.
                for (iZone = zones->begin(); iZone != zones->end(); iZone++) {
                    Zone* pZone = iZone->second;

                    pZone->initLoadValue();
                }
            }

            return false;
        }
    }

    // 평균 load
    // average를 90%로 잡은 경우
    int avgLoad = totalLoad * averageLoadPercent / maxGroup / 100;

    // 새로운 그룹의 load를 계산하기 위해서
    groups.reserve(maxGroup);
    for (i = 0; i < maxGroup; i++) {
        groups[i] = 0;
    }

    // balancing하기 전의 상태 출력

    //------------------------------------------------------------------
    //
    // load balancing
    //
    // 약간의 변화를 준? FirstFit 사용.
    //------------------------------------------------------------------
    LOAD_INFOS::const_iterator iInfo = loadInfos.begin();

    int index = 0;

    for (; iInfo != loadInfos.end(); iInfo++) {
        LoadInfo* pInfo = iInfo->second;

        // 들어갈 새 group을 찾는다.
        int newGroupID = -1;
        for (int k = 0; k < maxGroup; k++) {
            int groupLoad = groups[index];

            if (groupLoad + pInfo->load <= avgLoad) {
                newGroupID = index;

                if (++index >= maxGroup)
                    index = 0;

                break;
            }

            if (++index >= maxGroup)
                index = 0;
        }

        // 적절한 group을 못 찾았으면 젤 값이 적은 group에 넣는다.
        if (newGroupID == -1) {
            newGroupID = 0;
            for (int k = 1; k < maxGroup; k++) {
                if (groups[k] < groups[newGroupID]) {
                    newGroupID = k;
                }
            }
        }

        // newGroupID에다가 Info를 추가한다.
        pInfo->groupID = newGroupID + 1; // 1을 증가시켜줘야 한다. -_-;
        groups[newGroupID] += pInfo->load;
    }

    return true;
}

//---------------------------------------------------------------------------
// make DefaultLoadInfo
//---------------------------------------------------------------------------
// DB에 설정된 기본 ZoneGroup으로 설정한다.
//---------------------------------------------------------------------------
bool ZoneGroupManager::makeDefaultLoadInfo(LOAD_INFOS& loadInfos)

{
    __BEGIN_TRY
    __BEGIN_DEBUG

    list<ZoneGroupID_t> ZoneGroupIDList;

    // Read the zone group ids first.
    vector<int> zoneGroupIDs = defaultZoneInfoRepository().loadZoneGroupIDs(false);
    for (size_t g = 0; g < zoneGroupIDs.size(); g++) {
        ZoneGroupID_t ID = zoneGroupIDs[g];
        ZoneGroupIDList.push_back(ID);
    }


    list<ZoneGroupID_t>::iterator itr = ZoneGroupIDList.begin();
    for (; itr != ZoneGroupIDList.end(); itr++) {
        ZoneGroupID_t ID = *itr;

        vector<int> zoneIDs = defaultZoneInfoRepository().loadZoneIDsOfGroup(ID, false);

        for (size_t z = 0; z < zoneIDs.size(); z++) {
            ZoneID_t zoneID = zoneIDs[z];

            LoadInfo* pInfo = new LoadInfo;
            pInfo->id = zoneID;

            try {
                pInfo->oldGroupID = g_pZoneInfoManager->getZoneInfo(zoneID)->getZoneGroupID();
            } catch (NoSuchElementException&) {
                filelog("makeDefaultLoadInfoError.txt", "NoSuch ZoneInfo : %d", zoneID);
                pInfo->oldGroupID = ID; // just let it through
            }

            pInfo->groupID = ID;
            pInfo->load = 0; // meaningless here

            loadInfos[zoneID] = pInfo;
        }
    }

    ZoneGroupIDList.clear();

    __END_DEBUG
    __END_CATCH

    return true;
}

//---------------------------------------------------------------------------
// balance ZoneGroup ( bForce )
//---------------------------------------------------------------------------
//
// bForce : balacing할 필요가 없다고 판단되는 경우에도
//          강제로 ZoneGroup을 balancing할 경우에 사용된다.
//
// bDefault : DB에서 지정되어 있는 값으로 ZoneGroup을 설정한다.
//
// Zone마다의 10초간의 loop 처리 회수를 load값으로 한다.
// 계산에 편의를 위해서 실제 load는 다음과 같의 정의한다.
//
//     load = (loadLimit - load)*loadMultiplier;
//
//---------------------------------------------------------------------------
void ZoneGroupManager::balanceZoneGroup(bool bForce, bool bDefault)

{
    __BEGIN_TRY

    filelog("balanceZoneGroup.txt", "존그룹 밸런싱 안할래요.");
    return;

    __END_CATCH
}

//--------------------------------------------------------------------------------
// lock all ZoneGroup and LoginServerManager
//--------------------------------------------------------------------------------
void ZoneGroupManager::lockZoneGroups()

{
    __BEGIN_TRY

    //------------------------------------------------------------------
    // LoginServerManager UNLOCK
    //------------------------------------------------------------------
    g_pLoginServerManager->lock();

    //------------------------------------------------------------------
    //
    // 					LOCK all ZoneGroups
    //
    //------------------------------------------------------------------
    unordered_map<ZoneGroupID_t, ZoneGroup*>::const_iterator itr;

    for (itr = m_ZoneGroups.begin(); itr != m_ZoneGroups.end(); itr++) {
        ZoneGroup* pZoneGroup = itr->second;
        pZoneGroup->lock();
    }

    __END_CATCH
}

//--------------------------------------------------------------------------------
// lock all ZoneGroup and LoginServerManager
//--------------------------------------------------------------------------------
void ZoneGroupManager::unlockZoneGroups()

{
    __BEGIN_TRY

    //------------------------------------------------------------------
    //
    // 					UNLOCK all ZoneGroups
    //
    //------------------------------------------------------------------
    unordered_map<ZoneGroupID_t, ZoneGroup*>::const_iterator itr;

    for (itr = m_ZoneGroups.begin(); itr != m_ZoneGroups.end(); itr++) {
        ZoneGroup* pZoneGroup = itr->second;
        pZoneGroup->unlock();
    }

    //------------------------------------------------------------------
    // LoginServerManager UNLOCK
    //------------------------------------------------------------------
    g_pLoginServerManager->unlock();


    __END_CATCH
}

//--------------------------------------------------------------------------------
// get PlayerNum. by sigi. 2002.12.30
//--------------------------------------------------------------------------------
int ZoneGroupManager::getPlayerNum() const

{
    __BEGIN_TRY

    int numPC = 0;

    unordered_map<ZoneGroupID_t, ZoneGroup*>::const_iterator itr = m_ZoneGroups.begin();

    for (; itr != m_ZoneGroups.end(); itr++) {
        ZoneGroup* pZoneGroup = itr->second;

        // lock 걸 필요 없다
        numPC += pZoneGroup->getZonePlayerManager()->size();
    }

    return numPC;

    __END_CATCH
}

void ZoneGroupManager::removeFlag(Effect::EffectClass EC)

{
    __BEGIN_TRY

    ZoneGroup* pZoneGroup = NULL;

    unordered_map<ZoneGroupID_t, ZoneGroup*>::const_iterator itr = m_ZoneGroups.begin();

    for (; itr != m_ZoneGroups.end(); itr++) {
        pZoneGroup = itr->second;

        pZoneGroup->getZonePlayerManager()->removeFlag(EC);
    }

    __END_CATCH
}

//--------------------------------------------------------------------------------
// get debug string
//--------------------------------------------------------------------------------
string ZoneGroupManager::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "ZoneGroupManager(";

    for (unordered_map<ZoneGroupID_t, ZoneGroup*>::const_iterator itr = m_ZoneGroups.begin(); itr != m_ZoneGroups.end();
         itr++) {
        msg << itr->second->toString();
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}

// global variable definition
ZoneGroupManager* g_pZoneGroupManager = NULL;
