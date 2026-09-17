//////////////////////////////////////////////////////////////////////
//
// Filename    : GCShowGuildJoin.cpp
// Written By  :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCShowGuildJoin.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCShowGuildJoin::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    iStream.read(m_GuildID);
    de::wire::readString(iStream, m_GuildName, {1, 30}, "GuildName");
    iStream.read(m_GuildMemberRank);
    iStream.read(m_JoinFee);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCShowGuildJoin::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY


    oStream.write(m_GuildID);
    de::wire::writeString(oStream, m_GuildName, {1, 30}, "GuildName");
    oStream.write(m_GuildMemberRank);
    oStream.write(m_JoinFee);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCShowGuildJoin::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCShowGuildJoin(" << "GuildID:" << (int)m_GuildID << "GuildName:" << m_GuildName
        << "GuildMemberRank:" << (int)m_GuildMemberRank << "JoinFee:" << (int)m_JoinFee << ")";

    return msg.toString();

    __END_CATCH
}
