//////////////////////////////////////////////////////////////////////
//
// Filename    : GSExpelGuildMember.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GSExpelGuildMember.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Initialize from the datagram payload.
//////////////////////////////////////////////////////////////////////
void GSExpelGuildMember::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_Name, {1, 20}, "Name");
    de::wire::readString(iStream, m_Sender, {1, 20}, "Sender");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Write the datagram payload.
//////////////////////////////////////////////////////////////////////
void GSExpelGuildMember::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_Name, {1, 20}, "Name");
    de::wire::writeString(oStream, m_Sender, {1, 20}, "Sender");


    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
string GSExpelGuildMember::toString() const {
    StringStream msg;

    msg << "GSExpelGuildMember (" << "GuildID:" << (int)m_GuildID << "Name:" << m_Name << "Sender:" << m_Sender << " )";

    return msg.toString();
}
