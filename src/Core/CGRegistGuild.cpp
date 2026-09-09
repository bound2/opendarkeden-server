//////////////////////////////////////////////////////////////////////////////
// Filename    : CGRegistGuild.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGRegistGuild.h"

#include "WireString.h"


void CGRegistGuild::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    de::wire::readString(iStream, m_GuildName, {1, 30}, "GuildName");

    de::wire::readString(iStream, m_GuildIntro, {0, de::wire::kMaxByteStringLength}, "GuildIntro");

    __END_CATCH
}

void CGRegistGuild::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    de::wire::writeString(oStream, m_GuildName, {1, 30}, "GuildName");
    de::wire::writeString(oStream, m_GuildIntro, {0, GUILD_INTRO_MAX_LENGTH}, "GuildIntro");

    __END_CATCH
}

string CGRegistGuild::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGRegistGuild(" << "GuildName:" << m_GuildName << "GuildIntro:" << m_GuildIntro << ")";
    return msg.toString();

    __END_CATCH
}
