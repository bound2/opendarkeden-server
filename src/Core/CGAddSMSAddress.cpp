//////////////////////////////////////////////////////////////////////////////
// Filename    : CGAddSMSAddress.cpp
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGAddSMSAddress.h"

#include "WireString.h"

CGAddSMSAddress::CGAddSMSAddress()

    {__BEGIN_TRY __END_CATCH}

CGAddSMSAddress::~CGAddSMSAddress()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

void CGAddSMSAddress::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    de::wire::readString(iStream, m_CharacterName, {1, 20}, "CharacterName");
    de::wire::readString(iStream, m_CustomName, {1, 40}, "CustomName");
    de::wire::readString(iStream, m_Number, {1, 11}, "Number");

    __END_CATCH
}

void CGAddSMSAddress::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    de::wire::writeString(oStream, m_CharacterName, {0, 20}, "CharacterName");
    de::wire::writeString(oStream, m_CustomName, {0, 40}, "CustomName");
    de::wire::writeString(oStream, m_Number, {0, 11}, "Number");

    __END_CATCH
}

string CGAddSMSAddress::toString() const {
    __BEGIN_TRY

    StringStream msg;
    msg << "CGAddSMSAddress(" << ")";
    return msg.toString();

    __END_CATCH
}
