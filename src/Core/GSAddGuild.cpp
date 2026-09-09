//////////////////////////////////////////////////////////////////////
//
// Filename    : GSAddGuild.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GSAddGuild.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Datagram 객체로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GSAddGuild::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    de::wire::readString(iStream, m_GuildName, {1, 30}, "GuildName");

    de::wire::readString(iStream, m_GuildMaster, {1, 20}, "GuildMaster");

    de::wire::readString(iStream, m_GuildIntro, {0, de::wire::kMaxByteStringLength}, "GuildIntro");

    iStream.read(m_GuildState);
    iStream.read(m_GuildRace);
    iStream.read(m_ServerGroupID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Datagram 객체로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GSAddGuild::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    de::wire::writeString(oStream, m_GuildName, {1, 30}, "GuildName");
    de::wire::writeString(oStream, m_GuildMaster, {1, 20}, "GuildMaster");
    de::wire::writeString(oStream, m_GuildIntro, {0, GUILD_INTRO_MAX_LENGTH}, "GuildIntro");

    oStream.write(m_GuildState);
    oStream.write(m_GuildRace);
    oStream.write(m_ServerGroupID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
string GSAddGuild::toString() const {
    StringStream msg;

    msg << "GSAddGuild (" << "GuildName:" << m_GuildName << "GuildMaster:" << m_GuildMaster
        << "GuildIntro:" << m_GuildIntro << "GuildState:" << (int)m_GuildState << "GuildRace:" << (int)m_GuildRace
        << "ServerGroupID:" << (int)m_ServerGroupID << " )";

    return msg.toString();
}
