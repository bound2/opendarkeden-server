//////////////////////////////////////////////////////////////////////////////
// Filename    : CGPartySay.cpp
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGPartySay.h"

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
    BYTE szMessage;
    iStream.read(szMessage);

    // The factory max budgets 128 characters for the message.
    if (szMessage == 0)
        throw InvalidProtocolException("szMessage == 0");
    if (szMessage > 128)
        throw InvalidProtocolException("too long message length");

    iStream.read(m_Message, szMessage);

    __END_CATCH
}

void CGPartySay::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    if (m_Message.empty())
        throw InvalidProtocolException("szMessage == 0");
    if (m_Message.size() > 128)
        throw InvalidProtocolException("too long message length");

    oStream.write(m_Color);
    BYTE szMessage = m_Message.size();
    oStream.write(szMessage);
    oStream.write(m_Message);

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
