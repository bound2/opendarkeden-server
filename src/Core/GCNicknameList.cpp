//////////////////////////////////////////////////////////////////////
//
// Filename	: GCNicknameList.cpp
// Written By  : elca@ewestsoft.com
// Description : 자신에게 쓰는 기술의 성공을 알리기 위한 패킷 클래스의
//			   멤버 정의.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCNicknameList.h"

//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCNicknameList::GCNicknameList()

    {__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCNicknameList::~GCNicknameList()

{
    __BEGIN_TRY

    clearNicknames();

    __END_CATCH_NO_RETHROW
}

//////////////////////////////////////////////////////////////////////
// Drop the listing, freeing only the records read() allocated.
//////////////////////////////////////////////////////////////////////
void GCNicknameList::clearNicknames()

{
    if (m_bOwnsNicknames) {
        vector<NicknameInfo*>::iterator itr = m_Nicknames.begin();
        for (; itr != m_Nicknames.end(); ++itr)
            delete *itr;
    }

    m_Nicknames.clear();
    m_bOwnsNicknames = false;
}

//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCNicknameList::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The listing replaces the one the packet holds.
    clearNicknames();

    // The count byte carries no more than MAX_NICKNAME_NUM, so what it
    // announces is already inside the budget.
    BYTE Num;
    iStream.read(Num);

    m_bOwnsNicknames = true;

    for (int i = 0; i < Num; ++i) {
        NicknameInfo* pUnit = new NicknameInfo;
        m_Nicknames.push_back(pUnit);
        pUnit->read(iStream);
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCNicknameList::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    if (m_Nicknames.size() > MAX_NICKNAME_NUM)
        throw InvalidProtocolException("too many nickname records");

    BYTE Num = m_Nicknames.size();
    oStream.write(Num);

    vector<NicknameInfo*>::const_iterator itr = m_Nicknames.begin();
    vector<NicknameInfo*>::const_iterator endItr = m_Nicknames.end();

    for (; itr != endItr; ++itr) {
        (*itr)->write(oStream);
    }

    __END_CATCH
}

PacketSize_t GCNicknameList::getPacketSize() const {
    __BEGIN_TRY

    PacketSize_t ret = szBYTE;

    vector<NicknameInfo*>::const_iterator itr = m_Nicknames.begin();
    vector<NicknameInfo*>::const_iterator endItr = m_Nicknames.end();

    for (; itr != endItr; ++itr) {
        ret += (*itr)->getSize();
    }

    return ret;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCNicknameList::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "GCNicknameList(" << ")";
    return msg.toString();

    __END_CATCH
}
