//////////////////////////////////////////////////////////////////////
//
// Filename    : GCUntransformOK.cc
// Written By  : crazydog
// Description : Definition of the packet class functions that send an OK
//               sign back to the sender when the request arrives
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCUntransformOK.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCUntransformOK::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_Dir);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCUntransformOK::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_X);
    oStream.write(m_Y);
    oStream.write(m_Dir);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
//
// get packet's debug string
//
//////////////////////////////////////////////////////////////////////
string GCUntransformOK::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCUntransformOK(" << "X:" << (int)m_X << ",Y:" << (int)m_Y << ",Dir:" << dir2String(m_Dir) << ")";
    return msg.toString();

    __END_CATCH
}
