//////////////////////////////////////////////////////////////////////
//
// Filename    : GCSay.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCSay.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCSay::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);
    iStream.read(m_Color);

    de::wire::readString(iStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCSay::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_ObjectID);
    oStream.write(m_Color);

    de::wire::writeString(oStream, m_Message, {1, 128}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCSay::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCSay(" << "ObjectID:" << m_ObjectID << ",Color:" << m_Color << ",Message:" << m_Message << ")";
    return msg.toString();

    __END_CATCH
}
