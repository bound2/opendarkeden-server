//////////////////////////////////////////////////////////////////////////////
// Filename    : CGPhoneSay.cpp
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGPhoneSay.h"

#include "WireString.h"

void CGPhoneSay::read(SocketInputStream& iStream)

{
    __BEGIN_TRY


    iStream.read(m_SlotID);

    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}

void CGPhoneSay::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_SlotID);

    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}

string CGPhoneSay::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGPhoneSay(" << "SlotID :" << (int)m_SlotID << ",Message:" << m_Message << ")";
    return msg.toString();

    __END_CATCH
}
