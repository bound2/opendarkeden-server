//////////////////////////////////////////////////////////////////////////////
// Filename    : CGRangerSay.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGRangerSay.h"

#include "WireString.h"

void CGRangerSay::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // Read the message
    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}

void CGRangerSay::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    // Write the message
    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}

string CGRangerSay::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGRangerSay(Message: " << m_Message << ")";

    return msg.toString();

    __END_CATCH
}
