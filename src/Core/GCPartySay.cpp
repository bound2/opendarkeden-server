//////////////////////////////////////////////////////////////////////
//
// Filename    : GCPartySay.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCPartySay.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCPartySay::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The factory max budgets twenty characters for the sender's name and
    // 128 for the message.
    de::wire::readString(iStream, m_Name, {1, 20}, "Name");
    iStream.read(m_Color);

    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCPartySay::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");
    oStream.write(m_Color);
    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCPartySay::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCPartySay(" << "Name : " << m_Name << ", Message : " << m_Message << ")";
    return msg.toString();

    __END_CATCH
}
