//////////////////////////////////////////////////////////////////////
//
// Filename    : GSGuildMemberLogOn.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GSGuildMemberLogOn.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Datagram 객체로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GSGuildMemberLogOn::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_Name, {1, 20}, "Name");
    iStream.read(m_bLogOn);
    iStream.read(m_ServerID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Datagram 객체로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GSGuildMemberLogOn::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");
    oStream.write(m_bLogOn);
    oStream.write(m_ServerID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
string GSGuildMemberLogOn::toString() const

{
    StringStream msg;

    msg << "GSGuildMemberLogOn (" << "GuildID:" << (int)m_GuildID << "Name:" << m_Name << "LogOn:" << m_bLogOn
        << "ServerID:" << m_ServerID << " )";

    return msg.toString();
}
