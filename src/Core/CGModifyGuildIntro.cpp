//////////////////////////////////////////////////////////////////////////////
// Filename    : CGModifyGuildIntro.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGModifyGuildIntro.h"

#include "WireString.h"


void CGModifyGuildIntro::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_GuildIntro, {0, de::wire::kMaxByteStringLength}, "GuildIntro");

    __END_CATCH
}

void CGModifyGuildIntro::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_GuildIntro, {0, GUILD_INTRO_MAX_LENGTH}, "GuildIntro");

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
