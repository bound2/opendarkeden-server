//////////////////////////////////////////////////////////////////////
//
// Filename    : GCReconnect.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCReconnect.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCReconnect::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    de::wire::readString(iStream, m_Name, {1, 20}, "Name");

    de::wire::readString(iStream, m_ServerIP, {1, 15}, "ServerIP");

    iStream.read(m_Key);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCReconnect::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");

    de::wire::writeString(oStream, m_ServerIP, {1, 15}, "ServerIP");

    oStream.write(m_Key);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCReconnect::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCReconnect(" << "Name:" << m_Name << ",ServerIP:" << m_ServerIP << ",KEY:" << m_Key << ")";
    return msg.toString();

    __END_CATCH
}
