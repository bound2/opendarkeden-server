//////////////////////////////////////////////////////////////////////////////
// Filename    : CGModifyGuildMember.cpp
// Written By  :
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGModifyGuildMember.h"

#include "WireString.h"


void CGModifyGuildMember::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_Name, {1, 20}, "Name");
    iStream.read(m_GuildMemberRank);

    __END_CATCH
}

void CGModifyGuildMember::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");
    oStream.write(m_GuildMemberRank);

    __END_CATCH
}

string CGModifyGuildMember::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGModifyGuildMember(" << "GuildID:" << (int)m_GuildID << "Name:" << m_Name
        << "GuildMemberRank:" << m_GuildMemberRank << ")";
    return msg.toString();

    __END_CATCH
}
