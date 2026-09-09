//////////////////////////////////////////////////////////////////////////////
// Filename    : CGJoinGuild.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGJoinGuild.h"

#include "WireString.h"


void CGJoinGuild::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    iStream.read(m_GuildID);
    iStream.read(m_GuildMemberRank);
    de::wire::readString(iStream, m_GuildMemberIntro, {0, de::wire::kMaxByteStringLength}, "GuildMemberIntro");

    __END_CATCH
}

void CGJoinGuild::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    oStream.write(m_GuildID);
    oStream.write(m_GuildMemberRank);
    de::wire::writeString(oStream, m_GuildMemberIntro, {0, GUILD_INTRO_MAX_LENGTH}, "GuildMemberIntro");

    __END_CATCH
}

string CGJoinGuild::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGJoinGuild(" << "GuildID:" << (int)m_GuildID << "GuildMemberRank:" << (int)m_GuildMemberRank
        << "GuildMemberIntro:" << m_GuildMemberIntro << ")";
    return msg.toString();

    __END_CATCH
}
