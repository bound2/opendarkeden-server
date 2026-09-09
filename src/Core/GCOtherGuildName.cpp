//////////////////////////////////////////////////////////////////////
//
// Filename    : GCOtherGuildName.cpp
// Written By  : reiot@ewestsoft.com
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCOtherGuildName.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// 입력스트림(버퍼)으로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GCOtherGuildName::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);
    iStream.read(m_GuildID);

    // A guildless character carries a zero length and no name.
    de::wire::readString(iStream, m_GuildName, {0, 30}, "GuildName");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// 출력스트림(버퍼)으로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GCOtherGuildName::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    // if (szGuildName == 0 )
    //	throw InvalidProtocolException("szGuildName == 0");

    oStream.write(m_ObjectID);
    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_GuildName, {0, de::wire::kMaxByteStringLength}, "GuildName");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCOtherGuildName::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCOtherGuildName(" << "ObjectID:" << (int)m_ObjectID << "GuildID:" << (int)m_GuildID
        << "GuildName:" << m_GuildName << ")";

    return msg.toString();

    __END_CATCH
}
