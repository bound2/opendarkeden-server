//////////////////////////////////////////////////////////////////////////////
// Filename    : CGWhisper.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGWhisper.h"

#include "WireString.h"

void CGWhisper::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // 이름 읽기

    de::wire::readString(iStream, m_Name, {1, 10}, "Name");
    iStream.read(m_Color);

    // 메세지 읽기

    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}

void CGWhisper::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    // 이름 쓰기
    de::wire::writeString(oStream, m_Name, {1, 128}, "Name");

    oStream.write(m_Color);

    // 메세지 쓰기
    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}

string CGWhisper::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGWhisper(Name :" << m_Name << ", Color : " << m_Color << ", Message : " << m_Message << ")";
    return msg.toString();

    __END_CATCH
}
