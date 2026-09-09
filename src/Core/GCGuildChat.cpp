//////////////////////////////////////////////////////////////////////
//
// Filename    : GCGuildChat.cpp
// Written By  : elca@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCGuildChat.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCGuildChat::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_Type);

    if (m_Type != 0) {
        de::wire::readString(iStream, m_SendGuildName, {1, GUILD_NAME_MAX_LENGTH}, "SendGuildName");
    }

    de::wire::readString(iStream, m_Sender, {1, 10}, "Sender");
    iStream.read(m_Color);

    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCGuildChat::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_Type);
    if (m_Type != 0) {
        de::wire::writeString(oStream, m_SendGuildName, {1, GUILD_NAME_MAX_LENGTH}, "SendGuildName");
    }

    de::wire::writeString(oStream, m_Sender, {1, 10}, "Sender");
    oStream.write(m_Color);

    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCGuildChat::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCGuildChat(" << "Sener :" << m_Sender << ",Color :" << m_Color << ",Message:" << m_Message << ")";
    return msg.toString();

    __END_CATCH
}
