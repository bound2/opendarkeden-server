//////////////////////////////////////////////////////////////////////////////
// Filename    : CGSay.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGSay.h"

#include "WireString.h"

void CGSay::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_Color);

    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}

void CGSay::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_Color);

    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}

string CGSay::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGSay(Color:" << m_Color << ", Message:" << m_Message << ")";
    return msg.toString();

    __END_CATCH
}
