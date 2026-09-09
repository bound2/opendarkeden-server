//////////////////////////////////////////////////////////////////////
//
// Filename    : GCWhisper.cpp
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCWhisper.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCWhisper::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    de::wire::readString(iStream, m_Name, {1, 10}, "Name");

    iStream.read(m_Color);

    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    iStream.read(m_Race);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCWhisper::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Name, {1, 10}, "Name");

    oStream.write(m_Color);

    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    oStream.write(m_Race);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCWhisper::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCWhisper(" << "Name :" << m_Name << ",Color:" << m_Color << ",Message:" << m_Message
        << ",Race :" << (int)m_Race << ")";
    return msg.toString();

    __END_CATCH
}
