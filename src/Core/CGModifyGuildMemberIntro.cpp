//////////////////////////////////////////////////////////////////////////////
// Filename    : CGModifyGuildMemberIntro.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGModifyGuildMemberIntro.h"

#include "WireString.h"


void CGModifyGuildMemberIntro::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_GuildMemberIntro, {0, de::wire::kMaxByteStringLength}, "GuildMemberIntro");

    __END_CATCH
}

void CGModifyGuildMemberIntro::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_GuildMemberIntro, {0, GUILD_INTRO_MAX_LENGTH}, "GuildMemberIntro");

    __END_CATCH
}

string CGModifyGuildMemberIntro::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGModifyGuildMemberIntro(" << "GuildID:" << (int)m_GuildID << "GuildMemberIntro:" << m_GuildMemberIntro
        << ")";
    return msg.toString();

    __END_CATCH
}
