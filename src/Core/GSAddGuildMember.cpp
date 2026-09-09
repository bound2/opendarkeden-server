//////////////////////////////////////////////////////////////////////
//
// Filename    : GSAddGuildMember.cpp
// Written By  :
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GSAddGuildMember.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Datagram 객체로부터 데이타를 읽어서 패킷을 초기화한다.
//////////////////////////////////////////////////////////////////////
void GSAddGuildMember::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_Name, {1, 20}, "Name");
    iStream.read(m_GuildMemberRank);
    de::wire::readString(iStream, m_GuildMemberIntro, {0, de::wire::kMaxByteStringLength}, "GuildMemberIntro");

    iStream.read(m_ServerGroupID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Datagram 객체로 패킷의 바이너리 이미지를 보낸다.
//////////////////////////////////////////////////////////////////////
void GSAddGuildMember::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");
    oStream.write(m_GuildMemberRank);
    de::wire::writeString(oStream, m_GuildMemberIntro, {0, de::wire::kMaxByteStringLength}, "GuildMemberIntro");

    oStream.write(m_ServerGroupID);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
string GSAddGuildMember::toString() const {
    StringStream msg;

    msg << "GSAddGuildMember(" << "GuildID:" << m_GuildID << "Name:" << m_Name
        << "GuildMemberRank:" << m_GuildMemberRank << "GuildMemberIntro:" << m_GuildMemberIntro << ")";

    return msg.toString();
}
