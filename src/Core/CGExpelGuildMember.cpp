//////////////////////////////////////////////////////////////////////////////
// Filename    : CGExpelGuildMember.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGExpelGuildMember.h"

#include "WireString.h"


void CGExpelGuildMember::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_Name, {1, 20}, "Name");

    __END_CATCH
}

void CGExpelGuildMember::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");

    __END_CATCH
}

string CGExpelGuildMember::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGExpelGuildMember(" << "GuildID:" << (int)m_GuildID << "Name:" << m_Name << ")";
    return msg.toString();

    __END_CATCH
}
