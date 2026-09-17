//////////////////////////////////////////////////////////////////////////////
// Filename    : CGWhisper.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "CGWhisper.h"

#include "WireString.h"

void CGWhisper::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // Read the name

    de::wire::readString(iStream, m_Name, {1, 10}, "Name");
    iStream.read(m_Color);

    // Read the message

    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}

void CGWhisper::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    // Write the name. The bound matches read()'s and the factory's
    // kMaxSize budget: a name longer than 10 bytes cannot be read back.
    de::wire::writeString(oStream, m_Name, {1, 10}, "Name");

    oStream.write(m_Color);

    // Write the message
    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}

string CGWhisper::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "CGWhisper(Name :" << m_Name << ", Color : " << m_Color << ", Message : " << m_Message << ")";
    return msg.toString();

    __END_CATCH
}
