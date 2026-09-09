//////////////////////////////////////////////////////////////////////////////
// Filename    : CGModifyGuildIntro.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGModifyGuildIntro.h"


void CGModifyGuildIntro::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    BYTE szGuildIntro;

    iStream.read(m_GuildID);
    iStream.read(szGuildIntro);

    if (szGuildIntro > 0)
        iStream.read(m_GuildIntro, szGuildIntro);
    else
        m_GuildIntro = "";

    __END_CATCH
}

void CGModifyGuildIntro::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    if (m_GuildIntro.size() > GUILD_INTRO_MAX_LENGTH)
        throw InvalidProtocolException("too long szGuildIntro length");

    BYTE szGuildIntro = m_GuildIntro.size();

    oStream.write(m_GuildID);
    oStream.write(szGuildIntro);

    if (szGuildIntro > 0)
        oStream.write(m_GuildIntro);

    __END_CATCH
}

string CGModifyGuildIntro::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGModifyGuildIntro(" << "GuildID:" << (int)m_GuildID << "GuildIntro:" << m_GuildIntro << ")";
    return msg.toString();

    __END_CATCH
}
