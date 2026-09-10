//////////////////////////////////////////////////////////////////////
//
// Filename    : GCMineExplosionOK2.cpp
// Written By  : elca@ewestsoft.com
// Description : 자신에게 쓰는 기술의 성공을 알리기 위한 패킷 클래스의
//               멤버 정의.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCMineExplosionOK2.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCMineExplosionOK2::GCMineExplosionOK2()

    {__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCMineExplosionOK2::~GCMineExplosionOK2()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCMineExplosionOK2::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // 최적화 작업시 실제 크기를 명시하도록 한다.
    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_Dir);
    iStream.read(m_ItemType);
    BYTE CListNum = 0;
    iStream.read(CListNum);

    m_CList.clear();

    ObjectID_t m_Value;
    int i;

    for (i = 0; i < CListNum; i++) {
        iStream.read(m_Value);
        m_CList.push_back(m_Value);
    }


    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCMineExplosionOK2::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    if (m_CList.size() > kMaxCount)
        throw InvalidProtocolException("too many creatures in the list");

    // 최적화 작업시 실제 크기를 명시하도록 한다.
    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_Dir);
    oStream.write(m_ItemType);
    oStream.write((BYTE)m_CList.size());

    for (list<ObjectID_t>::const_iterator itr = m_CList.begin(); itr != m_CList.end(); itr++) {
        oStream.write(*itr);
    }


    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// GCMineExplosionOK2::addListElement()
//
// (변화부위, 변화수치 ) 의 한 셋을 리스트에 넣기 위한 멤버 함수.
//
//////////////////////////////////////////////////////////////////////
void GCMineExplosionOK2::addCListElement(ObjectID_t ObjectID)

{
    __BEGIN_TRY

    if (m_CList.size() >= kMaxCount)
        throw InvalidProtocolException("too many creatures in the list");

    m_CList.push_back(ObjectID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCMineExplosionOK2::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCMineExplosionOK2(X:" << (int)m_X << ",Y:" << (int)m_Y << ",Dir:" << (int)m_Dir
        << ",ItemType:" << (int)m_ItemType << ",CListNum: " << (int)m_CList.size() << " CListSet(";

    for (list<ObjectID_t>::const_iterator itr = m_CList.begin(); itr != m_CList.end(); itr++) {
        msg << (int)(*itr) << ",";
    }

    msg << ")";


    return msg.toString();

    __END_CATCH
}
