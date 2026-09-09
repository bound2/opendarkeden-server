//////////////////////////////////////////////////////////////////////////////
// Filename    : CLQueryCharacterName.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CLQueryCharacterName.h"

#include "WireString.h"

void CLQueryCharacterName::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // read player id

    de::wire::readString(iStream, m_CharacterName, {1, 20}, "CharacterName");

    __END_CATCH
}

void CLQueryCharacterName::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    // write player id
    de::wire::writeString(oStream, m_CharacterName, {1, 20}, "CharacterName");

    __END_CATCH
}

string CLQueryCharacterName::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CLQueryCharacterName(" << "CharacterName:" << m_CharacterName << ")";
    return msg.toString();

    __END_CATCH
}
