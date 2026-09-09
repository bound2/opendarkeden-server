//////////////////////////////////////////////////////////////////////////////
// Filename    : CGPartySay.cpp
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGPartySay.h"

#include "WireString.h"

CGPartySay::CGPartySay()

    {__BEGIN_TRY __END_CATCH}

CGPartySay::~CGPartySay()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

void CGPartySay::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_Color);

    // The factory max budgets 128 characters for the message.
    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}

void CGPartySay::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_Color);
    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}

string CGPartySay::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGPartySay(" << "Message :" << m_Message << ")";
    return msg.toString();

    __END_CATCH
}
