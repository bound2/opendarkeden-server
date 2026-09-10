//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSystemMessage.cpp
// Written By  : reiot@ewestsoft.com
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCSystemMessage.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCSystemMessage::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The length byte carries less than the factory max budgets, so the
    // byte's own range is the cap.
    de::wire::readString(iStream, m_Message, {1, de::wire::kMaxByteStringLength}, "Message");

    iStream.read(m_Color);

    BYTE t;
    iStream.read(t);
    m_Type = (SystemMessageType)t;

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCSystemMessage::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Message, {1, de::wire::kMaxByteStringLength}, "Message");

    oStream.write(m_Color);

    BYTE t = (BYTE)m_Type;
    oStream.write(t);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCSystemMessage::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCSystemMessage(" << "Type:" << (int)m_Type << ",Color:" << m_Color << ",Message:" << m_Message << ")";

    return msg.toString();

    __END_CATCH
}
