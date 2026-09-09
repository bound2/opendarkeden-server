//////////////////////////////////////////////////////////////////////////////
// Filename    : CLQueryPlayerID.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLQueryPlayerID.h"

#include "WireString.h"

void CLQueryPlayerID::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // read player id

    de::wire::readString(iStream, m_PlayerID, {1, 20}, "PlayerID");

    __END_CATCH
}

void CLQueryPlayerID::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    // write player id
    de::wire::writeString(oStream, m_PlayerID, {1, 20}, "PlayerID");

    __END_CATCH
}

string CLQueryPlayerID::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CLQueryPlayerID(" << "PlayerID:" << m_PlayerID << ")";
    return msg.toString();

    __END_CATCH
}
