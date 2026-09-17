//////////////////////////////////////////////////////////////////////
//
// Filename    : GCMove.cpp
// Written By  : reiot@ewestsoft.com
// Description :
//
//////////////////////////////////////////////////////////////////////

// include files
#include "GCMove.h"


//////////////////////////////////////////////////////////////////////
// Read data from the input stream (buffer) and initialise the packet.
//////////////////////////////////////////////////////////////////////
void GCMove::read(SocketInputStream& iStream)

{
    __BEGIN_TRY

    iStream.read(m_ObjectID);
    iStream.read(m_X);
    iStream.read(m_Y);
    iStream.read(m_Dir);

    __END_CATCH
}


//////////////////////////////////////////////////////////////////////
// Send the packet's binary image to the output stream (buffer).
//////////////////////////////////////////////////////////////////////
void GCMove::write(SocketOutputStream& oStream) const

{
    __BEGIN_TRY

    oStream.write(m_ObjectID);
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
string GCMove::toString() const

{
    __BEGIN_TRY

    StringStream msg;
    msg << "GCMove(ObjectID:" << m_ObjectID << ",X:" << (int)m_X << ",Y:" << (int)m_Y << ",Dir: " << dir2String(m_Dir)
        << ")";
    return msg.toString();

    __END_CATCH
}
