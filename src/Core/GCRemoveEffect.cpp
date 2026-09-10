//////////////////////////////////////////////////////////////////////
//
// Filename    : GCRemoveEffect.cpp
// Written By  : elca@ewestsoft.com
// Description : 자신에게 쓰는 기술의 성공을 알리기 위한 패킷 클래스의
//               멤버 정의.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////
#include "GCRemoveEffect.h"


//////////////////////////////////////////////////////////////////////
// constructor
//////////////////////////////////////////////////////////////////////
GCRemoveEffect::GCRemoveEffect()

    {__BEGIN_TRY __END_CATCH}


//////////////////////////////////////////////////////////////////////
// destructor
//////////////////////////////////////////////////////////////////////
GCRemoveEffect::~GCRemoveEffect()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}


//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCRemoveEffect::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);

    BYTE listNum = 0;
    iStream.read(listNum);
    if (listNum > kMaxCount)
        throw InvalidProtocolException("too many effects in the list");

    m_EffectList.clear();

    EffectID_t value;
    for (int i = 0; i < listNum; i++) {
        iStream.read(value);
        m_EffectList.push_back(value);
    }

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCRemoveEffect::write(SocketOutputStream& oStream) const {
    __BEGIN_TRY

    if (m_EffectList.size() > kMaxCount)
        throw InvalidProtocolException("too many effects in the list");

    oStream.write(m_ObjectID);
    oStream.write((BYTE)m_EffectList.size());

    for (list<EffectID_t>::const_iterator itr = m_EffectList.begin(); itr != m_EffectList.end(); itr++) {
        oStream.write(*itr);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// GCRemoveEffect::addListElement()
//
// (변화부위, 변화수치 ) 의 한 셋을 리스트에 넣기 위한 멤버 함수.
//
//////////////////////////////////////////////////////////////////////
void GCRemoveEffect::addEffectList(EffectID_t Value)

{
    __BEGIN_TRY

    if (m_EffectList.size() >= kMaxCount)
        throw InvalidProtocolException("too many effects in the list");

    m_EffectList.push_back(Value);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCRemoveEffect::toString() const {
    __BEGIN_TRY

    StringStream msg;

    msg << "GCRemoveEffect(" << ",ListNum:" << (int)m_EffectList.size() << ",ListSet(";
    for (list<EffectID_t>::const_iterator itr = m_EffectList.begin(); itr != m_EffectList.end(); itr++) {
        msg << (int)(*itr) << ",";
    }

    msg << ")";

    return msg.toString();

    __END_CATCH
}
