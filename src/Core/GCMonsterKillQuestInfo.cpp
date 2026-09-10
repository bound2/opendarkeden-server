//////////////////////////////////////////////////////////////////////////////
// Filename    : GCMonsterKillQuestInfo.cpp
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GCMonsterKillQuestInfo.h"

#include "Assert1.h"


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
GCMonsterKillQuestInfo::~GCMonsterKillQuestInfo()

{
    __BEGIN_TRY

    clearList();

    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////////////
// The packet owns the records it holds.
//////////////////////////////////////////////////////////////////////////////
void GCMonsterKillQuestInfo::clearList()

{
    list<QuestInfo*>::iterator itr = m_QuestInfoList.begin();
    list<QuestInfo*>::iterator endItr = m_QuestInfoList.end();

    for (; itr != endItr; ++itr) {
        if ((*itr) != NULL)
            SAFE_DELETE((*itr));
    }

    m_QuestInfoList.clear();
}

//////////////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////////////
void GCMonsterKillQuestInfo::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The list replaces the one the packet holds.
    clearList();

    BYTE num;

    iStream.read(num);

    for (int i = 0; i < num; ++i) {
        QuestInfo* pQI = new QuestInfo;
        addQuestInfo(pQI);

        iStream.read(pQI->questID);
        iStream.read(pQI->sType);
        iStream.read(pQI->goal);
        iStream.read(pQI->timeLimit);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////////////
void GCMonsterKillQuestInfo::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    if (m_QuestInfoList.size() > maxQuestNum)
        throw InvalidProtocolException("too many kill quest records");

    BYTE num = m_QuestInfoList.size();

    oStream.write(num);

    list<QuestInfo*>::const_iterator itr = m_QuestInfoList.begin();

    for (int i = 0; i < num; i++) {
        oStream.write((*itr)->questID);
        oStream.write((*itr)->sType);
        oStream.write((*itr)->goal);
        oStream.write((*itr)->timeLimit);

        ++itr;
    }

    __END_CATCH
}


PacketSize_t GCMonsterKillQuestInfo::getPacketSize() const

{
    __BEGIN_TRY

    PacketSize_t result = 0;

    result += szBYTE + szQuestInfo * m_QuestInfoList.size();

    return result;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////////////
string GCMonsterKillQuestInfo::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCMonsterKillQuestInfo(";

    list<QuestInfo*>::const_iterator itr = m_QuestInfoList.begin();
    for (; itr != m_QuestInfoList.end(); itr++) {
        msg << "< " << (*itr)->questID << ", " << (*itr)->sType << ", " << (*itr)->goal << ", " << (*itr)->timeLimit
            << " >";
    }
    msg << ")";

    return msg.toString();

    __END_CATCH
}
