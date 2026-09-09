//////////////////////////////////////////////////////////////////////////////
// Filename    : GCPartyLeave.cpp
// Written By  : 김성민
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "GCPartyLeave.h"

//////////////////////////////////////////////////////////////////////////////
// class GCPartyLeave member methods
//////////////////////////////////////////////////////////////////////////////

void GCPartyLeave::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    BYTE name_length = 0;

    // An empty expeller says the member left on its own; the factory max
    // budgets ten characters for each of the two names.
    iStream.read(name_length);

    if (name_length > 10)
        throw InvalidProtocolException("too long expeller name length");

    if (name_length > 0)
        iStream.read(m_Expeller, name_length);

    iStream.read(name_length);

    if (name_length == 0)
        throw InvalidProtocolException("expellee name == 0");
    if (name_length > 10)
        throw InvalidProtocolException("too long expellee name length");

    iStream.read(m_Expellee, name_length);

    __END_CATCH
}

void GCPartyLeave::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    if (m_Expeller.size() > 10)
        throw InvalidProtocolException("too long expeller name length");
    if (m_Expellee.empty())
        throw InvalidProtocolException("expellee name == 0");
    if (m_Expellee.size() > 10)
        throw InvalidProtocolException("too long expellee name length");

    BYTE name_length = 0;

    name_length = m_Expeller.size();
    oStream.write(name_length);
    if (name_length > 0)
        oStream.write(m_Expeller);

    name_length = m_Expellee.size();
    oStream.write(name_length);
    oStream.write(m_Expellee);

    __END_CATCH
}

string GCPartyLeave::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCPartyLeave(" << "Expeller:" << m_Expeller << "Expellee:" << m_Expellee << ")";
    return msg.toString();

    __END_CATCH
}
