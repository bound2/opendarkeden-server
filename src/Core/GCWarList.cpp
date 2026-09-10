//////////////////////////////////////////////////////////////////////////////
// Filename    : GCWarList.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GCWarList.h"

#include "Assert1.h"
#include "GuildWarInfo.h"
#include "LevelWarInfo.h"
#include "RaceWarInfo.h"

//////////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////////
GCWarList::GCWarList()

{
    __BEGIN_TRY

    __END_CATCH;
}

//////////////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////////////
GCWarList::~GCWarList()

{
    __BEGIN_TRY

    clear();

    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////////////
// clear
//////////////////////////////////////////////////////////////////////////////
void GCWarList::clear()

{
    __BEGIN_TRY

    WarInfoList::iterator itr = m_WarInfos.begin();

    for (; itr != m_WarInfos.end(); itr++) {
        WarInfo* pWarInfo = *itr;

        SAFE_DELETE(pWarInfo);
    }

    m_WarInfos.clear();

    __END_CATCH;
}

//////////////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////////////
void GCWarList::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The list replaces the one the packet holds.
    clear();

    BYTE count = 0;
    iStream.read(count);

    if (count > kMaxWars)
        throw InvalidProtocolException("too many wars");

    WarType_t warType;
    for (int i = 0; i < count; i++) {
        WarInfo* pWarInfo = NULL;
        iStream.read(warType);

        // The raw byte is tested before it reaches the enum.
        if (warType != WAR_GUILD && warType != WAR_RACE && warType != WAR_LEVEL)
            throw InvalidProtocolException("unknown war type");

        if (warType == WAR_GUILD)
            pWarInfo = new GuildWarInfo;
        else if (warType == WAR_RACE)
            pWarInfo = new RaceWarInfo;
        else
            pWarInfo = new LevelWarInfo;

        addWarInfo(pWarInfo);

        pWarInfo->read(iStream);
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////////////
void GCWarList::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    if (m_WarInfos.size() > kMaxWars)
        throw InvalidProtocolException("too many wars");

    BYTE count = m_WarInfos.size();

    oStream.write(count);

    WarInfoListItor itr = m_WarInfos.begin();

    for (; itr != m_WarInfos.end(); itr++) {
        WarInfo* pWarInfo = *itr;

        WarType_t warType = pWarInfo->getWarType();
        oStream.write(warType);

        pWarInfo->write(oStream);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

PacketSize_t GCWarList::getPacketSize() const

{
    __BEGIN_TRY

    PacketSize_t size = szBYTE;

    WarInfoListItor itr = m_WarInfos.begin();

    for (; itr != m_WarInfos.end(); itr++) {
        WarInfo* pWarInfo = *itr;
        size += szWarType;
        size += pWarInfo->getSize();
    }

    return size;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////////////
string GCWarList::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCWarList(" << "WarNum : " << m_WarInfos.size();

    WarInfoListItor itr = m_WarInfos.begin();

    for (; itr != m_WarInfos.end(); itr++) {
        WarInfo* pWarInfo = *itr;
        msg << pWarInfo->toString();
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}

WarInfo* GCWarList::popWarInfo()

{
    __BEGIN_TRY

    if (m_WarInfos.empty())
        return NULL;

    WarInfo* pWarInfo = m_WarInfos.front();
    m_WarInfos.pop_front();

    return pWarInfo;

    __END_CATCH
}

void GCWarList::operator=(const GCWarList& WL) {
    clear();

    WarInfoListItor itr = WL.m_WarInfos.begin();

    for (; itr != WL.m_WarInfos.end(); itr++) {
        WarInfo* pWarInfo = *itr;
        WarInfo* pNewWarInfo = NULL;

        switch (pWarInfo->getWarType()) {
        case WAR_GUILD: {
            GuildWarInfo* pGWI = dynamic_cast<GuildWarInfo*>(pWarInfo);
            Assert(pGWI != NULL);

            GuildWarInfo* pNewGWI = new GuildWarInfo;
            *pNewGWI = *pGWI;
            pNewWarInfo = pNewGWI;
        } break;

        case WAR_RACE: {
            RaceWarInfo* pRWI = dynamic_cast<RaceWarInfo*>(pWarInfo);
            Assert(pRWI != NULL);

            RaceWarInfo* pNewRWI = new RaceWarInfo;
            *pNewRWI = *pRWI;
            pNewWarInfo = pNewRWI;
        } break;

        case WAR_LEVEL: {
            LevelWarInfo* pLWI = dynamic_cast<LevelWarInfo*>(pWarInfo);
            Assert(pLWI != NULL);

            LevelWarInfo* pNewLWI = new LevelWarInfo;
            *pNewLWI = *pLWI;
            pNewWarInfo = pNewLWI;
        } break;

        default:
            throw Error("wrong WarType");
        }

        addWarInfo(pNewWarInfo);
        //		cout << "GCWarList::operator = New ()" << pNewWarInfo->getStartTime() << endl;
        //		cout << "GCWarList::operator = Ori ()" << pWarInfo->getStartTime() << endl;
    }
}
