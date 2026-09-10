//////////////////////////////////////////////////////////////////////////////
// Filename    : GCWarScheduleList.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GCWarScheduleList.h"

#include "Assert1.h"

//////////////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////////////
GCWarScheduleList::GCWarScheduleList()

{
    __BEGIN_TRY

    __END_CATCH;
}

//////////////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////////////
GCWarScheduleList::~GCWarScheduleList()

{
    __BEGIN_TRY

    clearList();

    __END_CATCH_NO_RETHROW;
}

//////////////////////////////////////////////////////////////////////////////
// The packet owns the entries it holds.
//////////////////////////////////////////////////////////////////////////////
void GCWarScheduleList::clearList()

{
    WarScheduleInfoList::iterator itr = m_WarScheduleList.begin();

    for (; itr != m_WarScheduleList.end(); itr++) {
        if (*itr != NULL)
            SAFE_DELETE(*itr);
    }

    m_WarScheduleList.clear();
}

//////////////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////////////
void GCWarScheduleList::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The schedule replaces the one the packet holds.
    clearList();

    BYTE count = 0;

    iStream.read(count);

    if (count > kMaxEntries)
        throw InvalidProtocolException("too many war schedules");

    for (int i = 0; i < count; i++) {
        WarScheduleInfo* newWarScheduleInfo;
        newWarScheduleInfo = new WarScheduleInfo;

        addWarScheduleInfo(newWarScheduleInfo);

        iStream.read(newWarScheduleInfo->warType);
        iStream.read(newWarScheduleInfo->year);
        iStream.read(newWarScheduleInfo->month);
        iStream.read(newWarScheduleInfo->day);
        iStream.read(newWarScheduleInfo->hour);
        if (newWarScheduleInfo->warType == 0) {
            for (int j = 0; j < 5; ++j) {
                iStream.read(newWarScheduleInfo->challengerGuildID[j]);
                de::wire::readString(iStream, newWarScheduleInfo->challengerGuildName[j], {0, kMaxGuildNameLength},
                                     "ChallengerGuildName");
            }

            iStream.read(newWarScheduleInfo->reinforceGuildID);
            de::wire::readString(iStream, newWarScheduleInfo->reinforceGuildName, {0, kMaxGuildNameLength},
                                 "ReinforceGuildName");
        }
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////////////
void GCWarScheduleList::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    if (m_WarScheduleList.size() > kMaxEntries)
        throw InvalidProtocolException("too many war schedules");

    BYTE ListNum = m_WarScheduleList.size();

    oStream.write(ListNum);

    WarScheduleInfoListItor itr = m_WarScheduleList.begin();

    for (; itr != m_WarScheduleList.end(); itr++) {
        oStream.write((*itr)->warType);
        oStream.write((*itr)->year);
        oStream.write((*itr)->month);
        oStream.write((*itr)->day);
        oStream.write((*itr)->hour);
        if ((*itr)->warType == 0) {
            for (int i = 0; i < 5; ++i) {
                oStream.write((*itr)->challengerGuildID[i]);
                de::wire::writeString(oStream, (*itr)->challengerGuildName[i], {0, kMaxGuildNameLength},
                                      "ChallengerGuildName");
            }

            oStream.write((*itr)->reinforceGuildID);
            de::wire::writeString(oStream, (*itr)->reinforceGuildName, {0, kMaxGuildNameLength}, "ReinforceGuildName");
        }
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

PacketSize_t GCWarScheduleList::getPacketSize() const

{
    __BEGIN_TRY

    PacketSize_t size = szBYTE;

    WarScheduleInfoListItor itr = m_WarScheduleList.begin();

    for (; itr != m_WarScheduleList.end(); itr++) {
        size += szBYTE + szWORD + szBYTE + szBYTE + szBYTE;

        if ((*itr)->warType == 0) {
            for (int i = 0; i < 5; ++i) {
                size += szGuildID;
                size += de::wire::stringWireSize((*itr)->challengerGuildName[i]);
            }

            size += szGuildID;
            size += de::wire::stringWireSize((*itr)->reinforceGuildName);
        }
    }

    return size;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////////////
string GCWarScheduleList::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCWarScheduleList(" << "WarNum : " << m_WarScheduleList.size();

    /*	WarScheduleInfoListItor itr = m_WarScheduleList.begin();

        for(; itr != m_WarScheduleList.end(); itr++ )
        {
            msg << ", (WarScheduleInfo : "
                << (((*itr)->warType)?"WAR_TYPE_DIFFERENT_RACE, ":"WAR_TYPE_SAME_RACE, " )
                << (*itr)->year << "년 "
                << (*itr)->month << "월 "
                << (*itr)->day << "일"
                << (*itr)->hour << "시";

            if((*itr)->warType == 0 )
            {
                msg << ", Challenger GuildID : " << (*itr)->challengerGuildID;
                msg << ", Challenger GuildName : " << (*itr)->challengerGuildName;
            }

            msg << ")";
        }*/

    msg << ")";

    return msg.toString();

    __END_CATCH
}

WarScheduleInfo* GCWarScheduleList::popWarScheduleInfo()

{
    __BEGIN_TRY

    if (m_WarScheduleList.empty())
        return NULL;

    WarScheduleInfo* ret = m_WarScheduleList.front();
    m_WarScheduleList.pop_front();

    return ret;

    __END_CATCH
}
