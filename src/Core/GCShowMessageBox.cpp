//////////////////////////////////////////////////////////////////////
//
// Filename    : GCShowMessageBox.cpp
// Written By  :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCShowMessageBox.h"

#include "WireString.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCShowMessageBox::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    // The length byte carries less than the factory max budgets, so the
    // byte's own range is the cap.
    de::wire::readString(iStream, m_Message, {1, de::wire::kMaxByteStringLength}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCShowMessageBox::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    de::wire::writeString(oStream, m_Message, {1, de::wire::kMaxByteStringLength}, "Message");

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// get packet's debug string
//////////////////////////////////////////////////////////////////////
string GCShowMessageBox::toString() const

{
    __BEGIN_TRY

    StringStream msg;

    msg << "GCShowMessageBox(" << "Message:" << m_Message << ")";

    return msg.toString();

    __END_CATCH
}
