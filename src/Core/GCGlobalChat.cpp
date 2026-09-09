//////////////////////////////////////////////////////////////////////
//
// Filename    : GCGlobalChat.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCGlobalChat.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCGlobalChat::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_Color);

    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    iStream.read(m_Race);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCGlobalChat::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_Color);

    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    oStream.write(m_Race);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCGlobalChat::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCGlobalChat(" << "Color:" << m_Color << ",Message:" << m_Message << ",Race:" << (int)m_Race << ")";
    return msg.toString();

    __END_CATCH
}
